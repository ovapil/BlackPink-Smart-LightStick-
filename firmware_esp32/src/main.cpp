#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <WiFiUdp.h>
#include <Adafruit_NeoPixel.h>

#include "input.h" 
#include "color.h"

#define LED_PIN     32
#define NUM_LEDS    7
#define BUTTON_PIN  4

#define CMD_BEAT_SYNC 0x01
#define CMD_FX_BLINK  0x03
#define CMD_FX_STATIC 0x04

IPAddress multicastAddress(239, 1, 1, 1);
unsigned int multicastPort = 1234;

struct UdpPacket {
  byte command;
  byte r;
  byte g;
  byte b;
  uint32_t packet_id; 
} __attribute__((packed));

enum LightstickState {
  MODE_OFF,
  MODE_LOCAL, 
  MODE_WIFI_CONNECTED,
  MODE_SYNC,
  MODE_SETUP
};
LightstickState currentState = MODE_OFF;

WiFiUDP udp;
Adafruit_NeoPixel circleL(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

bool udpInitialized = false; 
unsigned long lastUdpPacketTime = 0;
const unsigned long UDP_TIMEOUT = 6000;

void disconnectServer();
bool connectWiFi(); 
void checkWiFiConnection();
void resetWiFiAndGoToLocal();
bool startWiFiManagerPortal();

void setup() {
  Serial.begin(115200);
  input_setup(BUTTON_PIN); 

  circleL.begin();
  circleL.setBrightness(80); 
  color_setup(&circleL);

  Serial.println("Power on. Starting in OFF mode.");
  
  currentState = MODE_OFF;
  color_set_mode(MODE_COLOR_OFF);
  
  disconnectServer(); 
}

bool connectWiFi() {
  Serial.println("Trying to connect to saved WiFi...");
  
  WiFi.begin();

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 2000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    if (udp.beginMulticast(multicastAddress, multicastPort)) {
        Serial.println("UDP Multicast listener started.");
        udpInitialized = true;
    } else {
        Serial.println("UDP listener FAILED.");
        udpInitialized = false;
    }
    return true; 
  } else {
    Serial.println("\nFailed to connect.");
    return false;
  }
}

bool startWiFiManagerPortal() {
    Serial.println("Starting WiFiManager Portal for 60s...");
    color_set_mode(MODE_SIGN); 
    
    WiFiManager wm;
    wm.setConfigPortalTimeout(60); 
    
    if (wm.autoConnect("LightStick_Setup", "hellohello")) {
        Serial.println("Portal connect OK!");
        
        if (udp.beginMulticast(multicastAddress, multicastPort)) {
            Serial.println("UDP Multicast listener started.");
            udpInitialized = true;
        } else {
            Serial.println("UDP listener FAILED.");
            udpInitialized = false;
        }
        return true; 

    } else {
        Serial.println("Portal timed out");
        return false;
    }
}

void checkWiFiConnection() {
  if (currentState == MODE_OFF || currentState == MODE_SETUP) {
      if (WiFi.status() == WL_CONNECTED) {
          disconnectServer(); 
      }
      return;
  }
  if (currentState == MODE_LOCAL) {
      if (WiFi.status() == WL_CONNECTED) {
          Serial.println("\nWiFi re-connected! -> MODE_WIFI_CONNECTED");
          if (udp.beginMulticast(multicastAddress, multicastPort)) {
              Serial.println("UDP Multicast listener re-started.");
              udpInitialized = true;
          } else {
              Serial.println("UDP listener FAILED.");
              udpInitialized = false;
          }
          currentState = MODE_WIFI_CONNECTED;
          color_set_mode(MODE_STATIC_WHITE);
      }
      return;
  }
  if (currentState == MODE_WIFI_CONNECTED || currentState == MODE_SYNC) {
      if (WiFi.status() != WL_CONNECTED) {
          Serial.println("WiFi connection lost! -> MODE_LOCAL");
          udp.stop();
          udpInitialized = false;
          currentState = MODE_LOCAL; 
          color_set_mode(MODE_STATIC_WHITE);
      }
      return;
  }
}

void loop() {
  input_scan(); 
  checkWiFiConnection(); 

  if (udpInitialized) {
      int packetSize = udp.parsePacket();
      if (packetSize == sizeof(UdpPacket)) { 
          unsigned long recv_time_ms = millis(); 
          UdpPacket cmd;
          udp.read((byte*)&cmd, sizeof(cmd));
          Serial.printf("LOG,RECV,%u,%u,%u,%u,%u,%lu\n", 
              cmd.packet_id, cmd.command, cmd.r, cmd.g, cmd.b, recv_time_ms);
          lastUdpPacketTime = recv_time_ms;
          if (currentState != MODE_SYNC) {
              currentState = MODE_SYNC;
          }
          if (cmd.command == CMD_BEAT_SYNC || cmd.command == CMD_FX_STATIC || cmd.command == CMD_FX_BLINK) {
              if (cmd.r == 0 && cmd.g == 0 && cmd.b == 0) {
                  color_set_sync_color(0);
              } else {
                  color_set_sync_color(circleL.Color(cmd.r, cmd.g, cmd.b));
              }
          }
      } else if (packetSize > 0) {
          udp.flush();
      }
  }
  
  if (currentState == MODE_SYNC && (millis() - lastUdpPacketTime > UDP_TIMEOUT)) {
      Serial.println("UDP Timeout! State: SYNC -> WIFI_CONNECTED");
      currentState = MODE_WIFI_CONNECTED; 
      color_set_mode(MODE_STATIC_WHITE);
  }

  switch (currentState) {
    case MODE_OFF:
      if (input_Pressed()) {
        Serial.println("State: OFF -> (Connecting...)");
        color_set_mode(MODE_STATIC_WHITE); 
        if (connectWiFi()) { 
            Serial.println("Connect OK -> MODE_WIFI_CONNECTED");
            currentState = MODE_WIFI_CONNECTED;
        } else {
            Serial.println("Connect FAILED -> MODE_LOCAL");
            currentState = MODE_LOCAL;
        }
      }
      break;

    case MODE_LOCAL: 
      if (input_Pressed()) {
        Serial.println("State: LOCAL -> OFF");
        currentState = MODE_OFF;
        color_set_mode(MODE_COLOR_OFF); 
        disconnectServer();       
        delay(50); 
        while (digitalRead(BUTTON_PIN) == LOW) { 
          delay(20); 
        }
        delay(100);
      } 
      else if (input_LongPressed()) {
        Serial.println("State: LOCAL -> MODE_SETUP (Bluetooth)");
        currentState = MODE_SETUP; 
        color_set_mode(MODE_SIGN);
        disconnectServer();
        Serial.println("Bluetooth Mode Activated.");
      } 
      else if (input_DoublePressed()) {
        Serial.println("LOCAL: Double Press -> Next Mode");
        color_next_mode();
      }
      break;

    case MODE_WIFI_CONNECTED:
      if (input_Pressed()) {
        Serial.println("State: WIFI_CONNECTED -> OFF");
        currentState = MODE_OFF;
        color_set_mode(MODE_COLOR_OFF); 
        disconnectServer();       
        delay(50); 
        while (digitalRead(BUTTON_PIN) == LOW) { 
          delay(20); 
        }
        delay(100);
      } 
      else if (input_LongPressed()) {
        Serial.println("State: WIFI_CONNECTED -> RESET WIFI");
        resetWiFiAndGoToLocal();
      } 
      else if (input_DoublePressed()) {
        Serial.println("Double Press Ignored");
      } 
      break;

    case MODE_SYNC:
      if (input_Pressed()) {
        Serial.println("State: SYNC -> OFF");
        currentState = MODE_OFF;
        color_set_mode(MODE_COLOR_OFF); 
        disconnectServer();       
        delay(50); 
        while (digitalRead(BUTTON_PIN) == LOW) { 
          delay(20); 
        }
        delay(100);
        break; 
      }
      if (input_LongPressed()) {
        Serial.println("State: SYNC -> RESET WIFI");
        resetWiFiAndGoToLocal();
      }
      if (input_DoublePressed()) 
        Serial.println("Double Press Ignored");
      break;

    case MODE_SETUP:
      if (input_Pressed()) {
        Serial.println("State: SETUP -> LOCAL");
        currentState = MODE_LOCAL; 
        color_set_mode(MODE_STATIC_WHITE); 
      }
      else if (input_LongPressed()) {
        Serial.println("State: SETUP -> Starting WiFi Portal...");
        if (startWiFiManagerPortal()) {
            Serial.println("Portal OK -> MODE_WIFI_CONNECTED");
            currentState = MODE_WIFI_CONNECTED;
            color_set_mode(MODE_STATIC_WHITE);
        } else {
            Serial.println("Portal failed. -> MODE_LOCAL");
            currentState = MODE_LOCAL; 
            color_set_mode(MODE_STATIC_WHITE);
        }
      }
      break;
  }

  if (currentState != MODE_SYNC) {
    color_loop();
  }
}

void disconnectServer() {
  udp.stop();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  udpInitialized = false;
}

void resetWiFiAndGoToLocal() {
    Serial.println("Erasing WiFi settings...");
    color_set_mode(MODE_SIGN); 
    
    WiFiManager wm;
    wm.resetSettings(); 
    
    Serial.println("WiFi settings erased. Disconnecting...");
    disconnectServer();
    
    Serial.println("-> MODE_LOCAL.");
    currentState = MODE_LOCAL;
    color_set_mode(MODE_STATIC_WHITE);
    delay(500);
}
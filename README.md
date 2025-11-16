# BlackPink Smart LightStick
## 💡 Đồ Án: Hệ thống Lightstick Thông Minh Đồng Bộ Qua Wi-Fi Dựa Trên Giao Thức UDP Và Điều Khiển Theo Nhạc
> Đồ án thuộc khuôn khổ môn học Hệ Thống Nhúng Mạng Không Dây (NT131.Q11) & Phát triển ứng dụng trên thiết bị di động (NT118.Q14) - Trường Đại học Công nghệ Thông tin (UIT).
> 
<img width="1920" height="1080" alt="Lightstick" src="https://github.com/user-attachments/assets/4a6fdbb2-72d5-4c4e-b7b3-40c90db32528" />

## 🌟 Tính Năng Nổi Bật

| Tính năng | Mô tả |
|--------------|-----------|
| ⚡ **Đồng bộ thời gian thực** | Đồng bộ nhiều lightstick với độ trễ cực thấp (< 5 ms) qua UDP Multicast |
| 🎶 **Phân tích nhạc thông minh** | Phát hiện nhịp (Beat) và năng lượng (Intensity) trong bài hát bằng **librosa** |
| 🌐 **Web UI điều khiển** | Giao diện Flask cho phép upload nhạc, chọn hiệu ứng (Static / Blink / Beat Sync) |
| 📶 **Cài đặt Wi-Fi dễ dàng** | Tích hợp **WiFiManager** — cho phép người dùng kết nối Wi-Fi qua Portal mà không cần nạp lại firmware |
| 🧠 **Firmware ổn định** | Xây dựng trên **State Machine 5 trạng thái**: OFF, LOCAL, WIFI_CONNECTED, SYNC, SETUP |
| 📊 **Hệ thống Log & Packet Tracking** | Giao thức UDP tùy chỉnh (8 byte) có **Packet ID** để phân tích mất gói, độ trễ, và jitter |

## 💎 Kiến Trúc Hệ Thống
<p align="center">
  <img width="853" height="1038" alt="Lightstick demo" src="https://github.com/user-attachments/assets/a0d4af75-ec92-495f-a3f0-b2f983c22fbf" />
</p>

Hệ thống bao gồm hai thành phần chính:
1.  **Server (Python):**
    - Sử dụng Flask để tạo Web UI
    - Phân tích file nhạc bằng librosa
    - Gửi các gói tin UDP 8-byte qua **Multicast** đến địa chỉ 239.1.1.1
2.  **Client (ESP32):**
    - Sử dụng WiFiManager để kết nối Wi-Fi
    - Lắng nghe tại địa chỉ Multicast 239.1.1.1
    - Hoạt động dựa trên Máy Trạng Thái 5-state
    - Điều khiển LED WS2812 7 bits (NeoPixel)

## 🛠️ Công Nghệ Sử Dụng

- **Server:** Python, Flask, Librosa, NumPy
- **Client:** C++/Arduino, ESP32 WROOM
- **Thư viện Client:** WiFiManager, Adafruit_NeoPixel
- **Giao thức:** Wi-Fi (IEEE 802.11), UDP Multicast

## 📸 Video Demo
[Demo Smart LightStick](https://drive.google.com/drive/folders/1MlZb2zqXdkc3Idzolo9LwRjRb0AOgoYt?usp=drive_link)

## 🎓 Thành Viên Thực Hiện

| MSSV     | Họ và Tên              | Email                  | Github                                          |
|----------|------------------------|------------------------|-------------------------------------------------|
| 23520168 | Đoàn Ngọc Minh Châu    | 23520168@gm.uit.edu.vn | [Minh Châu](https://github.com/23520168)        |
| 23521040 | Lê Nhật Trinh Nguyên   | 23521040@gm.uit.edu.vn | [Trinh Nguyên](https://github.com/ovapil)       |

---
Đây chỉ là đồ án được thực hiện bởi những sinh viên hâm mộ nhóm nhạc BLACKPINK và phục vụ chủ yếu cho mục đích học tập, nghiên cứu. Dự án được lấy cảm hứng từ các công nghệ hiện có và không liên quan đến/được ủy quyền bởi BLACKPINK hay YG Entertainment.
Xin cảm ơn dự án [lightstick của mattywausb](https://github.com/mattywausb/lightstick) đã cung cấp nguồn tham khảo và cảm hứng ban đầu.


# STM32 Bootloader (FOTA)

Một repository triển khai bootloader và công cụ trên host để nạp firmware cho thiết bị STM32 (STM32F1 series).

---

## Giới thiệu

Bootloader là một chương trình dùng để nạp chương trình thực thi vào hệ thống. Bootloader được khởi chạy đầu tiên khi hệ thống khởi động, cho phép cấu hình hệ thống (cấp phát tài nguyên, lựa chọn phân vùng khởi động...).

Ví dụ: trên PC bootloader tồn tại ở dạng BIOS; nếu máy có hai hệ điều hành (Windows và Linux), bootloader sẽ cho phép chọn hệ điều hành để khởi động.

---

## Các loại bootloader

### Bootloader tích hợp

- **Ưu điểm**: phần cứng hỗ trợ, tiết kiệm chi phí R&D và sản xuất → nhanh đưa sản phẩm ra thị trường.  
- **Nhược điểm**: khó tùy chỉnh, bị bó hẹp khi ứng dụng mở rộng.

### Custom bootloader

- **Ưu điểm**: là giải pháp phần mềm, dễ tùy chỉnh phù hợp với các ứng dụng lớn, có độ mở cao.  
- **Nhược điểm**: tốn thêm chi phí R&D → tổng chi phí sản phẩm tăng.

---

## Giao tiếp Host ↔ Node (cấu trúc command)

Một command gửi từ Host tới Node cơ bản gồm các trường:

| Trường         | Mô tả                               |
|----------------|--------------------------------------|
| Length         | Độ dài của command                   |
| Command Type   | Loại lệnh (Read, Erase, Write)       |
| Data           | Dữ liệu kèm theo (nếu có)           |
| CRC            | Mã CRC tính từ trường Length tới Data |

Node sẽ tính lại CRC của dữ liệu nhận được; nếu CRC trùng khớp thì command được coi là hợp lệ.

---

## Response từ Node tới Host

Response đi từ Node về Host bao gồm:

| Trường       | Mô tả                                    |
|--------------|-------------------------------------------|
| ACK / NACK   | Xác nhận thành công/thất bại              |
| Data         | Dữ liệu phản hồi (nếu có)                 |
| Valid/Invalid| Cờ xác thực tính hợp lệ                   |

- Nếu CRC check pass: Node trả ACK, gửi dữ liệu (nếu lệnh yêu cầu) và đặt Valid flag.  
- Nếu CRC fail hoặc lỗi trong thực thi: Node trả NACK và/hoặc Invalid flag.

---

## Các loại command chính

### Read (đọc dữ liệu từ bộ nhớ Node)

- **Host gửi**: địa chỉ + số byte + CRC  
- **Node trả**: ACK + dữ liệu + Valid flag  

### Erase (xóa bộ nhớ Flash)

- **Host gửi**: số sector cần xóa + CRC  
- **Node trả**: ACK/Valid nếu thành công, NACK nếu lỗi  

### Write (lập trình bộ nhớ)

- **Host gửi**: địa chỉ + dữ liệu + CRC  
- **Node trả**: ACK/Valid hoặc NACK  

---

## Quá trình reset của STM32F1

- **Vector table**: vùng dữ liệu lưu trữ địa chỉ các hàm ngắt và địa chỉ main stack pointer.  
- Khi có ngắt, CPU dựa vào vector table (ID ngắt/địa chỉ) để nhảy tới hàm ngắt tương ứng.  

---

## Thiết kế chương trình Bootloader (driver)

1. **Init sequences**:  
   Bootloader Driver Init → Init UART → Enable UART interrupt → Init CRC hardware  

2. **Parse host command**: phân tích gói tin, xác định loại lệnh  

3. **Verify CRC sequence**:  
   - Nhận CRC từ Host  
   - Tự tính CRC  
   - So sánh  
   - Nếu pass → thực hiện lệnh  
   - Nếu fail → trả NACK  

---

## Quy trình tổng quan (sequence)

- Diagram  
- Code  

---

## Cấu trúc dự án

```
stm32Fota/
├── app/                  # Firmware và công cụ liên quan
│   ├── application.bin   # Firmware sẵn có
│   └── Hex_To_Bin/       # Công cụ chuyển đổi HEX sang BIN
│       ├── application.hex   # File hex của ứng dụng
│       ├── application.exe   # Tool chuyển đổi
│       ├── intel_hex.c       # Source code xử lý hex
│       ├── main.c            # Main của tool
│       └── makefile          # Makefile để build tool
├── hostPC/               # Công cụ host-side (Python)
│   ├── hostBootloader.py     # Main bootloader host
│   ├── bootloaderDriver.py   # Driver bootloader
│   ├── uartDriver.py         # Driver giao tiếp UART
│   ├── fileHandling.py       # Xử lý file firmware
│   └── utility.py            # Tiện ích hỗ trợ (CRC, etc.)
└── nodeMCU/              # Mã nguồn bootloader cho MCU
    ├── Core/                 # Core firmware
    ├── Drivers/              # Drivers phần cứng
    ├── flash_driver.c/h      # Driver flash memory
    ├── uart_driver.c/h       # Driver UART
    ├── jump_driver.c/h       # Driver jump to application
    └── MDK-ARM/              # Project Keil MDK-ARM
```

---

## Hướng dẫn sử dụng

### Build và triển khai

1. **Build bootloader cho MCU**:
   - Mở project trong thư mục `nodeMCU/MDK-ARM` bằng Keil uVision  
   - Build project và nạp vào MCU  

2. **Chuyển đổi firmware ứng dụng**:
   - Sử dụng tool trong `app/Hex_To_Bin/` để chuyển file hex sang bin  
   - Chạy `make` trong thư mục để build tool (nếu cần)  

3. **Nạp firmware từ Host**:
   - Chạy script Python `hostBootloader.py` trong thư mục `hostPC/`  
   - Script sẽ thiết lập kết nối UART và thực hiện quy trình nạp firmware  

---

## Quy trình nạp firmware

1. Khởi động bootloader trên MCU  
2. Thiết lập kết nối UART giữa Host và Node  
3. Gửi lệnh erase để xóa vùng nhớ flash cần thiết  
4. Gửi từng khối dữ liệu firmware kèm CRC  
5. Xác nhận firmware đã được nạp thành công  
6. Nhảy đến địa chỉ ứng dụng và thực thi  

---

## Tài liệu tham khảo

- Tài liệu kỹ thuật STM32F1 series  
- Application note AN3155: USB DFU protocol  
- Application note AN2606: STM32 microcontroller system memory boot mode  

---

## Đóng góp

Nếu bạn muốn đóng góp cho dự án, vui lòng:

1. Fork repository  
2. Tạo branch cho tính năng mới (`git checkout -b feature/AmazingFeature`)  
3. Commit các thay đổi (`git commit -m 'Add some AmazingFeature'`)  
4. Push đến branch (`git push origin feature/AmazingFeature`)  
5. Tạo Pull Request  

---

## Liên hệ

Thông tin liên hệ và hỗ trợ kỹ thuật có thể được tìm thấy trong phần **issues** của repository.

---

## Giấy phép

Dự án này được phân phối dưới giấy phép **MIT**. Xem file `LICENSE` để biết thêm chi tiết.

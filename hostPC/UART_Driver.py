import serial

#: @brief Đối tượng UART toàn cục.
UART = serial.Serial(
    port="COM11",
    baudrate=115200,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=1
)

def UART_Open():
    """
    @brief  Kiểm tra cổng UART có mở không.
    @return True nếu đang mở, ngược lại False.
    """
    return UART.isOpen()

def UART_Close():
    """
    @brief  Đóng cổng UART.
    @return None.
    """
    UART.close()

def UART_SendData(TX_data):
    """
    @brief  Gửi dữ liệu qua UART.
    @param  TX_data  Dữ liệu dạng bytes/bytearray.
    @return None.
    """
    UART.write(TX_data)

def UART_ReadData(RX_Data, Data_Length):
    """
    @brief  Đọc Data_Length byte từ UART.
    @param  RX_Data      Không sử dụng (giữ giao diện cũ).
    @param  Data_Length  Số byte cần đọc.
    @return bytes        Dữ liệu đã đọc (có thể ít hơn nếu timeout).
    """
    RX_Data = UART.read(Data_Length)
    return RX_Data

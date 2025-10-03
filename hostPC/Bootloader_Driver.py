#######################################################################
# -*- coding: utf-8 -*-
# @file    bootloader_driver.py
# @brief   Trình điều khiển bootloader cấp PC (tối giản chú thích).
#######################################################################

from UART_Driver import UART_SendData, UART_ReadData
from Utilily import log_info, log_error, log_debug, calc_checksum, bytes_to_hexstr
from File_Handler import *

#######################################################################
# Bootloader command definition
#######################################################################
CMD_ERASE   = 0x01  #: Xoá vùng app theo kích thước.
CMD_WRITE   = 0x02  #: Ghi một block dữ liệu.
CMD_VERIFY  = 0x03  #: Xác minh checksum/kích thước app.
CMD_JUMP    = 0x04  #: Nhảy vào ứng dụng.
CMD_READ    = 0x05  #: Đọc dữ liệu (nếu firmware hỗ trợ).

CMD_ACK     = 0x79  #: Phản hồi thành công.
CMD_NACK    = 0x1F  #: Phản hồi lỗi.

BLOCK_SIZE  = 256   #: Số byte gửi mỗi block.

#######################################################################
# Bootloader Driver
#######################################################################
class BootloaderDriver:
    """
    @class BootloaderDriver
    @brief Gửi lệnh bootloader qua UART.
    """

    def __init__(self):
        """
        @brief Khởi tạo đối tượng (không giữ state).
        """
        pass

    def send_cmd(self, cmd: int, payload: bytes = b"") -> bool:
        """
        @brief  Gửi gói lệnh bootloader dạng [CMD][LEN][DATA...][CHK].
        @param  cmd      Mã lệnh (ví dụ CMD_WRITE).
        @param  payload  Dữ liệu kèm theo (có thể rỗng).
        @return bool     True nếu nhận ACK, ngược lại False.
        """
        length = len(payload)
        packet = bytes([cmd, length]) + payload
        checksum = calc_checksum(packet).to_bytes(1, 'little')
        packet += checksum

        log_debug(f"TX: {bytes_to_hexstr(packet)}")
        UART_SendData(packet)

        # Đọc phản hồi 1 byte: ACK/NACK
        resp = UART_ReadData(b"", 1)
        if resp == bytes([CMD_ACK]):
            return True
        else:
            log_error("Bootloader NACK or no response")
            return False

    def erase_app(self, size: int) -> bool:
        """
        @brief  Gửi lệnh ERASE với kích thước app.
        @param  size  Kích thước app (byte).
        @return bool  True nếu ACK.
        """
        log_info("Sending ERASE command...")
        size_bytes = size.to_bytes(4, 'little')
        return self.send_cmd(CMD_ERASE, size_bytes)

    def write_block(self, addr: int, data: bytes) -> bool:
        """
        @brief  Gửi block dữ liệu để ghi vào địa chỉ app.
        @param  addr  Địa chỉ offset/bắt đầu trong app.
        @param  data  Dữ liệu cần ghi (<= BLOCK_SIZE).
        @return bool  True nếu ACK.
        """
        log_info(f"Sending WRITE block at 0x{addr:08X}, len={len(data)}")
        addr_bytes = addr.to_bytes(4, 'little')
        payload = addr_bytes + data
        return self.send_cmd(CMD_WRITE, payload)

    def verify_app(self, checksum: int, size: int) -> bool:
        """
        @brief  Yêu cầu thiết bị xác minh app.
        @param  checksum  Checksum 16-bit của toàn bộ app.
        @param  size      Kích thước app (byte).
        @return bool      True nếu ACK.
        """
        log_info("Sending VERIFY command...")
        payload = checksum.to_bytes(2, 'little') + size.to_bytes(4, 'little')
        return self.send_cmd(CMD_VERIFY, payload)

    def jump_to_app(self) -> bool:
        """
        @brief  Ra lệnh nhảy vào ứng dụng.
        @return bool  True nếu ACK.
        """
        log_info("Sending JUMP command...")
        return self.send_cmd(CMD_JUMP)

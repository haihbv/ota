#######################################################################
# Import libraries
#######################################################################
from uartDriver import UART_SendData, UART_ReadData
from utility import log_info, log_error, log_debug, calc_checksum, bytes_to_hexstr
from fileHandling import *

#######################################################################
# Bootloader command definition
#######################################################################
CMD_ERASE   = 0x01
CMD_WRITE   = 0x02
CMD_VERIFY  = 0x03
CMD_JUMP    = 0x04
CMD_READ    = 0x05

CMD_ACK     = 0x79
CMD_NACK    = 0x1F

BLOCK_SIZE  = 256   # số byte gửi mỗi block

#######################################################################
# Bootloader Driver
#######################################################################
class BootloaderDriver:

    def __init__(self):
        pass

    def send_cmd(self, cmd: int, payload: bytes = b"") -> bool:
        """
        Gửi lệnh bootloader: [CMD][LEN][DATA...][CHECKSUM]
        """
        length = len(payload)
        packet = bytes([cmd, length]) + payload
        checksum = calc_checksum(packet).to_bytes(1, 'little')
        packet += checksum

        log_debug(f"TX: {bytes_to_hexstr(packet)}")
        UART_SendData(packet)

        # đọc phản hồi ACK/NACK
        resp = UART_ReadData(b"", 1)
        if resp == bytes([CMD_ACK]):
            return True
        else:
            log_error("Bootloader NACK or no response")
            return False

    def erase_app(self, size: int) -> bool:
        log_info("Sending ERASE command...")
        size_bytes = size.to_bytes(4, 'little')
        return self.send_cmd(CMD_ERASE, size_bytes)

    def write_block(self, addr: int, data: bytes) -> bool:
        log_info(f"Sending WRITE block at 0x{addr:08X}, len={len(data)}")
        addr_bytes = addr.to_bytes(4, 'little')
        payload = addr_bytes + data
        return self.send_cmd(CMD_WRITE, payload)

    def verify_app(self, checksum: int, size: int) -> bool:
        log_info("Sending VERIFY command...")
        payload = checksum.to_bytes(2, 'little') + size.to_bytes(4, 'little')
        return self.send_cmd(CMD_VERIFY, payload)

    def jump_to_app(self) -> bool:
        log_info("Sending JUMP command...")
        return self.send_cmd(CMD_JUMP)

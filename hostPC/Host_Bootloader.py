#######################################################################
# Import libraries
#######################################################################
import argparse
import os
from File_Handler import *
from UART_Driver import UART_Open, UART_Close
from Bootloader_Driver import BootloaderDriver
from Utilily import log_info, log_error, log_warn, calc_checksum

#######################################################################
# Config
#######################################################################
BLOCK_SIZE = 128   #: @brief Kích thước block đọc/ghi (byte).

#######################################################################
# Main function
#######################################################################
def main():
    """
    @brief  Luồng chính: mở UART, mở file BIN, ERASE → WRITE (theo block) → VERIFY → JUMP.
    @return None.
    """
    # Parse arguments
    parser = argparse.ArgumentParser(description="STM32 UART Bootloader Host")
    parser.add_argument(
        "bin_path",
        nargs="?",
        default="C:/Users/Administrator/OneDrive/Desktop/OTA/app/Hex_To_Bin/pc13.bin",
        help="Duong dan toi file .bin"
    )
    args = parser.parse_args()
    bin_path = args.bin_path

    # Mo UART
    if not UART_Open():
        log_error("Cannot open UART port!")
        return

    log_info("UART opened successfully")
    bl = BootloaderDriver()

    # Mo file firmware
    if not os.path.isfile(bin_path):
        log_error(f"File not found: {bin_path}")
        UART_Close()
        return

    BinFile = Open_BinFile(bin_path)
    if BinFile is None:
        log_error(f"Cannot open {bin_path}")
        UART_Close()
        return

    FileSize = Calc_FileSize(BinFile)
    log_info(f"Firmware: {bin_path}")
    log_info(f"Firmware size: {FileSize} bytes")

    # Tinh checksum toan bo file de verify
    full_data = BinFile.read(FileSize)
    fw_checksum = calc_checksum(full_data)
    BinFile.seek(0)  # reset con tro file
    log_info(f"Firmware checksum: 0x{fw_checksum:02X}")

    # ERASE
    if not bl.erase_app(FileSize):
        log_error("Erase failed")
        Close_BinFile(BinFile)
        UART_Close()
        return

    # WRITE theo block
    addr = 0x08004000  # dia chi bat dau nap ung dung
    offset = 0

    while offset < FileSize:
        chunk = Read_BinFile(BinFile, BLOCK_SIZE)
        if not chunk:
            break

        if not bl.write_block(addr + offset, chunk):
            log_error(f"Write block failed at offset {offset}")
            Close_BinFile(BinFile)
            UART_Close()
            return

        offset += len(chunk)

        # Hien thi tien do (lam tron xuong)
        percent = (offset * 100) // FileSize
        log_info(f"Progress: {percent}%")

    log_info("All blocks written successfully")

    # VERIFY
    if not bl.verify_app(fw_checksum, FileSize):
        log_error("Verify failed")
        Close_BinFile(BinFile)
        UART_Close()
        return

    log_info("Firmware verified successfully")

    # JUMP
    if bl.jump_to_app():
        log_info("Jumped to application")
    else:
        log_warn("Jump command failed")

    # Dong file va UART
    Close_BinFile(BinFile)
    UART_Close()
    log_info("Update finished")


#######################################################################
# Entry point
#######################################################################
if __name__ == "__main__":
    main()

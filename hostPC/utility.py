#######################################################################
# Import libraries
#######################################################################
import binascii
import time

#######################################################################
# Checksum
#######################################################################
def calc_checksum(data: bytes) -> int:
    """
    Tính checksum đơn giản (cộng dồn modulo 256)
    """
    return sum(data) & 0xFF

#######################################################################
# Data convert
#######################################################################
def bytes_to_hexstr(data: bytes) -> str:
    """
    Convert mảng bytes -> chuỗi hex (cách nhau bởi space)
    """
    return " ".join(f"{b:02X}" for b in data)

def hexstr_to_bytes(hexstr: str) -> bytes:
    """
    Convert chuỗi hex -> mảng bytes
    VD: "01 02 0A" -> b'\x01\x02\x0A'
    """
    return bytes.fromhex(hexstr)

#######################################################################
# Logging
#######################################################################
def log_info(msg: str):
    print(f"[INFO] {msg}")

def log_warn(msg: str):
    print(f"\033[93m[WARN]\033[0m {msg}")

def log_error(msg: str):
    print(f"\033[91m[ERROR]\033[0m {msg}")

def log_debug(msg: str):
    print(f"\033[94m[DEBUG]\033[0m {msg}")

#######################################################################
# Timer helper
#######################################################################
def now_ms() -> int:
    """
    Trả về thời gian hiện tại tính bằng ms
    """
    return int(time.time() * 1000)

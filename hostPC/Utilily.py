import binascii
import time

#######################################################################
# Checksum
#######################################################################
def calc_checksum(data: bytes) -> int:
    """
    @brief  Tính checksum cộng dồn modulo 256.
    @param  data  Dữ liệu dạng bytes.
    @return int   Checksum (0..255).
    """
    return sum(data) & 0xFF

#######################################################################
# Data convert
#######################################################################
def bytes_to_hexstr(data: bytes) -> str:
    """
    @brief  Chuyển bytes -> chuỗi hex, cách nhau bằng khoảng trắng.
    @param  data  Dữ liệu dạng bytes.
    @return str   Chuỗi hex, ví dụ: "01 02 0A".
    """
    return " ".join(f"{b:02X}" for b in data)

def hexstr_to_bytes(hexstr: str) -> bytes:
    """
    @brief  Chuyển chuỗi hex -> bytes.
    @param  hexstr  Chuỗi hex, có thể có khoảng trắng. VD: "01 02 0A".
    @return bytes   Mảng bytes, VD: b'\\x01\\x02\\x0A'.
    """
    return bytes.fromhex(hexstr)

#######################################################################
# Logging
#######################################################################
def log_info(msg: str):
    """
    @brief  In log mức INFO.
    @param  msg  Nội dung.
    @return None.
    """
    print(f"[INFO] {msg}")

def log_warn(msg: str):
    """
    @brief  In log mức WARN (vàng).
    @param  msg  Nội dung.
    @return None.
    """
    print(f"\033[93m[WARN]\033[0m {msg}")

def log_error(msg: str):
    """
    @brief  In log mức ERROR (đỏ).
    @param  msg  Nội dung.
    @return None.
    """
    print(f"\033[91m[ERROR]\033[0m {msg}")

def log_debug(msg: str):
    """
    @brief  In log mức DEBUG (xanh dương).
    @param  msg  Nội dung.
    @return None.
    """
    print(f"\033[94m[DEBUG]\033[0m {msg}")

#######################################################################
# Timer helper
#######################################################################
def now_ms() -> int:
    """
    @brief  Trả về thời điểm hiện tại (ms từ epoch).
    @return int  Số millisecond.
    """
    return int(time.time() * 1000)

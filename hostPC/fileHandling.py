#######################################################################
# -*- coding: utf-8 -*-
# @file    fileHandling.py
# @brief   Tiện ích xử lý file nhị phân (tối giản chú thích).
#######################################################################

import os

#######################################################################
# Binary file handling
#######################################################################

def Open_BinFile(FilePath):
    """
    @brief  Mở file nhị phân để đọc.
    @param  FilePath  Đường dẫn file.
    @return file|None  Đối tượng file ở chế độ "rb" hoặc None nếu không tồn tại.
    """
    if os.path.isfile(FilePath):
        BinFile = open(FilePath, "rb")
        return BinFile
    else:
        return None

def Close_BinFile(BinFile):
    """
    @brief  Đóng file nhị phân.
    @param  BinFile  Đối tượng file đã mở.
    @return None.
    """
    BinFile.close()

def Calc_FileSize(BinFile):
    """
    @brief  Tính kích thước file (byte).
    @param  BinFile  Đối tượng file đã mở.
    @return int      Số byte của file.
    """
    BinFile.seek(0, os.SEEK_END)
    FileSize = BinFile.tell()
    BinFile.seek(0, os.SEEK_SET)
    return FileSize

def Read_BinFile(BinFile, Data_Length):
    """
    @brief  Đọc Data_Length byte từ file.
    @param  BinFile      Đối tượng file đã mở.
    @param  Data_Length  Số byte cần đọc.
    @return bytes        Dữ liệu đã đọc (có thể ít hơn nếu cuối file).
    """
    Data = BinFile.read(Data_Length)
    return Data

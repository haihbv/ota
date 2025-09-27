#######################################################################
# Import libraries
#######################################################################
import os

#######################################################################
# Binary file handling
#######################################################################

def Open_BinFile(FilePath):
    if os.path.isfile(FilePath):
        BinFile = open(FilePath, "rb")
        return BinFile
    else:
        return None

def Close_BinFile(BinFile):
    BinFile.close()

def Calc_FileSize(BinFile):
    BinFile.seek(0, os.SEEK_END)
    FileSize = BinFile.tell()
    BinFile.seek(0, os.SEEK_SET)
    return FileSize

def Read_BinFile(BinFile, Data_Length):
    Data = BinFile.read(Data_Length)
    return Data
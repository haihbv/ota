#######################################################################
# Import libraries
#######################################################################
import serial

#######################################################################
# UART
#######################################################################
global UART

UART = serial.Serial(
    port = "COM11", \
    baudrate = 115200, \
    parity = serial.PARITY_NONE, \
    stopbits = serial.STOPBITS_ONE, \
    bytesize = serial.EIGHTBITS, \
    timeout = 1
)

def UART_Open():
    return UART.isOpen()

def UART_Close():
    UART.close()

def UART_SendData(TX_data):
    UART.write(TX_data)

def UART_ReadData(RX_Data, Data_Length):
    RX_Data = UART.read(Data_Length)
    return RX_Data

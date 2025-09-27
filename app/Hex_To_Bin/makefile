# variables
CC     = gcc
DEPS   = intel_hex.h
OBJ    = main.o intel_hex.o
BIN    = application.bin
TARGET = application.exe

# include path
INC := .

# resources path
SRC := main.c intel_hex.c

# file HEX
HEX ?= firmware.hex

%.o: %.c $(DEPS)
	$(CC) -I$(INC) -c $< -o $@

$(TARGET): $(OBJ)
	$(CC) -o $@ $^

.PHONY: clean build convert run read

# build exe
build: $(TARGET)

# convert .hex -> .bin
convert: $(TARGET)
	$(eval HEX_FILE := $(if $(word 2,$(MAKECMDGOALS)),$(word 2,$(MAKECMDGOALS)),$(HEX)))
	$(eval BIN_FILE := $(basename $(notdir $(HEX_FILE))).bin)
	./$(TARGET) $(HEX_FILE) $(BIN_FILE)

# Dummy target to prevent make from complaining about unknown targets
%:
	@:

run: 
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJ) $(BIN)
	
read:
	hexdump -C $(BIN)
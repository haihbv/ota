#ifndef __BL_CMD_H
#define __BL_CMD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define CMD_ERASE   0x01
#define CMD_WRITE   0x02
#define CMD_VERIFY  0x03
#define CMD_JUMP    0x04
#define CMD_READ    0x05

#define CMD_ACK     0x79
#define CMD_NACK    0x1F

// Prototype
void BL_ProcessCommand(uint8_t *rx_buf, uint16_t len);
void BL_SendACK(void);
void BL_SendNACK(void);

#ifdef __cplusplus
}
#endif

#endif /* __BL_CMD_H */

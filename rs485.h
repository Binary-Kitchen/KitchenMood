#ifndef RS485_H
#define RS485_H

#define RS485_SUCCESS 0
#define RS485_FAIL 1

#define MAX_PAYLOAD 64

#define RS485_PREAMBLE 0x40

#define RS485_IDLE 0
#define RS485_RXACT 1
#define RS485_SRC 2
#define RS485_DST 3
#define RS485_PAYLOAD_LEN 4
#define RS485_PAYLOAD 5
#define RS485_CRC 6

extern void init_rs485(void);

extern volatile unsigned char buffer[MAX_PAYLOAD];
extern volatile unsigned char payload_length;
extern volatile unsigned char recv;

#endif

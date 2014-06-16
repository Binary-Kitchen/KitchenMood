#include <avr/io.h>
#include <avr/interrupt.h>

#include "common.h"
#include "rs485.h"

#define TIMER_OFF {\
TIMSK1 = 0;\
TCNT1 = 0;\
TCCR1B = 0;\
}

#define TIMER_ON {\
TIMSK1 |= (1<<OCIE1A);\
TIFR1 |= (1<<OCF1A);\
TCNT1 = 0;\
TCCR1B = (1<<CS10);\
}

volatile unsigned char buffer[MAX_PAYLOAD];

static volatile unsigned char rs485_myid;

static volatile unsigned char state = RS485_IDLE;
static volatile unsigned char src = 0;
static volatile unsigned char dst = 0;
volatile unsigned char payload_length = 0;
static volatile unsigned char *ptr = buffer;

volatile unsigned char recv = 0;

ISR(TIMER1_COMPA_vect)
{
    TCNT1 = 0;
    TIMER_OFF;

    state = RS485_IDLE;
    ACTOFF;
}

ISR(USART0_RX_vect)
{
    unsigned char in = UDR;

    static uint8_t my_payload_length = 0;

    ACTON;
    TIMER_ON;

    if (state == RS485_IDLE && in == RS485_PREAMBLE) {
        state = RS485_RXACT;
    } else if (state == RS485_RXACT) {
        state = RS485_SRC;
        src = in;
    } else if (state == RS485_SRC) {
        state = RS485_DST;
        dst = in;
    } else if (state == RS485_DST) {
        state = RS485_PAYLOAD_LEN;
        my_payload_length = payload_length = in;
        if (payload_length > MAX_PAYLOAD) {
            goto reset;
        } else if (payload_length == 0) {
            state = RS485_PAYLOAD;
        }
    } else if (state == RS485_PAYLOAD_LEN) {
        *ptr++ = in;
        my_payload_length--;

        if (my_payload_length == 0) {
            state = RS485_PAYLOAD;
        }
    } else if (state == RS485_PAYLOAD) {
        TIMER_OFF;

        uint8_t crc = in;
        if (crc == 0xaa) {
            if (dst == rs485_myid) {
                //ACTOFF;
                recv = 1;
                // Hey here we are
            }
        }
        goto reset;
    }

    return;

reset:
    ptr = buffer;
    my_payload_length = 0;
    state = RS485_IDLE;
}

void init_rs485(void)
{
    UBRRL = 8;
    UBRRH = 0;
    // Enable receiver + enable receive interrupt
    UCSRB = (1<<RXEN)|(1<<RXCIE);
    // 8n1
    UCSRC = (1<<UCSZ1)|(1<<UCSZ0);

    rs485_myid = 0x10;

    // Configure Timer
    TCCR1A = 0;
    TCCR1B = 0; // Timer deactivated

    TCNT1 = 0;
    //OCR1A = 1600; // ~100µS

    OCR1A = 16000; // ~1mS

    // Disable interrupt
    TIMER_OFF;

    sei();
}

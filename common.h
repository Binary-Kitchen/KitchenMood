#ifndef COMMON_H
#define COMMON_H

#define ACTON  PORTE |=  0x08;
#define ACTOFF PORTE &= ~0x08;
#define ONON   PORTE |=  0x40;
#define ONOFF  PORTE &= ~0x40;

#define PWMCHANNELS 30

#endif

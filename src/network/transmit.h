#ifndef TRANSMIT
#define TRANSMIT

#include <stdint.h>

void transmit_current_bit(void);
void transmit_update(void);
uint8_t transmit_is_idle(void);
void transmit_packet(uint8_t *data, uint8_t length);

#endif
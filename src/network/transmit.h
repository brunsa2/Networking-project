#ifndef TRANSMIT
#define TRANSMIT

#include <stdint.h>
#include "medium.h"

void transmit_set_bus_state_pointer(t_bus_state *state);
void transmit_current_bit(void);
void transmit_update(void);
uint8_t transmit_is_idle(void);
void transmit_packet(uint8_t *data, uint8_t length);

#endif
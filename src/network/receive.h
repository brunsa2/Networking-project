#ifndef RECEIVE
#define RECEIVE

#include <stdint.h>

void receive_reset(void);
void receive_add(uint8_t bit);
void receive_bit(uint8_t bit);

#endif
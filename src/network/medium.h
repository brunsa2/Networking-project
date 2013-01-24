#ifndef MEDIUM
#define MEDIUM

#include <stdint.h>

void medium_monitor(void);
uint8_t medium_is_idle(void);
uint8_t medium_is_collided(void);

#endif
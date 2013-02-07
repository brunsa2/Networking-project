#ifndef MEDIUM
#define MEDIUM

#include <stdint.h>

typedef enum { BUS_IDLE, BUSY, COLLISION } t_bus_state;

void medium_monitor(void);
t_bus_state *medium_get_bus_state_pointer(void);
uint8_t medium_is_idle(void);
uint8_t medium_is_collided(void);

#endif
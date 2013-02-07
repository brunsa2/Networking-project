#include <avr/io.h>
#include "network.h"
#include "receive.h"
#include "../usart.h"

#include "medium.h"

#define RX_BIT ((RX_PIN & (1 << RX_PIN_NUMBER)) >> 1)

t_bus_state bus_state = COLLISION;

static volatile uint8_t state = 0, old_state;
static volatile uint8_t state_transition_table[2][36] = {
    {
        1, 2, 3, 4, 5, 6, 7, 8,
        9,
        10, 11, 12, 13, 14, 35,
        16, 17, 18, 19, 20, 21, 22, 23, 24, 35,
        15, 15, 15, 15, 15, 15, 15, 15, 15, 8,
        35
    },
    {
        1, 2, 3, 4, 5, 6, 7, 8,
        8,
        0, 0, 25, 25, 25, 35,
        0, 0, 25, 25, 25, 35, 25, 25, 25, 35,
        26, 27, 28, 29, 30, 31, 32, 33, 34, 8,
        8
    }
};

// TODO: Choose to emit one or two 1s from going to idle based on number of bits/bytes received

void medium_monitor(void) {
    old_state = state;
    state = state_transition_table[RX_BIT][state];
    
    switch (state) {
        case 8:
            if (old_state == 34) {
                receive_add(1);
                receive_add(1);
                receive_reset();
                network_receive_reset();
            }
            if (old_state == 35) {
                receive_reset();
                network_receive_reset();
            }
            bus_state = BUS_IDLE;
            LINK_LIGHT_PORT |= (1 << LINK_LIGHT_PIN_NUMBER);
            COLLISION_LIGHT_PORT &= ~(1 << COLLISION_LIGHT_PIN_NUMBER);
            break;
        case 9:
            LINK_LIGHT_PORT |= (1 << LINK_LIGHT_PIN_NUMBER);
            COLLISION_LIGHT_PORT |= (1 << COLLISION_LIGHT_PIN_NUMBER);
            break;
        case 15:
            if (old_state >= 27 && old_state <= 30) {
                receive_add(1);
            } else if (old_state >= 31 && old_state <= 33) {
                receive_add(1);
                receive_add(1);
            }
        case 25:
            if ((old_state >= 11 && old_state <= 13) || (old_state >= 17 && old_state <= 19)) {
                receive_add(0);
            } else if (old_state >= 21 && old_state <= 23) {
                receive_add(0);
                receive_add(0);
            }
            break;
        case 35:
            bus_state = COLLISION;
            LINK_LIGHT_PORT &= ~(1 << LINK_LIGHT_PIN_NUMBER);
            COLLISION_LIGHT_PORT |= (1 << COLLISION_LIGHT_PIN_NUMBER);
            break;
    }
}

t_bus_state *medium_get_bus_state_pointer(void) {
    return &bus_state;
}
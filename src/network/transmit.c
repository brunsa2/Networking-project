#include <avr/io.h>
#include "medium.h"
#include "network.h"
#include "../usart.h"

#include "transmit.h"

volatile t_bus_state *bus_state;

volatile uint8_t current_byte = 0xff, current_bit = 1;
volatile uint8_t transmit_failures, is_seeded = 0, random_number;
volatile uint16_t collision_wait_ticks;

volatile uint8_t bit_patterns[2][8] = { { 0, 0, 0, 0, 1, 1, 1, 1}, { 1, 1, 1, 1, 0, 0, 0, 0 } };

volatile uint8_t manchester_table[2][8] = {
    {
        0, 0, 0, 0, 1, 1, 1, 1
    },
    {
        1, 1, 1, 1, 0, 0, 0, 0
    }
};

volatile uint8_t transmit_clock = 63;

volatile uint8_t *buffer_start, *buffer_end;
volatile uint8_t buffer_length, buffer_length_left;

volatile uint8_t ticks = 0;

typedef enum { TRANSMITTING_IDLE, WAITING, TRANSMITTING, BUS_COLLISION, COLLISION_WAIT } t_transmitter_state;
t_transmitter_state transmitter_state = TRANSMITTING_IDLE;

void transmit_set_bus_state_pointer(t_bus_state *state) {
    bus_state = state;
}

void transmit_current_bit(void) {
    uint8_t current_transmit_port = TX_PORT;
    current_transmit_port &= ~(1 << TX_PIN_NUMBER);
    current_transmit_port |= (current_bit << TX_PIN_NUMBER);
    TX_PORT = current_transmit_port;
}

uint8_t get_next_bit(void) {
    uint8_t masked_bit = current_byte & (1 << (transmit_clock >> 3));
    if (masked_bit) {
        masked_bit = 1;
    }
    return manchester_table[masked_bit][transmit_clock & 0x07];
}


// TODO: Convert to table method, check inside for needed flags
void transmit_update(void) {
    ticks++;
    
    switch (transmitter_state) {
            // None
        case TRANSMITTING_IDLE:
            current_bit = 1;
            current_byte = 0xff;
            transmit_failures = 0;
            break;
            
        case WAITING:
            // Is idle?
            current_bit = 1;
            current_byte = 0xff;
            if (/**bus_state == BUS_IDLE*/ medium_is_idle()) {
                buffer_end = buffer_start;
                buffer_length = buffer_length_left;
                transmit_clock = 0;
                transmitter_state = TRANSMITTING;
            }
            break;
            
        case TRANSMITTING:
            // Transmit clock and buffer length both 0
            // Collided
            current_bit = get_next_bit();
            if (0 == transmit_clock) {
                if (buffer_length != 0) {
                    current_byte = *buffer_end++;
                    buffer_length--;
                } else {
                    buffer_end = buffer_start;
                    transmitter_state = TRANSMITTING_IDLE;
                }
                transmit_clock = 63;
            } else {
                transmit_clock--;
            }
            if (/**bus_state == COLLISION*/medium_is_collided()) {
                current_bit = 1;
                current_byte = 0xff;
                transmit_failures++;
                transmitter_state = BUS_COLLISION;
            }
            break;
            
        case BUS_COLLISION:
            // Idle and less than 10 failures
            current_bit = 1;
            if (/**bus_state == BUS_IDLE*/ medium_is_idle()) {
                if (transmit_failures < 10) {
                    if (!is_seeded) { // TODO: Seed at each transmit, start with 89, drop costly if statement
                        random_number = ticks;
                        is_seeded = 1;
                    } else {
                        random_number = (random_number * 109 + 89) % 256;
                    }
                    transmitter_state = COLLISION_WAIT;
                    collision_wait_ticks = random_number * 96;
                    current_bit = 1;
                } else {
                    buffer_end = buffer_start;
                    transmitter_state = TRANSMITTING_IDLE;
                }
            }
            break;
            
        case COLLISION_WAIT:
            // Zero collision wait ticks
            current_bit = 1;
            current_byte = 0xff;
            collision_wait_ticks--;
            if (0 == collision_wait_ticks) {
                transmitter_state = WAITING;
            }
            break;
    }
}

uint8_t transmit_is_idle(void) {
    return transmitter_state == TRANSMITTING_IDLE;
}

void transmit_packet(uint8_t *data, uint8_t length) {
    buffer_start = data;
    buffer_end = buffer_start;
    buffer_length = length;
    buffer_length_left = buffer_length;
    transmitter_state = WAITING;
}
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include "usart.h"

static volatile uint8_t raw_buffer = 0;

static volatile uint8_t filter[8] = { 0, 0, 0, 1, 0, 1, 1, 1 };

typedef enum { START_UP, IDLE, FIRST_LOW, LOW, HIGH, COLLISION } t_bus_state;
t_bus_state bus_state = START_UP;

static volatile uint8_t start_up_timer = 32;
static volatile uint8_t count = 0;

int main() {
    DDRA = (1 << PA0) | (0 << PA1) | (1 << PA7) | (1 << PA6) | (1 << PA2) | (1 << PA3);
    
    // 38461.5 Hz to sample each bit sixteen times (eight high, eight low)
    OCR0A = 52;
    // CTC mode with CLK/8
    TCCR0A = (1 << WGM01);
    TCCR0B = (1 << CS01);
    // Enable interrupts
    TIMSK0 = (1 << OCIE0A);
    
    asm volatile("sei");
    
    while (1);
    return 0;
}

ISR(TIMER0_COMPA_vect) {
    raw_buffer = ((raw_buffer << 1) & 0b00000111) | RX_BIT;

    PORTA |= 0x80;
    
    switch (bus_state) {
        case START_UP:
            if (start_up_timer != 0) {
                start_up_timer--;
            } else {
                bus_state = IDLE;
            }
            break;
            
        case IDLE:
            PORTA |= 0x04;
            PORTA &= ~(0x08);
            if (FILTERED_RX_BIT == 0) {
                bus_state = FIRST_LOW;
                count = 1;
            }
            break;
            
        case FIRST_LOW:
            PORTA |= 0x0c;
            if (FILTERED_RX_BIT == 1) {
                if (count < 3) {
                    bus_state = IDLE;
                } else if (count >= 3 && count <= 5) {
                    count = 1;
                    bus_state = HIGH;
                } else {
                    bus_state = COLLISION;
                }
            } else {
                count++;
            }
            break;
            
        case LOW:
            if (FILTERED_RX_BIT == 1) {
                if (count < 3) {
                    bus_state = IDLE;
                } else if (count >= 3 && count <= 5) {
                    count = 1;
                    bus_state = HIGH;
                } else if (count > 5 && count < 7) {
                    bus_state = IDLE;
                } else if (count >= 7 && count <= 9) {
                    count = 1;
                    bus_state = HIGH;
                } else {
                    bus_state = COLLISION;
                }
            } else {
                count++;
                if (count > 9) {
                    bus_state = COLLISION;
                }
            }
            break;
            
        case HIGH:
            if (FILTERED_RX_BIT == 0) {
                if (count < 3) {
                    bus_state = IDLE;
                } else if (count >= 3 && count <= 5) {
                    count = 1;
                    bus_state = LOW;
                } else if (count > 5 && count < 7) {
                    bus_state = IDLE;
                } else if (count >= 7 && count <= 9) {
                    count = 1;
                    bus_state = LOW;
                } else {
                    bus_state = IDLE;
                }
            } else {
                count++;
                if (count > 9) {
                    bus_state = IDLE;
                }
                
            }
            break;
            
        case COLLISION:
            PORTA |= 0x08;
            PORTA &= ~(0x04);
            if (FILTERED_RX_BIT == 1) {
                bus_state = IDLE;
            }
            break;
    }
    
    PORTA &= ~(0x80);
}
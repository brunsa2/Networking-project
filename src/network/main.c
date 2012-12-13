#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>

static volatile uint8_t raw_buffer;
static volatile uint8_t filtered_buffer;

/*static uint8_t filter[32] = {
    0, 0, 0, 0, 0, 0, 0, 1,
    0, 0, 0, 1, 0, 1, 1, 1,
    0, 0, 0, 1, 0, 1, 1, 1,
    0, 1, 1, 1, 1, 1, 1, 1
};*/

//static uint8_t filter[2] = {0, 1};

typedef enum { IDLE, BUSY, COLLISION } bus_state;
static volatile bus_state state = IDLE;

int main() {
    DDRA = 0xfe;
    
    // 9615.4 Hz to sample each bit four times (two high, two low)
    OCR0 = 207;
    // CTC mode with CLK/8
    TCCR0 = (1 << WGM01) | (1 << CS01);
    // Enable interrupts
    TIMSK = (1 << OCIE0);
    
    asm volatile("sei");
    while (1) {
        switch (state) {
            case IDLE:
                PORTA |= (1 << PA4);
                PORTA &= ~(1 << PA5);
                break;
            case BUSY:
                PORTA &= ~((1 << PA4) | (1 << PA5));
                break;
            case COLLISION:
                PORTA |= (1 << PA5);
                PORTA &= ~(1 << PA4);
                break;
        }
    }
}

ISR(TIMER0_COMP_vect) {
    PORTA |= (1 << PA3);
    
    uint8_t in = PINA & 0x01;
    raw_buffer = (in << 7) | (raw_buffer >> 1);
    
    if (raw_buffer & 0x80) {
        PORTA |= (1 << PA2);
    } else {
        PORTA &= ~(1 << PA2);
    }
    
    if (raw_buffer & 0x40) {
        PORTA |= (1 << PA1);
    } else {
        PORTA &= ~(1 << PA1);
    }
    
    uint8_t buffer_top = (raw_buffer & 0b11000000) >> 6;
    
    switch (state) {
        case IDLE:
            if (0b0 == (raw_buffer >> 7)) {
                state = BUSY;
            } else if (0b0000 == (raw_buffer >> 4)) {
                state = COLLISION;
            }
            break;
        case BUSY:
            if (0b1111 == (raw_buffer >> 4)) {
                state = IDLE;
            } else if (0b0000 == (raw_buffer >> 4)) {
                state = COLLISION;
            }
            break;
        case COLLISION:
            if (0b1 == (raw_buffer >> 7)) {
                state = IDLE;
            }
            break;
    }
    
    PORTA &= ~(1 << PA3);
}
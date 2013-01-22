#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include "usart.h"

#define RX_BIT ((PINA & 0b00000010) >> 1)

#define BUFFER_SIZE 250

static volatile uint8_t ticks = 0;

typedef enum { BUS_IDLE, BUSY, COLLISION } t_bus_state;
t_bus_state bus_state = COLLISION;

static volatile uint8_t state = 0;
static volatile uint8_t state_transition_table[2][32] = {
    {
        1, 2, 3, 4, 5, 6, 7, 8,
        9,
        10, 17,
        12, 13, 14, 15, 16, 17, 18, 19, 20, 31,
        11, 11, 11, 11, 11, 11, 11, 11, 11, 8,
        31
    },
    {
        1, 2, 3, 4, 5, 6, 7, 8,
        8,
        0, 0,
        0, 0, 21, 21, 21, 31, 21, 21, 21, 31,
        22, 23, 24, 25, 26, 27, 28, 29, 30, 8,
        8
    }
};

static uint8_t get_next_bit(void);

static volatile uint8_t current_byte = 0xff, current_bit = 1;
static volatile uint8_t transmit_failures, is_seeded = 0, random_number;
static volatile uint16_t collision_wait_ticks;

static volatile uint8_t bit_patterns[2][8] = { { 0, 0, 0, 0, 1, 1, 1, 1}, { 1, 1, 1, 1, 0, 0, 0, 0 } };

typedef enum { TRANSMITTING_IDLE, WAITING, TRANSMITTING, BUS_COLLISION, COLLISION_WAIT } t_transmitter_state;
t_transmitter_state transmitter_state = TRANSMITTING_IDLE;

static volatile uint8_t data_buffer[BUFFER_SIZE];
static volatile uint8_t buffer_pointer;
static volatile uint8_t buffer_length = 0;
static volatile uint8_t transmit_clock = 63;
static volatile uint8_t manchester_table[2][8] = {
    {
        0, 0, 0, 0, 1, 1, 1, 1
    },
    {
        1, 1, 1, 1, 0, 0, 0, 0
    }
};



int main() {
    DDRA = (1 << PA0) | (0 << PA1) | (1 << PA7) | (1 << PA6) | (1 << PA2) | (1 << PA3);
    
    usart_init(16000000L, 1000000L, USART_TRANSMIT | USART_RECEIVE);
    
    // 19230.8 Hz to sample/send each bit eight times (four high, four low)
    OCR0A = 104;
    // CTC mode with CLK/8
    TCCR0A = (1 << WGM01);
    TCCR0B = (1 << CS01);
    // Enable interrupts
    TIMSK0 = (1 << OCIE0A);
    
    PORTA |= 0x01;
    asm volatile("sei");
    usart_puts("Enter string and press enter to send\r\n");
    while (1) {
        if (usart_hasc()) {
            char usart_char = usart_getc();
            if (usart_char != '\r' && usart_char != '\n') {
                if (transmitter_state == TRANSMITTING_IDLE && buffer_pointer <= BUFFER_SIZE) {
                    usart_putc(usart_char);
                    data_buffer[buffer_pointer++] = 0x80 | usart_char;
                    buffer_length++;
                }
            }
            if (usart_char == '\n') {
                usart_putc(usart_char);
                transmitter_state = WAITING;
            }
        }
    }
    return 0;
}

static uint8_t get_next_bit(void) {
    uint8_t masked_bit = current_byte & (1 << (transmit_clock >> 3));
    if (masked_bit) {
        masked_bit = 1;
    }
    return manchester_table[masked_bit][transmit_clock & 0x07];
}

ISR(TIMER0_COMPA_vect) {
    ticks++;
    
    uint8_t port = PORTA;
    port &= ~(0x01);
    port |= 0x80 | current_bit;
    PORTA = port;
    
    state = state_transition_table[RX_BIT][state];
    
    switch (state) {
        case 8:
            bus_state = BUS_IDLE;
            PORTA |= 0x04;
            PORTA &= ~(0x08);
            break;
        case 9:
            bus_state = BUSY;
            PORTA |= 0x0c;
            break;
        case 31:
            bus_state = COLLISION;
            PORTA |= 0x08;
            PORTA &= ~(0x04);
            break;
    }
    
    switch (transmitter_state) {
        case TRANSMITTING_IDLE:
            current_bit = 1;
            break;
            
        case WAITING:
            current_bit = 1;
            if (BUS_IDLE == bus_state) {
                transmit_failures = 0;
                buffer_pointer = 0;
                transmit_clock = 0;
                transmitter_state = TRANSMITTING;
            }
            break;
            
        case TRANSMITTING:
            current_bit = get_next_bit();
            if (0 == transmit_clock) {
                if (buffer_length != 0) {
                    current_byte = data_buffer[buffer_pointer++];
                    buffer_length--;
                } else {
                    buffer_pointer = 0;
                    transmitter_state = TRANSMITTING_IDLE;
                }
                transmit_clock = 63;
            } else {
                transmit_clock--;
            }
            if (bus_state == COLLISION) {
                if (!is_seeded) {
                    random_number = ticks;
                } else {
                    random_number = (random_number * 109 + 89) % 256;
                }
                transmit_failures++;
                transmitter_state = BUS_COLLISION;
            }
            break;
            
        case BUS_COLLISION:
            current_bit = 1;
            if (bus_state == BUS_IDLE) {
                buffer_pointer = 0;
                if (transmit_failures < 10) {
                    transmit_clock = 0;
                    transmitter_state = COLLISION_WAIT;
                    collision_wait_ticks = random_number * 96;
                } else {
                    transmitter_state = TRANSMITTING_IDLE;
                }
            }
            break;
            
        case COLLISION_WAIT:
            collision_wait_ticks--;
            if (0 == collision_wait_ticks) {
                transmitter_state = TRANSMITTING;
            }
            break;
    }
    
    PORTA &= ~(0xc0);
}
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>

#include "usart.h"

#define BUFFER_SIZE 128

static uint8_t get_next_bit(void);
static uint8_t buffer_is_empty(void);
static uint8_t buffer_is_full(void);
static void buffer_enqueue(uint8_t data);
static uint8_t buffer_dequeue(void);

static volatile uint8_t current_byte = 0xff, current_bit = 1;
static volatile uint8_t bit_number = 0, is_transmitting = 0;

static volatile uint8_t bit_patterns[2][2] = { { 0, 1 }, { 1, 0 } };

static volatile uint8_t data_buffer[BUFFER_SIZE];
static volatile uint8_t buffer_head = 0, buffer_tail = 0;

int main() {
    DDRA = 0xff;
    PORTA = 0x01;
    
    // 9615.4 Hz to drive each bit four times (two high, two low)
    OCR0 = 207;
    // CTC mode with CLK/8
    TCCR0 = (1 << WGM01) | (1 << CS01);
    // Enable interrupts for timer
    TIMSK = (1 << OCIE0);
    
    usart_init(8000000, 9600, USART_TRANSMIT | USART_RECEIVE);
    
    asm volatile("sei");
    
    while (1) {
        if (usart_hasc()) {
            char usart_char = usart_getc();
            usart_putc(usart_char);
            if (usart_char != '\r' && usart_char != '\n') {
                buffer_enqueue(0b10000000 | usart_char);
            }
            if (usart_char == '\n') {
                current_byte = buffer_dequeue();
                bit_number = 15;
                is_transmitting = 1;
            }
        }
    }
}

static uint8_t get_next_bit(void) {
    if (bit_number > 15) {
        return 1;
    }
    if (!is_transmitting) {
        return 1;
    }
    uint8_t masked_bit = current_byte & (1 << (bit_number / 2));
    uint8_t bit_to_transmit = masked_bit > 0;
    return bit_patterns[bit_to_transmit][bit_number % 2];
}

static uint8_t buffer_is_empty(void) {
    uint8_t test;
    asm volatile("cli");
    test = buffer_head == buffer_tail;
    asm volatile("sei");
    return test;
}

static uint8_t buffer_is_full(void) {
    uint8_t test;
    asm volatile("cli");
    test = (buffer_tail + 1) % BUFFER_SIZE == buffer_head;
    asm volatile("sei");
    return test;
}

static void buffer_enqueue(uint8_t data) {
    while (buffer_is_full());
    data_buffer[buffer_tail] = data;
    buffer_tail++;
}

static uint8_t buffer_dequeue(void) {
    while (buffer_is_empty());
    uint8_t data = data_buffer[buffer_head];
    buffer_head++;
    return data;
}

ISR(TIMER0_COMP_vect) {
    PORTA = (PORTA & 0xf8) | 0b010 | current_bit;
    current_bit = get_next_bit();
    if (bit_number == 0) {
        PORTA |= 0b100;
        if (is_transmitting && !buffer_is_empty()) {
            current_byte = buffer_dequeue();
        } else {
            is_transmitting = 0;
        }
    }
    bit_number = bit_number == 0 ? 15 : bit_number - 1;
    PORTA &= ~(0b110);
}
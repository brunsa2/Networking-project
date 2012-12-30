#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

//#include "i2c_.h"
#include "../common/i2c.h"
#include "usart.h"

/*static void usart_put_hex_digit(uint8_t data) {
    if (data >= 0 && data <= 9) {
        usart_putc(data + '0');
    } else {
        usart_putc((data - 10) + 'a');
    }
}

static void usart_put_hex(uint8_t data) {
    usart_puts("0x");
    uint8_t high = (data & 0xf0) >> 4, low = data & 0x0f;
    usart_put_hex_digit(high);
    usart_put_hex_digit(low);
}

static void usart_put_hex_word(uint16_t data) {
    usart_puts("0x");
    uint8_t high_high = (data & 0xf000) >> 12, high_low = (data & 0xf00) >> 8;
    uint8_t low_high = (data & 0xf0) >> 4, low_low = data & 0xf;
    usart_put_hex_digit(high_high);
    usart_put_hex_digit(high_low);
    usart_put_hex_digit(low_high);
    usart_put_hex_digit(low_low);
}*/

int main() {
    uint8_t buffer[8];
    DDRD = 0xff;
    asm volatile("sei");
    
    i2c_init();
    
    i2c_transmit(0x35, (uint8_t *) "\x12\x34\x56\x78", 4);
    while (i2c_is_working());
    
    i2c_receive(0x35, &(buffer[0]), 4);
    while (i2c_is_working());
    
    PORTD = 0x01;
    
    while (1);
    return 0;
}

#include <avr/io.h>
#include <stdint.h>

#include "i2c.h"
#include "usart.h"

static void usart_put_hex_digit(uint8_t data) {
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
}

int main() {
    i2c_init();
    usart_init(0, 0, USART_TRANSMIT);
    
    uint8_t buffer[8];
    
    volatile uint16_t x, y;
    for (x = 0; x < 30000; x++) for (y = 0; y < 10; y++);
    
    int16_t status1 = i2c_receive(0x35, &(buffer[0]), 2);
    i2c_stop();
    usart_put_hex_word(status1);
    usart_puts("\r\n");
    return 0;
}
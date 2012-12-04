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
    
    /*volatile uint16_t x;
    
    i2c_start();
    i2c_send_address(0xa0);
    i2c_send_data(0x00);
    i2c_send_data(0x00);
    i2c_stop();

    for (x = 0; x<100; x++);
    
    i2c_start();
    i2c_send_address(0xa1);
    uint8_t y = i2c_receive_data_and_nack();
    i2c_stop();
    
    usart_put_hex(y);
    usart_puts("\r\n");*/
    
    char buffer[9];
    
    int16_t status1 = i2c_transmit(0x50, "\x00\x00", 2), status2 = -1;
    if (status1 == 2) {
        status2 = i2c_receive(0x50, &(buffer[0]), 9);
    }
    i2c_stop();
    usart_put_hex_word(status1);
    usart_puts("\r\n");
    usart_put_hex_word(status2);
    usart_puts("\r\n");
    
    if (status2 >= 0) {
        uint8_t x;
        for (x = 0; x < 9; x++) {
            usart_put_hex(buffer[x]);
            usart_puts("\r\n");
        }
    }
    
    return 0;
}
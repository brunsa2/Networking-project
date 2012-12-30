#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

//#include "i2c_.h"
#include "../common/i2c.h"
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


static uint8_t read_write = I2C_WRITE;
uint8_t read_or_write(void) {
    return read_write++;
}

uint8_t get_address(void) {
    return 0x35;
}

uint8_t get_data(void) {
    return 0xaa;
}

static uint8_t count = 4;

uint8_t should_stop(void) {
    if (count-- > 0) {
        return I2C_MASTER_SEND_NO_STOP;
    } else {
        return I2C_MASTER_SEND_STOP;
    }
}

static uint8_t count2 = 4;

uint8_t should_ack(void) {
    if (count2-- > 0) {
        return I2C_MASTER_SEND_ACK;
    } else {
        return I2C_MASTER_SEND_NACK;
    }
}

int main() {
    DDRD = 0xff;
    /*i2c_init();
    usart_init(0, 0, USART_TRANSMIT);
    
    uint8_t buffer[8];
    
    volatile uint16_t x, y;
    for (x = 0; x < 30000; x++) for (y = 0; y < 10; y++);
    
    int16_t status1 = i2c_transmit(0x35, "\xaa\xbb\xcc\xdd", 4);
    i2c_stop();
    i2c_receive(0x35, &(buffer[0]), 4);
    usart_put_hex_word(status1);
    usart_puts("\r\n");
    
    /*i2c_master master;
    i2c_init_master(&master);*/
    
    i2c_master master;
    // TODO: Rename all master and slave fxns to remove master_ or slave_ prefix
    // JA!
    master.use_interrupts = 1;
    master.master_no_slave_present = 0;
    master.master_get_address = get_address;
    master.master_read_or_write = read_or_write;
    master.master_get_data = get_data;
    master.master_should_stop = should_stop;
    master.master_should_ack = should_ack;
    master.master_set_data = 0;
    i2c_master_init(&master);
    asm volatile("sei");
    i2c_start();
    while(i2c_is_busy());
    i2c_start();
    
    while (1);
    return 0;
}

ISR(TWI_vect) {
    i2c_handle_event();
}


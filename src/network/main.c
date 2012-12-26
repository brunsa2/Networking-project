#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>

#include "i2c_slave.h"

static uint8_t count = 5;

uint8_t receive_address(void) {
    count--;
    if (count > 0) {
        return I2C_SLAVE_ACK;
    } else {
        return I2C_SLAVE_NACK;
    }
}

uint8_t receive_byte(uint8_t data) {
    if (data == 0xaa) {
        PORTA |= 0x80;
    } else if (data == 0xbb) {
        PORTA |= 0x40;
    } else if (data == 0xcc) {
        PORTA |= 0x20;
    } else if (data == 0xdd) {
        PORTA |= 0x10;
    }
    
    count--;
    if (count > 0) {
        return I2C_SLAVE_ACK;
    } else {
        return I2C_SLAVE_NACK;
    }
}

uint8_t transmit_byte(void) {
    return count++;
}

int main() {
    DDRA = 0xff;
    PORTA = 0x00;
    
    i2c_slave_handle i2c;
    i2c.address = 0x35;
    i2c.slave_receive_address = receive_address;
    i2c.slave_receive_byte = receive_byte;
    i2c.slave_transmit_byte = transmit_byte;
    
    asm("sei");
    
    i2c_slave_init(&i2c);
    
    while (1);
}
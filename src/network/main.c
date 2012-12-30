#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>

//#define I2C_INCLUDE_SLAVE

#include "../common/i2c.h"

static uint8_t count = 4;
static uint8_t count2 = 4;
static uint8_t data = 0;

uint8_t should_send_ack(void) {
    count--;
    if (count > 0) {
        return I2C_SLAVE_SEND_ACK;
    } else {
        return I2C_SLAVE_SEND_NACK;
    }
}

uint8_t should_send_data(void) {
    count2--;
    if (count2 > 0) {
        return I2C_SLAVE_SEND_DATA;
    } else {
        return I2C_SLAVE_SEND_NO_DATA;
    }
}

uint8_t transmit_byte(void) {
    return data++;
}

int main() {
    i2c_slave i2c;
    i2c.address = 0x35;
    i2c.accept_global_call = 1;
    i2c.use_interrupts = 0;
    
    i2c.slave_general_call = 0;
    i2c.slave_stop = 0;
    i2c.slave_should_send_ack = should_send_ack;
    i2c.slave_receive_byte = 0;
    
    i2c.slave_transmit_byte = transmit_byte;
    i2c.slave_should_send_data = should_send_data;
    
    i2c_slave_init(&i2c);
    
    while (1) {
        while (!i2c_has_event());
        i2c_handle_event();
    }
}
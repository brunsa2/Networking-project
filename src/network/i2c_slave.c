#include <avr/io.h>
#include <avr/interrupt.h>

#include "i2c_slave.h"

static i2c_slave_handle *slave_handle;

void i2c_slave_init(i2c_slave_handle *handle) {
    slave_handle = handle;
    // 100 kHz
    TWBR = 32;
    // Enable device and clear flags
    TWCR = (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
    // Prescale 1
    TWSR = 0;
    // Assign address
    TWAR = handle->address << 1;
}

ISR(TWI_vect) {
    uint8_t should_ack;
    switch (TWSR & 0xf8) {
        case I2C_SR_MASTER_WRITE_ACK:
            should_ack = slave_handle->slave_receive_address();
            if (I2C_SLAVE_ACK == should_ack) {
                TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
            } else {
                TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
            }
            break;
        case I2C_SR_DATA_RECEIVED_ACK_SENT:
            should_ack = slave_handle->slave_receive_byte(TWDR);
            if (I2C_SLAVE_ACK == should_ack) {
                TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
            } else {
                TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
            }
            break;
        case I2C_SR_DATA_RECEIVED_NACK_SENT:
            slave_handle->slave_receive_byte(TWDR);
            TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
            break;
        case I2C_ST_MASTER_READ_ACK:
            TWDR = slave_handle->slave_transmit_byte();
            TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
            break;
        case I2C_ST_DATA_SENT_ACK_RECEIVED:
            TWDR = slave_handle->slave_transmit_byte();
            TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
            break;
        case I2C_ST_DATA_SENT_NACK_RECEIVED:
            TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
            break;
    }
}
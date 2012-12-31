#include <avr/io.h>
#include <avr/interrupt.h>

#include "i2c.h"

#define DELAY_TIME 8

static volatile uint8_t slave_address;
static volatile uint8_t *buffer, bytes_left, bytes_transferred;
static volatile uint8_t is_working = 0;
static volatile uint8_t use_repeated_start = 0;

static volatile uint8_t is_slave = 0;
static volatile uint8_t *receive_buffer = 0, receive_buffer_head = 0, receive_buffer_size = 0;
static volatile uint8_t is_general_call = 0;
static void (*receive_callback)(uint8_t);
static volatile uint8_t *transmit_buffer = 0, transmit_buffer_head = 0, transmit_buffer_size = 0;
static void (*transmit_callback)(void);

void i2c_init(void) {
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
    TWSR = 0;
#if F_CPU < 3600000UL
    TWBR = 10;
#else
    TWBR = (F_CPU / 100000UL - 16) / 2;
#endif
}

// TODO: Need critical sections around global data
// TODO: Implement master transmit/receive callbacks
// TODO: Rename slave functions to include "slave" in name

void i2c_init_slave(uint8_t address, uint8_t accept_general_call) {
    is_slave = 1;
    TWAR = address << 1 | (accept_general_call & 0x01);
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (1 << TWEA);
}

void i2c_set_slave_status(uint8_t status) {
    is_slave = status == 1;
}

void i2c_set_receive_buffer(uint8_t *data, uint8_t size) {
    receive_buffer = data;
    receive_buffer_size = size != 0 ? size : 1;
    receive_buffer_head = 0;
}

void i2c_set_receive_callback(void (*callback)(uint8_t)) {
    receive_callback = callback;
}

void i2c_set_transmit_buffer(uint8_t *data, uint8_t size) {
    transmit_buffer = data;
    transmit_buffer_size = size != 0 ? size : 1;
    transmit_buffer_head = 0;
}

void i2c_set_transmit_callback(void (*callback)(void)) {
    transmit_callback = callback;
}

void i2c_transmit(uint8_t address, uint8_t *data, uint8_t size) {
    slave_address = address << 1;
    buffer = data;
    bytes_left = size != 0 ? size : 1;
    bytes_transferred = 0;
    is_working = 1;
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (1 << TWSTA);
}

void i2c_receive(uint8_t address, uint8_t *data, uint8_t size) {
    slave_address = address << 1 | 1;
    buffer = data;
    bytes_left = size != 0 ? size : 1;
    bytes_transferred = 0;
    is_working = 1;
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (1 << TWSTA);
}

uint8_t i2c_is_working(void) {
    return is_working;
}

void i2c_repeated_start_after_transmission(void) {
    use_repeated_start = 1;
}

void i2c_stop_after_transmission(void) {
    use_repeated_start = 0;
}

ISR(TWI_vect) {
    volatile uint8_t delay_counter;
    switch (TWSR & 0xf8) {
        case I2C_START_SENT:
        case I2C_REPEAT_START_SENT:
            TWDR = slave_address;
            TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
            is_working = 1;
            break;
        
        case I2C_MT_ADDRESS_SENT_ACK_RECEIVED:
        case I2C_MT_DATA_SENT_ACK_RECEIVED:
            if (bytes_left != 0) {
                TWDR = *(buffer++);
                bytes_left--;
                bytes_transferred++;
                TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
            } else {
                if (use_repeated_start) {
                    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (1 << TWSTA) | (is_slave << TWEA);
                } else {
                    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (1 << TWSTO) | (is_slave << TWEA);
                }
                for (delay_counter = DELAY_TIME; delay_counter != 0; delay_counter--);
                is_working = 0;
            }
            break;
        case I2C_MT_ADDRESS_SENT_NACK_RECEIVED:
        case I2C_MT_DATA_SENT_NACK_RECEIVED:
        case I2C_MT_ARB_LOST:
            TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (1 << TWSTO) | (is_slave << TWEA);
            for (delay_counter = DELAY_TIME; delay_counter != 0; delay_counter--);
            is_working = 0;
            break;
            
        case I2C_MR_ADDRESS_SENT_NACK_RECEIVED:
            TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (1 << TWSTO) | (is_slave << TWEA);
            for (delay_counter = DELAY_TIME; delay_counter != 0; delay_counter--);
            is_working = 0;
            break;
        case I2C_MR_ADDRESS_SENT_ACK_RECEIVED:
            if (bytes_left > 1) {
                TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (1 << TWEA);
            } else {
                TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
            }
            break;
        case I2C_MR_DATA_RECEIVED_ACK_SENT:
            *(buffer++) = TWDR;
            bytes_left--;
            bytes_transferred++;
            if (bytes_left > 1) {
                TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (1 << TWEA);
            } else {
                TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
            }
            break;
        case I2C_MR_DATA_RECEIVED_NACK_SENT:
            if (use_repeated_start) {
                TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (1 << TWSTA) | (is_slave << TWEA);
            } else {
                TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (1 << TWSTO) | (is_slave << TWEA);
            }
            for (delay_counter = DELAY_TIME; delay_counter != 0; delay_counter--);
            is_working = 0;
            break;
            
        case I2C_SR_ADDRESS_RECEIVED:
        case I2C_SR_ADDRESS_RECEIVED_ARB_LOST:
            is_working = 1;
            is_general_call = 0;
            receive_buffer_head = 0;
            if (receive_buffer_size - receive_buffer_head > 1) {
                TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (1 << TWEA);
            } else {
                TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
            }
            break;
        case I2C_SR_GENERAL_ADDRESS_RECEIVED:
        case I2C_SR_GENERAL_ADDRESS_RECEIVED_ARB_LOST:
            is_working = 1;
            is_general_call = 1;
            receive_buffer_head = 0;
            if (receive_buffer_size - receive_buffer_head > 1) {
                TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (1 << TWEA);
            } else {
                TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
            }
            break;
        case I2C_SR_DATA_RECEIVED_ACK_SENT:
        case I2C_SR_DATA_RECEIVED_GENERAL_ACK_SENT:
            if (receive_buffer) {
                receive_buffer[receive_buffer_head] = TWDR;
                receive_buffer_head++;
            }
            if (receive_buffer_size - receive_buffer_head > 1) {
                TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (1 << TWEA);
            } else {
                TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
            }
            break;
        case I2C_SR_DATA_RECEIVED_NACK_SENT:
        case I2C_SR_DATA_RECEIVED_GENERAL_NACK_SENT:
            if (receive_buffer) {
                receive_buffer[receive_buffer_head] = TWDR;
                receive_buffer_head++;
            }
            TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (is_slave << TWEA);
            is_working = 0;
            break;
        case I2C_SR_STOP_OR_REPEAT_START:
            is_working = 0;
            TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (is_slave << TWEA);
            for (delay_counter = DELAY_TIME; delay_counter != 0; delay_counter--);
            if (receive_callback) {
                receive_callback(is_general_call);
            }
            break;
            
        case I2C_ST_ADDRESS_RECEIVED:
        case I2C_ST_ADDRESS_RECEIVED_ARB_LOST:
            is_working = 1;
            transmit_buffer_head = 0;
        case I2C_ST_DATA_SENT_ACK_RECEIVED:
            if (transmit_buffer) {
                TWDR = transmit_buffer[transmit_buffer_head];
                transmit_buffer_head++;
            } else {
                TWDR = 0xff;
            }
            if (transmit_buffer_size - transmit_buffer_head != 0) {
                TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (1 << TWEA);
            } else {
                TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
            }
            break;
        case I2C_ST_DATA_SENT_NACK_RECEIVED:
            is_working = 0;
        case I2C_ST_NO_DATA_SENT_ACK_RECEIVED:
            TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (is_slave << TWEA);
            for (delay_counter = DELAY_TIME; delay_counter != 0; delay_counter--);
            if (transmit_callback) {
                transmit_callback();
            }
            break;
        
            
    }
}
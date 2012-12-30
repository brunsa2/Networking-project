#include <avr/io.h>
#include <avr/interrupt.h>

#include "i2c.h"

static i2c_master *current_master;
static i2c_slave *current_slave;
static uint8_t acknowledge_address = 0, use_interrupts = 0, is_busy = I2C_FREE;

static void _i2c_init(void) {
    // 100 kHz (or slower if clock is slow), clear flags and enable
#if F_CPU < 3600000UL
    TWBR = 10;
#else
    TWBR = (F_CPU / 100000UL - 16) / 2;
#endif
    TWCR = (1 << TWINT) | (1 << TWIE) | (1 << TWEN) | (1 << TWEA);
    TWSR = 0;
}

void i2c_master_init(i2c_master *master) {
    _i2c_init();
    current_master = master;
    use_interrupts = master->use_interrupts;
}

void i2c_slave_init(i2c_slave *slave) {
    _i2c_init();
    current_slave = slave;
    acknowledge_address = 1;
    use_interrupts = slave->use_interrupts;
    TWAR = slave->address << 1 | (slave->accept_global_call & 0x01);
}

uint8_t i2c_has_event(void) {
    if (TWCR & (1 << TWINT)) {
        return I2C_HAS_EVENT;
    } else {
        return I2C_HAS_NO_EVENT;
    }
}

uint8_t i2c_is_busy(void) {
    return is_busy;
}

void i2c_start(void) {
    TWCR = (1 << TWINT) | (use_interrupts << TWIE) | (acknowledge_address << TWEA) | (1 << TWEN) | (1 << TWSTA);
}

void i2c_handle_event(void) {
    uint8_t should_ack = 0;
    switch (TWSR & 0xf8) {
        /*case I2C_START_SENT:
        case I2C_REPEAT_START_SENT:
            is_busy = I2C_BUSY;
            if (current_master->master_get_address && current_master->master_read_or_write) {
                TWDR = current_master->master_get_address() << 1 | current_master->master_read_or_write();
            } else {
                TWDR = 0;
            }
            TWCR = (1 << TWINT) | (acknowledge_address << TWEA) | (1 << TWEN) | (use_interrupts << TWIE);
            break;
        case I2C_MT_ADDRESS_SENT_NACK_RECEIVED:
            if (current_master->master_no_slave_present) {
                current_master->master_no_slave_present();
            }
            TWCR = (1 << TWINT) | (acknowledge_address << TWEA) | (1 << TWEN) | (use_interrupts << TWIE);
            is_busy = I2C_FREE;
            break;
        case I2C_MT_ADDRESS_SENT_ACK_RECEIVED:
        case I2C_MT_DATA_SENT_ACK_RECEIVED:
            if (current_master->master_should_stop && I2C_MASTER_SEND_STOP == current_master->master_should_stop()) {
                TWCR = (1 << TWINT) | (acknowledge_address << TWEA) | (1 << TWEN) | (use_interrupts << TWIE) | (1 << TWSTO);
            } else {
                if (current_master->master_get_data) {
                    TWDR = current_master->master_get_data();
                } else {
                    TWDR = 0xff;
                }
                TWCR = (1 << TWINT) | (acknowledge_address << TWEA) | (1 << TWEN) | (use_interrupts << TWIE);
            }
            break;
        case I2C_MT_DATA_SENT_NACK_RECEIVED:
            TWCR = (1 << TWINT) | (acknowledge_address << TWEA) | (1 << TWEN) | (use_interrupts << TWIE) | (1 << TWSTO);
            is_busy = I2C_FREE;
            break;
        case I2C_MT_ARB_LOST:
            TWCR = (1 << TWINT) | (acknowledge_address << TWEA) | (1 << TWEN) | (use_interrupts << TWIE);
            is_busy = I2C_FREE;
            break;
            
        case I2C_MR_ADDRESS_SENT_NACK_RECEIVED:
            if (current_master->master_no_slave_present) {
                current_master->master_no_slave_present();
            }
            TWCR = (1 << TWINT) | (acknowledge_address << TWEA) | (1 << TWEN) | (use_interrupts << TWIE);
            break;
        case I2C_MR_ADDRESS_SENT_ACK_RECEIVED:
        case I2C_MR_DATA_RECEIVED_ACK_SENT:
            if (current_master->master_set_data) {
                current_master->master_set_data(TWDR);
            }
            if (current_master->master_should_ack && current_master->master_should_ack()) {
                TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (use_interrupts << TWIE);
            } else {
                TWCR = (1 << TWINT) | (1 << TWEN) | (use_interrupts << TWIE);
            }
            break;*/
            
            
        case I2C_SR_DATA_RECEIVED_ACK_SENT:
        case I2C_SR_DATA_RECEIVED_GENERAL_ACK_SENT:
            if (current_slave->slave_receive_byte) {
                current_slave->slave_receive_byte(TWDR);
            }
            if (current_slave->slave_should_send_ack) {
                should_ack = current_slave->slave_should_send_ack();
            }
            TWCR = (1 << TWINT) | ((acknowledge_address & should_ack) << TWEA) | (1 << TWEN) | (use_interrupts << TWIE);
            break;
        case I2C_SR_DATA_RECEIVED_NACK_SENT:
        case I2C_SR_DATA_RECEIVED_GENERAL_NACK_SENT:
            if (current_slave->slave_receive_byte) {
                current_slave->slave_receive_byte(TWDR);
            }
            if (current_slave->slave_should_send_ack) {
                should_ack = current_slave->slave_should_send_ack();
            }
            TWCR = (1 << TWINT) | (acknowledge_address << TWEA) | (1 << TWEN) | (use_interrupts << TWIE);
            break;
        case I2C_SR_GENERAL_ADDRESS_RECEIVED:
        case I2C_SR_GENERAL_ADDRESS_RECEIVED_ARB_LOST:
            if (current_slave->slave_general_call) {
                current_slave->slave_general_call();
            }
            if (current_slave->slave_should_send_ack) {
                should_ack = current_slave->slave_should_send_ack();
            }
            TWCR = (1 << TWINT) | ((acknowledge_address & should_ack) << TWEA) | (1 << TWEN) | (use_interrupts << TWIE);
            break;        case I2C_SR_ADDRESS_RECEIVED:
        case I2C_SR_ADDRESS_RECEIVED_ARB_LOST:
            if (current_slave->slave_should_send_ack) {
                should_ack = current_slave->slave_should_send_ack();
            }
            TWCR = (1 << TWINT) | ((acknowledge_address & should_ack) << TWEA) | (1 << TWEN) | (use_interrupts << TWIE);
            break;
        case I2C_SR_STOP_OR_REPEAT_START:
            if (current_slave->slave_stop) {
                current_slave->slave_stop();
            }
            TWCR = (1 << TWINT) | (acknowledge_address << TWEA) | (1 << TWEN) | (use_interrupts << TWIE);
            break;
            
        case I2C_ST_ADDRESS_RECEIVED:
        case I2C_ST_ADDRESS_RECEIVED_ARB_LOST:
        case I2C_ST_DATA_SENT_ACK_RECEIVED:
            if (current_slave->slave_transmit_byte) {
                TWDR = current_slave->slave_transmit_byte();
            } else {
                TWDR = 0xff;
            }
            if (current_slave->slave_should_send_data) {
                should_ack = current_slave->slave_should_send_data();
            }
            TWCR = (1 << TWINT) | ((acknowledge_address & should_ack) << TWEA) | (1 << TWEN) | (use_interrupts << TWIE);
            break;
        case I2C_ST_DATA_SENT_NACK_RECEIVED:
        case I2C_ST_NO_DATA_SENT_ACK_RECEIVED:
            TWCR = (1 << TWINT) | (acknowledge_address << TWEA) | (1 << TWEN) | (use_interrupts << TWIE);
            break;
    }
    PORTD = 0x00;
}
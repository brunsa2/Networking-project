/**
 * I2C
 *
 * ATmega I2C interface driver
 *
 * @author Jeff Stubler
 * @date November 13 2012
 */

#include "i2c_.h"

/**
 * Initialize I2C for 100 kHz operation
 */
void i2c_init(void) {
    // 100 kHz
    TWBR = 32;
    // Enable device and clear flags
    TWCR = (1 << TWEA) | (1 << TWEN);
    // Prescale 1
    TWSR = 0;
}

/**
 * Send start condition
 */
/*int8_t i2c_start(void) {
    // Start
    TWCR |= (1 << TWINT) | (1 << TWEN) | (1 << TWSTA);
    // Wait for TWINT
    while (!(TWCR & (1 << TWINT)));
    if ((TWSR & 0xf8) != I2C_START && (TWSR & 0xf8) != I2C_REPEATED_START) {
        return TWSR & 0xf8;
    }
    return 0;
}*/

/**
 * Send address and read/write byte
 * @param address Address and read/write bit
 * @return Error code
 */
int8_t i2c_send_address(uint8_t address) {
    TWDR = address;
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA); // Clear interrupt flag to send
    while (!(TWCR & (1 << TWINT))); // Wait for TWINT
    if ((TWSR & 0xf8) != I2C_MT_ADDRESS_SENT_SLAVE_ACK && (TWSR && 0xf8) != I2C_MR_ADDRESS_SENT_SLAVE_ACK) {
        return TWSR & 0xf8;
    }
    return 0;
}

/**
 * Send data byte
 * @param data Data byte
 * @return Error code
 */
int8_t i2c_send_data(uint8_t data) {
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN); // Clear interrupt to send
    while (!(TWCR & (1 << TWINT))); // Wait for TWINT
    if ((TWSR & 0xf8) != I2C_MT_DATA_SENT_SLAVE_ACK) {
        return TWSR & 0xf8;
    }
    return 0;
}

/**
 * Receive data byte
 * @return Data byte
 */
uint8_t i2c_receive_data(uint8_t *data) {
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    while (!(TWCR & (1 << TWINT))); // Wait for TWINT
    if ((TWSR & 0xf8) != I2C_MR_DATA_RECEIVED_MASTER_ACK) {
        return TWSR & 0xf8;
    }
    *data = TWDR;
    return 0;
}

/**
 * Receive data byte and nack
 * @return Data byte
 */
uint8_t i2c_receive_data_and_nack(uint8_t *data) {
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
    if ((TWSR & 0xf8) != I2C_MR_DATA_RECEIVED_MASTER_NACK) {
        return TWSR & 0xf8;
    }
    *data = TWDR;
    return 0;
}

/**
 * Send stop condition
 * @return Error code
 */
int8_t i2c_stop(void) {
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO); // Stop
    return 0;
}

/**
 * Send data to a slave device
 * @param address Address of device to send to
 * @param data Pointer to data to send
 * @param size Length of data to send
 * @return Length of data sent, or error code
 */
/*int16_t i2c_transmit(uint8_t address, char *data, uint8_t size) {
    uint8_t return_code, size_transmitted = 0;
    return_code = i2c_start();
    if (0 != return_code) {
        i2c_stop();
        return I2C_BUS_ISSUE;
    }
    return_code = i2c_send_data(address << 1 | I2C_WRITE_ADDRESS);
    if (I2C_MT_ADDRESS_SENT_SLAVE_NACK == return_code) {
        i2c_stop();
        return I2C_NO_DEVICE;
    }
    if (I2C_MT_ARBITRATION_LOST == return_code) {
        TWCR = (1 << TWINT);
        return I2C_BUS_BUSY;
    }
    if (size == 0) {
        return 0;
    }
    while (size-- > 0) {
        return_code = i2c_send_data(*data++);
        size_transmitted++;
        if (I2C_MT_DATA_SENT_SLAVE_NACK == return_code) {
            return size_transmitted;
        }
    }
    return size_transmitted;
}*/

/**
 * Receive data from slave device
 * @param address Address of device to receive from
 * @param data Pointer to data buffer
 * @param size Maximum transmission size
 * @retrun Length of data reeived, or error code
 */
/*int16_t i2c_receive(uint8_t address, void *data, uint8_t size) {
    uint8_t return_code, size_received = 0;
    return_code = i2c_start();
    if (0 != return_code) {
        i2c_stop();
        return I2C_BUS_ISSUE;
    }
    return_code = i2c_send_data(address << 1 | I2C_READ_ADDRESS);
    if (I2C_MR_ADDRESS_SENT_SLAVE_NACK == return_code) {
        i2c_stop();
        return I2C_NO_DEVICE;
    }
    if (I2C_MT_ARBITRATION_LOST == return_code) {
        TWCR = (1 << TWINT);
        return I2C_BUS_BUSY;
    }
    // TODO: Make sure it can handle reading 0 or 1 bytes
    while (size-- > 1) {
        return_code = i2c_receive_data(data);
        data++;
        size_received++;
        if (0 != return_code) {
            return I2C_BUS_ISSUE;
        }
    }
    return_code = i2c_receive_data_and_nack(data);
    size_received++;
    if (0 != return_code) {
        return I2C_BUS_ISSUE;
    }
    return size_received;
}*/

/**
 * Finish an I2C operation
 */
void i2c_finish(void) {
    i2c_stop();
}
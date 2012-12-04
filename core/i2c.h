/**
 * I2C
 *
 * ATmega I2C interface driver
 *
 * @author Jeff Stubler
 * @date November 13 2012
 */

#ifndef I2C
#define I2C

#include <avr/io.h>

#define I2C_WRITE_ADDRESS 0x00
#define I2C_READ_ADDRESS 0x01

#define I2C_START 0x08
#define I2C_REPEATED_START 0x10

#define I2C_MT_ADDRESS_SENT_SLAVE_ACK 0x18
#define I2C_MT_ADDRESS_SENT_SLAVE_NACK 0x20
#define I2C_MT_DATA_SENT_SLAVE_ACK 0x28
#define I2C_MT_DATA_SENT_SLAVE_NACK 0x30

#define I2C_MR_ADDRESS_SENT_SLAVE_ACK 0x40
#define I2C_MR_ADDRESS_SENT_SLAVE_NACK 0x48
#define I2C_MR_DATA_RECEIVED_MASTER_ACK 0x50
#define I2C_MR_DATA_RECEIVED_MASTER_NACK 0x58

#define I2C_MT_ARBITRATION_LOST 0x38
#define I2C_NO_INFO 0xf8
#define I2C_BUS_ERROR 0x00

#define I2C_BUS_ISSUE -1
#define I2C_NO_DEVICE -2
#define I2C_BUS_BUSY -3

/**
 * Initialize I2C for 100 kHz operation
 */
void i2c_init(void);

/**
 * Send start condition
 * @return Error code
 */
int8_t i2c_start(void);

/**
 * Send address and read/write byte
 * @param address Address and read/write bit
 * @return Error code
 */
int8_t i2c_send_address(uint8_t address);

/**
 * Send data byte
 * @param data Data byte
 * @return Error code
 */
int8_t i2c_send_data(uint8_t data);

/**
 * Receive data byte
 * @return Data byte
 */
uint8_t i2c_receive_data(uint8_t *data);

/**
 * Receive data byte and nack
 * @return Data byte
 */
uint8_t i2c_receive_data_and_nack(uint8_t *data);

/**
 * Send data to a slave device
 * @param address Address of device to send to
 * @param data Pointer to data to send
 * @param size Length of data to send
 * @return Length of data sent, or error code
 */
int16_t i2c_transmit(uint8_t address, char *data, uint8_t size);

/**
 * Receive data from slave device
 * @param address Address of device to receive from
 * @param data Pointer to data buffer
 * @param size Maximum transmission size
 * @retrun Length of data reeived, or error code
 */
int16_t i2c_receive(uint8_t address, void *data, uint8_t size);

/**
 * Finish an I2C operation
 */
void i2c_finish(void);

/**
 * Send stop condition
 * @return Error code
 */
int8_t i2c_stop(void);


#endif
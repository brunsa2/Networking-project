#ifndef I2C
#define I2C

#include <stdint.h>

#define I2C_ACCEPT_GENERAL_CALL 1
#define I2C_REJECT_GENERAL_CALL 0
#define I2C_ENABLE_SLAVE 1
#define I2C_DISABLE_SLAVE 0

#define I2C_START_SENT 0x08
#define I2C_REPEAT_START_SENT 0x10

#define I2C_MT_ADDRESS_SENT_ACK_RECEIVED 0x18
#define I2C_MT_ADDRESS_SENT_NACK_RECEIVED 0x20
#define I2C_MT_DATA_SENT_ACK_RECEIVED 0x28
#define I2C_MT_DATA_SENT_NACK_RECEIVED 0x30
#define I2C_MT_ARB_LOST 0x38

#define I2C_MR_ARB_LOST 0x38
#define I2C_MR_ADDRESS_SENT_ACK_RECEIVED 0x40
#define I2C_MR_ADDRESS_SENT_NACK_RECEIVED 0x48
#define I2C_MR_DATA_RECEIVED_ACK_SENT 0x50
#define I2C_MR_DATA_RECEIVED_NACK_SENT 0x58

#define I2C_SR_ADDRESS_RECEIVED 0x60
#define I2C_SR_ADDRESS_RECEIVED_ARB_LOST 0x68
#define I2C_SR_GENERAL_ADDRESS_RECEIVED 0x70
#define I2C_SR_GENERAL_ADDRESS_RECEIVED_ARB_LOST 0x78
#define I2C_SR_DATA_RECEIVED_ACK_SENT 0x80
#define I2C_SR_DATA_RECEIVED_NACK_SENT 0x88
#define I2C_SR_DATA_RECEIVED_GENERAL_ACK_SENT 0x90
#define I2C_SR_DATA_RECEIVED_GENERAL_NACK_SENT 0x98
#define I2C_SR_STOP_OR_REPEAT_START 0xa0

#define I2C_ST_ADDRESS_RECEIVED 0xa8
#define I2C_ST_ADDRESS_RECEIVED_ARB_LOST 0xb0
#define I2C_ST_DATA_SENT_ACK_RECEIVED 0xb8
#define I2C_ST_DATA_SENT_NACK_RECEIVED 0xc0
#define I2C_ST_NO_DATA_SENT_ACK_RECEIVED 0xc8

void i2c_init(void);
void i2c_init_slave(uint8_t address, uint8_t accept_general_call);
void i2c_set_slave_status(uint8_t status);
void i2c_set_receive_buffer(uint8_t *data, uint8_t size);
void i2c_set_receive_callback(void (*callback)(uint8_t is_general_call));
void i2c_set_transmit_buffer(uint8_t *data, uint8_t size);
void i2c_set_transmit_callback(void (*callback)(void));
void i2c_transmit(uint8_t address, uint8_t *data, uint8_t size);
void i2c_receive(uint8_t address, uint8_t *data, uint8_t size);
void i2c_repeated_start_after_transmission(void);
void i2c_stop_after_transmission(void);
uint8_t i2c_is_working(void);

#endif
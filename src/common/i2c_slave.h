#ifndef I2C_SLAVE
#define I2C_SLAVE

typedef struct {
    uint8_t address;
    uint8_t (*slave_receive_address)(void);
    uint8_t (*slave_receive_byte)(uint8_t data);
    uint8_t (*slave_transmit_byte)(void);
} i2c_slave_handle;

#define I2C_SLAVE_ACK 1
#define I2C_SLAVE_NACK 0

#define I2C_SR_MASTER_WRITE_ACK 0x60
#define I2C_SR_DATA_RECEIVED_ACK_SENT 0x80
#define I2C_SR_DATA_RECEIVED_NACK_SENT 0x88

#define I2C_ST_MASTER_READ_ACK 0xa8
#define I2C_ST_DATA_SENT_ACK_RECEIVED 0xb8
#define I2C_ST_DATA_SENT_NACK_RECEIVED 0xc0

void i2c_slave_init(i2c_slave_handle *handle);

#endif
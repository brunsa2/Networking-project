#ifndef I2C
#define I2C

#include <stdint.h>

#define I2C_MASTER_SEND_STOP 1
#define I2C_MASTER_SEND_NO_STOP 0
#define I2C_MASTER_SEND_ACK 1
#define I2C_MASTER_SEND_NACK 0
#define I2C_SLAVE_SEND_ACK 1
#define I2C_SLAVE_SEND_NACK 0
#define I2C_SLAVE_SEND_DATA 1
#define I2C_SLAVE_SEND_NO_DATA 0
#define I2C_HAS_EVENT 1
#define I2C_HAS_NO_EVENT 0
#define I2C_READ 1
#define I2C_WRITE 0
#define I2C_BUSY 1
#define I2C_FREE 0

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

//#ifdef I2C_INCLUDE_MASTER
typedef struct {
    uint8_t use_interrupts;
    
    uint8_t (*master_get_address)(void);
    void (*master_no_slave_present)(void);
    uint8_t (*master_get_data)(void);
    uint8_t (*master_should_stop)(void);
    uint8_t (*master_read_or_write)(void);
    uint8_t (*master_should_ack)(void);
    void (*master_set_data)(uint8_t data);
} i2c_master;
//#endif

//#ifdef I2C_INCLUDE_SLAVE
typedef struct {
    uint8_t address;
    uint8_t accept_global_call;
    uint8_t use_interrupts;
    
    uint8_t (*slave_should_send_ack)(void);
    void (*slave_receive_byte)(uint8_t data);
    void (*slave_general_call)(void);
    void (*slave_stop)(void);
    
    uint8_t (*slave_transmit_byte)(void);
    uint8_t (*slave_should_send_data)(void);
} i2c_slave;
//#endif

//#ifdef I2C_INCLUDE_MASTER
void i2c_master_init(i2c_master *master);
//#endif
//#ifdef I2C_INCLUDE_SLAVE
void i2c_slave_init(i2c_slave *slave);
//#endif
uint8_t i2c_has_event(void);
void i2c_handle_event(void);
uint8_t i2c_is_busy(void);

void i2c_start(void);

#endif
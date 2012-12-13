#ifndef I2C
#define I2C

#include <avr/io.h>

#define DDR_USI DDRA
#define PORT_USI PORTA
#define PIN_USI PINA
#define P_SCL PA4
#define P_SDA PA6

/**
 * Initialize I2C for 100 kHz operation
 */
void i2c_slave_init(uint8_t device_address);

#endif
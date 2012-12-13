#include <avr/interrupt.h>

#include "i2c_slave.h"

static uint8_t address;

typedef enum {
    CHECK_ADDRESS, ACK_SENT
} i2c_state;

i2c_state state;

/**
 * Initialize I2C for 100 kHz operation
 */
void i2c_slave_init(uint8_t device_address) {
    address = device_address;
    
    // Clear any pull-up resistors
    DDR_USI &= ~(1 << P_SDA);
    PORT_USI &= ~(1 << P_SDA);
    DDR_USI &= ~(1 << P_SCL);
    PORT_USI &= ~(1 << P_SCL);
    
    /*DDR_USI |= (1 << P_SDA);
    PORT_USI |= (1 << P_SDA);
    DDR_USI &= ~(1 << P_SDA);
    DDR_USI |= (1 << P_SCL);
    PORT_USI |= (1 << P_SCL);*/
    
    // Start condition interrupt on, no overflow interrupt, TWI mode, no USI counter overflow hold,
    // Shift reg clock source = external, + edge, 4 bit source = external, both ends, no toggle clock-port pin
    USICR = (1 << USISIE) | (1 << USIWM1) | (1 << USICS1);
    // Clear interrupts, reset overflow
    USISR = (1 << USISIF) | (1 << USIOIF) | (1 << USIDC);
}

ISR(USI_START_vect) {
    //PORTA &= ~(1 << PA1);
    // SDA input
    DDR_USI &= ~(1 << P_SDA);
    //PORTA &= ~(1 << PA2);
    // Wait for start to finish
    //while ((PIN_USI & (1 << P_SCL)) && !(PIN_USI & (1 << P_SDA)));
    //PORTA &= ~(1 << PA3);
    
    // Stop condition detect
    if (PIN_USI & (1 << P_SDA)) {
        i2c_slave_init(address);
        return;
    }
    
    state = CHECK_ADDRESS;
    
    // Start condition interrupt on, overflow interrupt, TWI mode, USI counter overflow hold,
    // Shift reg clock source = external, + edge, 4 bit source = external, both ends, no toggle clock-port pin
    USICR = (1 << USISIE) | (1 << USIOIE) | (1 << USIWM1) | (1 << USIWM0) | (1 << USICS1);
    // Clear interrupts, reset overflow
    USISR = (1 << USISIF) | (1 << USIOIF) | (1 << USIDC);
}

ISR(USI_OVF_vect) {
    uint8_t data = USIDR;
    uint8_t overflow_counter = 0x00;
    uint8_t detected_address;
    
    switch (state) {
        case CHECK_ADDRESS:
            detected_address = (data & 0xfe) >> 1;
            
            if (address == detected_address) {
                PORTA &= ~(1 << PA1);
                USIDR = 0;
                overflow_counter = 0x0e;
                DDR_USI |= (1 << P_SDA);
                state = ACK_SENT;
            } else {
                PORTA &= ~(1 << PA2);
                USIDR = 0;
                overflow_counter = 0x00;
                i2c_slave_init(address);
            }
            
            break;
        case ACK_SENT:
            overflow_counter = 0x00;
            DDR_USI &= ~(1 << P_SDA);
            break;
    }
    
    USISR = (1 << USIOIF) | (1 << USIDC) | (overflow_counter << USICNT0);
}
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>

#include "../common/i2c.h"

uint8_t r_buffer[8];

static void receive(uint8_t is_general_call) {
    //if (r_buffer[0] == 0x12 && r_buffer[1] == 0x34 && r_buffer[2] == 0x56 && r_buffer[3] == 0x78) PORTA |= 0x10;
}

static void transmit(void) {
    i2c_transmit(0x35, (uint8_t *) " ", 1);
}

int main() {
    DDRA = 0xff;
    PORTA = 0x00;
    i2c_init();
    i2c_init_slave(0x35, I2C_REJECT_GENERAL_CALL);
    i2c_set_receive_buffer(&(r_buffer[0]), 2);
    i2c_set_receive_callback(receive);
    i2c_set_transmit_buffer("\x14\x25\x36\x47\x58\x69    ", 6);
    i2c_set_transmit_callback(transmit);
    
    asm volatile("sei");
    
    while (1);
    return 0;
}
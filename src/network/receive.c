#include <avr/io.h>
#include "network.h"
#include "../usart.h"

#include "receive.h"

static uint8_t current_manchester_bits = 0, number_of_manchester_bits = 0;
static uint8_t current_byte = 0, number_of_byte_bits = 8;

void receive_reset(void) {
    number_of_manchester_bits = 2;
    number_of_byte_bits = 8;
    usart_puts("\r\n");
}

void receive_add(uint8_t bit) {
    /*if (bit) {
        usart_putsf("1");
    } else {
        usart_putsf("0");
    }*/
    current_manchester_bits = current_manchester_bits << 1 | bit;
    number_of_manchester_bits--;
    
    if (0 == number_of_manchester_bits) {
        number_of_manchester_bits = 2;
        
        if ((current_manchester_bits & 0b00000011) == 0b10) {
            receive_bit(0);
        } else if ((current_manchester_bits & 0b00000011) == 0b01) {
            receive_bit(1);
        } else {
            
        }
    }
}

void receive_bit(uint8_t bit) {
    if (bit) {
        usart_putc('#');
    } else {
        usart_putc('_');
    }
    current_byte = current_byte << 1 | bit;
    number_of_byte_bits--;
    
    if (0 == number_of_byte_bits) {
        number_of_byte_bits = 8;
        usart_putc('\r');
        usart_putc('\n');
        network_receive_byte(current_byte);
    }
}
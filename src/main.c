#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include "usart.h"
#include "network/network.h"
#include "network/medium.h"
#include "network/transmit.h"

#define BUFFER_SIZE 250

static uint8_t data_buffer[BUFFER_SIZE];
static uint8_t buffer_pointer = 0;



int main() {
    usart_init(16000000L, 1000000L, USART_TRANSMIT | USART_RECEIVE);
    
    network_init();
    
    
    usart_putsf("Enter string and press enter to send\r\n");
    
    while (1) {
        if (usart_hasc()) {
            char usart_char = usart_getc();
            if (usart_char != '\r' && usart_char != '\n') {
                if (transmit_is_idle() && buffer_pointer <= BUFFER_SIZE) {
                    usart_putc(usart_char);
                    data_buffer[buffer_pointer++] = 0x80 | usart_char;
                }
            }
            if (usart_char == '\n') {
                usart_putc(usart_char);
                transmit_packet(data_buffer, buffer_pointer);
                buffer_pointer = 0;
            }
        }
    }
    return 0;
}

/*ISR(TIMER0_COMPA_vect) {
   
    
        
}*/
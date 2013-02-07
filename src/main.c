#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include "usart.h"
#include "network/network.h"

#define BUFFER_SIZE 250
#define ADDRESS_0 133
#define ADDRESS_1 134

static uint8_t data_buffer[BUFFER_SIZE];
static uint8_t buffer_pointer = 0;

void receive(t_address destination, t_address source, uint8_t is_valid, uint8_t length, uint8_t *data) {
    usart_putc('*');
}

int main() {
    usart_init(0, USART_TRANSMIT | USART_RECEIVE);
    usart_putsf("Enter string and press enter to send\r\n");
    
    network_init();
    network_add_address(133);
    
    while (1) {
        if (usart_hasc()) {
            char usart_char = usart_getc();
            if (usart_char != '\r' && usart_char != '\n') {
                if (!network_is_transmitting() && buffer_pointer <= BUFFER_SIZE) {
                    usart_putc(usart_char);
                    data_buffer[buffer_pointer++] = 0x80 | usart_char;
                }
            }
            if (usart_char == '\n') {
                usart_putc(usart_char);
                
                t_packet packet;
                packet.crc = CRC_NONE;
                packet.source = 133;
                packet.destination = 93;
                packet.length = buffer_pointer;
                packet.data = data_buffer;
                
                network_transmit_packet(&packet);
                
                buffer_pointer = 0;
            }
        }
    }
    return 0;
}
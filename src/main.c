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
    
    t_address destination;
    t_receive_packet packet;
    
    uint8_t position = 0;
    t_address destination_address;
    
    while (1) { 
        if (usart_hasc()) {
            char usart_char = usart_getc();
            if (usart_char != '\r' && usart_char != '\n') {
                if (position == 0) {
                    usart_putc(usart_char);
                    destination_address = 100 * (usart_char - '0');
                    position = 1;
                } else if (position == 1) {
                    usart_putc(usart_char);
                    destination_address += 10 * (usart_char - '0');
                    position = 2;
                } else if (position == 2) {
                    usart_putc(usart_char);
                    destination_address += usart_char - '0';
                    position = 3;
                } else if (position == 3) {
                    usart_putc(usart_char);
                    position = 4;
                } else if (!network_is_transmitting() && buffer_pointer <= BUFFER_SIZE) {
                    usart_putc(usart_char);
                    data_buffer[buffer_pointer++] = 0x80 | usart_char;
                }
            }
            if (usart_char == '\n') {
                position = 0;
                usart_putc(usart_char);
                
                t_packet packet;
                packet.crc = CRC_NONE;
                packet.source = 133;
                packet.destination = destination_address;
                packet.length = buffer_pointer;
                packet.data = data_buffer;
                
                network_transmit_packet(&packet);
                
                buffer_pointer = 0;
            }
        }
        asm volatile("cli");
        if (network_has_received_packet(&destination) && (destination == 133 || destination == 0 || destination == 255)) {
            network_get_received_packet(destination, &packet);
            usart_putsf("\r\n(From %d): %s\r\n", packet.source, packet.data);
        } else {
            network_discard_received_packet();
        }
        asm volatile("sei");
    }
    return 0;
}
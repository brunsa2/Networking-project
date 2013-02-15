#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "../usart.h"
#include "medium.h"
#include "transmit.h"
#include "receive.h"

#include "network.h"

static volatile t_address addresses[NUMBER_OF_ADDRESSES];

static volatile uint8_t transmit_buffer[HEADER_SIZE + DATA_SIZE + DATA_CRC_SIZE];

static volatile uint8_t receive_length_left;
static volatile uint8_t receive_crc_mode;
static volatile uint16_t receive_data_crc;
static volatile uint8_t receive_buffer_pointer = 0;
static volatile t_receive_packet_state receive_packet_state = START;

static volatile t_packet receive_fifo[1 << FIFO_SIZE];
static volatile uint8_t fifo_head = 0, fifo_tail = 0, fifo_size = 0;

void network_init(void) {
#ifndef NO_TIMER
#ifdef __AVR_ATmega1284P__
    // Sample each bit Manchester bit four times, whole bit eight times
    OCR0A = (F_CPU >> 3) / (F_BAUD << 2);
    // CTC mode with CLK/8
    TCCR0A = (1 << WGM01);
    TCCR0B = (1 << CS01);
    // Enable interrupts
    TIMSK0 = (1 << OCIE0A);
#endif
#endif
    
    TX_DDR |= (1 << TX_PIN_NUMBER);
    RX_DDR &= ~(1 << RX_PIN_NUMBER);
    TX_PORT |= (1 << TX_PIN_NUMBER);
    LINK_LIGHT_DDR |= (1 << LINK_LIGHT_PIN_NUMBER);
    COLLISION_LIGHT_DDR |= (1 << COLLISION_LIGHT_PIN_NUMBER);
    
    transmit_set_bus_state_pointer(medium_get_bus_state_pointer());
    
    receive_reset();
    network_receive_reset();
#ifndef NO_TIMER
    asm volatile("sei");
#endif
}

#ifndef NO_TIMER
ISR(TIMER0_COMPA_vect) {
    transmit_current_bit();
    medium_monitor();
    transmit_update();
}
#endif

uint8_t network_is_transmitting(void) {
    return transmit_is_idle() ^ 1;
}

int8_t network_add_address(t_address address) {
    if (address == 0 || address == 255) {
        return INVALID_ADDRESS;
    }
    uint8_t address_table_location;
    for (address_table_location = 0; addresses[address_table_location] != 0; address_table_location++);
    if ( address_table_location >= NUMBER_OF_ADDRESSES) {
        return NO_MORE_ADDRESSES;
    }
    addresses[address_table_location] = address;
    return 0;
}

int8_t network_transmit_packet(t_packet *packet) {
    uint8_t address_table_location;
    for (address_table_location = 0; addresses[address_table_location] != packet->source; address_table_location++);
    if ( address_table_location >= NUMBER_OF_ADDRESSES) {
        return INVALID_ADDRESS;
    }
    
    // Magic number
    transmit_buffer[0] = 0xfa;
    // Version
    transmit_buffer[1] = VERSION_CODE;
    // Source address
    transmit_buffer[2] = packet->source;
    // Destination
    transmit_buffer[3] = packet->destination;
    // Data length
    transmit_buffer[4] = packet->length;
    // CRC mode
    transmit_buffer[5] = packet->crc;
    // Header CRC
    transmit_buffer[6] = 0xff;
    // Data
    memcpy((char *) (transmit_buffer + HEADER_SIZE), packet->data, packet->length);
    // Data CRC
    transmit_buffer[HEADER_SIZE + packet->length] = 0xff;
    transmit_buffer[HEADER_SIZE + packet->length + 1] = 0xff;
    
    transmit_packet((uint8_t *) transmit_buffer, HEADER_SIZE + packet->length + DATA_CRC_SIZE);
    return 0;
}

void network_receive_byte(uint8_t data) {
    switch (receive_packet_state) {
        case START:
            if (data == 0xfa) {
                //usart_putsf("Frame start\r\n");
                receive_packet_state = VERSION;
            }
            break;
            
        case VERSION:
            //if (data == VERSION_CODE) {
                //usart_putsf("Version 1\r\n");
            //    receive_packet_state = SOURCE;
            //} else {
            //    usart_putsf("Bad version %d\r\n", data);
            //    network_receive_reset();
            //}
            receive_packet_state = SOURCE;
            break;
        
        case SOURCE:
            receive_fifo[fifo_tail].source = data;
            receive_packet_state = DESTINATION;
            
            //usart_putsf("From %d\r\n", data);
            break;
            
        case DESTINATION:
            receive_fifo[fifo_tail].destination = data;
            receive_packet_state = LENGTH;
            
            //usart_putsf("To %d\r\n", data);
            break;
            
        case LENGTH:
            receive_fifo[fifo_tail].length = data;
            receive_length_left = data;
            receive_packet_state = CRC_MODE;
            
            //usart_putsf("%d bytes\r\n", data);
            break;
            
        case CRC_MODE:
            receive_crc_mode = data;
            receive_packet_state = HEADER_CRC;
            //usart_putsf("CRC mode %d\r\n", data);
            break;
            
        case HEADER_CRC:
            receive_packet_state = DATA;
            //usart_putsf("CRC %d\r\n", data);
            
            if (receive_fifo[fifo_tail].data) {
                free(receive_fifo[fifo_tail].data);
            }
            receive_fifo[fifo_tail].data = malloc(receive_fifo[fifo_tail].length + 1);
            
            break;
            
        case DATA:
            if (0 == receive_length_left) {
                receive_data_crc = data;
                
                receive_fifo[fifo_tail].data[receive_buffer_pointer] = '\0';
                receive_packet_state = DATA_CRC_LOW;
            } else {
                receive_fifo[fifo_tail].data[receive_buffer_pointer++] = 0x7f & data;
                receive_length_left--;
                
                if (0 == receive_length_left) {
                    receive_packet_state = DATA_CRC_HIGH;
                }
            }
            
            break;
            
        case DATA_CRC_HIGH:
            receive_data_crc = data;
            
            receive_fifo[fifo_tail].data[receive_buffer_pointer] = '\0';
            
            //usart_putsf("Data: %s\r\n", receive_fifo[fifo_tail].data);
            
            
            receive_packet_state = DATA_CRC_LOW;
            break;
            
        case DATA_CRC_LOW:
            receive_data_crc = receive_data_crc << 8 | data;
            receive_packet_state = START;
            //usart_putsf("CRC %l\r\n", receive_data_crc);
            
            if (fifo_size == (1 << FIFO_SIZE)) {
                fifo_head = (fifo_head + 1) & ((1 << FIFO_SIZE) - 1);
                fifo_size--;
            }
            fifo_tail = (fifo_tail + 1) & ((1 << FIFO_SIZE) - 1);
            fifo_size++;
            
            break;
    }
}

void network_receive_reset(void) {
    receive_length_left = 0;
    receive_buffer_pointer = 0;
    receive_packet_state = START;
}

uint8_t network_has_received_packet(t_address *destination) {
    *destination = receive_fifo[fifo_head].destination;
    return fifo_size != 0;
}

uint8_t network_get_received_packet(t_address destination, t_receive_packet *packet) {
    uint8_t packets_sent = 0;
    if (fifo_size > 0) {
        if (receive_fifo[fifo_head].destination == destination) {
            packet->source = receive_fifo[fifo_head].source;
            packet->destination = receive_fifo[fifo_head].destination;
            packet->length = receive_fifo[fifo_head].length;
            uint8_t *src = receive_fifo[fifo_head].data;
            uint8_t *dest = packet->data;
            memcpy(dest, src, receive_fifo[fifo_head].length);
            packet->data[receive_fifo[fifo_head].length] = '\0';
            packets_sent = 1;
        }
        fifo_head = (fifo_head + 1) & ((1 << FIFO_SIZE) - 1);
        fifo_size--;
    }
    return packets_sent;
}

void network_discard_received_packet(void) {
    if (fifo_size > 0) {
        fifo_head = (fifo_head + 1) & ((1 << FIFO_SIZE) - 1);
        fifo_size--;
    }
}

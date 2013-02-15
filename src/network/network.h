#ifndef NETWORK
#define NETWORK

// Configure with the following settings
// TODO: More light options
// TODO: Detect link system
// TODO: Light polarity
// TODO: Light stretching

#define F_BAUD 4800L

#define TX_DDR DDRA
#define TX_PORT PORTA
#define TX_PIN_NUMBER 0

#define RX_DDR DDRA
#define RX_PORT PORTA
#define RX_PIN PINA
#define RX_PIN_NUMBER 1

#define LINK_LIGHT_DDR DDRC
#define LINK_LIGHT_PORT PORTC
#define LINK_LIGHT_PIN_NUMBER 0

#define COLLISION_LIGHT_DDR DDRC
#define COLLISION_LIGHT_PORT PORTC
#define COLLISION_LIGHT_PIN_NUMBER 1

#define NUMBER_OF_ENDPOINTS 4

// Do not modify below this line

#define CRC_NONE 0
#define CRC_HEADER 1
#define CRC_MESSAGE 2
#define CRC_HEADER_AND_MESSAGE 3

#define ENDPOINT_RECEIVES_BROADCAST 0x01
#define ENDPOINT_RECEIVES_LOOPBACK 0x02
#define ENDPOINT_IS_PROMISCUOUS 0x03

#define NUMBER_OF_ADDRESSES 4
#define FIFO_SIZE 2

#define HEADER_SIZE 7
#define DATA_SIZE 250
#define DATA_CRC_SIZE 2
#define VERSION_CODE 0

#define INVALID_ADDRESS -1
#define NO_MORE_ADDRESSES -2

// TODO: Make lights active low
// TODO: Use a falling interrupt on receive line for starting/stopping timer

typedef uint8_t t_address;

typedef enum { START, VERSION, SOURCE, DESTINATION, LENGTH, CRC_MODE, HEADER_CRC, DATA, DATA_CRC_HIGH, DATA_CRC_LOW } t_receive_packet_state;

typedef struct {
    t_address source, destination;
    uint8_t length, is_valid;
    uint8_t data[DATA_SIZE + 1];
} t_receive_packet;

typedef struct {
    uint8_t crc;
    t_address source, destination;
    uint8_t length;
    uint8_t *data;
} t_packet;

void network_init(void);
uint8_t network_is_transmitting(void);
int8_t network_transmit_packet(t_packet *packet);
int8_t network_add_address(t_address address);
void network_receive_byte(uint8_t byte);
void network_receive_reset(void);
uint8_t network_has_received_packet(t_address *destination);
uint8_t network_get_received_packet(t_address destination, t_receive_packet *packet);
void network_discard_received_packet(void);

#endif
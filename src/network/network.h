#ifndef NETWORK
#define NETWORK

#define F_BAUD 4800

#define TX_DDR DDRA
#define TX_PORT PORTA
#define TX_PIN_NUMBER 0

#define RX_DDR DDRA
#define RX_PORT PORTA
#define RX_PIN PINA
#define RX_PIN_NUMBER 1

#define LINK_LIGHT_DDR DDRA
#define LINK_LIGHT_PORT PORTA
#define LINK_LIGHT_PIN_NUMBER 2

#define COLLISION_LIGHT_DDR DDRA
#define COLLISION_LIGHT_PORT PORTA
#define COLLISION_LIGHT_PIN_NUMBER 3

void network_init(void);

#endif
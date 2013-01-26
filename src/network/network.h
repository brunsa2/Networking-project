#ifndef NETWORK
#define NETWORK

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

// TODO: Make lights active low
// TODO: Use a falling interrupt on receive line for starting/stopping timer

void network_init(void);

#endif
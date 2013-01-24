#include <avr/io.h>
#include <avr/interrupt.h>
#include "medium.h"
#include "transmit.h"
#include "receive.h"

#include "network.h"

void network_init(void) {
#ifndef NO_TIMER
#ifdef __AVR_ATmega1284P__
    // 19230.8 Hz to sample/send each bit eight times (four high, four low)
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
    
    receive_reset();
#ifndef NO_TIMER
    asm volatile("sei");
#endif
}

#ifndef NO_TIMER
/*ISR(TIMER0_COMPA_vect) {
    transmit_current_bit();
    
    
    
    medium_monitor();
    transmit_update();
}*/
#endif
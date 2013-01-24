/**
 * USART driver
 *
 * Enables asynchronous serial communication, both receiving and transmitting, at single-character level
 *
 * @author Jeff Stubler
 * @date March 9, 2012
 */

#include "usart.h"

volatile static uint8_t usart_is_initialized = 0;

#define BAUD_HIGH 0
#define BAUD_LOW 16

 /**
  * Initialize USART with specified baud rate and options
  * @param clock MCU clock rate (Hz) to calculate the baud rate value for
  * @param baud Baud rate
  * @param flags Flags for options for serial port
  */
void usart_init(uint32_t clock, uint32_t baud, uint8_t flags) {
	UCSR0B = 0;
	if (flags & USART_TRANSMIT) {
		// Enable transmitting
		UCSR0B |= (1 << TXEN0);
	}
	if (flags & USART_RECEIVE) {
		// Enable receiving with receive interrupts enabled
		UCSR0B |= (1 << RXEN0);
	}
	UCSR0A = (1 << U2X0);
	UBRR0H = (unsigned char) BAUD_HIGH;
	UBRR0L = (unsigned char) BAUD_LOW;

	usart_is_initialized = 1;
}

/**
 * Send one byte over USART
 * @param data Data byte to transmit
 */
 // TODO: Error codes
int usart_putc(char data) {
	if (usart_is_initialized) {
		while (!(UCSR0A & (1 << UDRE0)));
		UDR0 = data;
	}
	return 0;
}

/**
 * Send string over USART
 * @param string
 */
void usart_puts(char *string) {
    while (*string) {
        usart_putc(*string++);
    }
}

/**
 * Receive one byte over USART
 * @return Byte from USART
 */
// TODO: Error codes
int usart_getc(void) {
	if (usart_is_initialized) {
		while (!(UCSR0A & (1 << RXC0)));
		return UDR0;
	}
	return 0;
}

/**
 * Check if the USART has received a character
 * @return Number of available characters from the USART
 */
int usart_hasc(void) {
	if (usart_is_initialized) {
		return (UCSR0A & (1 << RXC0)) >> 1;
	}
	return 0;
}

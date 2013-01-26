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

/**
 * @author http://www.menie.org/georges/embedded/printf.html
 */
static void usart_putsf_(int *varg) {
    register char *format_string = (char *)(*varg++);
    for (; *format_string != 0; ++format_string) {
        if (*format_string == '%') {
            format_string++;
            switch (*format_string) {
                case '%':
                    usart_putc('%');
                    break;
                    
                case 'c':
                    usart_putc(*varg++);
                    break;
                    
                case 's':
                    usart_puts(*((char **)varg++));
                    break;
                    
                case 'd':
                    usart_putd(*varg++, 10);
                    break;
                    
                case 'l':
                    usart_putl(*varg++, 10);
                    break;
                    
                case 'x':
                    usart_putd(*varg++, 16);
                    
                default:
                    format_string++;
            }
        } else {
            usart_putc(*format_string);
        }
    }
}

void usart_putd(uint8_t number, uint8_t radix) {
    char buffer[12];
    register uint8_t number_left = number, digit;
    register char *current_position = buffer + 12 - 1;
    *current_position = '\0';
    while (number_left) {
        digit = number_left % radix;
        if (digit >= 10) {
            digit += 'a' - '0' - 10;
        }
        *--current_position = digit + '0';
        number_left /= radix;
    }
    
    usart_puts(current_position);
}

void usart_putl(uint16_t number, uint8_t radix) {
    char buffer[12];
    register uint16_t number_left = number, digit;
    register char *current_position = buffer + 12 - 1;
    *current_position = '\0';
    if (number_left == 0) {
        *--current_position = '0';
    } else {
        while (number_left) {
            digit = number_left % radix;
            if (digit >= 10) {
                digit += 'a' - '0' - 10;
            }
            *--current_position = digit + '0';
            number_left /= radix;
        }
    }
    
    usart_puts(current_position);
}

/**
 * Sent formatted string over USART
 * @param string Formatted string to send
 * @param ... Items to senf
*/
void usart_putsf(const char *string, ...) {
    register int *varg = (int *) (&string);
    usart_putsf_(varg);
}

#include <avr/io.h>

int main(void) {
    DDRA = 0xff;
    PORTA = 0xff;
    while (1);
}
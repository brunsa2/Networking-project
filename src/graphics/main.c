#include <avr/io.h>
#include "gdm12864h.h"

int main(void) {
    DDRA = 0xff;
    PORTA = 0xff;
    while (1);
}
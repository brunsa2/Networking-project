#include <avr/io.h>
#include <stdint.h>

int main() {
    DDRB |= (1 << 1);
    PORTB |= (1 << 1);
    for (;;) {
        PORTB ^= (1 << 1);
        volatile uint16_t x;
        for (x = 0; x < 30000; x++);
    }
    return 0;
}
#include <avr/io.h>
#include <stdint.h>

int main() {
    DDRA = 0b00000010;
    while (1) {
        PORTA = (PINA & 0x01) << 1;
        //volatile uint8_t x = 1;
    }
}
/*
#include <avr/io.h>
#include <stdint.h>

int main() {
    DDRA = 0x02;
    PORTA = 0x02;
    for (;;) {
        PORTA ^= 0x02;
        volatile uint16_t x;
        for (x = 0; x < 30000; x++);
    }
    return 0;
}*/
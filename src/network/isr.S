#include <avr/io.h>

/*.section .text
.global TIMER0_COMP_vect
TIMER0_COMP_vect:
    push r16
    push r17

    in r16, _SFR_IO_ADDR(PORTA)
    ldi r17, 0b10000000
    eor r16, r17
    out _SFR_IO_ADDR(PORTA), r16

    pop r17
    pop r16

    reti

.end
*/
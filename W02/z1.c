#include <avr/io.h>
#include <inttypes.h>
#include <stdio.h>
#include <util/delay.h>

#define LED_STATE(state) PORTB = state ? PORTB | _BV(PB5) : PORTB & ~_BV(PB5);

int8_t buffer[100];  // initialized 0, 10ms delay in read

int main() {
    PORTD |= _BV(PD2);  // enable pull-up resistor on D2
    DDRB |= _BV(PB5);   // D13 as output
    int8_t cIt = 0;
    while (1) {
        LED_STATE(buffer[cIt]);
        buffer[cIt] = !(PIND & _BV(PD2));
        _delay_ms(10);
        cIt = cIt > 99 ? 0 : cIt+1; // cyclic iter ++
    }
    return 0;
}
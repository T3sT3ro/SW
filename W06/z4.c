#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <stdint.h>
#include <stdlib.h>
#include <util/delay.h>

#define LA(hi) (PORTB = (hi) ? PORTB | _BV(PB1) : PORTB & ~_BV(PB1))
#define OE(hi) (PORTB = (hi) ? PORTB | _BV(PB2) : PORTB & ~_BV(PB2))

ISR(SPI_STC_vect) {  // on transfer complete
    LA(0);           // stop transmiting
    OE(0);           // LEDs ON
}

const uint8_t LEDS[10] = {0b11000000, 0b11111001, 0b10100100, 0b10110000,
                          0b10011001, 0b10010010, 0b00000010, 0b11111000,
                          0b10000000, 0b00010000};
int main() {
    DDRB = _BV(PB1) | _BV(PB2) | _BV(PB5) | _BV(PB3);
    SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPIE) | _BV(SPR1);

    sei();
    char i = 0;
    while (1) {
        OE(1);  // LEDs OFF
        LA(1);
        SPDR = ~LEDS[i];
        _delay_ms(1000);
        i = i >= 9 ? 0 : i + 1;
    }
}
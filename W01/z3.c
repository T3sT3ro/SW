#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>




int main() {
    UCSR0B &= ~_BV(RXEN0) & ~_BV(TXEN0);  // TX, RX write setup
    DDRD = 0xFF;                          // set all PD to write
    PORTD = 0x80;
    register uint8_t rsh;
    while (1) {
        rsh = PORTD == 0x80 ? 0xFF : PORTD == 0x01 ? 0x00 : rsh;
        _delay_ms(60);
        PORTD = ~rsh & (PORTD << 1) | rsh &(PORTD >> 1);
    }
}
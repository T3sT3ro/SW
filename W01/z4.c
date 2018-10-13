#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

int8_t figures[10] = {0b11000000, 0b11111001, 0b10100100, 0b10110000,
                      0b10011001, 0b10010010, 0b00000010, 0b11111000,
                      0b10000000, 0b00010000};
int main() {
    UCSR0B &= ~_BV(RXEN0) & ~_BV(TXEN0);  // TX, RX write setup
    DDRD = 0xFF;                          // set all PD to write
    int8_t iter = 9;
    while (1) {
        PORTD = figures[iter];
        _delay_ms(1000);
        iter = iter == 0 ? 9 : iter - 1;
    }
}
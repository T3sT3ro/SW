#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

int8_t figures[10] = {0b11000000, 0b11111001, 0b10100100, 0b10110000,
                      0b10011001, 0b10010010, 0b00000010, 0b11111000,
                      0b10000000, 0b00010000};

int main() {
    UCSR0B &= ~_BV(RXEN0) & ~_BV(TXEN0); 
    DDRC = _BV(PC0) | _BV(PC1); // outputs
    DDRD = 0xff;

    uint16_t time = 0;
    while (1) {
        time = time >= 60000 ? 0 : time + 1;
        PORTC &= ~_BV(PC0);            // 0V on PC0 -> transistor ON
        PORTC |= _BV(PC1);             // 5V on PC1 -> transistor OFF
        PORTD = figures[time / 10000];  // decimals display

        _delay_ms(1);
        PORTC |= _BV(PC0);                   // 5V on PC0 -> transistor OFF
        PORTC &= ~_BV(PC1);                  // 0V on PC1 -> transistor ON
        PORTD = figures[(time / 1000) % 10];  // units display
        // PORTD = 0x00;
        // PORTC = 0x00;
        _delay_ms(1);
        time = time >= 60000 ? 0 : time + 1;

    }
    return 0;
}
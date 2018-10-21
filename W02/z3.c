#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

#define BTN_RESET_ON() (!(PINC & _BV(PC0)))
#define BTN_PREV_ON() (!(PINC & _BV(PC1)))
#define BTN_NEXT_ON() (!(PINC & _BV(PC2)))

int main() {
    UCSR0B &= ~_BV(RXEN0) & ~_BV(TXEN0);        // TX, RX write setup
    DDRD = 0xFF;                                // set all PD to output
    DDRC &= ~(_BV(PC0) | _BV(PC1) | _BV(PC2));  // PC0..2 as btn inputs
    PORTC |= _BV(PC0) | _BV(PC1) | _BV(PC2);  // without this it doesn't work ?
    uint8_t code = 0;
    int8_t bttn_flag = 0b00;
    while (1) {
        if (BTN_NEXT_ON() && !(bttn_flag & 0b10)) {
            code++;
            bttn_flag |= 0b10;
        }
        if (BTN_PREV_ON() && !(bttn_flag & 0b01)) {
            code--;
            bttn_flag |= 0b01;
        }
        if (BTN_RESET_ON()) code = 0;
        if (!BTN_NEXT_ON()) bttn_flag &= ~0b10;
        if (!BTN_PREV_ON()) bttn_flag &= ~0b01;

        PORTD = (code ^ (code >> 1));
    }
}
#include <avr/io.h>
#include <inttypes.h>
#include <stdio.h>
#include <util/delay.h>

int main() {
    ADMUX = _BV(REFS0);                              // Vref on AVCC
    ADCSRA |= _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2);  // prescaler = 1/128
    ADCSRA |= _BV(ADEN);                             // enable ADC
    _delay_ms(2);

    // timer 1 setup in fast PWM TOP=IRC1, prescaler 8
    TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS11);
    TCCR1A = _BV(WGM11) | _BV(COM1A1) | _BV(COM1B1);

    ICR1 = 3072;
    // PB3 - control EN
    DDRB |= _BV(PB1) | _BV(PB2) | _BV(PB3);
    PORTB |= _BV(PB3);

    while (1) {
        ADCSRA |= _BV(ADSC);
        while (!(ADCSRA & _BV(ADIF)))
            ;
        ADCSRA |= _BV(ADIF);
        uint16_t val = ADC;
        if(val > 512){
            OCR1A = 0;
            OCR1B = (val-512)*6;
        } else {
            OCR1A = (512-val)*6;
            OCR1B = 0;
        }
    }
}
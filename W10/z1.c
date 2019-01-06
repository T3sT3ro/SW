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
    TCCR1A = _BV(WGM11) | _BV(COM1A1);

    ICR1 = 3072;  // bo 16e6/(8*(3*1023+1)) = 651Hz
    // OCR1A = {min:256, max:510}
    DDRB |= _BV(PB1);

    while (1) {
        ADCSRA |= _BV(ADSC);
        while (!(ADCSRA & _BV(ADIF)))
            ;
        ADCSRA |= _BV(ADIF);
        OCR1A = ADC*3;
    }
}
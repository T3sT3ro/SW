#include <avr/io.h>
#include <inttypes.h>
#include <stdio.h>
#include <util/delay.h>

int main() {
    ADMUX = _BV(REFS0);                              // Vref on AVCC
    ADCSRA |= _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2);  // prescaler = 1/128
    ADCSRA |= _BV(ADEN);                             // enable ADC
    _delay_ms(2);

    // timer 1 setup in fast PWM TOP=IRC1, prescaler 64
    TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS11) | _BV(CS10);
    TCCR1A = _BV(WGM11) | _BV(COM1B1);

    ICR1 = 5115;  // bo 16e6/(64*(5115+1)) = 50Hz
    // OCR1A = {min:256, max:510}
    DDRB |= _BV(PB2);

    while (1) {
        ADCSRA |= _BV(ADSC);
        while (!(ADCSRA & _BV(ADIF)))
            ;
        ADCSRA |= _BV(ADIF);
        int32_t min = 150;
        int32_t max = 600;
        uint32_t val = ADC * (max - min) / 1024 + min;
        val = val < min ? min : val > max ? max : val;
        OCR1B = val;
    }
}
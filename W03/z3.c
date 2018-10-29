#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

int main() {
    uint8_t ledValue;
    uint16_t adcValue;
    DDRD |= _BV(PB5);

    ADMUX = _BV(REFS0);                              // Vref on AVCC
    ADCSRA |= _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2);  // prescaler = 1/128
    ADCSRA |= _BV(ADEN);                             // enable ADC
    _delay_ms(2);

    while (1) {
        ADCSRA |= _BV(ADSC);
        while (!(ADCSRA & _BV(ADIF)))
            ;
        ADCSRA |= _BV(ADIF);
        adcValue = ADC;
        ledValue = adcValue >> 2;

        for (int8_t k = 0; k < 5; k++) {
            PORTB = 0;

            for(uint16_t i=0; i*((i+10)/9) < ledValue*(ledValue/9);i++)
                _delay_us(1);

            PORTB |= _BV(PB5);
        }
    }
}
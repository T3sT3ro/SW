#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <stdint.h>
#include <stdlib.h>
#include <util/delay.h>

#define LED_STATE(state) PORTB = state ? PORTB | _BV(PB5) : PORTB & ~_BV(PB5);

int8_t buffer[60];

ISR(TIMER2_OVF_vect) {  // on timer2 overflow
    static int8_t intCnt = -1;
    if (++intCnt >= sizeof(buffer)/sizeof(int8_t))
        intCnt = 0;
    LED_STATE(buffer[intCnt]);
    buffer[intCnt] = !(PIND & _BV(PD3));
}

int main() {
    DDRB |= _BV(PB5);  // D13 as output
    PORTD |= _BV(PD3);  // enable pull-up resistor on D2

    // timer2 setup
    TCCR2B = _BV(CS20) | _BV(CS21) | _BV(CS22);  // 1024
    TIMSK2 |= _BV(TOIE2);                        // interrupt mask

    set_sleep_mode(SLEEP_MODE_PWR_SAVE);
    sei();  // set external interrupt
    while (1) sleep_mode();
}
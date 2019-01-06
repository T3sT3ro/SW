#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>
#include <stdio.h>
#include <util/delay.h>
#define BAUD 9600                             // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD)-1)  // zgodnie ze wzorem

// inicjalizacja UART
void uart_init() {
    UBRR0 = UBRR_VALUE;                  // ustaw baudrate
    UCSR0A = 0;                          // wyczyść rejestr UCSR0A
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);    // włącz odbiornik i nadajnik
    UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);  // ustaw format 8n1
}

int uart_transmit(char data, FILE *stream) {  // czekaj aż transmiter gotowy
    while (!(UCSR0A & _BV(UDRE0)))            // czekaj aż transmiter gotowy
        ;
    UDR0 = data;
    return 0;
}

int uart_receive(FILE *stream) {   // odczyt jednego znaku
    while (!(UCSR0A & _BV(RXC0)))  // czekaj aż znak dostępny
        ;
    return UDR0;
}

FILE uart_file;

volatile uint16_t val = 0;

ISR(ADC_vect) {
    val = ADC;
    OCR1A = val;
}

ISR(TIMER1_OVF_vect) { ADCSRA |= _BV(ADSC); }

ISR(TIMER1_CAPT_vect) { ADCSRA |= _BV(ADSC); }

int main() {
    uart_init();  // zainicjalizuj UART
    // skonfiguruj strumienie wejścia/wyjścia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;

    ADMUX = _BV(REFS0);  // Vref on AVCC, prescaler = 1/128
    ADCSRA |= _BV(ADEN) | _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2) | _BV(ADIE);
    DIDR0 = _BV(ADC0D);  // A0 analog input enable
    _delay_ms(2); // bo mogę

    // timer 1 setup in phase freq correct PWM TOP=IRC1, prescaler 8
    TCCR1B = _BV(WGM13) | _BV(CS11);
    TCCR1A = _BV(COM1A1);  // non inverting mode
    TIMSK1 = _BV(ICIE1) | _BV(TOIE1);    // capture event + timer overflow interrupts
    ICR1 = 1024;                         // bo 16e6/(2*8*1024) = 976Hz

    DDRB |= _BV(PB1);

    sei();
    while (1) {
        printf("mV: %lu\r\n", (5000*(uint32_t)val/1024));
        _delay_ms(100);
    }
}
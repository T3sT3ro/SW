#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
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

volatile int freq = 0, avg = 0, samples = 0;
ISR(INT0_vect) {
    ++samples;
    volatile int t = TCNT1;
    TCNT1 = 0;           // operujemy w trybie 16MHz/1024 = ~15kHz
    avg += (15000 / t);  // czyli freq = ~15600/dt
    if (samples == 10) {  // średnia arytmetyczna
        samples = 0;
        freq = avg / 10;
        avg = 0;
    }
}

int main() {
    EICRA |= _BV(ISC01) | _BV(ISC00);
    EIMSK = _BV(INT0);
    sei();

    uart_init();  // zainicjalizuj UART
    // skonfiguruj strumienie wejścia/wyjścia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;

    TCCR1B = _BV(CS12) | _BV(CS10);  // prescaler 1024, max 1Hz
    while (1) {
        printf("freq: %04d\r", freq);
    }
}
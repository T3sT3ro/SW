#include <avr/io.h>
#include <stdint.h>
#include <stdlib.h>
#include <util/delay.h>

#include <stdio.h>

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

inline uint16_t max(a, b) { return a > b ? a : b; }
inline uint16_t min(a, b) { return a < b ? a : b; }

int main() {
    uart_init();  // zainicjalizuj UART
    // skonfiguruj strumienie wejścia/wyjścia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;

    TCCR1A |= _BV(WGM10) | _BV(WGM11) | _BV(COM1B1);
    TCCR1B |= _BV(WGM13) | _BV(WGM12) | _BV(CS11) | _BV(ICES1);
    OCR1A = 53;  // TOP 16Mhz/8/37.9kHz
    OCR1B = OCR1A / 2;
    DDRB |= _BV(PB5);

    uint16_t x = ICR1;
    while (1) {
        PORTB &= ~_BV(PB5);
        if (x != ICR1) {
            PORTB |= _BV(PB5);
            x = ICR1;
        }
        DDRB |= _BV(PB2);
        _delay_us(600);
        DDRB &= ~_BV(PB2);
        _delay_us(600);
        printf("%d", ICR1);
    }
}

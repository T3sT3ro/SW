#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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

int uart_transmit(char data, FILE *stream) {
    while (!(UCSR0A & _BV(UDRE0)))
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

#define MY_MISO PC3
#define MY_MOSI PC2
#define MY_SCK PC1
#define MY_SS PC0

#define MY_SET(what, hi) (PORTC = hi ? PORTC | _BV(what) : PORTC & ~_BV(what))

int main() {
    uart_init();
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;

    DDRC |= _BV(MY_MOSI) | _BV(MY_SS) | _BV(MY_SCK);
    DDRB |= _BV(PB4);     // MISO
    SPCR = _BV(SPE);      // SPI enable
    PORTC |= _BV(MY_SS);  // if 1 then we transmit

    uint8_t i = 0;
    while (1) {
        uint8_t b = 0x80;
        uint8_t _;
        // simulate transmit
        PORTC &= ~_BV(MY_SS);  // start transm.
        do {
            MY_SET(MY_MOSI, i & b);
            MY_SET(MY_SCK, 1);
            _ = PORTC & _BV(MY_MISO);  // response from slave
            MY_SET(MY_SCK, 0);
            b >>= 1;
        } while (b != 0);
        PORTC |= _BV(MY_SS);  // end transm.

        while (!(SPSR & _BV(SPIF))) // receive 
            ;
        SPSR |= _BV(SPIF);
        uint8_t data = SPDR;
        printf("[%d]", data);
        _delay_ms(50);
        i++;
    }
}
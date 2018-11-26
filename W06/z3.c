#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include "./music.h"

#define PUT(PORT, OFF, hi) (PORT = (hi) ? PORT | _BV(OFF) : PORT & ~_BV(OFF))
#define BAUD 9600                             // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD)-1)  // zgodnie ze wzorem

// inicjalizacja UART
void uart_init() {
    // ustaw baudrate
    UBRR0 = UBRR_VALUE;
    // włącz odbiornik i nadajnik
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);
    // ustaw format 8n1
    UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

// transmisja jednego znaku
int uart_transmit(char data, FILE *stream) {
    // czekaj aż transmiter gotowy
    while (!(UCSR0A & _BV(UDRE0)))
        ;
    UDR0 = data;
    return 0;
}

// odczyt jednego znaku
int uart_receive(FILE *stream) {
    // czekaj aż znak dostępny
    while (!(UCSR0A & _BV(RXC0)))
        ;
    return UDR0;
}
FILE uart_file;

#define CS PB2
int main() {
    uart_init();
    // skonfiguruj strumienie wejścia/wyjścia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;

    DDRB = _BV(PB2) | _BV(PB5) | _BV(PB3);
    PUT(PORTB, CS, 1);
    SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPR1);

    // ADMUX = _BV(REFS0);  // referencja AVcc, wejście ADC0
    // DIDR0 = _BV(ADC0D);  // wyłącz wejście cyfrowe na ADC0
    // // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
    // ADCSRA = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2);  // preskaler 128
    // ADCSRA |= _BV(ADEN);                            // włącz ADC

    int i = 0;
    uint8_t x = 0;
    while (1) {
        PUT(PORTB, CS, 0);
        uint8_t note = pgm_read_byte(&music[i++]);
        
        SPDR = 0x30 + (note >> 4);  // config + half of note
        loop_until_bit_is_set(SPSR, SPIF);
        SPSR &= ~_BV(SPIF);
        
        
        SPDR = note << 4;  // second half of note
        loop_until_bit_is_set(SPSR, SPIF);
        SPSR &= ~_BV(SPIF);

        PUT(PORTB, CS, 1);

    //     ADCSRA |= _BV(ADSC); // wykonaj konwersję
    // while (!(ADCSRA & _BV(ADIF))); // czekaj na wynik
    // ADCSRA |= _BV(ADIF); // wyczyść bit ADIF (pisząc 1!)
    // uint16_t v = ADC; // weź zmierzoną wartość (0..1023)
    // printf("%"PRIu8"\r\n", note);
        //_delay_us(1);
        
        if (i >= sizeof(music) / sizeof(uint8_t)) i = 0;
    }
}
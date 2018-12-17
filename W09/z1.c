#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <inttypes.h>
#include <stdio.h>
#include <util/delay.h>
#include <math.h>

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

int main() {
    uart_init();  // zainicjalizuj UART
    // skonfiguruj strumienie wejścia/wyjścia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;

    ADMUX = _BV(REFS0) | _BV(REFS1);  // referencja AVcc, wejście ADC0
    DIDR0 = _BV(ADC0D);               // wyłącz wejście cyfrowe na ADC0
    // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
    ADCSRA = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2);  // preskaler 128
    ADCSRA |= _BV(ADEN);                            // włącz ADC

    while (1) {
        ADCSRA |= _BV(ADSC);  // wykonaj konwersję
        while (!(ADCSRA & _BV(ADIF)))
            ;                 // czekaj na wynik
        ADCSRA |= _BV(ADIF);  // wyczyść bit ADIF (pisząc 1!)
        uint16_t v = ADC;     // weź zmierzoną wartość (0..1023)
        float r = 4700.0*v/457.4; // 457.4 -> v_out w 25C
        float c = (4050/(log(r)+4.937)) - 273;
        printf("T: %03d.%02d\r", (int)c,((int)(c*100.0))%100);
        _delay_ms(500);
    }
}
#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include <stdio.h>
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
    
    ADMUX = _BV(REFS0);                              // Vref on AVCC
    ADCSRA |= _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2);  // prescaler = 1/128
    ADCSRA |= _BV(ADEN);                             // enable ADC
    _delay_ms(2);

    uint16_t adcVal;

    while (1) {
        ADCSRA |= _BV(ADSC);
        while (!(ADCSRA & _BV(ADIF))) // wait 
            ;
        ADCSRA |= _BV(ADIF); //clear

        adcVal = ADC; // Uout
        float R= 4700.0*adcVal/(1024.0-adcVal);
        float T= 4008.0/(log(R/4700.0)+4008.0*(1/298.0));
        printf("T: %d.%d \t R: %d\r\n", (int)(T-273), ((int)(T*100)%(100)), (int)R, ADC);


        _delay_ms(1000);
    }
}
#include <avr/io.h>
#include <stdint.h>
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

int main() {
    uart_init();  // zainicjalizuj UART
    // skonfiguruj strumienie wejścia/wyjścia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    int8_t a8 = 56, b8 = 3;
    int16_t a16 = 10000, b16 = 2356;
    int32_t a32 = 1234567, b32 = 100000;
    int64_t a64 = 100000000000000, b64 = 99999999999;
    float af = 123.4243124, bf = 2.23123;
    volatile register int8_t r8;
    volatile register int16_t r16;
    volatile register int32_t r32;
    volatile register int64_t r64;
    volatile register float rf;

    TCCR1B = _BV(CS10);  // enable clock - no prescaler. WGM0 = 0 - normal

    volatile uint16_t ad, ml, dv;
    
    _delay_ms(10);

    // liczy w ilości cykli
    TCNT1 = 0;  //cls
    r8 = a8 + b8; //op1
    ad = TCNT1; //calc
    TCNT1 = 0;    r8 = a8 * b8;    ml = TCNT1;
    TCNT1 = 0;    r8 = a8 / b8;    dv = TCNT1;
    printf("8  : +:%"PRIu16" *:%"PRIu16" /:%"PRIu16"\r\n", ad, ml, dv);
    TCNT1 = 0;    r16 = a16 + b16;    ad = TCNT1;
    TCNT1 = 0;    r16 = a16 * b16;    ml = TCNT1;
    TCNT1 = 0;    r16 = a16 / b16;    dv = TCNT1;
    printf("16 : +:%"PRIu16" *:%"PRIu16" /:%"PRIu16"\r\n", ad, ml, dv);
    TCNT1 = 0;    r32 = a32 + b32;    ad = TCNT1;
    TCNT1 = 0;    r32 = a32 * b32;    ml = TCNT1;
    TCNT1 = 0;    r32 = a32 / b32;    dv = TCNT1;
    printf("32 : +:%"PRIu16" *:%"PRIu16" /:%"PRIu16"\r\n", ad, ml, dv);
    TCNT1 = 0;    r64 = a64 + b64;    ad = TCNT1;
    TCNT1 = 0;    r64 = a64 * b64;    ml = TCNT1;
    TCNT1 = 0;    r64 = a64 / b64;    dv = TCNT1;
    printf("64 : +:%"PRIu16" *:%"PRIu16" /:%"PRIu16"\r\n", ad, ml, dv);
    TCNT1 = 0;    rf = af + bf;    ad = TCNT1;
    TCNT1 = 0;    rf = af * bf;    ml = TCNT1;
    TCNT1 = 0;    rf = af / bf;    dv = TCNT1;
    printf("f  : +:%"PRIu16" *:%"PRIu16" /:%"PRIu16"\r\n", ad, ml, dv);

    return 0;
}


/*
8  : +:4 *:4 /:4
16 : +:7 *:7 /:7
32 : +:13 *:13 /:13
64 : +:23 *:24 /:18
f  : +:13 *:13 /:13
*/
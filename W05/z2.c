#include <avr/interrupt.h>
#include <avr/io.h>
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
volatile static int val = 0;

ISR(INT0_vect) {
    loop_until_bit_is_clear(ADCSRA, ADIF);
    ADCSRA |= _BV(ADSC);  // convert start
    printf("converting...\r\n");
}

ISR(ADC_vect) {
    printf("conversion ended.\r\n");
    ADCSRA |= _BV(ADIF);  // clear ADIF
    val = ADC;
    printf("%d\r\n", val);
}

int main() {
    uart_init();
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;

    // ADC
    ADMUX = _BV(REFS0);  // Aref
    DIDR0 = _BV(ADC0D);
    ADCSRA = _BV(ADEN) | _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2);  // 128
    
    // INTERRUPTS SETUP
    DDRB |= _BV(DDB5);               
    EICRA |= _BV(ISC00) | _BV(ISC01);
    EIMSK |= _BV(INT0);              
    sei();

    // timer init
    ICR1 = 1024;
    TCCR1A = _BV(COM1A1) | _BV(WGM11);
    TCCR1B = _BV(WGM12) | _BV(WGM13) | _BV(CS10);
    DDRB |= _BV(PB1);                            
    
    ADCSRA |= _BV(ADIE); // ADC interrupt enable

    while (1) {
        float x = (float)val;
        x = (10000.0*x)/(1024.0-x); // from equation
        printf("%"PRId32"\r\n", (int32_t)(x));
        _delay_ms(1000);
    }
}

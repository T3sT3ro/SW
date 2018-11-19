// #include <avr/io.h>
// #include <stdint.h>
// #include <stdlib.h>
// #include <util/delay.h>

// #include <stdio.h>

// #define BAUD 9600                             // baudrate
// #define UBRR_VALUE ((F_CPU) / 16 / (BAUD)-1)  // zgodnie ze wzorem

// // inicjalizacja UART
// void uart_init() {
//     UBRR0 = UBRR_VALUE;                  // ustaw baudrate
//     UCSR0A = 0;                          // wyczyść rejestr UCSR0A
//     UCSR0B = _BV(RXEN0) | _BV(TXEN0);    // włącz odbiornik i nadajnik
//     UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);  // ustaw format 8n1
// }

// int uart_transmit(char data, FILE *stream) {  // czekaj aż transmiter gotowy
//     while (!(UCSR0A & _BV(UDRE0)))            // czekaj aż transmiter gotowy
//         ;
//     UDR0 = data;
//     return 0;
// }

// int uart_receive(FILE *stream) {   // odczyt jednego znaku
//     while (!(UCSR0A & _BV(RXC0)))  // czekaj aż znak dostępny
//         ;
//     return UDR0;
// }

// FILE uart_file;

// ISR(ADC_vect){

// }
// int main() {
//     uart_init();  // zainicjalizuj UART
//     // skonfiguruj strumienie wejścia/wyjścia
//     fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
//     stdin = stdout = stderr = &uart_file;
//     // ustaw tryb licznika
//     // COM1A = 10   -- non-inverting mode
//     // WGM1  = 1110 -- fast PWM top=ICR1
//     // CS1   = 10  -- prescaler 8
//     // ICR1  = 15624 -- próg resetu
    
//     // ADC
//     ADMUX = _BV(REFS0);   // Vref on AVCC
    
//     ADCSRA |= _BV(ADEN) | _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2);  // enable ADC, prescaler 128

//     sei();
//     _delay_ms(2); // stabilize ADC
    
//     timer1_init()
//     ADCSRA |= _BV(ADSC);
//     loop_until_bit_is_clear(ADCSRA, ADIF);

//     while (1) {
        
//     }
// }

#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include <math.h>
#define BAUD 9600                            // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD)-1) // zgodnie ze wzorem

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

void delay(int x)
{
    for (int i = 0; i < x; i++)
        _delay_ms(1);
}

volatile static int x = 0;
ISR(INT0_vect)
{
    ADCSRA |= _BV(ADSC); // wykonaj konwersję
    printf("start knowersji\r\n");
}

ISR(ADC_vect)
{
    ADCSRA |= _BV(ADIF); // wyczyść bit ADIF (pisząc 1!)
    printf("koniec knowersji\r\n");
    x = ADC;
    printf("%d\r\n", ADC);
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
    DDRB |= _BV(DDB5);    // ustaw wyzwalanie przerwania na INT0 i INT1 zboczem narastającym
    EICRA |= _BV(ISC00) | _BV(ISC01);    // odmaskuj przerwania dla INT0 i INT1
    EIMSK |= _BV(INT0);    // odmaskuj przerwania
    sei();

    // timer init
    ICR1 = 1024;
    TCCR1A = _BV(COM1A1) | _BV(WGM11);
    TCCR1B = _BV(WGM12) | _BV(WGM13) | _BV(CS10); // | _BV(CS12);
    DDRB |= _BV(PB1);  // ustaw pin OC1A (PB1) jako wyjście
    //ckl
    ADCSRA |= (1 << ADIE); // enable ADC interrupt

    while (1)
    {
        printf("%d\r\n", x);
        //  while (!(ADCSRA & _BV(ADIF))); // czekaj na wynik
        //  printf("koniec");
        delay(3000);
    }
}

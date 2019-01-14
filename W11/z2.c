// sterowanie silnikiem z potencjometru(PD2)
#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <util/delay.h>
#include "pid.h"

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

volatile uint16_t eng_MOSFET_OFF = 0;  // pomiar gdy MOSFET zamknięty
volatile uint16_t pot = 0;             // wartość z potencjometru

#define K_P 0.5
#define K_I 0.03
#define K_D 0.013
volatile uint8_t pidTimer = true;  // Flag for status information
struct PID_DATA pidData;           // Parameters for regulator
#define TIME_INTERVAL 157          // Sampling Time Interval TIME_INTERVAL = ( desired interval [sec] ) * ( frequency [Hz] ) / 255

// konwersja na przerwanie OCR1A~wypełnienie
ISR(ADC_vect) {
    if (ADMUX & _BV(MUX1)) {  // potencjometr A2
        pot = ADC;
        //OCR1A = pot;
        ADMUX &= ~_BV(MUX1);  // A2 OFF
        ADMUX |= _BV(MUX0);   // A1 ON
    } else {                  // silnik
        eng_MOSFET_OFF = ADC;
        ADMUX |= _BV(MUX1);   // A2 ON
        ADMUX &= ~_BV(MUX0);  // A1 OFF
        ADCSRA |= _BV(ADSC);  // start na potencjometr
    }
    pidTimer = true;
}

ISR(TIMER1_CAPT_vect) {  //top, MOSFET otwarty, ADC na silnik
    ADCSRA |= _BV(ADSC);
}

int main() {
    uart_init();  // zainicjalizuj UART
    // skonfiguruj strumienie wejścia/wyjścia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;

    // ADC init
    ADMUX = _BV(REFS0) | _BV(MUX0);  // Vref na AVCC, prescaler = 1/32, silnik A1
    ADCSRA |= _BV(ADEN) | _BV(ADPS2) | _BV(ADPS0) | _BV(ADIE);
    DIDR0 = _BV(ADC1D) | _BV(ADC2D);  // digital off A1(silnik) A2(potencjometr)

    // timer 1 setup in phase freq correct PWM TOP=IRC1, prescaler 8
    TCCR1B = _BV(WGM13) | _BV(CS11);  // PFC PWM, top=ICR1
    TCCR1A = _BV(COM1A1);             // non inverting mode
    TIMSK1 = _BV(ICIE1);              // capture event + timer overflow interrupts
    ICR1 = 1024;                      // bo 16e6/(2*8*1024) = 976Hz
    DDRB |= _BV(PB1);                 // Timer1 OC1A output

    pid_Init(K_P * SCALING_FACTOR, K_I * SCALING_FACTOR, K_D * SCALING_FACTOR, &pidData);

    // register interrupt handlers
    sei();

    int16_t referenceValue, measurementValue, inputValue;
    while (1) {
        if (pidTimer) {
            // na podstawie pomiarów mniej więcej liniowe zachowanie
            referenceValue = (int16_t)(((int32_t)-7 * (int32_t)pot / (int32_t)3) + (int32_t)1024);
            inputValue = pid_Controller(referenceValue, eng_MOSFET_OFF, &pidData);
            if(inputValue > (uint16_t)1023)
                inputValue = 1023;
            OCR1A = inputValue;
            pidTimer = FALSE;
            printf("pot[%.3u / 1024] \tMOSFET_OFF:[%.3u] \tref:[%.3u] \tIn:[%.3u]\r\n", pot, eng_MOSFET_OFF, referenceValue, inputValue);
            _delay_ms(10);
        }
    }
}
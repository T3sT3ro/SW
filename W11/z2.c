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

bool MOSFET_ON = 0;                    // przełącza zmienną pomiaru
volatile uint16_t eng_MOSFET_ON = 0;   // pomiar gdy MOSFET otwarty
volatile uint16_t eng_MOSFET_OFF = 0;  // pomiar gdy MOSFET zamknięty
volatile uint16_t pot = 0;             // wartość z potencjometru

// konwersja na przerwanie OCR1A~wypełnienie
ISR(ADC_vect) {
    if (ADMUX & _BV(MUX1)) {  // potencjometr A2
        pot = ADC;
        OCR1A = pot;
        ADMUX &= ~_BV(MUX1);  // A2 OFF
        ADMUX |= _BV(MUX0);   // A1 ON
    } else {                  // silnik
        if (MOSFET_ON)
            eng_MOSFET_ON = ADC;
        else
            eng_MOSFET_OFF = ADC;
        ADMUX |= _BV(MUX1);   // A2 ON
        ADMUX &= ~_BV(MUX0);  // A1 OFF
        ADCSRA |= _BV(ADSC);  // start na potencjometr
    }
}

ISR(TIMER1_OVF_vect) {  // bottom, ADC na potencjometr
    MOSFET_ON = false;
    ADCSRA |= _BV(ADSC);
}

ISR(TIMER1_CAPT_vect) {  //top, MOSFET otwarty, ADC na silnik
    MOSFET_ON = true;
    ADCSRA |= _BV(ADSC);
}

#define K_P 1.00
#define K_I 0.00
#define K_D 0.00
uint8_t pidTimer = true;   // Flag for status information
struct PID_DATA pidData;   // Parameters for regulator
#define TIME_INTERVAL 157  // Sampling Time Interval TIME_INTERVAL = ( desired interval [sec] ) * ( frequency [Hz] ) / 255(timer0)

ISR(TIMER0_OVF_vect) {  // timer 0 overflow, PID sample after sertain time
    static uint16_t i = 0;
    if (i < TIME_INTERVAL)
        i++;
    else {
        pidTimer = true;
        i = 0;
    }
}

int main() {
    uart_init();  // zainicjalizuj UART
    // skonfiguruj strumienie wejścia/wyjścia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;

    // ADC init
    ADMUX = _BV(REFS0) | _BV(MUX1);  // Vref na AVCC, prescaler = 1/128, potencjometr A2
    ADCSRA |= _BV(ADEN) | _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2) | _BV(ADIE);
    DIDR0 = _BV(ADC1D) | _BV(ADC2D);  // digital off A1(silnik) A2(potencjometr)

    // timer 1 setup in phase freq correct PWM TOP=IRC1, prescaler 8
    TCCR1B = _BV(WGM13) | _BV(CS11);   // PFC PWM, top=ICR1
    TCCR1A = _BV(COM1A1);              // non inverting mode
    TIMSK1 = _BV(ICIE1) | _BV(TOIE1);  // capture event + timer overflow interrupts
    ICR1 = 1024;                       // bo 16e6/(2*8*1024) = 976Hz
    DDRB |= _BV(PB1);                  // Timer1 OC1A output

    // timer 0 setup for PID controller
    TCCR0A = _BV(CS00);   // enable timer 0, no prescaler
    TIMSK0 = _BV(TOIE0);  // interrupt enable on overflow
    TCNT0 = 0;

    pid_Init(K_P * SCALING_FACTOR, K_I * SCALING_FACTOR, K_D * SCALING_FACTOR, &pidData);

    // register interrupt handlers
    sei();

    int16_t referenceValue, measurementValue, inputValue;
    while (1) {
        printf("pot[%lu / 1024] \tMOSFET_OFF:[%lu mV]\r\n", pot, (5000 * (uint32_t)eng_MOSFET_OFF / 1024));
        // if (pidTimer) {
        //     referenceValue = Get_Reference();
        //     measurementValue = Get_Measurement();

        //     inputValue = pid_Controller(referenceValue, measurementValue, &pidData);

        //     //Set_Input(inputValue);
        //     pidTimer = FALSE;
        // }
    }
}

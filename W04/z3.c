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

#define  max(a, b) ((a) > (b) ? (a) : (b))
#define  min(a, b) ((a) < (b) ? (a) : (b))

static const uint8_t _ledTable[256] = {
    0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
    1,   1,   1,   2,   2,   2,   2,   2,   2,   2,   2,   3,   3,   3,   3,
    3,   4,   4,   4,   4,   4,   5,   5,   5,   5,   6,   6,   6,   6,   7,
    7,   7,   7,   8,   8,   8,   9,   9,   9,   10,  10,  10,  11,  11,  12,
    12,  12,  13,  13,  14,  14,  15,  15,  15,  16,  16,  17,  17,  18,  18,
    19,  19,  20,  20,  21,  22,  22,  23,  23,  24,  25,  25,  26,  26,  27,
    28,  28,  29,  30,  30,  31,  32,  33,  33,  34,  35,  36,  36,  37,  38,
    39,  40,  40,  41,  42,  43,  44,  45,  46,  46,  47,  48,  49,  50,  51,
    52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,  64,  65,  67,
    68,  69,  70,  71,  72,  73,  75,  76,  77,  78,  80,  81,  82,  83,  85,
    86,  87,  89,  90,  91,  93,  94,  95,  97,  98,  99,  101, 102, 104, 105,
    107, 108, 110, 111, 113, 114, 116, 117, 119, 121, 122, 124, 125, 127, 129,
    130, 132, 134, 135, 137, 139, 141, 142, 144, 146, 148, 150, 151, 153, 155,
    157, 159, 161, 163, 165, 166, 168, 170, 172, 174, 176, 178, 180, 182, 184,
    186, 189, 191, 193, 195, 197, 199, 201, 204, 206, 208, 210, 212, 215, 217,
    219, 221, 224, 226, 228, 231, 233, 235, 238, 240, 244, 247, 250, 253, 255,
    255};



void transition() {
    uint32_t v = rand() % 255;
    
    int16_t r=0, g=0, b=0;
    switch (v%6){
        case 0: r=255; g=v; b=0; break;
        case 1: r=v; g=255; b=0; break;
        case 2: r=0; g=255; b=v; break;
        case 3: r=0; g=v; b=255; break;
        case 4: r=v; g=0; b=255; break;
        case 5: r=255; g=0; b=v; break;
        
    }
    int16_t ri = 0, gi = 0, bi = 0;
    for(int16_t i=0; i<255; i++){
        OCR0B = _ledTable[255 - min(r, i)];
        OCR0A = _ledTable[255 - min(g, i)];
        OCR2A = _ledTable[255 - min(b, i)];
        _delay_ms(10);
    }
    _delay_ms(200);

    for(int16_t i=255; i>=0; i--){
        OCR0B = _ledTable[255 - min(r, i)];
        OCR0A = _ledTable[255 - min(g, i)];
        OCR2A = _ledTable[255 - min(b, i)];
        _delay_ms(10);
    }

}

int main() {
    uart_init();  // zainicjalizuj UART
    // skonfiguruj strumienie wejścia/wyjścia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    // ustaw tryb licznika
    // COM1A = 10   -- non-inverting mode
    // WGM1  = 1110 -- fast PWM top=ICR1
    // CS1   = 10  -- prescaler 8
    // ICR1  = 15624 -- próg resetu
    TCCR0A = _BV(WGM01) | _BV(WGM00) | _BV(COM0A1) | _BV(COM0B1);
    TCCR0B = _BV(CS01);
    TCCR2A = _BV(WGM21) | _BV(WGM20) | _BV(COM2A1) | _BV(COM2B1);
    TCCR2B = _BV(CS21);

    DDRB |= _BV(PB3);
    DDRD |= _BV(PD6) | _BV(PD5);

    // RNG
    ADMUX = _BV(REFS0);   // Vref on AVCC
    ADCSRA |= _BV(ADEN);  // enable ADC
    _delay_ms(2);
    ADCSRA |= _BV(ADSC);
    loop_until_bit_is_clear(ADCSRA, ADIF);
    srand(ADC);

    while (1) {
        transition();
        _delay_ms(400);
    }
}

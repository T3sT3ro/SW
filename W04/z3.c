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

#define _R(rgb) ((rgb >> 16) & 0xff)
#define _G(rgb) ((rgb >> 8) & 0xff)
#define _B(rgb) (rgb & 0xff)

inline uint16_t max(a, b) { return a > b ? a : b; }
inline uint16_t min(a, b) { return a < b ? a : b; }

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

// convert 0->256 to RGB (maps to 0->360)
uint32_t hue_to_RGB(int8_t hue) {
    uint32_t R = 0, G = 0, B = 0;
    R = min(255, max(0, abs(hue - 128)));
    G = min(255, max(0, 255 - abs(hue - 85)));
    B = min(255, max(0, 255 - abs(hue - 170)));
    return (R << 16) + (G << 8) + (B);
}

void transition() {
    uint32_t rgb = hue_to_RGB(rand() % 255);

    int16_t r = _R(rgb), g = _G(rgb), b = _B(rgb);
    int16_t ri = 0, gi = 0, bi = 0;
    while (ri < r || gi < g || bi < b) {
        OCR0B = _ledTable[255 - ri];
        OCR0A = _ledTable[255 - gi];
        OCR2A = _ledTable[255 - bi];
        ri = min(255, ri + 1);
        bi = min(255, bi + 1);
        gi = min(255, gi + 1);
        _delay_ms(10);
    }

    while (ri > 0 || gi > 0 || bi > 0) {
        OCR0B = _ledTable[255 - ri];
        OCR0A = _ledTable[255 - gi];
        OCR2A = _ledTable[255 - bi];
        ri = max(0, ri - 1);
        bi = max(0, bi - 1);
        gi = max(0, gi - 1);
        _delay_ms(10);
    }

    _delay_ms(200);
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
        uint8_t x = rand() % 256;
        printf("%hhd = %" PRIx32 "\r\n", x, hue_to_RGB(x));
        transition();
        _delay_ms(200);
    }
}

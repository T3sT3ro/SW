#include <avr/io.h>
#include <inttypes.h>
#include <stdio.h>
#include <util/delay.h>

#define BAUD 9600                             // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD)-1)  // zgodnie ze wzorem

#define T_UNIT 100  // in miliseconds, probing
#define T_PROBE 20
#define T_SMBL_SPC (1 * T_UNIT)
#define T_LTR_SPC (3 * T_UNIT)
#define T_WRD_SPC (7 * T_UNIT)
#define T_DIT (1 * T_UNIT)
#define T_DAH (3 * T_UNIT)

#define LED_SETUP() DDRB |= _BV(PB5)
#define LED_STATE(state) PORTB = state ? PORTB |= _BV(PB5) : PORTB & ~_BV(PB5)

#define BTN PD2
#define BTN_PIN PIND
#define BTN_PORT PORTD

#define min(a, b) (a > b ? b : a)

static const char M_ALNUM[64] =
    "  ETIANMSURWDKGOHVF*L*PJBXCYZQ**54*3***2**+****16=/*****7***8*90";

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

    LED_SETUP();
    PORTD |= _BV(BTN);
    int16_t dt_on = 0;
    int16_t dt_off = T_WRD_SPC;
    int8_t buffer = 1;
    int8_t smbl = 0;
    int8_t flag = 0b01;  // 1 if already printed char
    LED_STATE(0);
    while (1) {
        if (dt_on > 0 && dt_on <= T_DIT)  // DIT
            smbl = 0;
        else if (dt_on >= T_DAH) {  // DAH
            smbl = 1;
            LED_STATE(1);
        }
        if (dt_on > 0) flag = 0b000;

        if (!(flag & 0b100) && dt_off > 0 &&
            dt_off <= T_SMBL_SPC) {  // smbl spc
            buffer = (buffer << 1) + smbl;
            LED_STATE(0);
            flag = 0b100;
        } else if (!(flag & 0b010) && dt_off >= T_LTR_SPC &&
                   dt_off < T_WRD_SPC) {
            printf("%c", buffer >= 64 ? '*' : M_ALNUM[buffer]);
            buffer = 1;
            flag = 0b010;
        } else if (!(flag & 0b01) && dt_off >= T_WRD_SPC) {  // wrd spc
            printf(" ");
            flag = 0b001;
        }
        dt_on = BTN_PIN & _BV(BTN) ? 0 : min(T_WRD_SPC, dt_on + T_PROBE);
        dt_off = BTN_PIN & _BV(BTN) ? min(T_WRD_SPC, dt_off + T_PROBE) : 0;
        // printf("[%" PRId16 " %" PRId16 "]\r\n", dt_on, dt_off);
        _delay_ms(T_PROBE);
    }
}

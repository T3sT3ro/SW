#include <avr/io.h>
#include <inttypes.h>
#include <stdio.h>
#include <util/delay.h>

#define BAUD 9600                             // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD)-1)  // zgodnie ze wzorem

#define T_UNIT 100  // in miliseconds
#define T_SMBL_SPC T_UNIT
#define T_LTR_SPC 3 * T_UNIT
#define T_WRD_SPC 7 * T_UNIT
#define T_DIT T_UNIT
#define T_DAH 3 * T_UNIT

#define LED_SETUP() (DDRB |= _BV(PB5))
#define LED_ON() (PORTB |= _BV(PB5))
#define LED_OFF() (PORTB &= ~_BV(PB5))
#define LED_TOGGLE() (PORTB ^= _BV(PB5))

#define STR_BUF_SZ 128

// 0b [0..0]  [1]   [0|1]
//    spacing begin morse_mask -> 0=dit 1=dah
static const int8_t M_ALNUM[36] = {
    // NUM
    0b00111111, 0b00101111, 0b00100111, 0b00100011, 0b00100001, 0b00100000,
    0b00110000, 0b00111000, 0b00111100, 0b00111110,
    // ALPHA
    0b00000101, 0b00011000, 0b00011010, 0b00001100, 0b00000010, 0b00010010,
    0b00001110, 0b00010000, 0b00000100, 0b00010111, 0b00001101, 0b00010100,
    0b00000111, 0b00000110, 0b00001111, 0b00010110, 0b00011101, 0b00001010,
    0b00001000, 0b00000011, 0b00001001, 0b00010001, 0b00001011, 0b00011001,
    0b00011011, 0b00011100};

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

    char str[STR_BUF_SZ];
    while (1) {
        int8_t cnt = 0;
        do {
            char c = getchar();
            str[cnt] = c;
            // filter characters to alphanumeric and space
            if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || c == '\r' ||
                c == ' ')
                printf("%c", c);
            else
                str[cnt--] = 0;  // repeat
        } while (str[cnt] != '\r' && ++cnt < 128);

        printf("\r\n");

        LED_OFF();
        for (int8_t i = 0, w_begin = 1; i < STR_BUF_SZ && str[i] != '\r'; i++) {
            char c = str[i];
            if (c != ' ') {
                if (!w_begin) {
                    printf(" ");
                    _delay_ms(T_LTR_SPC);  // letters spacing
                }

                int8_t bit = -1,
                       morse_mask = M_ALNUM[c > '9' ? c - 'a' + 10 : c - '0'],
                       letter_begin = 1;
                while (!(morse_mask & (0x80 >> ++bit)))  // signal seek
                    ;
                while (++bit < 8) {  // decode mask
                    // delay between dits/dahs in letter if not first symbol
                    if (!letter_begin)
                        _delay_ms(T_SMBL_SPC);

                    LED_ON();
                    if (morse_mask & (0x80 >> bit)) {
                        printf("-");
                        _delay_ms(T_DAH);
                    } else {
                        printf(".");
                        _delay_ms(T_DIT);
                    }
                    LED_OFF();
                    // set flag for inserting symbol spacings
                    letter_begin = 0;
                }
                // set flag for inserting word spacings
                w_begin = 0;
            } else {
                printf("       ");
                _delay_ms(T_WRD_SPC);
                w_begin = 1;
            }
        }
        printf("\r\n");
    }
}

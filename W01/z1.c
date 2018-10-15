#include <avr/io.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
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
long long atoll(const char * ptr){
    long long ret = 0;
    int8_t i = -1;
    while(ptr[++i] != '\0')
        ret = ret*10 + (ptr[i] - '0');
    return ret;
};

int main() {
    uart_init();  // zainicjalizuj UART
    // skonfiguruj strumienie wejścia/wyjścia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    int8_t a1, a2;
    int16_t b1, b2;
    int32_t c1, c2;
    union ll {
        int64_t d;
        int32_t t[2]
    } d1, d2;
    float f1, f2;
    char buf[128];
    scanf("%" SCNd8, &a1);    printf("\r\n%" PRId8, a1);
    scanf("%" SCNd16, &b1);    printf("\r\n%" PRId16, b1);
    scanf("%" SCNd32, &c1);    printf("\r\n%" PRId32, c1);
    scanf("%s", &buf);    printf("\r\n%s", buf);    d1.d = atoll(buf);
    scanf("%s", &buf);    printf("\r\n%s", f1);    f1 = atof(buf);

    scanf("%" SCNd8, &a2);    printf("\r\n%" PRId8, a2);
    scanf("%" SCNd16, &b2);    printf("\r\n%" PRId16, b2);
    scanf("%" SCNd32, &c2);    printf("\r\n%" PRId32, c2);
    scanf("%s", &buf);    printf("\r\n%s", buf);    d2.d = atoll(buf);
    scanf("%s", &buf);    printf("\r\n%s", f2);    f2 = atof(buf);

    char obuf2[256];
    union ll r;
    r.d = d1.d + d2.d;
    dtostrf(f1 + f2, 2, 16, obuf2);
    printf("\r\n+: %" PRId8 " %" PRId16 " %" PRId32 " %x%x %s\r\n", a1 + a2,
           b1 + b2, c1 + c2, r.t[1], r.t[0], obuf2); // little endian ?

    r.d = d1.d * d2.d;
    dtostrf(f1 * f2, 2, 16, obuf2);
    printf("\r\n*: %" PRId8 " %" PRId16 " %" PRId32 " %x%x %f\r\n", a1 * a2,
           b1 * b2, c1 * c2, r.t[1], r.t[0], obuf2);

    r.d = d1.d / d2.d;
    dtostrf(f1 / f2, 2, 16, obuf2);
    printf("\r\n/: %" PRId8 " %" PRId16 " %" PRId32 " %x%x %f\r\n", a1 / a2,
           b1 / b2, c1 / c2,  r.t[1], r.t[0], obuf2);
    return 0;
}

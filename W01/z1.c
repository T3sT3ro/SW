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

int uart_transmit(char data, FILE* stream) {  // czekaj aż transmiter gotowy
    while (!(UCSR0A & _BV(UDRE0)))            // czekaj aż transmiter gotowy
        ;
    UDR0 = data;
    return 0;
}

int uart_receive(FILE* stream) {   // odczyt jednego znaku
    while (!(UCSR0A & _BV(RXC0)))  // czekaj aż znak dostępny
        ;
    return UDR0;
}

FILE uart_file;
int64_t atoll(const char* ptr) {
    int64_t ret = 0;
    int8_t i = -1;
    while (ptr[++i] != '\0') ret = ret * 10 + (ptr[i] - '0');
    return ret;
};

char* lltoa(int64_t ll, char* llbuf) { // buffer of size 20 suffices
    int8_t cntr = 0;
    while (ll > 0) {
        llbuf[cntr++] = ll % 10 + '0';
        ll /= 10;
    }
    //rotate number
    for(int8_t i=0; i<cntr; i++){
        char c = llbuf[i];
        llbuf[i] = llbuf[cntr-i-1];
        llbuf[cntr-i-1] = c;
    }
    llbuf[cntr] = '\0';
    return llbuf;
}

int main() {
    uart_init();  // zainicjalizuj UART
    // skonfiguruj strumienie wejścia/wyjścia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    int8_t a1, a2;
    int16_t b1, b2;
    int32_t c1, c2;
    int64_t d1, d2;

    float f1, f2;
    char buf[128];
    scanf("%" SCNd8, &a1);    printf("\r\n%" PRId8, a1);
    scanf("%" SCNd16, &b1);    printf("\r\n%" PRId16, b1);
    scanf("%" SCNd32, &c1);    printf("\r\n%" PRId32, c1);    
    scanf("%s", buf);    d1 = atoll(buf);    printf("\r\n%s", buf);
    scanf("%s", buf);    f1 = (float)atof(buf);    printf("\r\n%s", buf);

    scanf("%" SCNd8, &a2);    printf("\r\n%" PRId8, a2);
    scanf("%" SCNd16, &b2);    printf("\r\n%" PRId16, b2);
    scanf("%" SCNd32, &c2);    printf("\r\n%" PRId32, c2);
    scanf("%s", buf);    d2 = atoll(buf);    printf("\r\n%s", buf);
    scanf("%s", buf);    f2 = (float)atof(buf);    printf("\r\n%s", buf);

    char obuf1[20];
    char obuf2[256];
    dtostrf((double)(f1 + f2), 10, 16, obuf2);
    printf("\r\n+: %" PRId8 " %" PRId16 " %" PRId32 " %s %s\r\n",
           a1 + a2, b1 + b2, c1 + c2, lltoa(d1+d2, obuf1), obuf2);

    dtostrf((double)(f1 * f2), 10, 16, obuf2);
    printf("\r\n*: %" PRId8 " %" PRId16 " %" PRId32 " %s %s\r\n",
           a1 * a2, b1 * b2, c1 * c2, lltoa(d1*d2, obuf1), obuf2);

    dtostrf((double)(f1 / f2), 10, 16, obuf2);

    printf("\r\n/: %" PRId8 " %" PRId16 " %" PRId32 " %s %s\r\n",
           a1 / a2, b1 / b2, c1 / c2, lltoa(d1/d2, obuf1), obuf2);
    return 0;
}

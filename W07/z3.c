#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>
#include "i2c.h"

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

//#define MY_SET(what, hi) (PORTC = hi ? PORTC | _BV(what) : PORTC & ~_BV(what))

int main() {
    uart_init();
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;

    i2cInit();

    while (1) {
        char buf[16];
        scanf("%s", &buf[0]);
        if (strcmp(buf, "date") == 0) {
            i2cStart();
            i2cSend(0xD0);
            i2cSend(0x04);  // read addr
            i2cStart();
            i2cSend(0xD1);
            uint8_t d = i2cReadAck();
            uint8_t m = i2cReadAck();
            uint8_t y = i2cReadNoAck();
            i2cStop();
            printf("%02d-%02d-20%02d\r\n", ((d & 0x30) >> 4) * 10 + (d & 0x0F),
                   ((m & 0x10) >> 4) * 10 + (m & 0x0F),
                   ((y & 0xF0) >> 4) * 10 + (y & 0x0F));
        } else if (strcmp(buf, "time") == 0) {
            i2cStart();
            i2cSend(0xD0);
            i2cSend(0x00);
            i2cStart();
            i2cSend(0xD1);
            uint8_t s = i2cReadAck();
            uint8_t m = i2cReadAck();
            uint8_t h = i2cReadNoAck();
            i2cStop();
            printf("%02d:%02d:%02d\r\n", ((h & 0x30) >> 4) * 10 + (h & 0x0F),
                   ((m & 0x70) >> 4) * 10 + (m & 0x0F),
                   ((s & 0x70) >> 4) * 10 + (s & 0x0F));
        } else {  // set
            scanf("%s", &buf[0]);
            if (strcmp(buf, "date") == 0) {
                int8_t d, d10, m, m10, y, y10;
                fflush(&uart_file);
                scanf("%10s", buf);
                d = atoi(&buf[0]);
                m = atoi(&buf[3]);
                y = atoi(&buf[8]);
                printf("set date %02d-%02d-20%02d\r\n", d, m, y);
                
                d10 = d / 10;
                d = d % 10;
                m10 = m / 10;
                m = m % 10;
                y10 = y / 10;
                y = y % 10;
                i2cStart();
                i2cSend(0xD0);
                i2cSend(0x04);
                i2cSend((d10 << 4) | d);
                i2cSend((m10 << 4) | m);
                i2cSend((y10 << 4) | y);
                i2cStop();
            } else {  // time
                int8_t h, h10, m, m10, s, s10;
                fflush(&uart_file);
                scanf("%8s", buf);
                h = atoi(&buf[0]);
                m = atoi(&buf[3]);
                s = atoi(&buf[6]);
                printf("set time %02d:%02d:%02d\r\n", h, m, s);
                s10 = s / 10;
                s = s % 10;
                m10 = m / 10;
                m = m % 10;
                h10 = h / 10;
                h = h % 10;
                i2cStart();
                i2cSend(0xD0);
                i2cSend(0x00);
                i2cSend((s10 << 4) | s);
                i2cSend((m10 << 4) | m);
                i2cSend((h10 << 4) | h);
                i2cStop();
            }
        }
    }
}
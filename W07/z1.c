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
        fflush(&uart_file);
        scanf("%16s", &buf[0]);
        if (strcmp(buf, "read") == 0) {
            fflush(&uart_file);
            scanf("%s", buf);

            uint16_t addr;
            addr = strtol(buf, NULL, 16);
            i2cStart();
            i2cSend(0b10100000 | ((addr & 0x10) >> 7));  // write to eeprom
            i2cSend(addr & 0xff);
            i2cStart();
            i2cSend(0b10100000 | ((addr & 0x10) >> 7) | 0x01);  // read from eeprom
            uint8_t data = i2cReadNoAck();
            i2cStop();
            printf("<%03x>:  %02x\r\n", addr, data);
        } else {
            fflush(&uart_file);
            scanf("%s", buf);
            uint16_t addr;
            addr = strtol(buf, NULL, 16);
            fflush(&uart_file);
            scanf("%s", buf);
            uint8_t val;
            val = strtol(buf, NULL, 16);

            i2cStart();
            i2cSend(0b10100000 | ((addr & 0x10) >> 7));  // write to eeprom
            i2cSend(addr & 0xff);
            i2cSend(val);
            i2cStop();

            printf("<%03x>:= %02x\r\n", addr, val);
        }
    }
}
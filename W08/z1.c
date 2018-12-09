#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <inttypes.h>
#include <stdio.h>
#include <util/delay.h>
#include "hd44780.h"

int hd44780_transmit(char data, FILE *stream) {
    LCD_WriteData(data);
    return 0;
}

FILE hd44780_file;

#define BAUD 9600
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD)-1)

// inicjalizacja UART
void uart_init() {
    // interrupts should be disabled during initialization
    UBRR0 = UBRR_VALUE;
    UCSR0A = 0;
    UCSR0B = _BV(RXEN0) | _BV(RXCIE0);
    UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

ISR(USART_RX_vect) {  // receive byte
    static uint8_t cnt = 0;
    static char buf[16];
    buf[16] = '\0'; // guard

    buf[cnt] = UDR0;
    if (buf[cnt] == '\r') {
        buf[cnt] = '\0';
        LCD_Clear();
        LCD_GoTo(0, 0);
        printf("%s", buf);
        LCD_GoTo(0, 1);
        cnt=-1;
    } else
        LCD_WriteData(buf[cnt]);

    if (++cnt == 16) {
        LCD_GoTo(0, 1);
        cnt = 0;
    }
    buf[cnt] = '\0';
    //LCD_WriteCommand(0x14);
}

int main() {
    // interrupt-driven UART setup
    uart_init();
    sei();

    // skonfiguruj wyświetlacz
    LCD_Initialize();
    LCD_Clear();
    // skonfiguruj strumienie wyjściowe
    fdev_setup_stream(&hd44780_file, hd44780_transmit, NULL, _FDEV_SETUP_WRITE);
    stdout = stderr = &hd44780_file;

    LCD_WriteCommand(0x0F);  // cursor blink
    LCD_GoTo(0, 1);
    set_sleep_mode(SLEEP_MODE_IDLE);
    while (1)
        sleep_mode();
}

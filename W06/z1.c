#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <inttypes.h>
#include <util/delay.h>

#define BAUD 9600
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD)-1)

// inicjalizacja UART
void uart_init() {
    // interrupts should be disabled during initialization
    UBRR0 = UBRR_VALUE;
    UCSR0A = 0;
    UCSR0B = _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0);
    UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

ISR(USART_RX_vect) {  // receive byte
    static volatile char byte;
    byte = UDR0;
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = byte;
}

int main() {
    uart_init();
    sei();
    

    set_sleep_mode(SLEEP_MODE_IDLE);
    while (1)
        sleep_mode();
}
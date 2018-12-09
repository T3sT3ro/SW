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

void display_progress(uint8_t value){
    value = value > 80 ? 80 : value;
    LCD_Clear();
    LCD_GoTo(0, 0);
    for(uint8_t i=0; i<value/5; i++)
        LCD_WriteData(0xff);

    value = value % 5;
    if(value > 0) 
        LCD_WriteData(value);
}

int main() {
    // skonfiguruj wyświetlacz
    LCD_Initialize();
    LCD_Clear();
    // skonfiguruj strumienie wyjściowe
    fdev_setup_stream(&hd44780_file, hd44780_transmit, NULL, _FDEV_SETUP_WRITE);
    stdout = stderr = &hd44780_file;

    for (uint8_t character = 0, mask = 0x10; character<5; character++, mask |= mask>> 1)
        for (uint8_t line = 0; line < 8; line++) {
            LCD_WriteCommand(HD44780_CGRAM_SET | (character << 3) | (line));
            LCD_WriteData(mask);
        }

    uint8_t value = 0;
    while (1) {
        display_progress(value);
        value = value >= 80 ? 0 : value + 1;
        _delay_ms(50);
    }
}

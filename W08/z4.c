#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <inttypes.h>
#include <stdio.h>
#include <util/delay.h>

#include <avr/pgmspace.h>
#include "./music.h"

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <util/delay.h>

#define PIN PB1
#define PORT_DDR DDRB
#define PORT PORTB
#define PIN_STATE(state) PORT = state ? PORT | _BV(PIN) : PORT & ~_BV(PIN)
#define BPM 210
#define MEASURE_1_32_MS \
    (1000 / BPM * 60 / 32 * 1000)  // ms duration for 1/32 note

void play(const NOTE* note) {
    // to play full note play 32= 1<<5 1/32 notes
    uint8_t repetitions = 1 << pgm_read_byte(&(note->duration));
    uint8_t played = 0;
    // duration in microseconds
    TONE tone = pgm_read_byte(&(note->tone));
    uint8_t octave = pgm_read_byte(&(note->octave));

    uint16_t delay = tone == T_PAUSE
                         ? 0
                         : octave >= 4 ? TONES[tone] >> (octave - 4)
                                       : TONES[tone] << -(octave - 4);

    // play x* 1/32 note at specific octave
    while (played < repetitions) {
        uint16_t ums = 0;  // counts the 1/32 note duration
        while (ums < MEASURE_1_32_MS) {
            if (tone == T_PAUSE) {
                ums++;
                _delay_us(1);
            };
            PIN_STATE(1);
            uint16_t ums_elapsed = 0;
            while (ums_elapsed < delay) {
                _delay_us(1);
                ums_elapsed++;
            }
            ums_elapsed = 0;
            PIN_STATE(0);
            while (ums_elapsed < delay) {
                _delay_us(1);
                ums_elapsed++;
            }
            ums += 2 * delay;
        }
        played++;
    }
}

int main() {
    PORT_DDR |= _BV(PIN);
    PIN_STATE(0);
    int16_t current_note = 0;
    while (1) {
        play(&music[current_note]);
        current_note = (current_note + 1) % (sizeof(music)/sizeof(NOTE));
    }
}
/******************************************************************************
 * Header file inclusions.
 ******************************************************************************/
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#include <avr/io.h>

#include <stdio.h>
#include "uart.h"

/******************************************************************************
 * Private macro definitions.
 ******************************************************************************/

#define mainLED_TASK_PRIORITY 1

#define mainSERIAL_TASK_PRIORITY 1

/******************************************************************************
 * Private function prototypes.
 ******************************************************************************/

static void vBlinkLed(void* pvParameters);

static void vSerial(void* pvParameters);

/******************************************************************************
 * Public function definitions.
 ******************************************************************************/

/**************************************************************************/ /**
 * \fn int main(void)
 *
 * \brief Main function.
 *
 * \return
 ******************************************************************************/
int main(void) {
    // Create task.
    xTaskHandle blink_handle;
    xTaskHandle serial_handle;

    xTaskCreate(
        vBlinkLed,
        "blink",
        configMINIMAL_STACK_SIZE,
        NULL,
        mainLED_TASK_PRIORITY,
        &blink_handle);

    xTaskCreate(
        vSerial,
        "serial",
        configMINIMAL_STACK_SIZE,
        NULL,
        mainSERIAL_TASK_PRIORITY,
        &serial_handle);

    // Start scheduler.
    vTaskStartScheduler();

    return 0;
}

/**************************************************************************/ /**
 * \fn static vApplicationIdleHook(void)
 *
 * \brief
 ******************************************************************************/
void vApplicationIdleHook(void) {
}

void vApplicationTickHook(void){}

/******************************************************************************
 * Private function definitions.
 ******************************************************************************/

/**************************************************************************/ /**
 * \fn static void vBlinkLed(void* pvParameters)
 *
 * \brief
 *
 * \param[in]   pvParameters
 ******************************************************************************/
#define LED_STATE(state) PORTB = state ? PORTB | _BV(PB5) : PORTB & ~_BV(PB5);
uint8_t buffer[100];
static void vBlinkLed(void* pvParameters) {
    DDRB |= _BV(PB5);   // D13 as output
    int8_t cIt = 0;
    while (1) {
        LED_STATE(buffer[cIt]);
        buffer[cIt] = !(PINB & _BV(PB1));
        vTaskDelay(pdMS_TO_TICKS(10));
        cIt = cIt > 99 ? 0 : cIt+1; // cyclic iter ++
    }
}

/**************************************************************************/ /**
 * \fn static void vSerial(void* pvParameters)
 *
 * \brief
 *
 * \param[in]   pvParameters
 ******************************************************************************/
static void vSerial(void* pvParameters) {
    UCSR0B &= ~_BV(RXEN0) & ~_BV(TXEN0);  // TX, RX write setup
    DDRD = 0xFF;                          // set all PD to write
    PORTD = 0x80;
    register uint8_t rsh;
    while (1) {
        rsh = PORTD == 0x80 ? 0xFF : PORTD == 0x01 ? 0x00 : rsh;
        vTaskDelay(pdMS_TO_TICKS(60));
        PORTD = ~rsh & (PORTD << 1) | rsh &(PORTD >> 1);
    }
}

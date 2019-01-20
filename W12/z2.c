/******************************************************************************
 * Header file inclusions.
 ******************************************************************************/
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#include <avr/io.h>
#include <stdio.h>
#include "queue.h"
#include "uart.h"

/******************************************************************************
 * Private macro definitions.
 ******************************************************************************/

#define mainLED_TASK_PRIORITY 1

#define mainSERIAL_TASK_PRIORITY 1

/******************************************************************************
 * Private function prototypes.
 ******************************************************************************/

static void vUartReceive(void* pvParameters);

static void vLedBlink(void* pvParameters);

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
    uart_init();
    stdout = stdin = &uart_file;

    // Create task.
    xTaskHandle blink_handle;
    xTaskHandle serial_handle;
    xQueueHandle Q = xQueueCreate(5, sizeof(uint16_t));

    xTaskCreate(
        vUartReceive,
        "blink",
        configMINIMAL_STACK_SIZE,
        (void*)Q,
        mainLED_TASK_PRIORITY,
        &blink_handle);

    xTaskCreate(
        vLedBlink,
        "serial",
        configMINIMAL_STACK_SIZE,
        (void*)Q,
        mainSERIAL_TASK_PRIORITY,
        &serial_handle);

    // Start scheduler.
    vTaskStartScheduler();

    return 0;
}

/**************************************************************************/
void vApplicationIdleHook(void) {}

static void vLedBlink(void* pvParameters) {
    xQueueHandle Q = (xQueueHandle)pvParameters;

    DDRB |= _BV(PB5);  // D5 as output
    uint16_t val = 0;
    while (1) {
        if (xQueueReceive(Q, &val, 0) == pdTRUE) {
            PORTB |= _BV(PB5);
            vTaskDelay(val / portTICK_PERIOD_MS);
            PORTB &= ~_BV(PB5);
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
    }
}

static void vUartReceive(void* pvParameters) {
    xQueueHandle Q = (xQueueHandle)pvParameters;

    char c = 0;
    uint16_t l = 0;
    while (1) {
        l = 0;
        c = '0';
        do {
            l = l * 10 + c - '0';
            c = getchar();
        } while (c >= '0' && c <= '9');
        printf("%d\r\n", l);
        if (xQueueSend(Q, &l, 10) == pdFALSE)
            puts("FAIL\r");
        taskYIELD();
    }
}

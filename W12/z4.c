/******************************************************************************
 * Header file inclusions.
 ******************************************************************************/
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "semphr.h"
#include "task.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include "uart.h"

/******************************************************************************
 * Private macro definitions.
 ******************************************************************************/

#define mainADC_TASK_PRIORITY 1

/******************************************************************************
 * Private function prototypes.
 ******************************************************************************/

static void vFunction(void* pvParameters);

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

xTaskHandle blocked = NULL;
SemaphoreHandle_t mutex, tokenMutex, adcSem;  // adcSem as semaphore(mutex without an owner)
uint8_t token = 1, goldenToken = 1;           // token value=0 means the process haven't pulled the token yet

ISR(ADC_vect) { xSemaphoreGiveFromISR(adcSem, NULL); }  // post adcSem for blocked task to take

int main(void) {
    mutex = xSemaphoreCreateMutex();
    tokenMutex = xSemaphoreCreateMutex();
    adcSem = xSemaphoreCreateMutex();
    xSemaphoreTake(adcSem, portMAX_DELAY);  // lower adcSem to value=0, so that ADC will block

    uart_init();
    stdout = stdin = &uart_file;

    ADCSRA |= _BV(ADEN) | _BV(ADIE) | _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2);
    DIDR0 |= _BV(ADC0D) | _BV(ADC1D) | _BV(ADC2D);
    ADMUX |= _BV(REFS0);

    sei();

    // Create task.
    xTaskHandle handlers[3];

    for (uint8_t i = 0; i < 3; i++) {
        xTaskCreate(
            vFunction,
            i == 0 ? "h1" : i == 2 ? "h2" : "h3",
            configMINIMAL_STACK_SIZE,
            (void*)(i + 1),
            mainADC_TASK_PRIORITY,
            &handlers[i]);
    }

    // Start scheduler.
    vTaskStartScheduler();

    return 0;
}

/**************************************************************************/
void vApplicationIdleHook(void) {}

uint16_t readADC(uint8_t MUX) {
    uint16_t adcVal;
    uint8_t myToken = 0;

    xSemaphoreTake(tokenMutex, portMAX_DELAY);
    myToken = token++;  // token protected by `tokenMutex`
    xSemaphoreGive(tokenMutex);

tryAgain:
    xSemaphoreTake(mutex, portMAX_DELAY);  // round robin prevents starvation
    if (myToken != goldenToken) {          // goldenToken is secured by `mutex`
        xSemaphoreGive(mutex);
        goto tryAgain;
    }

    blocked = xTaskGetCurrentTaskHandle();

    ADMUX = _BV(REFS0) | _BV(MUX);  //enable MUX
    ADCSRA |= _BV(ADSC);

    xSemaphoreTake(adcSem, portMAX_DELAY);  // block untill ISR posts on adcSem, resume at conversion end
    adcVal = ADC;

    ++goldenToken;  // next one can enter
    xSemaphoreGive(mutex);
    return adcVal;
}

static void vFunction(void* pvParameters) {
    uint8_t ID = (uint8_t)pvParameters;
    uint16_t delay = (uint16_t)ID * 256 / portTICK_PERIOD_MS;
    uint16_t val = 0;
    for (;;) {
        vTaskDelay(delay);
        val = readADC(ID == 1 ? MUX0 : ID == 2 ? MUX1 : MUX2);
        if(ID>1)putchar('\t');
        if(ID>2)putchar('\t');
        putchar('0' + ID);
        putchar(':');
        putchar('0' + val / 1000);
        putchar('0' + (val / 100) % 10);
        putchar('0' + (val / 10) % 10);
        putchar('0' + val % 10);
        putchar('\r');
        putchar('\n');
    }
}

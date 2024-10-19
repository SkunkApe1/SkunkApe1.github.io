/*
 * Copyright (c) 2015-2020, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== gpiointerrupt.c ========
 */

//Graham Swenson
//SNHU - CS350
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>  // For malloc and free

/* TI Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/UART2.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/Timer.h>

/* Driver configuration */
#include "ti_drivers_config.h"

/* Constants for intervals in milliseconds */
#define BUTTON_CHECK_INTERVAL_MS 200
#define TEMP_CHECK_INTERVAL_MS   500
#define REPORT_INTERVAL_MS       1000
#define TIMER_PERIOD_MS           100  // Timer callback every 100ms

/* Sensor Configuration */
typedef struct {
    uint8_t address;
    uint8_t resultReg;
    const char* id;
} Sensor;

/* Use dynamic memory allocation for sensors */
Sensor* sensors;
size_t sensor_count = 3;  // Number of sensors

/* UART Buffer */
#define OUTPUT_BUFFER_SIZE 64
char output[OUTPUT_BUFFER_SIZE];

/* UART, I2C, and Timer Handles */
UART2_Handle uart2;
I2C_Handle i2c;
Timer_Handle timer0;
I2C_Transaction i2cTransaction;

/* Buffers for I2C Communication */
uint8_t txBuffer[1];
uint8_t rxBuffer[2];

/* Variables for Temperature Control */
volatile int heat = 0;
volatile int setTemp = 24;          // Initial set temperature
volatile int16_t currentTemp = 0;

/* Button State Variables */
volatile unsigned int buttonUp = 0;
volatile unsigned int buttonDown = 0;

/* Task Structure */
typedef struct {
    int state;
    unsigned long period;
    unsigned long elapsedTime;
    int (*TickT)(int);
} Task;

#define TASKS_NUM 2
Task tasks[TASKS_NUM];

/* Task Timing */
const unsigned long TASK_TIME_MS = 100;

/* Timer Flag */
volatile unsigned TimerFlag = 0;

/* Function Prototypes */
int TickT_CheckButtons(int state);
int TickT_SetHeat(int state);
int16_t readTemp(void);
void timerCallback(Timer_Handle myHandle, int_fast16_t status);
void initTimer(void);
void initUART2(void);
void initI2C(void);
void gpioButtonFxn0(uint_least8_t index);
void gpioButtonFxn1(uint_least8_t index);

/* Stub for DISPLAY macro (Replace with actual implementation) */
#define DISPLAY(x) do { UART2_write(uart2, x, strlen(x)); } while(0)

/* UART2 Write Wrapper */
int uart2_write(UART2_Handle handle, const char* buf, size_t count) {
    return UART2_write(handle, buf, count);
}

/* Temperature Reading Function */
int16_t readTemp(void) {
    memset(&i2cTransaction, 0, sizeof(i2cTransaction));
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 2;

    int16_t temperature = 0;

    /* Iterate through sensors to find a valid one */
    for (size_t i = 0; i < sensor_count; ++i) {
        i2cTransaction.slaveAddress = sensors[i].address;
        txBuffer[0] = sensors[i].resultReg;
        i2cTransaction.writeBuf = txBuffer;
        i2cTransaction.writeCount = 1;
        i2cTransaction.readBuf = rxBuffer;
        i2cTransaction.readCount = 2;

        if (I2C_transfer(i2c, &i2cTransaction)) {
            /* Combine the two bytes from the sensor */
            temperature = (rxBuffer[0] << 8) | rxBuffer[1];
            temperature *= 0.0078125; // Convert to temperature

            /* Handle negative temperatures if necessary */
            if (rxBuffer[0] & 0x80) {
                temperature |= 0xF000;
            }
            return temperature;
        }
    }

    /* If no sensor is found or transfer fails */
    DISPLAY((char*)"Error reading temperature sensors.\n\r");
    return 0;
}

/* Timer Callback Function */
void timerCallback(Timer_Handle myHandle, int_fast16_t status) {
    /* Update each task */
    for (int i = 0; i < TASKS_NUM; ++i) {
        if (tasks[i].elapsedTime >= tasks[i].period) {
            tasks[i].state = tasks[i].TickT(tasks[i].state);
            tasks[i].elapsedTime = 0;
        }
        tasks[i].elapsedTime += TASK_TIME_MS;
    }
    TimerFlag = 1;
}

/* Initialize Timer */
void initTimer(void) {
    Timer_Params params;
    Timer_init();
    Timer_Params_init(&params);
    params.period = TIMER_PERIOD_MS * 1000; // Convert ms to us
    params.periodUnits = Timer_PERIOD_US;
    params.timerMode = Timer_CONTINUOUS_CALLBACK;
    params.timerCallback = timerCallback;

    timer0 = Timer_open(CONFIG_TIMER_0, &params);
    if (timer0 == NULL) {
        DISPLAY((char*)"Failed to open Timer.\n\r");
        while (1);
    }

    if (Timer_start(timer0) == Timer_STATUS_ERROR) {
        DISPLAY((char*)"Failed to start Timer.\n\r");
        while (1);
    }
}

/* Initialize UART2 */
void initUART2(void) {
    UART2_Params uart2Params;
    UART2_init();
    UART2_Params_init(&uart2Params);
    uart2Params.writeMode = UART2_MODE_BLOCKING;
    uart2Params.readMode = UART2_MODE_BLOCKING;
    uart2Params.baudRate = 115200;

    uart2 = UART2_open(CONFIG_UART2_0, &uart2Params);
    if (uart2 == NULL) {
        /* UART2_open failed */
        while (1);
    }
}

/* Initialize I2C and Detect Sensors */
void initI2C(void) {
    int found = 0;
    I2C_Params i2cParams;

    DISPLAY((char*)"Initializing I2C Driver...\n\r");
    I2C_init();

    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;
    i2cParams.transferMode = I2C_MODE_BLOCKING;

    i2c = I2C_open(CONFIG_I2C_0, &i2cParams);
    if (i2c == NULL) {
        DISPLAY((char*)"Failed to open I2C.\n\r");
        while (1);
    }
    DISPLAY((char*)"I2C Initialization PASS\n\r");

    /* Prepare I2C Transaction */
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 2;

    /* Scan for sensors */
    for (size_t i = 0; i < sensor_count; ++i) {
        i2cTransaction.slaveAddress = sensors[i].address;
        txBuffer[0] = sensors[i].resultReg;

        if (I2C_transfer(i2c, &i2cTransaction)) {
            snprintf(output, OUTPUT_BUFFER_SIZE, "Sensor %s found at address 0x%02X\n\r", sensors[i].id, sensors[i].address);
            DISPLAY(output);
            found = 1;
            break;
        }
        else {
            snprintf(output, OUTPUT_BUFFER_SIZE, "Sensor %s NOT found at address 0x%02X\n\r", sensors[i].id, sensors[i].address);
            DISPLAY(output);
        }
    }

    if (!found) {
        DISPLAY((char*)"No temperature sensors detected.\n\r");
    }
}

/* GPIO Button Callback for Button Up */
void gpioButtonFxn0(uint_least8_t index) {
    buttonUp++;
}

/* GPIO Button Callback for Button Down */
void gpioButtonFxn1(uint_least8_t index) {
    buttonDown++;
}

/* State Machine for Checking Buttons */
int TickT_CheckButtons(int state) {
    switch (state) {
    case CheckButtons_Start:
        state = CheckButtons_Idle;
        break;
    case CheckButtons_Idle:
        /* Remain in Idle */
        break;
    default:
        state = CheckButtons_Start;
        break;
    }

    switch (state) {
    case CheckButtons_Idle:
        if (buttonUp > 0 && setTemp < 99) {
            setTemp += 1;
            buttonUp = 0;
        }
        if (buttonDown > 0 && setTemp > 0) {
            setTemp -= 1;
            buttonDown = 0;
        }
        break;
    }
    return state;
}

/* State Machine for Setting Heat */
int TickT_SetHeat(int state) {
    switch (state) {
    case SetHeat_Start:
        state = SetHeat_Idle;
        break;
    case SetHeat_Idle:
        currentTemp = readTemp();
        snprintf(output, OUTPUT_BUFFER_SIZE, "Set: %02d, Temp: %02d\n\r", setTemp, currentTemp);
        DISPLAY(output);

        if (currentTemp < setTemp) {
            heat = 1;
        }
        else {
            heat = 0;
        }
        break;
    default:
        state = SetHeat_Start;
        break;
    }

    switch (state) {
    case SetHeat_Idle:
        snprintf(output, OUTPUT_BUFFER_SIZE, "HEAT %s\n\r", heat ? "ON" : "OFF");
        DISPLAY(output);
        break;
    }
    return state;
}

/* Main Entry Point */
int main(void) {
    /* Initialize Drivers */
    GPIO_init();
    Power_init();

    /* Initialize Button Callbacks */
    GPIO_setCallback(CONFIG_GPIO_BUTTON_0, gpioButtonFxn0);
    GPIO_enableInt(CONFIG_GPIO_BUTTON_0);
    GPIO_setCallback(CONFIG_GPIO_BUTTON_1, gpioButtonFxn1);
    GPIO_enableInt(CONFIG_GPIO_BUTTON_1);

    /* Allocate dynamic memory for the sensors array */
    sensors = (Sensor*)malloc(sensor_count * sizeof(Sensor));
    if (sensors == NULL) {
        DISPLAY((char*)"Memory allocation for sensors failed.\n\r");
        while (1);
    }

    /* Define sensor details */
    sensors[0] = (Sensor){ .address = 0x48, .resultReg = 0x00, .id = "11X" };
    sensors[1] = (Sensor){ .address = 0x49, .resultReg = 0x00, .id = "116" };
    sensors[2] = (Sensor){ .address = 0x41, .resultReg = 0x00, .id = "006" };

    /* Initialize UART, I2C, and Timer */
    initUART2();
    initI2C();
    initTimer();

    /* Setup Tasks */
    tasks[0].state = CheckButtons_Start;
    tasks[0].period = BUTTON_CHECK_INTERVAL_MS;
    tasks[0].elapsedTime = tasks[0].period;
    tasks[0].TickT = &TickT_CheckButtons;

    tasks[1].state = SetHeat_Start;
    tasks[1].period = TEMP_CHECK_INTERVAL_MS;
    tasks[1].elapsedTime = tasks[1].period;
    tasks[1].TickT = &TickT_SetHeat;

    /* Main Loop */
    while (1) {
        while (!TimerFlag); // Wait for Timer flag
        TimerFlag = 0;
    }

    /* Free dynamically allocated memory */
    free(sensors);

    return 0;
}

/* END CODE */

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


/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/UART2.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/Timer.h>

/* Driver configuration */
#include "ti_drivers_config.h"

#define BUTTON_CHECK_INTERVAL 200
#define TEMP_CHECK_INTERVAL 500
#define REPORT_INTERVAL 1000

int uart2_write(UART2_Handle handle, const char *buf, size_t count){
    return 0;
}

static const struct {
    uint8_t address;
    uint8_t resultReg;
    char *id;
} sensors[3] = {
              {0x48, 0x0000, "11X"},
              {0x49, 0x0000, "116"},
              {0x41, 0x0001, "006"}
};

uint8_t txBuffer[1];
uint8_t rxBuffer[2];

char output[64];
int SendBytes;

Timer_Handle timer0;
I2C_Handle i2c;
I2C_Transaction i2cTransaction;
UART2_Handle uart2;

int heat;
int seconds;
int setTemp;
int16_t currentTemp;

typedef struct task{ //Structure for defining task
    int state;
    unsigned long period;
    unsigned long elapsedTime;
    int (*TickT)(int);
}task;

task tasks[2];

const unsigned char tasksNum = 2;
const unsigned long taskTime = 100;
const unsigned long heatTime = 500;
const unsigned long buttonTime = 200;
const unsigned long tasksTime = 100;

volatile unsigned TimerFlag = 0;
int timerCount = 0;
int buttonUp = 0;
int buttonDown = 0;

enum CheckButton_States {CheckButtons_Start, CheckButtons_1}; //Button checking states
int TickT_CheckButtons(int state);
enum SetHeat_States {SetHeat_Start, SetHeat_1}; //Heat Setting States
int TickT_SetHeat(int state);

int16_t readTemp(void){ //Function that will read temp from sensor
    int g;
    int16_t temperature = 0;
    i2cTransaction.readCount = 2;
    if (I2C_transfer(i2c, &i2cTransaction)){
        temperature = (rxBuffer[0] << 8) | (rxBuffer[1]);
        temperature *= 0.0078125;

        if (rxBuffer[0] & 0x80){
            temperature |= 0xF000;
        }
    }
    else{
        DISPLAY(snprintf(output, 64, "Error with reading temp sensor (%d)\n\r", i2cTransaction.status));
                DISPLAY(snprintf(output, 64, "Please unplug then plug USB back in.\n\r"));
    }
    return temperature;
}
void timerCallback(Timer_Handle myHandle, int_fast16_t status){ //Callback for timer
    unsigned char i;
    for (i=0; i<tasksNum; ++i){
        if (tasks[i].elapsedTime >= tasks[i].period){
            tasks[i].state = tasks[i].TickT(tasks[i].state);
            tasks[i].elapsedTime=0;
        }
        tasks[i].elapsedTime += tasksTime;
    }
    TimerFlag = 1;
}

void initTimer(void){ //function that initializes the time
    Timer_Params params;
    Timer_init();
    Timer_Params_init(&params);
    params.period = 10000000;
    params.periodUnits = Timer_PERIOD_US;
    params.timerMode = Timer_CONTINUOUS_CALLBACK;
    params.timerCallback = timerCallback;

    timer0 = Timer_open(CONFIG_TIMER_0, &params);

    if(timer0 == NULL){
        while(1){}
    }
    if(Timer_start(timer0) == Timer_STATUS_ERROR){
        while(1){}
    }
}

void inituart2(void){ //function to initialize uart2
    UART2_Params uart2Params;
    UART2_init();
    UART2_Params_init(&uart2Params);
    uart2Params.writeMode = UART2_DATA_BINARY;
    uart2Params.readMode = UART2_DATA_BINARY;
    uart2Params.readReturnMode = UART2_RETURN_FULL;
    uart2Params.baudRate = 115200;

    uart2 = uart2_open(CONFIG_UART2_0, &uart2Params);

    if(uart2 == NULL){
        while(1);
    }
}

void initI2C(void){ //Initialize I2C and begin detecting sensors
    int8_t i, found;
    I2C_Params i2cParams;
    DISPLAY(snprintf(output, 64, "I2C Driver Initialize"));

    I2C_init();

    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;
    i2cParams.transferMode = I2C_MODE_BLOCKING;

    i2c = I2C_open(CONFIG_I2C_0, &i2cParams);

    if (i2c == NULL){
        DISPLAY(snprintf(output, 64, "FAIL\n\r", ""));
                while(1);
    }
    DISPLAY("PASS\n\r");

    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 0;
    found = false;

    for (i=0; i<3; ++i){
        i2cTransaction.slaveAddress = sensors[i].address;
        txBuffer[0] = sensors[i].resultReg;

        if (I2C_transfer(i2c, &i2cTransaction)){
            DISPLAY(snprint(output, 64, "FOUND\n\r", ""));
            found = true;
            break;
        }
        else{
            DISPLAY("NO\n\r");
        }

    }
    if(found){
        DISPLAY(snprintf(output, 64, "DETECTED TMP5s I2C address:%x\n\r", sensors[i].id, i2cTransaction.slaveAddress));
    }
    else{
        DISPLAY(snprintf(output, 64, "TEMP SENSOR NOT FOUND\n\r"));
    }
}

void gpioButtonFxn0(uint_least8_t index){ //Callback for pressing button
    buttonUp +=1;

}

int TickT_SetHeat(int state){ //State machine for setting heat
    switch(state){
    case SetHeat_Start:
        state = SetHeat_1;
        break;
    case SetHeat_1:
        state = SetHeat_1;
        break;
    default:
        state = SetHeat_1;
        break;
    }

    switch(state){
    case SetHeat_1:
        if (setTemp <= currentTemp){
            heat = 0;
            GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);
        }
        else{
            heat = 1;
            GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_OFF);
        }
        break;
    }
    return state;
}

int TickT_CheckButtons(int state){ //State Machine for checking buttons
    switch(state){
    case CheckButtons_Start:
        state = CheckButtons_1;
        break;
    case CheckButtons_1:
        state = CheckButtons_1;
        break;
    default:
        state = CheckButtons_Start;
        break;
    }
    switch(state){
    case CheckButtons_1:
        if(buttonUp > 0 && setTemp < 99){
            setTemp += 1;
        }
        if(buttonDown > 0 && setTemp > 0){
            setTemp -= 1;

        }
            buttonUp = 0;
            buttonDown = 0;
            break;
    }
    return state;
}

void gpioButtonFxn1(uint_least8_t index){
    buttonDown += 1;
}

/* MAIN */

void *mainThread(void *arg0){
    GPIO_init();
    inituart2();
    initI2C();
    initTimer();

    GPIO_setConfig(CONFIG_GPIO_LED_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_BUTTON_0, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING);

    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);

    GPIO_setCallback(CONFIG_GPIO_BUTTON_0, gpioButtonFxn0);

    if (CONFIG_GPIO_BUTTON_0 != CONFIG_GPIO_BUTTON_1){
        GPIO_setConfig(CONFIG_GPIO_BUTTON_1, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING);
        GPIO_setCallback(CONFIG_GPIO_BUTTON_1, gpioButtonFxn1);
        GPIO_enableInt(CONFIG_GPIO_BUTTON_1);
    }

    unsigned long seconds = 0;
    unsigned long lastButtonCheck = 0;
    unsigned long lastTempCheck = 0;
    unsigned char i = 0;
    int buttonState = 0;
    unsigned long lastReport = 0;

        tasks[i].state = CheckButtons_Start;
        tasks[i].period = buttonTime;
        tasks[i].elapsedTime = tasks[i].period;
        tasks[i].TickT = &TickT_CheckButtons;
        i++;
        tasks[i].state = SetHeat_Start;
        tasks[i].period = heatTime;
        tasks[i].elapsedTime = tasks[i].period;
        tasks[i].TickT = &TickT_SetHeat;
        heat = 0;
        seconds = 0;
        setTemp = 24;

        while(1){ //FOREVER LOOP
            currentTemp = readTemp();

            if (TimerFlag && (Timer_getCount(timer0) - lastButtonCheck >= BUTTON_CHECK_INTERVAL)){ //Button Check every 200ms
                lastButtonCheck = Timer_getCount(timer0);

                if (GPIO_read(CONFIG_GPIO_BUTTON_0) == CONFIG_GPIO_BUTTON_ON){ //checks if button is pressed
                    buttonState = 1;
                }
                else{
                    buttonState = 0;
                }

                if (buttonState){ //adjust temp with button press
                    setTemp++;
                }
                else{
                    setTemp--;
                }

                if (setTemp < 0){
                    setTemp = 0;
                }
                else if (setTemp > 99){
                    setTemp = 99;
                }

                if (TimerFlag && (Timer_getCount(timer0) - lastTempCheck >= TEMP_CHECK_INTERVAL)){ //Reading the temp every 500ms
                    lastTempCheck = Timer_getCount(timer0);
                    currentTemp = rand() % 100;
                }

                if (currentTemp> setTemp){ //LED's based off temp
                    heat = 0;
                    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);
                }
                else{
                    heat = 1;
                    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_OFF);
                }

                if (TimerFlag && (Timer_getCount(timer0) - lastReport >= REPORT_INTERVAL)){
                    lastReport = Timer_getCount(timer0);

                    unsigned char heatStatus = (heat == 0) ? '0' : '1';
                    unsigned char outputBuffer[64];
                    DISPLAY(snpringf(outputBuffer, 64, "<%02d, %02d, %c, %04d>\n\r", currentTemp, setTemp, heatStatus, seconds));

                    uart2_write(uart2, outputBuffer, strlen(outputBuffer));
                }
                while (!TimerFlag) {}
                TimerFlag = 0;
                seconds++;
            }
           return (NULL);
        }

}


/* END CODE */

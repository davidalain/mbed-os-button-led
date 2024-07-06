/*
 * Copyright (c) 2017-2020 Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include <chrono>

using namespace std::chrono;

/*
    Estado inicial: LED desligado

    Em 2s:
        Pressionar botão 1x:            Acender o LED
        Pressionar botão 2x:            Piscar o LED com ciclo de 1s
        Pressionar botão 3x:            Piscar o LED com ciclo de 500ms
        Pressionar e segurar o botão:   Desligar o LED
*/

#define BT_PRESSED  0
#define BT_RELEASED 1

#define LED_ON      0
#define LED_OFF  	1

typedef enum LED_State_ {
    LED_STATE_OFF,
    LED_STATE_ON,
    LED_STATE_BLINK_1S,
    LED_STATE_BLINK_500MS,
} LEDState;

/*
*    Situação do botão nos últimos 2s
*/
typedef enum Button_State_ {
    BUTTON_STATE_NOT_PRESSED,
    BUTTON_STATE_PRESSED_1X,
    BUTTON_STATE_PRESSED_2X,
    BUTTON_STATE_PRESSED_3X,
    BUTTON_STATE_PRESSED_HOLD,
} ButtonState;

Thread threadLed;
Thread threadButton;

DigitalIn button(BUTTON1);
DigitalOut led(LED1);

Timer timerButton;
Timer timerLed;

LEDState currentLedStateMachine = LED_STATE_OFF;
ButtonState currentButtonStateMachine = BUTTON_STATE_NOT_PRESSED;

void led_thread(){

    while(true){

        if(currentLedStateMachine == LED_STATE_OFF){
            led = LED_OFF;
        }else if(currentLedStateMachine == LED_STATE_ON){
            led = LED_ON;
        }else if(currentLedStateMachine == LED_STATE_BLINK_1S){
            if(timerLed.elapsed_time() >= 1s){
                led = !led;
                timerLed.reset();
            }
        }else if(currentLedStateMachine == LED_STATE_BLINK_500MS){
            if(timerLed.elapsed_time() >= 500ms){
                led = !led;
                timerLed.reset();
            }
        }

    }

}

void button_thread(){

    int buttonLast = button.read();
    int buttonCurrent = button.read();

    while(true){

        ThisThread::sleep_for(50ms);

        buttonLast = buttonCurrent;
        buttonCurrent = button.read();

        if(		(currentButtonStateMachine == BUTTON_STATE_NOT_PRESSED) 
            && 	(buttonLast == BT_PRESSED) 
            &&	(buttonCurrent == BT_RELEASED))
        {
            
            timerButton.reset();
            timerButton.start();
            currentButtonStateMachine = BUTTON_STATE_PRESSED_1X;
            
        }
        else
        if(     (currentButtonStateMachine == BUTTON_STATE_PRESSED_1X) 
            && 	(buttonLast == BT_PRESSED) 
            &&	(buttonCurrent == BT_RELEASED))
        {
            
            currentButtonStateMachine = BUTTON_STATE_PRESSED_2X;
            
        }
        else
        if(     (currentButtonStateMachine == BUTTON_STATE_PRESSED_2X) 
            && 	(buttonLast == BT_PRESSED) 
            &&	(buttonCurrent == BT_RELEASED))
        {
            
            currentButtonStateMachine = BUTTON_STATE_PRESSED_3X;
            
        }

        //Após se passarem 2 segundos:
        //Verifica em qual estado da máquina do botão está e
        //  muda o estado da máquina de estados do LED
        if(timerButton.elapsed_time() >= 2s){
            
            timerButton.stop();
            timerButton.reset();
            
            if(currentButtonStateMachine == BUTTON_STATE_NOT_PRESSED){

                currentLedStateMachine = LED_STATE_OFF;
                timerLed.stop();

            }else if(currentButtonStateMachine == BUTTON_STATE_PRESSED_1X){
                
                currentLedStateMachine = LED_STATE_ON;
                timerLed.stop();
                
            }else if(currentButtonStateMachine == BUTTON_STATE_PRESSED_2X){
                
                currentLedStateMachine = LED_STATE_BLINK_1S;
                timerLed.reset();
                timerLed.start();
                
            }else if(currentButtonStateMachine == BUTTON_STATE_PRESSED_3X){
                
                currentLedStateMachine = LED_STATE_BLINK_500MS;
                timerLed.reset();
                timerLed.start();
                
            }

            //Volta pra o estado da máquina de estados de botão não pressionado
            currentButtonStateMachine = BUTTON_STATE_NOT_PRESSED;
            
        }

    }

}

int main()
{
    threadLed.start(callback(led_thread));
    threadButton.start(callback(button_thread));
    
    while(true){
        ThisThread::sleep_for(10s);
    }
    
}

#include <stdbool.h> // booleans, i.e. true and false
#include <stdio.h>   // sprintf() function
#include <stdlib.h>  // srand() and random() functions
#include <string.h>
#include "ece198.h"

#include "LiquidCrystal.h"

// float get_distance();
void CHECK_PIN();
void LED();

bool answer;
size_t count;

//definitions to move to top 
int Ticks;
const float speed_of_sound = 0.0343/2;
int distance = 6;
char buff[100];
int get_distance(void);
int edge_calculator(TIM_HandleTypeDef *htim);

void SysTick_Handler(void);


//the following code is just to test the LED and CHECK_PIN, if you change the values the corresponding light will turn on!
int player_array[4];
int correct_array[4];

int main(void)
{
   
    HAL_Init(); // initialize the Hardware Abstraction Layer

    // Peripherals (including GPIOs) are disabled by default to save power, so we
    // use the Reset and Clock Control registers to enable the GPIO peripherals that we're using.

    __HAL_RCC_GPIOA_CLK_ENABLE(); // enable port A (for the on-board LED, for example)
    __HAL_RCC_GPIOB_CLK_ENABLE(); // enable port B (for the rotary encoder inputs, for example)
    __HAL_RCC_GPIOC_CLK_ENABLE(); // enable port C (for the on-board blue pushbutton, for example)

    // initialize the pins to be input, output, alternate function, etc...

    InitializePin(GPIOA, GPIO_PIN_5, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, 0);  // on-board LED

    // note: the on-board pushbutton is fine with the default values (no internal pull-up resistor
    // is required, since there's one on the board)

    // set up for serial communication to the host computer
    // (anything we write to the serial port will appear in the terminal (i.e. serial monitor) in VSCode)

    SerialSetup(9600);

    InitializePin(GPIOC, GPIO_PIN_0, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, 0);
    InitializePin(GPIOC, GPIO_PIN_1, GPIO_MODE_INPUT, GPIO_NOPULL, 0);
    
   
    while(true){

        sprintf(buff, "distance (cm) =  %d\n", get_distance());
        SerialPuts(buff);

        HAL_Delay(1000);
    }
   
return 0;
}

   
    // as mentioned above, only one of the following code sections will be used
    // (depending on which of the #define statements at the top of this file has been uncommented)
/*
Program Description: 
- When the game starts, RAND_QUEST runs to generate a random question which will be output into a LCD display: 
Questions generated are multiple choices with A, B, C, D as answers which correspond to the PIN numbers 1, 2, 3, 4. 
- The program will then repeatedly check the DISTANCE_1, DISTANCE_2, DISTANCE_3, DISTANCE_4.
- These four functions will read the distance received from the sensors and return a boolean variable indicating whether there has been a change in distance. 
Change in distance means the tin can is knocked down which determines the player's answer to the question. 
- When a sensor returns a false variable, function ANSWER will store the answer into an array of type int. 
- The Program repeated 3 more times until 4 questions have been answered. At this point, the player will have an array of answers
that will be run by CHECK_PIN. 
- CHECK_PIN stores an array that corresponds to the correct answers to the 4 questions. 
CHECK_PIN returns a boolean variable upon checking if the player's array of answers is equal to the correct array. 
- If CHECK_PIN is true, the pin will be displayed and LED will turn green. If CHECK_PIN is false, a life will be lost and LED will turn red
*/

// This function is called by the HAL once every millisecond
//void SysTick_Handler(void)
//{
    //HAL_IncTick(); // tell HAL that a new tick has happened
    // we can do other things in here too if we need to, but be careful
//}

void LED (void){
    // If CHECKPIN is true, pin will be displayed and LED will turn green. If CHECKPIN is false, a life will be lost and LED will turn red
InitializePin(GPIOA, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, 0);  // initialize color LED output pins
    int colour = 0;
    if(answer == true) {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, colour = 2);  // green (hex 2 == 0010 binary)
        }else {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, colour = 4);  // red   (hex 4 == 0100 binary)
        }
}

void CHECK_PIN (void) {
// Checks if player's array = correct array. (their answers were all right) 

//false until player answers all questions
answer = false;
int correct_count = 0;

//compare every entry of player's array to answer array
for(int i = 0; i < 4; ++i){
    if(player_array[i] == correct_array[i]){
        correct_count++;
    }

    if (correct_count == 4){
        answer = true;
        }
    }
}

//sensors 
uint32_t start_tick = 0;
uint32_t end_tick = 0;
uint8_t icFlag = 0;
uint8_t cindex = 0;
int edge1 = 0;
int edge2 = 0;


int edge_calculator(TIM_HandleTypeDef *htim){
    if(cindex == 0){
        edge1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
        cindex = 1;
    }else if(cindex == 1){
        edge2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
        cindex = 0;
        icFlag = 1;
    }

    return edge1;
    return edge2;
}


int get_distance(void){
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
    HAL_Delay(3);

    //set trigger pin high for 10 uS and then set it low
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
   
    //start input capture timer 
    TIM_HandleTypeDef htim2; //initializes timer 2

    __TIM2_CLK_ENABLE();  // enable timer 2
    
    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);          // initialize one channel (can use others for motors, etc)

    start_tick = HAL_GetTick();
    do {
        if(icFlag) break;
    }while((HAL_GetTick()- start_tick) < 500);{
        icFlag = 0;
        HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_1);
    }

    //calculate distance in cm
    distance = (edge2 - edge1)*speed_of_sound;
    
    return distance;
 }

void SysTick_Handler(void)
{
	HAL_IncTick();
}

/* 
Program Description: The first part of this program outputs the results of its ADC conversion over a serial communications port. The second part of this program makes the buzzer on the breadboard sound and then makes the LED on A1 blink. The tone of the buzzer changes depending on the light falling on the sensing LED.
Date: 22/02/22
Author: Paulina Czarnota
*/


#include <stdint.h>
#include <stm32l031xx.h>
#include "stm32l031lib.h"


void SysTick_Handler(void);
void ADCBegin(void);
uint16_t ADCRead(void);

void ADCBegin()
{
    RCC->APB2ENR |= (1u << 9); // Turn on ADC 
    RCC->IOPENR |= 1; // Enable GPIOA
    pinMode(GPIOA,4,3); // Make GPIOA_4 an analogue input
    ADC1->CR = 0; // Disable ADC before making changes
    ADC1->CR |= (1u << 28); // Turn on the voltage regulator
    ADC1->CR |= (1u << 31); // Start calibration
    while ( (ADC1->CR & (1u << 31)) != 0); // Wait for calibration to complete.
    ADC1->CHSELR = (1 << 4); // Select channel4  
    ADC1->CR |= 1; // Enable the ADC
    
}
uint16_t ADCRead(void)
{
    ADC1->CR |= (1 << 2); // Start a conversion
    while ( (ADC1->CR & (1 << 2)) != 0); // Wait for conversion to complete.
    return (uint16_t)ADC1->DR;
}
int main()
{
    initClock(); // Set the system clock running at 16MHz
    RCC->IOPENR |= 1; // Enable GPIOA
    pinMode(GPIOA,0,1); // Make PORTA Bit 0 an output
    pinMode(GPIOA,1,1); // Make PORTA Bit 1 an output
    
    SysTick->CTRL = 7; // Enable systick counting and interrupts, use core clock
    enable_interrupts();
    SerialBegin();
    ADCBegin();
    pinMode(GPIOA,5,1);
    pinMode(GPIOA,1,1);
    GPIOA->ODR |= (1 << 5);
    while(1)
    {
        printDecimal(ADCRead());
        eputs("\r\n");
        SysTick->LOAD = 10*ADCRead(); // This will make the hz of the buzzer directly proportional to the amount of light read from ADC 
    }
}
void SysTick_Handler(void)
{
    static int ms_counter = 0;
    GPIOA->ODR ^= 1; // Toggle Port A bit 0
    ms_counter ++;   // Another millisecond has passed
    if (ms_counter == 1000) // 1 second passed ?
    {
        ms_counter = 0; // Zero millisecond counter
        GPIOA->ODR ^= 2; // Toggle LED
    }
}

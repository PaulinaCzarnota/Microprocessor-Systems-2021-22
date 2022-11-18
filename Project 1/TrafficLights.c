#include <stdint.h> 
#include <stm32l031xx.h>

#define WAIT 1000000

void pinMode(GPIO_TypeDef *Port, uint32_t BitNumber, uint32_t Mode); 
void enablePullUp(GPIO_TypeDef *Port, uint32_t BitNumber); 
void delay(volatile uint32_t dly); 
void RedOn(void); // turns on the red LED without changing the others
void RedOff(void); // turns off the red LED without changing the others
void YellowOn(void); // turns on the yellow LED without changing the others
void YellowOff(void); // turns off the yellow LED without changing the others
void GreenOn(void); // turns on the green LED without changing the others
void GreenOff(void); // turns off the green LED without changing the others

int main(void)
{ 
	RCC->IOPENR = RCC->IOPENR | (3); // Enable GPIOB and GPIOA
	pinMode(GPIOB,3,1); // Make GPIOB_3 (LD3) an output
	pinMode(GPIOB,4,0); // Make GPIOB_4 (Button) an input
	enablePullUp(GPIOB,4); // enable pull-up for GPIOB_4
	pinMode(GPIOA, 0, 1); // Make GPIOA_0 an output
	pinMode(GPIOA, 1, 1); // Make GPIOA_1 an output
	pinMode(GPIOA, 2, 1); // Make GPIOA_2 an output
	
while(1)
{ 
	if (0 == (GPIOB->IDR & (1u << 4))) // IF statement allowing button press to control LED lights
	{
		// Button pressed 
		GreenOn(); // Calling GreenOn Function
		delay(WAIT); // Wait 
		GreenOff(); // Calling GreenOff Function
		YellowOn(); // Calling YellowOn Function 
		delay(WAIT); // Wait 
		YellowOff(); // Calling YellowOff Function 
		RedOn(); // Calling RedOn Function
		delay(WAIT); // Wait 
		RedOff(); // Calling RedOff Function
		GreenOn(); // Calling GreenOn Function
	} 
	else // if button is not pressed, the green LED stays on indefinitely
	{ 
		GreenOn(); 
	}
}
}
	
void RedOn(void) // Function to turn Red LED on 
{
	GPIOA->ODR = GPIOA->ODR | (1u << 2); 
}

void RedOff(void) // Function to turn Red LED off 
{		
	GPIOA->ODR = GPIOA->ODR & ~(1u << 2); 
}

void YellowOn(void) // Function to turn Yellow LED on 
{ 
	GPIOA->ODR = GPIOA->ODR | (1u << 1); 
}

void YellowOff(void) // Function to turn Yellow LED off 
{ 
	GPIOA->ODR = GPIOA->ODR & ~(1u << 1); 
}

void GreenOn(void) // Function to turn Green LED on 
{ 
	GPIOA->ODR = GPIOA->ODR | (1u << 0); 
}

void GreenOff(void) // Function to turn Green LED off 
{ 
	GPIOA->ODR = GPIOA->ODR & ~(1u << 0); 
}

void enablePullUp(GPIO_TypeDef *Port, uint32_t BitNumber) 
{
	Port->PUPDR = Port->PUPDR & ~(3u << BitNumber*2); // clear pull-up resistor bits 
	Port->PUPDR = Port->PUPDR | (1u << BitNumber*2); // set pull-up bit 
}
void pinMode(GPIO_TypeDef *Port, uint32_t BitNumber, uint32_t Mode) 
{
	// This function writes the given Mode bits to the appropriate location for
	// the given BitNumber in the Port specified. It leaves other bits unchanged
	// Mode values:
	// 0 : digital input 
	// 1 : digital output 
	// 2 : Alternative function 
	// 3 : Analog input
	uint32_t mode_value = Port->MODER; // read current value of Mode register 
	Mode = Mode << (2 * BitNumber);    // There are two Mode bits per port bit so need to shift
																	   // the mask for Mode up to the proper location
	mode_value = mode_value & ~(3u << (BitNumber * 2)); // Clear out old mode bits
	mode_value = mode_value | Mode; // set new bits
	Port->MODER = mode_value; // write back to port mode register
}
void delay(volatile uint32_t dly) 
{ 
	while(dly--); 
}

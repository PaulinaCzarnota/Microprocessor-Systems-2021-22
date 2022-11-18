
/*
Copyright (C) 2014  Frank Duignan

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/* 
Date: 14/03/2022
Modified by: Paulina Czarnota C21365726
*/


#include "realm.h"
#include "stm32l031lib.h"
#include <stm32l031xx.h>

// Find types: h(ealth),s(trength),m(agic),g(old),w(eapon)
static const char FindTypes[]={'h','s','m','g','w'};

// The following arrays define the bad guys and 
// their battle properies - ordering matters!
// Baddie types : O(gre),T(roll),D(ragon),H(ag)
static const char Baddies[]={'O','T','D','H'};

// The following is 4 sets of 4 damage types
static const byte WeaponDamage[]={
// O  T  D  H
   0, 0, 0, 0, // Weapon 0 - Empty
  10,10, 5,25, // Weapon 1 - Axe
  10,15, 5,15, // Weapon 2 - Sword
   5, 5, 2,10  // Weapon 2 - Flail
};

#define ICE_SPELL_COST 10
#define FIRE_SPELL_COST 20
#define LIGHTNING_SPELL_COST 30

// The sound frequency and music notes
#define CPU_FREQ 16000000 

#define A4 440
#define B4 494
#define E4 329
#define Fs4 370
#define Gs4 415
#define A5 880
#define G5 783
#define Fs5 739
#define F5 698
#define E5 659
#define Ds5 622
#define D5 587
#define Cs5 554
#define As5 932
#define Gs5 830
#define E4 329
#define C5 523
#define D4	293
#define G4	392
#define F4	349
#define	B3	246
#define C4 261
#define As3	233
#define E3 164
#define A3	220
#define D3 146
#define F3 174
#define As4	466
#define Ds4	311
#define	G3	196
#define Fs3 185
#define Cs4 277

										// O  T  D  H
static const byte FreezeSpellDamage[]   ={10,20, 5, 0};
static const byte FireSpellDamage[]     ={20,10, 5, 0};
static const byte LightningSpellDamage[]={15,10,25, 0};
static const byte BadGuyDamage[]        ={10,10,15, 5};
static int GameStarted = 0;
static tPlayer thePlayer;
static tRealm theRealm;

void randomize(void);
void ADCBegin(void);
uint16_t ADCRead(void);

void SysTick_Handler(void); // Function to play sounds
void delayms(uint32_t dly);
void initSound(void);
typedef struct {
	uint32_t frequency;
	uint32_t duration;
	uint32_t wait;
} note;
void playNote(note *n);
static volatile uint32_t SoundDuration = 0;
static volatile uint32_t ms_counter = 0;

static note SpringNotesOriginal[]={  {D4,50,100}
};	

static note SpringNotes1[]={

	{D4,50,100},{D4,50,100},{D5,200,100},{A4,200,150},{Gs4,100,150},{G4,100,150},{F4,200,100},{D4,100,100},{F4,100,100},{G4,100,100},
	{C4,50,100},{C4,50,100},{D5,200,100},{A4,200,150},{Gs4,100,150},{G4,100,150},{F4,200,100},{D4,100,100},{F4,100,100},{G4,100,100},
	{B3,50,100},{B3,50,100},{D5,200,100},{A4,200,150},{Gs4,100,150},{G4,100,150},{F4,200,100},{D4,100,100},{F4,100,100},{G4,100,100},
	{As3,50,100},{As3,50,100},{D5,200,100},{A4,200,150},{Gs4,100,150},{G4,100,150},{F4,200,100},{D4,100,100},{F4,100,100},{G4,100,100}
};

static note SpringNotes2[]={  {E4,100,100 }, {Gs4,100,100 },{Gs4,100,100 },{Gs4,100,100 },{Fs4,100,100 },
							  {E4,100,100 }, {B4,300,100 }, {B4, 50, 100 },{A4,50,100 },{Gs4,100,100 },{E4,100,100 },{E4,100,100 },
							  {Fs4,100,100 },{E4,100,100 },{B4,300,100 }, {B4, 50, 100 },{A4,50,100 },{Gs4,100,100 }
};

#define NOTE_COUNT (sizeof(SpringNotesOriginal)/sizeof(note))
#define NOTE_COUNT1 (sizeof(SpringNotes1)/sizeof(note))
#define NOTE_COUNT2 (sizeof(SpringNotes2)/sizeof(note))

void Dragon_ASCII_Art(void); // Function To display the dragon
void Monster_ASCII_Art(void); // Function for random monster
void Hero_ASCII_Art(void); // Function for hero ascii art

void flashing_led(void); // Function when hero and monster fights
void flashing_led_items(void); // Function to flash led when hero find some items

__attribute__((noreturn)) void runGame(void)  
{
	initClock(); // Set the system clock running at 16MHz
	RCC->IOPENR |= 2; // Enable GPIOB
	RCC->IOPENR |= 1; // Enable GPIOA
	
	pinMode(GPIOA , 7 , 1); // MAKING BIT 0 or A0 OUTPUT Monster Red Light
	pinMode(GPIOA, 0 , 1); // Make PORTA Bit 0 an output
	pinMode(GPIOA, 1, 1); // Make PORTA Bit 1 an output
	pinMode(GPIOA , 6 , 1 ); // Making bit 6 or A5 output for health
	pinMode(GPIOA , 5 , 1 ); // Making bit 5 or A4 output for health
	pinMode(GPIOA , 4 , 1 ); // Making bit 4 or A3 output for health
	pinMode(GPIOA , 3 , 1 ); // Making bit 3 or A2 output for health

	// Music
	initSound();
	SysTick->LOAD = 15999; // 16MHz / 16000 = 1kHz
	SysTick->CTRL = 7; // Enable systick counting and interrupts, use core clock
	enable_interrupts();
	GPIOA->ODR = 1;
	
	char ch;
	randomize(); // Generate a random seed
	eputs("MicroRealms on the STM32L031\r\n");
	showHelp();		
	while(GameStarted == 0)
	{
		
		showGameMessage("Press S to start a new game\r\n");
		ch = getUserInput();			
		
		if ( (ch == 'S') || (ch == 's') )
			
				GPIOA -> ODR |= (1u << 1); // Turning on green led for Hero 
				GPIOA -> ODR |= (1u << 6); // Turning on health led for Hero 
				GPIOA -> ODR |= (1u << 5); // Turning on health led for Hero
				GPIOA -> ODR |= (1u << 4); // Turning on health led for Hero
				GPIOA -> ODR |= (1u << 3); // Turning on health led for Hero
			
				GameStarted = 1;
	}
	
	initRealm(&theRealm);	
	initPlayer(&thePlayer,&theRealm);
	showPlayer(&thePlayer);
	showRealm(&theRealm,&thePlayer);
	showGameMessage("Press H for help");
	
	RCC->IOPENR |= (1 << 0); // Enable GPIOA 5
	pinMode(GPIOA,5,1);
	
	RCC->IOPENR |= (1 << 0); // Enable GPIOA 4
	pinMode(GPIOA,4,1);
	
	while (1)
	{
		//music
		for (unsigned int i=0; i < NOTE_COUNT; i++)
		{
			playNote(&SpringNotesOriginal[i]); 
			GPIOA->ODR ^= 1;
			GPIOA->ODR ^= 2;
		}
		delayms(500);
		
		ch = getUserInput();
		ch = ch | 32; // Enforce lower case
		switch (ch) {
			case 'h' : {
				showHelp();
				break;
			}
			case 'w' : {
				showGameMessage("Up");
				GPIOA->ODR |= (1<<4);
				delay(250000);
				GPIOA->ODR &= ~(1u<<4);
				step('w',&thePlayer,&theRealm);
				break;
			}
			case 's' : {
				showGameMessage("Down");
				GPIOA->ODR |= (1<<4);
				delay(250000);
				GPIOA->ODR &= ~(1u<<4);
				step('s',&thePlayer,&theRealm);
				break;
			}
			case 'd' : {
				showGameMessage("Right");
				GPIOA->ODR |= (1<<4);
				delay(250000);
				GPIOA->ODR &= ~(1u<<4);
				step('d',&thePlayer,&theRealm);
				break;
			}
			case 'a' : {
				showGameMessage("Left");
				GPIOA->ODR |= (1<<4);
				delay(250000);
				GPIOA->ODR &= ~(1u<<4);
				step('a',&thePlayer,&theRealm);
				break;
			}
			case '#' : {		
				if (thePlayer.wealth)		
				{
					GameStarted = 1;
					if (thePlayer.wealth>0) thePlayer.wealth--;
				}
				else
					showGameMessage("No gold!");
				break;
			}
			case 'p' : {				
				showPlayer(&thePlayer);
				break;
			}
		} // end switch
	} // end while
}

void playNote(note *n)
{
	uint32_t Period = 1000000 / n->frequency;
	TIM2->CR1 |= 1; // Enable counter 
	TIM2->ARR = Period; // Set initial reload value of 1000 microseconds
	TIM2->CCR2 = TIM2->ARR/2; // 50% duty;
	SoundDuration = n->duration;
	while(SoundDuration != 0); // Wait
	TIM2->ARR = 1;
	TIM2->CR1 |= 1; // Enable counter 
	delayms(n->wait);
}

void SysTick_Handler(void)
{
	if (SoundDuration > 0)
	{
		SoundDuration --;
	}
	ms_counter ++; // Another millisecond has passed
}

void delayms(uint32_t dly)
{
	dly = dly+ms_counter;
	while(ms_counter < dly);
}

void step(char Direction,tPlayer *Player,tRealm *Realm)
{
	uint8_t new_x, new_y;
	new_x = Player->x;
	new_y = Player->y;
	byte AreaContents;
	
	switch (Direction) {
		case 'n' :
		{
			if (new_y > 0)
				new_y--;
			break;
		}
		case 's' :
		{
			if (new_y < MAP_HEIGHT-1)
				new_y++;
			break;
		}
		case 'e' :
		{
			if (new_x <  MAP_WIDTH-1)
				new_x++;
			break;
		}
		case 'w' :
		{
			if (new_x > 0)
				new_x--;
			break;
		}		
	}
	AreaContents = Realm->map[new_y][new_x];
	if ( AreaContents == '*')
	{
		GPIOA->ODR |= (1 << 1);
		GPIOA->ODR |= (1 << 3);
		GPIOA->ODR |= (1 << 4);
		delay(100000);
		
		GPIOA->ODR &= (1u << 1);
		GPIOA->ODR &= (1u << 3);
		GPIOA->ODR &= (1u << 4);
		delay(100000);
		
		GPIOA->ODR |= (1 << 1);
		GPIOA->ODR |= (1 << 3);
		GPIOA->ODR |= (1 << 4);
		delay(100000);
		
		GPIOA->ODR &= (1u << 1);
		GPIOA->ODR &= (1u << 3);
		GPIOA->ODR &= (1u << 4);

		showGameMessage("A rock blocks your path.");
		return;
	}
	Player->x = new_x;
	Player->y = new_y;
	int Consumed = 0;
	switch (AreaContents)
	{
		// const char Baddies[]={'O','T','B','H'};
		case 'O' :{
			showGameMessage("A smelly green Ogre appears before you.");
			GPIOA->ODR |= (1<<5); // Turn light on
			
			// Music
			for (unsigned int i=0; i < NOTE_COUNT2; i++)
			{
				initSound();
				playNote(&SpringNotes2[i]); 
				GPIOA->ODR ^= 1;
				GPIOA->ODR ^= 2;
			}
			delayms(500);
			
			Monster_ASCII_Art(); // Function for random monster
			Consumed = doChallenge(Player,0);
			GPIOA->ODR &= ~(1u<<5); // Turn light off
			break;
		}
		case 'T' :{
			showGameMessage("An evil troll challenges you");
			GPIOA->ODR |= (1<<5); // Turn light on
			
			// Music
			for (unsigned int i=0; i < NOTE_COUNT2; i++)
			{
				initSound();
				playNote(&SpringNotes2[i]); 
				GPIOA->ODR ^= 1;
				GPIOA->ODR ^= 2;
			}
			delayms(500);
			
			Monster_ASCII_Art(); // Function for random monster
			Consumed = doChallenge(Player,1);
			GPIOA->ODR &= ~(1u<<5); // Turn light off
			break;
		}
		case 'D' :{
			showGameMessage("A smouldering Dragon blocks your way !");
			GPIOA->ODR |= (1<<5); // Turn light on
			
			// Music
			for (unsigned int i=0; i < NOTE_COUNT2; i++)
			{
				initSound();
				playNote(&SpringNotes2[i]); 
				GPIOA->ODR ^= 1;
				GPIOA->ODR ^= 2;
			}
			delayms(500);
			
			Dragon_ASCII_Art(); // Function for random monster
			Consumed = doChallenge(Player,2);
			GPIOA->ODR &= ~(1u<<5); // Turn light off
			break;
		}
		case 'H' :{
			showGameMessage("A withered hag cackles at you wickedly");
			GPIOA->ODR |= (1<<5); // Turn light on

			// Music
			for (unsigned int i=0; i < NOTE_COUNT2; i++)
			{
				initSound();
				playNote(&SpringNotes2[i]); 
				GPIOA->ODR ^= 1;
				GPIOA->ODR ^= 2;
			}
			delayms(500);
			
			Monster_ASCII_Art(); // Function for random monster
			Consumed = doChallenge(Player,3);
			GPIOA->ODR &= ~(1u<<5); // Turn light off
			break;
		}
		case 'h' :{ 
			
			showGameMessage("You find an elixer of health");
			GPIOA->ODR |= (1<<5); // Turn light on
			// initSound();
			
			// Music
			for (unsigned int i=0; i < NOTE_COUNT1; i++)
			{
				initSound();
				playNote(&SpringNotes1[i]); 
				GPIOA->ODR ^= 1;
				GPIOA->ODR ^= 2;
			}
			delayms(500);
			
			setHealth(Player,Player->health+10);
			Consumed = 1;	
			delay(1000000);
			GPIOA->ODR &= ~(1u<<5);
			break;
			
		}
		case 's' :{
			
			showGameMessage("You find a potion of strength");
			GPIOA->ODR |= (1<<5); // Turn light on
			
			// Music
			for (unsigned int i=0; i < NOTE_COUNT1; i++)
			{
				initSound();
				playNote(&SpringNotes1[i]); 
				GPIOA->ODR ^= 1;
				GPIOA->ODR ^= 2;
			}
			delayms(500);
			
			setHealth(Player,Player->health+10);
			Consumed = 1;	
			delay(1000000);
			GPIOA->ODR &= ~(1u<<5); // Turn light off
			break;
		}
		case 'g' :{
			
			showGameMessage("You find a shiny golden nugget");
			GPIOA->ODR |= (1<<5); // Turn light on
			
			// Music
			for (unsigned int i=0; i < NOTE_COUNT1; i++)
			{
				initSound();
				playNote(&SpringNotes1[i]); 
				GPIOA->ODR ^= 1;
				GPIOA->ODR ^= 2;
			}
			delayms(500);
			
			Player->wealth++;			
			Consumed = 1;
			delay(1000000);
			GPIOA->ODR &= ~(1u<<5); // Turn light off
			break;
		}
		case 'm' :{
			
			showGameMessage("You find a magic charm");
			GPIOA->ODR |= (1<<5); // Turn light on
			
			// Music
			for (unsigned int i=0; i < NOTE_COUNT1; i++)
			{
				initSound();
				playNote(&SpringNotes1[i]); 
				GPIOA->ODR ^= 1;
				GPIOA->ODR ^= 2;
				delay(1000000);
			}
			delayms(500);
			Player->magic++;
			Consumed = 1;
			GPIOA->ODR &= ~(1u<<5); // Turn light off
			break;
		}
		case 'w' :{
			Consumed = addWeapon(Player,(int)random(MAX_WEAPONS-1)+1);
			showPlayer(Player);
			break;			
		}
		case 'X' : {
			// Player landed on the exit
			eputs("A door! You exit into a new realm");
			setHealth(Player,100); // Maximize health
			
			GPIOA -> ODR |= (1u << 6); // Turning on health led for Hero 
			GPIOA -> ODR |= (1u << 5); // Turning on health led for Hero
			GPIOA -> ODR |= (1u << 4); // Turning on health led for Hero
			GPIOA -> ODR |= (1u << 3); // Turning on health led for Hero
			
			initRealm(&theRealm);
			showRealm(&theRealm,Player);
		}
	}
	if (Consumed)
		Realm->map[new_y][new_x] = '.'; // Remove any item that was found
}
int doChallenge(tPlayer *Player,int BadGuyIndex)
{
	char ch;
	char Damage;
	const byte *dmg;
	int BadGuyHealth = 100;
	
	eputs("Press F to fight");
	ch = getUserInput() | 32; // Get user input and force lower case
	if (ch == 'f')
	{
		GPIOA->ODR |= (1u << 7); // LED ON WHEN MONSTER CAME
		Hero_ASCII_Art(); // Function for hero ascii art
		
		eputs("\r\nChoose action");
		while ( (Player->health > 0) && (BadGuyHealth > 0) )
		{
			eputs("\r\n");
			// Player takes turn first
			if (Player->magic > ICE_SPELL_COST)
				eputs("(I)CE spell");
			if (Player->magic > FIRE_SPELL_COST)
				eputs("(F)ire spell");
			if (Player->magic > LIGHTNING_SPELL_COST)
				eputs("(L)ightning spell");
			if (Player->Weapon1)
			{
				eputs("(1)Use ");
				eputs(getWeaponName(Player->Weapon1));
			}	
			if (Player->Weapon2)
			{
				eputs("(2)Use ");
				eputs(getWeaponName(Player->Weapon2));
			}
			eputs("(P)unch");
			ch = getUserInput();
			switch (ch)
			{
				case 'i':
				case 'I':
				{
					eputs("FREEZE!");
					flashing_led(); // Blinks led of monster and hero
					Player->magic -= ICE_SPELL_COST;
					BadGuyHealth -= FreezeSpellDamage[BadGuyIndex]+random(10);
						zap();
					break;
				}
				case 'f':
				case 'F':
				{
					eputs("BURN!");
					flashing_led(); // Blinks led of monster and hero
					Player->magic -= FIRE_SPELL_COST;
					BadGuyHealth -= FireSpellDamage[BadGuyIndex]+random(10);
						zap();
					break;
				}
				case 'l':
				case 'L':
				{
					eputs("ZAP!");
					flashing_led(); // Blinks led of monster and hero
					Player->magic -= LIGHTNING_SPELL_COST;
					BadGuyHealth -= LightningSpellDamage[BadGuyIndex]+random(10);
						zap();
					break;
				}
				case '1':
				{
					dmg = WeaponDamage+(Player->Weapon1<<2)+BadGuyIndex;
					eputs("Take that!");
					flashing_led(); // Blinks led of monster and hero
					BadGuyHealth -= *dmg + random(Player->strength);
					setStrength(Player,Player->strength-1);
					break;
				}
				case '2':
				{
					dmg = WeaponDamage+(Player->Weapon2<<2)+BadGuyIndex;
					eputs("Take that!");
					flashing_led(); // Blinks led of monster and hero
					BadGuyHealth -= *dmg + random(Player->strength);
					setStrength(Player,Player->strength-1);
					break;
				}
				case 'p':
				case 'P':
				{
					eputs("Thump!");
					flashing_led(); // Blinks led of monster and hero
					BadGuyHealth -= 1+random(Player->strength);
					setStrength(Player,Player->strength-1);
					break;
				}
				default: {
					eputs("You fumble. Uh oh");
				}
			}
			// Bad guy then gets a go 
			
			if (BadGuyHealth < 0)
			BadGuyHealth = 0;
			Damage = (uint8_t)(BadGuyDamage[BadGuyIndex]+(int)random(5));
			setHealth(Player,Player->health - Damage);
			eputs("Health: you "); printDecimal(Player->health);
			eputs(", them " );printDecimal((uint32_t)BadGuyHealth);
			eputs("\r\n");
			
			     // Funtion to indicate health through leds
			
				if ( (Player->health) < 75 && (Player->health) > 50 )
				{
					GPIOA->ODR &=~ (1u << 3);
				}
				else if ( (Player->health) < 50 && (Player->health) > 25 )
				{
					GPIOA->ODR &=~ (1u << 4);
				}
				else if ( (Player->health) < 25 && (Player->health) > 0 )
				{
					GPIOA->ODR &=~ (1u << 5);
				}
				else if ( (Player->health) == 0 )
				{
					GPIOA->ODR &=~ (1u << 6);
				}
				
		}
		if (Player->health == 0)
		{ // You died
			eputs("You are dead. Press Reset to restart");
			
			GPIOA -> ODR &=~ ( 1u << 1 ) ; // Hero DIED green led off
		
			while(1);
			
		}
		else
		{ // You won!
			Hero_ASCII_Art(); // Function for hero ascii art
			Player->wealth = (uint8_t)(50 + random(50));			
			showGameMessage("You win! Their gold is yours");			
			
			GPIOA->ODR &=~ ( 1u << 7); // Monster goes off red led off
			
			return 1;
		}
		
	}
	else
	{
		showGameMessage("Our 'hero' chickens out");
		return 0;
	}
}
int addWeapon(tPlayer *Player, int Weapon)
{
	char c;
	eputs("You stumble upon ");
	switch (Weapon)
	{
		case 1:
		{	
			eputs("a mighty axe");
			break;
		}
		case 2:
		{	
			eputs("a sword with mystical runes");
			break;
		}
		case 3:
		{	
			eputs("a bloody flail");
			break;
		}		
		default:
			printDecimal((uint32_t)Weapon);
	}
	if ( (Player->Weapon1) && (Player->Weapon2) )
	{
		// The player has two weapons already.
		showPlayer(Player);
		eputs("You already have two weapons\r\n");		
		eputs("(1) drop Weapon1, (2) for Weapon2, (0) skip");
		c = getUserInput();
		eputchar(c);
		switch(c)
		{
			case '0':{
				return 0; // Don't pick up
			}
			case '1':{
				Player->Weapon1 = (uint8_t)Weapon;
				return 1;
			}
			case '2':{
				Player->Weapon2 = (uint8_t)Weapon;
				return 1;
			}
		}
	}
	else
	{
		if (!Player->Weapon1)
		{
			Player->Weapon1 = (uint8_t)Weapon;	
		}
		else if (!Player->Weapon2)
		{
			Player->Weapon2 = (uint8_t)Weapon;
		}
		return 1;
	}	
	return 0;
}
const char *getWeaponName(int index)
{
	switch (index)
	{
		case 0:return "Empty"; 
		case 1:return "Axe";
		case 2:return "Sword"; 
		case 3:return "Flail"; 
	}
	return "Unknown";
}

void setHealth(tPlayer *Player,int health)
{
	if (health > 100)
		health = 100;
	if (health < 0)
		health = 0;
	Player->health = (uint8_t)health;
}	
void setStrength(tPlayer *Player, byte strength)
{
	if (strength > 100)
		strength = 100;
	Player->strength = strength;
}
void initPlayer(tPlayer *Player,tRealm *Realm)
{
	// Get the player name
	int index=0;
	byte x,y;
	char ch=0;
	
	// Initialize the player's attributes
	eputs("Enter the player's name: ");
	while ( (index < MAX_NAME_LEN) && (ch != '\n') && (ch != '\r'))
	{
		ch = getUserInput();
		if ( ch > '0' ) // Strip conrol characters
		{
			Player->name[index++]=ch;
			eputchar(ch);
		}
	}
	Player->name[index]=0; // Terminate the name
	setHealth(Player,100);
	Player->strength=(uint8_t)(50+random(50));
	Player->magic=(uint8_t)(50+random(50));	
	Player->wealth=(uint8_t)(10+random(10));
	Player->Weapon1 = 0;
	Player->Weapon2 = 0;
	// Initialize the player's location
	// Make sure the player does not land
	// on an occupied space to begin with
	do {
		x=(uint8_t)random(MAP_WIDTH);
		y=(uint8_t)random(MAP_HEIGHT);
		
	} while(Realm->map[y][x] != '.');
	Player->x=x;
	Player->y=y;
}
void showPlayer(tPlayer *Player)
{
	eputs("\r\nName: ");
	eputs(Player->name);
	eputs("health: ");
	printDecimal(Player->health);
	eputs("\r\nstrength: ");
	printDecimal(Player->strength);
	eputs("\r\nmagic: ");
	printDecimal(Player->magic);
	eputs("\r\nwealth: ");
	printDecimal(Player->wealth);	
	eputs("\r\nLocation : ");
	printDecimal(Player->x);
	eputs(" , ");
	printDecimal(Player->y);	
	eputs("\r\nWeapon1 : ");
	eputs(getWeaponName(Player->Weapon1));
	eputs(" Weapon2 : ");
	eputs(getWeaponName(Player->Weapon2));
}
void initRealm(tRealm *Realm)
{
	unsigned int x,y;
	unsigned int Rnd;
	// Clear the map to begin with
	for (y=0;y < MAP_HEIGHT; y++)
	{
		for (x=0; x < MAP_WIDTH; x++)
		{
			Rnd = random(100);
			
			if (Rnd >= 98) // Put in some baddies
				Realm->map[y][x]=	Baddies[random(sizeof(Baddies))];
			else if (Rnd >= 95) // Put in some good stuff
				Realm->map[y][x]=	FindTypes[random(sizeof(FindTypes))];
			else if (Rnd >= 90) // Put in some rocks
				Realm->map[y][x]='*'; 
			else // Put in empty space
				Realm->map[y][x] = '.';	
		}
	}
	
	// Finally put the exit to the next level in
	x = random(MAP_WIDTH);
	y = random(MAP_HEIGHT);
	Realm->map[y][x]='X';
}
void showRealm(tRealm *Realm,tPlayer *Player)
{
	int x,y;
	eputs("\r\nThe Realm:\r\n");	
	for (y=0;y<MAP_HEIGHT;y++)
	{
		for (x=0;x<MAP_WIDTH;x++)
		{
			
			if ( (x==Player->x) && (y==Player->y))
				eputchar('@');
			else
				eputchar(Realm->map[y][x]);
		}
		eputs("\r\n");
	}
	GameStarted = 0;
	eputs("\r\nLegend\r\n");
	eputs("(T)roll, (O)gre, (D)ragon, (H)ag, e(X)it\r\n");
	eputs("(w)eapon, (g)old), (m)agic, (s)trength\r\n");
	eputs("@=You\r\n");
}
void showHelp()
{
	eputs("Help\r\n");
	eputs("N,S,E,W : go North, South, East, West\r\n");
	eputs("# : show map (cost: 1 gold piece)\r\n");
	eputs("(H)elp\r\n");
	eputs("(P)layer details\r\n");
}

void showGameMessage(char *Msg)
{
	eputs(Msg);
	eputs("\r\nReady\r\n");	
}
char getUserInput()
{
	char ch = 0;
	
	while (ch == 0)
		ch = egetchar();
	return ch;
}
unsigned random(unsigned range)
{
	// Implementing my own version of modulus
	// as it is a lot smaller than the library version
	// To prevent very long subtract loops, the
	// size of the value returned from prbs has been
	// restricted to 8 bits.
	unsigned Rvalue = (prbs()&0xff);
	while (Rvalue >= range)
		Rvalue -= range; 
	return Rvalue;
}

void zap(void)
{
   
}

void ADCBegin(void)
{
RCC->APB2ENR |= (1u << 9); // Turn on ADC
RCC->IOPENR |= 1; // Enable GPIOA
pinMode(GPIOA,4,3); // Make GPIOA_4 an analogue input
ADC1->CR = 0; // Disable ADC before making changes
ADC1->CR |= (1u << 28); // Turn on the voltage regulator
ADC1->CR |= (1u << 31); // Start calibration
while ( (ADC1->CR & (1u << 31)) != 0); // Wait for calibration to complete.
ADC1->CHSELR = (1 << 8); // Select channel4
ADC1->CR |= 1; // Enable the ADC
}
uint16_t ADCRead(void)
{
ADC1->CR |= (1 << 2); // Start a conversion
while ( (ADC1->CR & (1 << 2)) != 0); // Wait for conversion to complete.
return (uint16_t)ADC1->DR;
}
static unsigned long shift_register = 0;

void randomize(void)
{
// To generate a "true" random seed for the prbs generator
// Repeatedly read a floating analogue input (32 times)
// and the least significant bit of that read to set each
// bit of the prbs shift registers.
// Must make sure that all of its buts are not zero when
// done.
RCC->IOPENR |= (1 << 1); // Enable port B
pinMode(GPIOB,0,3); // Use PB0 as the source of noise. This ADC channel 8
ADCBegin();
shift_register = 0;
while(shift_register == 0)
{
for (int i=0; i < 32; i++)
{
	shift_register = shift_register << 1;
	shift_register |= (ADCRead() & 1);
	delay(10000);
	printDecimal(shift_register);
	eputs("\r\n");
}
}
}
uint32_t prbs()
{

// This is an unverified 31 bit PRBS generator
// It should be maximum length but this has not been verified

unsigned long new_bit=0;

new_bit = (shift_register >> 27) ^ (shift_register >> 30);
new_bit = new_bit & 1;
shift_register=shift_register << 1;
shift_register=shift_register | (new_bit);

return shift_register & 0x7fffffff; // Return 31 LSB's
}

// Function to flash led when both monster and hero are fighting
void flashing_led(void)
{
	for (int i = 0 ; i < 10 ; i++) // Loop for flashing
		{
			GPIOA->ODR &=~ (1u << 7); // Monster LED will off
			GPIOA->ODR &=~ (1u << 1); // Hero LED will off
			delay(50000);
			GPIOA->ODR |= (1u << 7); // Monster led will on
			GPIOA->ODR |= (1u << 1); // Hero LED will off
			delay(50000);
		}
}
void flashing_led_items(void) // Function to flash led when hero gets items
{
		for (int i = 0 ; i < 6 ; i++) // Loop for flashing
		{
			GPIOA->ODR &=~ (1u << 1); // Hero LED will off
			delay(50000);
			GPIOA->ODR |= (1u << 1); // Hero LED will off
			delay(50000);
		}
}

void Dragon_ASCII_Art(void) //  Function To display the dragon
{
	eputs("\n\r");
	eputs("\n\r");
	eputs("\n\r");
	eputs("\n\r");
	eputs("\n\r                \\||/						");
	eputs("\n\r                |  @___oo				");
	eputs("\n\r      /\\  /\\   / (__,,,,|			");
	eputs("\n\r     ) /^\\) ^\\/ _)							");
	eputs("\n\r     )   /^\\/   _)							");
	eputs("\n\r     )   _ /  / _)								");
	eputs("\n\r /\\  )/\\/ ||  | )_)						");
	eputs("\n\r<  >      |(,,) )__)							");
	eputs("\n\r||      /    \\)___)\\						");
	eputs("\n\r | \\____(      )___) )___				");
	eputs("\n\r  \\______(_______;;; __;;;			");
	eputs("\n\r");
	eputs("\n\r");
	eputs("\n\r");
}

void Monster_ASCII_Art(void) // Function for random monster
{
	eputs("\n\r");
	eputs("\n\r                 /^^^^\\");
	eputs("\n\r    /^^\\________/0     \\");
	eputs("\n\r   (                    `~+++,,_____,,++~^^^^^^^^");
	eputs("\n\r ...V^V^V^V^V^V^\\................................");
	eputs("\n\r");
	eputs("\n\r");
}
void Hero_ASCII_Art(void) // Function for hero 
{
	eputs("\n\r             .-----.   	");
	eputs("\n\r            /       \\ ");
	eputs("\n\r            \\       / ");
	eputs("\n\r     .-----.-`.-.-.<  _ ");
	eputs("\n\r    /      _,-\\ ()()_/:) ");
	eputs("\n\r    \\     / ,  `     `| ");
	eputs("\n\r     '-..-| \\-.,___,  / ");
	eputs("\n\r           \\ `-.__/  / ");
	eputs("\n\r          / `-.__.-\\` ");
	eputs("\n\r         / /|    ___\\ ");
	eputs("\n\r        ( ( |.--`   `'\\ ");
	eputs("\n\r         \\ \\/    {}{}  | ");
	eputs("\n\r          \\|           / ");
	eputs("\n\r           \\        , / ");
	eputs("\n\r           ( __`;-;'__`) ");
	eputs("\n\r           `//'`   `||` ");
	eputs("\n\r          _//       || ");
	eputs("\n\r  .---._,(__)     .(__).----. ");
	eputs("\n\r /          \\    /           \\ ");
	eputs("\n\r \\          /    \\           / ");
	eputs("\n\r  `'-------`      `--------'` ");
	eputs("\n\r ");
	eputs("\n\r ");	
	eputs("\n\r");
	eputs("\n\r");
	eputs("\n\r");
}

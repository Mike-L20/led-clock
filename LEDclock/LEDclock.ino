/* --------------------------------------------------------------------------------------------------
 * LED CLOCK
 * Program for controling clock - 2 rings with addressable led strips
 * 
 * Ver.:  8.0
 * Auth.: Grega Morano
 * Date.: 29.4.2020
 * 
 * --------------------------------------------------------------------------------------------------
 * User can change modes of the clock with BTN 1, types with BTN 2
 * 4 Modes with different types:
 * 	- MODE 1: (default) clock where user can define colors and brightness. User can circle through 
 * 	  different types with button 2 - for type. Values are changed with potenciometter. 
 * 			TYPE 1: Set brightness of the clock
 * 			TYPE 2: Set compas color
 * 			TYPE 3: Set minute indicator color
 * 			TYPE 4: Set hour indicator color
 * 
 * - MODE 2: Display clock with indicator for seconds. Here colors are predefined, but user can change
 * 	 all of them together. It can be done same as before, with BNT2 for type in circular order. Values 
 *   are changed with potenciometter. 
 * 			TYPE 1: Set brightness of the clock
 * 			TYPE 2: Set color of the clock
 * 
 * - MODE 3: Some colors with LED-S
 * 			TYPE 1: Rainbow circling around - user can change its speed with pot.
 * 			TYPE 2: Sinelon back and forward - user can change its color with pot.
 * 			TYPE 3: Chasing dots - user can just observe dots chasing each other.
 * 			TYPE 4: Fireworks - user must observe all 3 rockets - it can't be changed, but it will
 * 					automatically shifted back to rainbow (type 1) after 3 rockets.
 * 
 * - MODE 4: Thermometer 
 * 
 * If both buttons are pressed at the same time, user can change the time of the clock
 * If both buttons are pressed before startup, clock will go in the mantainance mode - it will display
 * compas (for mounting purpose) and later all effect between hours (demonstration purpose)
 *-----------------------------------------------------------------------------------------------------*/



// TODO: testjrej BD show zdej ku si spremenu ku nej vč for zanke...tudi mantainance mode je zdej drgačn

/* ----------------------------------------------------------------------------------------------------*/
#include <FastLED.h>
#include "DS3231.h"
#include "ring.h"


#define DEBUG 1

#define STR(s) (#s)

#if DEBUG
	#define LOG_INFO(s, x) Serial.print(STR(s)); \
							Serial.print(" "); \
							Serial.println((x))
#else
	#define LOG_INFO(s, x)
#endif
/* ----------------------------------------------------------------------------------------------------*/
/*--------------- DEFINES -----------------------------------------------------------------------------*/
/* ----------------------------------------------------------------------------------------------------*/
#define BTN1_PIN 7
#define BTN2_PIN 9

#define POT_PIN 0

#define PHOTO_PIN 2

#define START_BRIGHTNESS 100

/* ----------------------------------------------------------------------------------------------------*/
/*-------------- GLOBAL VARIABLES ---------------------------------------------------------------------*/
/* ----------------------------------------------------------------------------------------------------*/

// DS3231 clock module library declaration
DS3231  Clock;

// FastLED strip declarations
CRGB HourLeds[NUM_LED_HOUR];
CRGB MinLeds[NUM_LED_MIN];

// Ring class defines
MinRing  mRing;
HourRing hRing;


// Variable for clock modes
int mode = 2;
int mode_num = 4;

// Variable for different types of same mode
int type = 1;
int type_num = 4;

// Value of potenciometer
int aVal = 0;
int avgVal = 0;
int potVal[5];
int cnt = 0;

// Value of photo resistor
int photo = 1023;

// Buttons variables
int btn1_last_state = 1;
int btn2_last_state = 1;
int btn1 = 1;
int btn2 = 1;
unsigned long btn1_debounce_time;
unsigned long btn2_debounce_time;
int debounce_delay = 50;

// Variable for color of the clock
user_color clock_color;

/* ----------------------------------------------------------------------------------------------------*/
/*--------------- FUNCTION DECLARATIONS ---------------------------------------------------------------*/
/* ----------------------------------------------------------------------------------------------------*/
void CLOCK_FadeToBlack			(void);
void CLOCK_SetBrightness		(void);

void CLOCK_DisplayTime			(int tip, int val, int btn);
void CLOCK_DisplayLamp			(int tip, int val);
void CLOCK_DisplayTemperature	(void);

void CLOCK_DisplayFullHour		(int val);
void CLOCK_DisplayHalfHour		(int val);
void CLOCK_DisplayQuarter		(int val, myTime t);
void CLOCK_DisplayTriQuarter	(int val, myTime t);
void CLOCK_DisplayFirework		(void);

void CLOCK_SetTime				(void);
void CLOCK_MaintainanceMode		(void);


/* ----------------------------------------------------------------------------------------------------*/
/*--------------- SETUP -------------------------------------------------------------------------------*/
/* ----------------------------------------------------------------------------------------------------*/
void setup() {
	delay(100);

	// Initialize serial connection - debugging purposes  
	Serial.begin(115200);	// TODO dodej v #ifdef #endif

	Serial.println("Debugging state: %d", DEBUG);

	// Initialize GPIO pins for buttons
	pinMode(BTN1_PIN, INPUT_PULLUP);
	pinMode(BTN2_PIN, INPUT_PULLUP);

	// Begin with I2C connection (for DS3231 module)
	Wire.begin();

	// Configure clock module
	Clock.setClockMode(false);  // set to 24h

	// TODO 
	Clock.setYear(2020);
	Clock.setMonth(3);
	Clock.setDate(8);
	Clock.setHour(13);
	Clock.setMinute(20);
	Clock.setSecond(0);

	// Set initial values of the user clock color
	clock_color.compas = 255;
	clock_color.hour_color = 200;
	clock_color.min_color = 200;

	// If both BTN are pressed durring startup, go to maintainance mode
	if(!digitalRead(BTN1_PIN) && !digitalRead(BTN2_PIN))
	{
		CLOCK_MaintainanceMode();
	}  

	FastLED.setBrightness(START_BRIGHTNESS);
}


/* ----------------------------------------------------------------------------------------------------*/
/*--------------- MAIN LOOP ---------------------------------------------------------------------------*/
/* ----------------------------------------------------------------------------------------------------*/
void loop() {

	/*---------BUTTON STATE CHECK------------------------------*/
	uint8_t btn1_read = digitalRead(BTN1_PIN);
	uint8_t btn2_read = digitalRead(BTN2_PIN);

	// Check how long the button is pressed to prevent debouncing
	if(btn1_read != btn1_last_state)
	{
		btn1_debounce_time = millis();
	}
	if(btn2_read != btn2_last_state)
	{
		btn2_debounce_time = millis();
	}

	// If BTN 1 is pressed
	if((millis() - btn1_debounce_time) > debounce_delay)
	{
		if(btn1_read != btn1)
		{
			btn1 = btn1_read;
			if(btn1 == LOW)
			{	
				//If later BTN 2 is also pressed
				if(btn2 == LOW){
					CLOCK_SetTime();
				}
				else{
					//change mode in circular order
					mode++;
					if(mode >= mode_num){
						mode = 0;
					}
					type = 0; //reset type counter so it allways starts from begining

					LOG_INFO(Mode:, mode);
				}   
			}
		}
	}
	//If BTN 2 is pressed
	if((millis() - btn2_debounce_time) > debounce_delay)
	{
		if(btn2_read != btn2)
		{
			btn2 = btn2_read;
			if(btn2 == LOW)
			{
				//If later BTN 1 is also pressed
				if(btn1 == LOW){
					CLOCK_SetTime();
				}
				else{
					//change type in circular order
					type++;
					if(type >= type_num){
						type = 0;
					}
					LOG_INFO(Type:, type);
				}        
			}
		}  
	} 
	btn1_last_state = btn1_read;
	btn2_last_state = btn2_read;

	/*---------GET POTENTIOMETER VALUE------------------------------*/
	// Average reading of 5 pot values, because pot varies up/down

	potVal[cnt] = analogRead(POT_PIN);
	cnt++;

	if(cnt == 5){
		avgVal = 0;
		for (int i=0; i<5; i++){
			avgVal = avgVal + potVal[i];
		}
		avgVal = avgVal/5;
		cnt = 0;
	}

	aVal = map(avgVal,0,1023,0,255);
  
	//LOG_INFO(Pot value:, aVal);

	/*---------SET BRIGHTNESS ACORDING TO PHOTO RESISTOR--------------*/
	photo = analogRead(PHOTO_PIN);

	/*
	Serial.println(photo);
	photo = map(photo,0,1023,0,255);
	FastLED.setBrightness(photo);
	if(photo < 100){
		FastLED.setBrightness(0);
	}*/

	/*
	if(photo < 120){
		FastLED.setBrightness(0);
	}
	else if(photo > 900){
		FastLED.setBrightness(200);
	}
	else{
		FastLED.setBrightness(100);
	}
	*/

	/*---------DISPLAY LEDS IN GIVEN MODE-------------------------------*/
	switch(mode){
		case 0:
			CLOCK_DisplayTime(0, aVal, type);
			break;

		case 1:
			CLOCK_DisplayTime(1, aVal, type);
			break;

		case 2:
			CLOCK_DisplayLamp(type, aVal);
			break;  

		case 3:
			CLOCK_DisplayTemperature();
			break;

		default:
			CLOCK_DisplayLamp(type, aVal);
			break;  
	}  

	FastLED.show();
}


/* ----------------------------------------------------------------------------------------------------*/
/*--------------- FUNCTION DEFINITIONS ----------------------------------------------------------------*/
/* ----------------------------------------------------------------------------------------------------*/


// Slowly fade away
void CLOCK_FadeToBlack(void)
{
	for(uint8_t i = 0; i < 50; i++)
	{
		for(uint8_t j = 0; j < NUM_LED_MIN; j++)
		{
			MinLeds[j].fadeToBlackBy(16);
		}
		for(uint8_t k = 0; k < NUM_LED_HOUR; k++)
		{
			HourLeds[k].fadeToBlackBy(16);
		}
		FastLED.delay(10);
	}
	mRing.setBlack();
	hRing.setBlack();
}


/* ----------------------------------------------------------------------------------------------------*/
// TIP 0: User colors - without seconds
// TIP 1: Variable colors -  with seconds
// TIP 2: Predefined colors - with smooth seconds
void CLOCK_DisplayTime(int tip, int val, int btn){

	static int val_color = 0;
	static int val_bright = START_BRIGHTNESS;
	bool h12 = false;

	myTime t;
	t.hour = Clock.getHour(h12,h12);
	t.min = Clock.getMinute();
	t.sec = Clock.getSecond();

	// Update position of clock pixels
	hRing.updateClockPosition(t);
	mRing.updateClockPosition(t);

	switch(tip){
		case 0:
			if(btn == 0)
			{
				val_bright = val;
			} 
			else if(btn == 1)
			{
				clock_color.compas = val;
			} 
			else if(btn == 2)
			{
				clock_color.min_color = val;
			}
			else if(btn == 3)
			{
				clock_color.hour_color = val;
			}
			FastLED.setBrightness(val_bright);

			mRing.displayClockUserColor(clock_color);
			hRing.displayClockUserColor(clock_color);
			break;

		case 2:
			//toggle between Color and Brightness settings - depends on BTN2
			if(btn%2 == 0){
				val_bright = val;
			} else{
				val_color = val;
			}
			FastLED.setBrightness(val_bright);

			mRing.displayClockVariableColor(val_color);
			hRing.displayClockVariableColor(val_color);
			break;
			
		case 1:
			//toggle between Color and Brightness settings - depends on BTN2
			if(btn%2 == 0){
				val_bright = val;
			} else{
				val_color = val;
			}
			FastLED.setBrightness(val_bright);

			mRing.displayClockPredefinedColor(val_color);
			hRing.displayClockPredefinedColor(val_color);
			break;
	}


	// Disable special effects durring night
	// So it doesn't wake up whole neighbour-hood 
	if(photo > 100 && (t.hour > 8 && t.hour < 23))
	{
		// Every 15min display one fo the effects
		if((t.min == 0) && (t.sec == 5))
		{
			CLOCK_DisplayFullHour(val_color);
		}
		else if((t.min == 15) && (t.sec == 5))
		{
			CLOCK_DisplayQuarter(val, t);
		}
		else if((t.min == 30) && (t.sec == 5))
		{
			CLOCK_DisplayHalfHour(val_color);
		}
		else if((t.min == 45) && (t.sec == 5))
		{
			CLOCK_DisplayTriQuarter(val_color, t);
		}
		else if((t.hour == 13) && (t.min == 20) && (t.sec == 5))
		{	
			for(uint8_t i=0; i<7; i++){
				CLOCK_DisplayFirework();
			}
		}
	}

	// For testing purpose TODO delete
	#if DEBUG
		if((t.sec == 5) || (t.sec == 35)){

		}
	#endif
}


/* ----------------------------------------------------------------------------------------------------*/
void CLOCK_DisplayFullHour(int val)
{
	CLOCK_FadeToBlack();
	mRing.rainbowFillin(val);
}


/* ----------------------------------------------------------------------------------------------------*/
void CLOCK_DisplayQuarter(int val, myTime t)
{
	int start_px_hour = (t.hour%12);
	int start_px_min  = 15;

	//same as CLOCK_FadeToBlack() but keep 15min compas
	for(int i=0; i<50; i++){
		for(int j=0; j<NUM_LED_MIN; j++){
			MinLeds[j].fadeToBlackBy(16);
			mRing.setHSV(14, val, 250, 150);
			mRing.setHSV(15, val, 250, 255);
			mRing.setHSV(16, val, 250, 150);
		}
		for(int k=0; k<NUM_LED_HOUR; k++){
			HourLeds[k].fadeToBlackBy(16);
			hRing.setHSV(start_px_hour, val, 250, 250);
			hRing.setHSV(start_px_hour+1, val, 250, 250);
		}
		FastLED.delay(10);
	}


	mRing.colorWipe(val, 40, start_px_min);
	hRing.colorWipe(val, 40, start_px_hour);

	FastLED.delay(300);

	hRing.colorWipe(-1, 30, start_px_hour);
	mRing.colorWipe(-1, 30, start_px_min);
}


/* ----------------------------------------------------------------------------------------------------*/
void CLOCK_DisplayHalfHour(int val)
{
	//CLOCK_FadeToBlack() but keep half hour compas
	for(int i=0; i<50; i++){
		for(int j=0; j<NUM_LED_MIN; j++){
			MinLeds[j].fadeToBlackBy(16);

			mRing.setHSV(31, val, 255, 250);
			mRing.setHSV(30, val-9, 255, 250);
			mRing.setHSV(29, val, 255, 250);
		}
		for(int k=0; k<NUM_LED_HOUR; k++){
			HourLeds[k].fadeToBlackBy(16);
		}
	FastLED.delay(10);
	}

	mRing.rainbowFaling(val);
	CLOCK_FadeToBlack();  
}


/* ----------------------------------------------------------------------------------------------------*/
void CLOCK_DisplayTriQuarter(int val, myTime t)
{	
	int start_px_hour = (t.hour%12);

	CLOCK_FadeToBlack();  

	mRing.quarterWipe(val, 60);
	FastLED.delay(50);
	mRing.quarterWipe(-1, 60);

	hRing.colorWipe(val, 40, start_px_hour);
	FastLED.delay(50);
	mRing.quarterWipe(val, 60);
	hRing.colorWipe(-1, 30, start_px_hour);
	mRing.quarterWipe(-1, 60);

	CLOCK_FadeToBlack(); 
}


/* ----------------------------------------------------------------------------------------------------*/
void CLOCK_DisplayLamp(int tip, int val){
	static int rainbow_val = 0;
	static int rainbow_count = 0;

	static uint8_t pos;
	uint8_t hour_br;

	byte dothue = 0;

	switch(tip){
		// Rainbow cycling in a circle 
		case 0:
			mRing.rainbow(rainbow_val);
			hRing.rainbow(rainbow_val);

			// Costum Delay for rainbow to cycle
			rainbow_count++;
			if (rainbow_count > (val/10)+1 ){
				rainbow_val+=2;
				rainbow_count = 0;
			}
			if (rainbow_val >= 255){
				rainbow_val = 0;
			}
			break;


		// Sinelon left and right whit
		case 1:
			// Minute ring
			pos = beatsin8(15, 0, NUM_LED_MIN-1);
			MinLeds[pos] = CHSV(val, 200, 255);
			nscale8(MinLeds,NUM_LED_MIN,249); 

			hour_br = map(pos, 0, 59, 0, 255);
			// Low quality LEDs go funky at low brightness
			if(hour_br < 25){
				hour_br = 0;
			}

			hRing.setAllHSV(val, 240, hour_br);
			//nscale8(HourLeds,NUM_LED_HOUR,252);
			break;


		// Chasing dots
		case 2:   
			nscale8(MinLeds,NUM_LED_MIN,150); 
			for(int i=0; i<8; i++){
				MinLeds[beatsin16(i+7,0,NUM_LED_MIN-1)] |= CHSV(dothue,200,255);
				dothue += 32;
			}
			hRing.setBlack();
			break;
			

		// Firework (one time)
		case 3:
			CLOCK_DisplayFirework();
			type = 0;
			FastLED.delay(100);
			break;	
			

		/*All leds glow in one colour - depends on potenciometer state*/
		default:
			if(val > 253){
				mRing.setWhite();
				hRing.setWhite();
			}
			else{
				mRing.setAllHSV(val,255,220);
				hRing.setAllHSV(val,255,220);
			}
			break;
	}
}


/* ----------------------------------------------------------------------------------------------------*/
void CLOCK_DisplayTemperature(void){
  int temp = Clock.getTemperature();

  hRing.setBlack();
  mRing.setBlack();

  if(temp < 18){
	fill_gradient(MinLeds, temp, CHSV(175,255,255), CHSV(171,255,255), SHORTEST_HUES);
	hRing.setAllHSV(175, 255, 255);
  }
  else if(temp > 26){
	fill_gradient(MinLeds, temp, CHSV(175,255,255), CHSV(0,255,255), LONGEST_HUES);
	hRing.setAllHSV(0, 255, 255);
	
  }
  else{
	switch(temp){
	  //blue
	  case 18:
		fill_gradient(MinLeds, temp, CHSV(175,255,255), CHSV(169,255,255), SHORTEST_HUES);
		hRing.setAllHSV(171, 255, 255);
	  break;
	  //
	  case 19:
		fill_gradient(MinLeds, temp, CHSV(175,255,255), CHSV(150,255,255), SHORTEST_HUES);
		hRing.setAllHSV(150, 255, 255);
	  break;
	  //
	  case 20:
		fill_gradient(MinLeds, temp, CHSV(175,255,255), CHSV(128,255,255), SHORTEST_HUES);
		hRing.setAllHSV(128, 255, 255);
	  break;
	  //
	  case 21:
		fill_gradient(MinLeds, temp, CHSV(175,255,255), CHSV(103,255,255), SHORTEST_HUES);
		hRing.setAllHSV(103, 255, 255);
	  break;
	  //
	  case 22:
		fill_gradient(MinLeds, temp, CHSV(175,255,255), CHSV(80,255,255), SHORTEST_HUES);
		hRing.setAllHSV(80, 255, 255);
	  break;
	  //
	  case 23:
		fill_gradient(MinLeds, temp, CHSV(175,255,255), CHSV(60,255,255), SHORTEST_HUES);
		hRing.setAllHSV(60, 255, 255);
	  break;
	  //
	  case 24:
		fill_gradient(MinLeds, temp, CHSV(175,255,255), CHSV(42,255,255), LONGEST_HUES);
		hRing.setAllHSV(42, 255, 255);
	  break;
	  //
	  case 25:
		fill_gradient(MinLeds, temp, CHSV(175,255,255), CHSV(20,255,255), LONGEST_HUES);
		hRing.setAllHSV(20, 255, 255);
	  break;
	  //
	  case 26:
		fill_gradient(MinLeds, temp, CHSV(175,255,255), CHSV(1,255,255), LONGEST_HUES);
		hRing.setAllHSV(1, 255, 255);
	  break;
	  //
	  default:
	  break;
	}
  }
}


/* ----------------------------------------------------------------------------------------------------*/
void CLOCK_SetTime(void){
	int pot_hour = 0;
	int hour_pos;
	int pot_min = 0;
	int min_pos = 0;
	int pot_val[5];

	CLOCK_FadeToBlack();

	FastLED.delay(500);

	// Hour setup
	hRing.displayCompas(128); //aqua compas
	FastLED.delay(500);

	while(digitalRead(BTN1_PIN) != 0){
		pot_hour = analogRead(POT_PIN);
		pot_hour = map(pot_hour, 0, 1023, 1, 12);

		hour_pos = ((pot_hour*2)-1);

		nscale8(HourLeds,NUM_LED_HOUR,250);

		hRing.displayCompas(128);
		hRing.setHSV(hour_pos, 40, 255, 255);   
		hRing.setHSV(hour_pos-1, 40, 255, 255);

		FastLED.show();
	}

	FastLED.delay(500);

	//minute setup
	mRing.displayCompas(128);
	FastLED.delay(500);

	while(digitalRead(BTN1_PIN) != 0){

		//get average value of 5 readings
		pot_min = 0;
		for(uint8_t i = 0; i < 5; i++){
			pot_val[i] = analogRead(POT_PIN);
			pot_min += pot_val[i];
		}
		pot_min /= 5;
		pot_min = map(pot_min, 0, 1023, 0, NUM_LED_MIN-1);

		//In case of wrong aligment you can fix it here
		min_pos = pot_min;

		//dimm the lights
		nscale8(MinLeds, NUM_LED_MIN, 250);

		//show position and compas
		mRing.displayCompas(128);
		mRing.setHSV((min_pos), 40, 255, 255);
		FastLED.show();
	}

	Clock.setHour(pot_hour);
	Clock.setMinute(pot_min);
	Clock.setSecond(0);

	CLOCK_FadeToBlack();
}


/* ----------------------------------------------------------------------------------------------------*/
void CLOCK_DisplayFirework(void)
{
	CLOCK_FadeToBlack();  

	uint8_t racket_pos, racket_go = 0, racket_color = 35, racket_direction;

	//CLOCK_FadeToBlack(); 

	racket_direction = random8(2);

	FastLED.delay(300);

	mRing.setHSV(30, racket_color, 255, 255);
	FastLED.show();

	if(racket_direction){
		// Go right
		while(1){
			racket_pos = beatsin8(15, 0, 30);

			if(racket_pos == 30){
				racket_go = 1;
				FastLED.delay(100);
			}

			if(racket_go){
				mRing.setBlack();
				mRing.setHSV(racket_pos, racket_color, 255, 255);
				FastLED.show();

				if(racket_pos == 0){
					break;
				}
			}
		}
	} else{
		// Go left
		while(1)
		{
			racket_pos = beatsin8(15, 30, 59);

			if(racket_pos == 30){
				racket_go = 1;
				//FastLED.delay(100);
			}

			if(racket_go){
				mRing.setBlack();
				mRing.setHSV(racket_pos, racket_color, 255, 255);
				FastLED.show();

				if(racket_pos == 59){
					FastLED.delay(80);
					mRing.setBlack();
					mRing.setHSV(0, racket_color, 255, 255);
					FastLED.show();
					break;
				}
			}
		}
	}

	// Rocket on the top
	FastLED.delay(100);

	for(uint8_t i=0; i<20; i++){
		MinLeds[0].fadeToBlackBy(16);
		FastLED.delay(10);
	}

	// Explosion with hour ring
	hRing.setAllHSV(20,180,120);

	// Fade to black
	for(uint8_t i=0; i<40; i++){
		MinLeds[0].fadeToBlackBy(16);
		for(int k=0; k<NUM_LED_HOUR; k++){
			HourLeds[k].fadeToBlackBy(16);
		}
		FastLED.delay(30);
	}


	// Show the sparks
	uint8_t spark[6], spark_color[6];
	
	spark[0] = random8(1, 5);
	spark[1] = random8(55, 59);
	spark[2] = random8(5, 10);
	spark[3] = random8(50, 55);
	spark[4] = random8(10, 15);
	spark[5] = random8(45, 50);

	for(uint8_t i=0; i<6; i++){
		spark_color[i] = random8(0, 255);
	}

	for(uint8_t i = 0; i < 6; i++)
	{
		MinLeds[spark[i]].setHSV(spark_color[i], 250, 250);
		FastLED.show();
		FastLED.delay(random8(30,100));
	}
	

	// Drop them down
	uint8_t dilej = 50, sajn = 250;

	for(uint8_t down = 1; down < 14; down++)
	{
		fadeToBlackBy(MinLeds, NUM_LED_MIN, 200);
		for(uint8_t j = 0; j < 6; j++){
			if(j%2 == 0){
				spark[j]++;
			}
			else{
				spark[j]--;
			}	

			MinLeds[spark[j]].setHSV(spark_color[j], 250, sajn);
		}
		FastLED.show();
		FastLED.delay(dilej);
		dilej += 7;
		sajn -= 20;
	}
	mRing.setBlack();
	FastLED.show();
}


/* ----------------------------------------------------------------------------------------------------*/
void CLOCK_MaintainanceMode(void){
	 // For displaying position / orientation
	mRing.setBlack();
	hRing.setBlack();
	mRing.setHSV(0, 255, 255, 255);
	mRing.setHSV(15, 255, 255, 255);
	mRing.setHSV(30, 255, 255, 255);
	mRing.setHSV(45, 255, 255, 255);

	hRing.displayCompas(255);

	FastLED.show();

	// Wait until the button is released
	while(!digitalRead(BTN1_PIN)){}

	FastLED.delay(300);

	// Wait until the button is pressed again
	while(digitalRead(BTN1_PIN)){}

	CLOCK_FadeToBlack();  

	myTime t;
	t.hour = 12;
	t.min = 15;
	CLOCK_DisplayQuarter(255, t);
	CLOCK_FadeToBlack();  
	if(digitalRead(BTN1_PIN) == 0){
		return;
	}

	CLOCK_DisplayHalfHour(150);
	CLOCK_FadeToBlack();  
	if(digitalRead(BTN1_PIN) == 0){
		return;
	}

	t.min = 45;
	CLOCK_DisplayTriQuarter(255, t);
	CLOCK_FadeToBlack();  
	if(digitalRead(BTN1_PIN) == 0){
		return;
	}

	CLOCK_DisplayFullHour(130);
	if(digitalRead(BTN1_PIN) == 0){
		return;
	}

	while(1){
		CLOCK_DisplayFirework();

		if(digitalRead(BTN1_PIN) == 0){
			break;
		}
	}	
}
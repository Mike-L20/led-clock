/* --------------------------------------------------------------------------------------------------
 * TODO
 * 
 * Ver.:  8.1
 * Auth.: Grega Morano
 * Date.: 7.6.2020
 * 
 * --------------------------------------------------------------------------------------------------*/

#include "LEDclock.h"

/* ----------------------------------------------------------------------------------------------------*/
/*-------------- GLOBAL VARIABLES ---------------------------------------------------------------------*/
/* ----------------------------------------------------------------------------------------------------*/

// Extern - from LEDclock.ino
extern CRGB HourLeds[NUM_LED_HOUR];
extern CRGB MinLeds[NUM_LED_MIN];


/* ----------------------------------------------------------------------------------------------------*/
/*--------------- TYPEDEFs ----------------------------------------------------------------------------*/
/* ----------------------------------------------------------------------------------------------------*/

typedef struct myTime{
  int hour;
  int min;
  int sec;
};

typedef struct user_color{
  uint8_t compas;
  uint8_t hour_color;
  uint8_t min_color;
};


/* ----------------------------------------------------------------------------------------------------*/
/*--------------- HOUR RING CLASS ---------------------------------------------------------------------*/
/* ----------------------------------------------------------------------------------------------------*/

class HourRing{
  public:
    uint8_t px_hou;
    uint8_t last_compas_hue;

    //Constructor
    HourRing();

    //Functions for mode: lamp
    void setBlack();
    void setWhite();
    void setRGB(int pixel, uint8_t r, uint8_t g, uint8_t b);
    void setHSV(int pixel, uint8_t h, uint8_t s, uint8_t v);
    void setAllHSV(uint8_t h, uint8_t s, uint8_t v);
    void rainbow(uint8_t start_hue);
    void colorWipe(int hue, int wait, int start_px);

    //Functions for mode: clock
    void updateClockPosition(myTime t);
    void displayCompas(uint8_t hue);
    void displayClockVariableColor(int color);
    void displayClockUserColor(user_color uc);
    void displayClockPredefinedColor(int fact); 

    uint8_t getCompasHue(void);
};


/* ----------------------------------------------------------------------------------------------------*/
/*--------------- MIN RING CLASS ----------------------------------------------------------------------*/
/* ----------------------------------------------------------------------------------------------------*/

class MinRing{
  public:
    uint8_t px_min;
    uint8_t px_sec;
    uint8_t px_mil;

    //Constructor
    MinRing();

    //Functions for mode: lamp
    void setBlack();
    void setWhite();
    void setRGB(int pixel, uint8_t r, uint8_t g, uint8_t b);
    void setHSV(int pixel, uint8_t h, uint8_t s, uint8_t v);
    void setAllHSV(uint8_t h, uint8_t s, uint8_t v);
    void rainbow(uint8_t start_hue);
    void rainbowFaling (int hue);


    //With FastLED.show() in the function
    void colorWipe(int hue, int wait, int start_px);
    void quarterWipe(int hue, int wait);
    void rainbowCycle(int delay_time);
    void rainbowFillin(uint8_t start_hue);

    //Functions for mode: clock
    void updateClockPosition(myTime t);
    void displayCompas(uint8_t hue);
    void displayCompasBig(uint8_t hue);
    void displayCompasSmall(uint8_t hue);
    void displayClockUserColor(user_color uc);
    void displayClockPredefinedColor(int fact);
    void displayClockVariableColor(int color);
};



#ifndef _BUTTONS_H
#define _BUTTONS_H
#include "stm32f4xx.h"
#include "bastypes.h"
#include "stdbool.h"

#define BUTTONS_COUNT 6
typedef enum
{
  BUTTON_2_BRIGHT = 0,
  BUTTON_3_LEFT,
  BUTTON_4_RIGHT,
  BUTTON_5_UP,
  BUTTON_6_DOWN,
  BUTTON_7_MENU,  
}buttonsNumber_t;

typedef enum
{
  BTN_PRESS = 1,
  BTN_RELEASE =0
}buttonsState_t;

void buttons_init();
void buttons_EXTIProcessing(buttonsNumber_t buttonsNumb);
void buttons_timerProcessing(buttonsNumber_t buttonsNumb);
uint8_t buttons_getState(buttonsNumber_t buttonNumb);
uint8_t buttons_isChange(buttonsNumber_t buttonNumb);
tU8 buttons_getBitmap();


#endif
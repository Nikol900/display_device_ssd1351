#ifndef _DISPLAY_H
#define _DISPLAY_H
#include "bastypes.h"
#include "stm32f4xx.h"
#include "ssd1351.h"
#include "fonts.h"

#include "stdbool.h"



#define DISPLAY_DEFAULT_SYMBOL ' '

#define DISPLAY_SYMBOL_START_NUM 32
//#define DISPLAY_SYMBOL_END_NUM 255-6//черный список из 6 

#define DISPLAY_BRIGTHNESS_100  100
#define DISPLAY_BRIGTHNESS_0    0

typedef enum
{
  MODE_4_LINE = 0,
  MODE_5_LINE,
  MODE_6_LINE,
}display_mode_t;


void Display_init();

bool Display_setBrightnessLvl(uint8_t lvl);
bool display_setMode(display_mode_t displayMode);

void display_clearAll();
bool display_drawFormatString(tU8 rowNumb,tU8 rowLen, tU8 *rowData);
bool display_drawFormatAllStrings(tU8 dataLen, tU8 *rowData);
bool display_drawPartialString(tU8 rowNumb,tU8 colNumb, tU8 rowLen, tU8 *rowData);

void display_drawFillOver(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,ssd1351_Colors_t color);
void Display_testDrawSymbols();
#endif
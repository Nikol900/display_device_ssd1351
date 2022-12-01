#ifndef _SSD1305_H
#define _SSD1305_H
#include "stm32f4xx.h" 
#include "bastypes.h"
#include "fonts.h"

#define DMA_BUFFER_SIZE 1024//1024


// #RES pin action
#define LCD_RES_LO GPIO_ResetBits(GPIOB, GPIO_Pin_0)//(GPIOB->BSRRH = GPIO_Pin_0)//GPIO_ResetBits(GPIOB, GPIO_Pin_0)
#define LCD_RES_HI GPIO_SetBits(GPIOB, GPIO_Pin_0)//(GPIOB->BSRRL = GPIO_Pin_0)//GPIO_SetBits(GPIOB, GPIO_Pin_0)

// D/#C pin action
#define LCD_COMMAND GPIO_ResetBits(GPIOA, GPIO_Pin_6) //(GPIOA->BSRRH = GPIO_Pin_6) //GPIO_ResetBits(GPIOA, GPIO_Pin_6)
#define LCD_DATA    GPIO_SetBits(GPIOA, GPIO_Pin_6)//(GPIOA->BSRRL = GPIO_Pin_6) //GPIO_SetBits(GPIOA, GPIO_Pin_6)

// NSS pin action
#define LCD_NSS_LO GPIO_ResetBits(GPIOA, GPIO_Pin_4)//(GPIOA->BSRRH = GPIO_Pin_4) //GPIO_ResetBits(GPIOA, GPIO_Pin_4)
#define LCD_NSS_HI GPIO_SetBits(GPIOA, GPIO_Pin_4)//(GPIOA->BSRRL = GPIO_Pin_4) //GPIO_SetBits(GPIOA, GPIO_Pin_4)
// SSD1351 size params
#define SSD1351_WIDTH  128
#define SSD1351_HEIGHT 128


extern tU8 dmaDisplaylBuffer[DMA_BUFFER_SIZE];

//display color in 8bit format
//#define SSD1351_COLOR565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))
typedef enum DISLAY_COLORS
{
  SSD1351_BLACK   = 0x0000,
  SSD1351_BLUE    = 0x001F,
  SSD1351_RED     = 0xF800,
  SSD1351_GREEN   = 0x07E0,
  SSD1351_CYAN    = 0x07FF,
  SSD1351_MAGENTA = 0xF81F,
  SSD1351_YELLOW  = 0xFFE0,
  SSD1351_WHITE   = 0xFFFF,
}ssd1351_Colors_t;


typedef enum DISPLAY_COMMANDS
{
  SSD1351_CONTRASTABC           = 0xC1,
  SSD1351_CONTRASTMASTER        = 0xC7,
  SSD1351_SLEEP_MODE_ON         = 0xAE,         
  SSD1351_SLEEP_MODE_OFF        = 0xAF,
}ssd1351_commands_t;

void SSD1351_SetBrightness(uint8_t val);

void SSD1351_Init(bool rotate180);


void ssd1351_setCommand(ssd1351_commands_t cmd, uint8_t *data, uint8_t data_size);

void ssd1351_nss_on();
void ssd1351_nss_release();
void ssd1351_cs_on();
void ssd1351_cs_release();
void ssd1351_data_on();
void ssd1351_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) ;
void ssd1351_test_fill(uint16_t color);

#endif // _SSD1305_H
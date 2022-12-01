#include "stm32f4xx.h"
#include "ssd1351.h"

void write_cmd(unsigned char);
void write_data(uint8_t *data,uint16_t dataSize);
void SPI_SendByte(unsigned char byte);


 void ssd1351_nss_on() 
{
  LCD_NSS_LO;
}

void ssd1351_nss_release()
{
  LCD_NSS_HI;
}
static void ssd1351_Reset() {
   LCD_RES_HI;
   for(u8 i=0x070;i>0;i--){}//delay to establish work
   LCD_RES_LO;
   for(u8 i=0x070;i>0;i--){}//delay to establish work
   LCD_RES_HI;
   for(u8 i=0x070;i>0;i--){}//delay to establish work 
}

void SSD1351_Init(bool rotate180)
{
  
  //https://www.eevblog.com/forum/beginners/requesting-help-with-powering-down-an-ssd1351-display-module/
    ssd1351_nss_on();
    ssd1351_Reset();
    write_cmd(0xFD); // COMMANDLOCK
    {
        uint8_t data[] = { 0x12 };
        write_data(data, sizeof(data));
    }
    write_cmd(0xFD); // COMMANDLOCK
    {
        uint8_t data[] = { 0xB1 };
        write_data(data, sizeof(data));
    }
    write_cmd(0xAE); // DISPLAYOFF
    write_cmd(0xB3); // CLOCKDIV
    write_cmd(0xF1); // 7:4 = Oscillator Frequency, 3:0 = CLK Div Ratio (A[3:0]+1 = 1..16)
    write_cmd(0xCA); // MUXRATIO
    {
        uint8_t data[] = { 0x7F }; // 127
        write_data(data, sizeof(data));
    }
    write_cmd(0xA0); // SETREMAP
    {
      uint8_t data[] = { 0x74 };//0x66
      if(rotate180)
        data[0] = 0x66;   
        write_data(data, sizeof(data));
    }
    write_cmd(0x15); // SETCOLUMN
    {
        uint8_t data[] = { 0x00, 0x7F };
        write_data(data, sizeof(data));
    }
    write_cmd(0x75); // SETROW
    {
        uint8_t data[] = { 0x00, 0x7F };
        write_data(data, sizeof(data));
    }
    write_cmd(0xA1); // STARTLINE
    {
        uint8_t data[] = { 0x00 }; // 96 if display height == 96
        write_data(data, sizeof(data));
    }
    write_cmd(0xA2); // DISPLAYOFFSET
    {
        uint8_t data[] = { 0x00 };
        write_data(data, sizeof(data));
    }
    write_cmd(0xB5); // SETGPIO
    {
        uint8_t data[] = { 0x00 };
        write_data(data, sizeof(data));
    }
    write_cmd(0xAB); // FUNCTIONSELECT
    {
        uint8_t data[] = { 0x01 };
        write_data(data, sizeof(data));
    }
    write_cmd(0xB1); // PRECHARGE
    {
        uint8_t data[] = { 0x32 };
        write_data(data, sizeof(data));
    }
    
    
    
    write_cmd(0xBE); // VCOMH
    {
        uint8_t data[] = { 0x05 };
        write_data(data, sizeof(data));
    }
    write_cmd(0xA6); // NORMALDISPLAY (don't invert)
    write_cmd(0xC1); // CONTRASTABC
    {
       // uint8_t data[] = { 0xC8, 0x80, 0xC8 };
        uint8_t data[] = { 0x20, 0x20, 0x20};
        write_data(data, sizeof(data));
    }
    write_cmd(0xC7); // CONTRASTMASTER
    {
      uint8_t data[] = {0x08};//{ 0x0F };
        write_data(data, sizeof(data));
    }
    write_cmd(0xB4); // SETVSL
    {
        uint8_t data[] = { 0xA0, 0xB5, 0x55 };
        write_data(data, sizeof(data));
    }
    write_cmd(0xB6); // PRECHARGE2
    {
        uint8_t data[] = { 0x01 };
        write_data(data, sizeof(data));
    }
    write_cmd(0xAF); // DISPLAYON

  ssd1351_nss_release();
}

void ssd1351_setCommand(ssd1351_commands_t cmd, uint8_t *data, uint8_t data_size) {
  ssd1351_nss_on();
  write_cmd(cmd); 
  if(data_size)
    write_data(data, data_size);    
  ssd1351_nss_release();  
}

void SSD1351_SetBrightness(uint8_t lvl)
{
  ssd1351_nss_on();
  uint8_t colorABC = 0; // 0x10 - 0xFF
  uint8_t colorCurrContrast = 0;//0x00- 0x0f
  switch(lvl)
  {        
    case 10: {colorABC = 0xff;colorCurrContrast = 0x0f;break;}//max
    case 9: {colorABC = 0xc0;colorCurrContrast = 0x0d;break;}
    case 8: {colorABC = 0xb0;colorCurrContrast = 0x0b;break;}
    case 7: {colorABC = 0xa0;colorCurrContrast = 0x0a;break;}
    case 6: {colorABC = 0x90;colorCurrContrast = 0x08;break;}
    case 5: {colorABC = 0x80;colorCurrContrast = 0x06;break;}
    case 4: {colorABC = 0x70;colorCurrContrast = 0x05;break;}
    case 3: {colorABC = 0x60;colorCurrContrast = 0x04;break;}
    case 2: {colorABC = 0x50;colorCurrContrast = 0x03;break;}
    case 1: {colorABC = 0x40;colorCurrContrast = 0x02;break;}//min
    case 0: {colorABC = 0x40;colorCurrContrast = 0x01;break;}//off
  }
      
      

  {
     write_cmd(0xC1); // CONTRASTABC
    {
       // uint8_t data[] = { 0xC8, 0x80, 0xC8 };
       // uint8_t data[] = { 0xff, 0xff, 0xff};
       uint8_t data[] = { colorABC, colorABC, colorABC};
        write_data(data, sizeof(data));
    }
  }
 

      
    write_cmd(0xC7); // CONTRASTMASTER
    {
      //uint8_t data[] = {val};//{ 0x0F };
      uint8_t data[] = {colorCurrContrast};//{ 0x0F };
      
        write_data(data, sizeof(data));
    }
      ssd1351_nss_release();
    /*
    > 0 lvl here
    */
        //write_cmd(0xAF); // DISPLAYON/
   
}
void ssd1351_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) 
{
    // column address set
    write_cmd(0x15); // SETCOLUMN
    {
        uint8_t data[] = { x0 & 0x7F, x1 & 0x7F };
        write_data(data, sizeof(data));
    }
    // row address set
    write_cmd(0x75); // SETROW
    {
        uint8_t data[] = { y0 & 0x7F, y1 & 0x7F };
        write_data(data, sizeof(data));
    }
    // write to RAM
    write_cmd(0x5C); // WRITERAM
}

void SPI_SendByte(unsigned char byte)
{
   //  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
       while(((SPI1->SR & SPI_I2S_FLAG_BSY) != (uint16_t)RESET));

//     LCD_NSS_LO;
     /* Send 16-bit word through the SPI1 peripheral */
//     SPI_I2S_SendData(SPI1, byte);
       SPI1->DR = byte;
     /* Loop while BUSY in communication or TX buffer not emplty
      */
     while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
//       while(((SPI1->SR & SPI_I2S_FLAG_BSY) != (uint16_t)RESET));
//     LCD_NSS_HI;
}



void write_cmd(unsigned char cmd)
{
  LCD_COMMAND;
  
  for(u32 i=0x070;i>0;i--){}//delay to establish work0x010
  (void)SPI_SendByte(cmd);
}

void write_data(uint8_t *data,uint16_t dataSize)
{
  LCD_DATA;
  for(u32 i=0x070;i>0;i--){}//delay to establish work0x010
  for(uint8_t i = 0 ; i < dataSize; i++)
  (void)SPI_SendByte(data[i]);
}



void ssd1351_test_fill(uint16_t color)
{
  ssd1351_nss_on();
  ssd1351_SetAddressWindow(0, 0, SSD1351_WIDTH-1, SSD1351_HEIGHT-1);
  //uint16_t color = ssd1351_RED;
  uint8_t data[] = { color >> 8, color & 0xFF };
  LCD_DATA;
      for(uint8_t y = SSD1351_WIDTH; y > 0; y--) {
        for(uint8_t x = SSD1351_WIDTH; x > 0; x--) {
          SPI_SendByte(data[0]);
          SPI_SendByte(data[1]);
          //  HAL_SPI_Transmit(&ssd1351_SPI_PORT, data, sizeof(data), HAL_MAX_DELAY);
        }
    }
    ssd1351_nss_release();         
}



////////////////////////////////
////////////////////////////////
////////////////////////////////
void ssd1351_cs_release()
{
  for(u8 i=0x70;i>0;i--){}//delay to establish work
  for(u8 i=0x070;i>0;i--){}//delay to establish work
  for(u8 i=0x070;i>0;i--){}//delay to establish work
  LCD_NSS_HI;
}
void ssd1351_cs_on()
{
  LCD_NSS_LO;
}
void ssd1351_data_on()
{
  LCD_DATA;
}
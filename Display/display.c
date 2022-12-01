#include "display.h"
#include "string.h"
#include "crc16.h"
//#define DMA_BUFFER_SIZE 4096//1024
#define OP_SIZE 2//размер одной точки (2 байта)


#define COLOR_FRONT SSD1351_GREEN
#define COLOR_BACK SSD1351_BLACK

#define MAX_ROW_LEN 8
#define MAX_ROW_NUMB 6
#define SYMBOL_SPEC_START_POS 18
#define SYMBOL_SPEC_END_POS (SYMBOL_SPEC_START_POS+8)
tU8 dmaDisplaylBuffer[DMA_BUFFER_SIZE];//16x32*2

display_mode_t DisplayMode = MODE_4_LINE;

typedef enum
{
  STYLE_NORMAL = 0,
  STYLE_UNDERLINED = 2,
}font_styles_t;


/////////prots
static inline void dmaProcessingFull(tU32 currentBufferSize);
FontType display_getFontForStyle(font_styles_t fontStyle);
bool isEnverted(tU8 dataByte);
tU8 checkRowLen(tU8 rowLen);
tU8 checkRowNumb(tU8 rowNumb);

void display_drawCharacter(char ch,tU8 x, tU8 y,FontType font, tU16 colorFront, tU16 colorBack,bool inverse);

bool isFontStyleValid(tU8 *data, tU8 dataLen);
//////////////




//* * * * * * * * * * Init * * * * * * * * 
void Display_init() {
  
  EXTI->IMR |= EXTI_Line0;//init EXTI 0
  SSD1351_Init(true);
  display_drawFillOver(0,0,SSD1351_WIDTH-1, SSD1351_HEIGHT-1,SSD1351_BLACK); 
}

//* * * * * * * * Modes * * * * * * * * * * * 

bool display_setMode(display_mode_t displayMode)
{
  if((displayMode == MODE_4_LINE) || 
     (displayMode == MODE_5_LINE) || 
     (displayMode == MODE_6_LINE))
  { 
    //When changing the mode, clean the screen
    if (DisplayMode != displayMode)
      display_clearAll();
    DisplayMode = displayMode;
    return true;
  }
  return false;
}

//* * * * * * * BrightnessLvl * * * * * * 
bool Display_setBrightnessLvl(uint8_t lvl) {
 
  if(lvl > DISPLAY_BRIGTHNESS_100) return false;
  uint8_t colorABC = 0; // 0x10 - 0xFF
  uint8_t colorCurrContrast = 0;//0x00- 0x0f
  
  colorABC = (tU8) (0xbf * lvl / 100);//(0xff-0x40)  * lvl
  colorCurrContrast = (tU8) (0x0E * lvl / 100); //0x0f-0x01
//  switch(lvl)
//  {        
//    case 10: {colorABC = 0xff;colorCurrContrast = 0x0f;break;}//DISPLAY_BRIGTHNESS_100
//    case 9: {colorABC = 0xc0;colorCurrContrast = 0x0d;break;}
//    case 8: {colorABC = 0xb0;colorCurrContrast = 0x0b;break;}
//    case 7: {colorABC = 0xa0;colorCurrContrast = 0x0a;break;}
//    case 6: {colorABC = 0x90;colorCurrContrast = 0x08;break;}
//    case 5: {colorABC = 0x80;colorCurrContrast = 0x06;break;}
//    case 4: {colorABC = 0x70;colorCurrContrast = 0x05;break;}
//    case 3: {colorABC = 0x60;colorCurrContrast = 0x04;break;}
//    case 2: {colorABC = 0x50;colorCurrContrast = 0x03;break;}
//    case 1: {colorABC = 0x40;colorCurrContrast = 0x02;break;}//min
//    case 0: {colorABC = 0x40;colorCurrContrast = 0x01;break;}//DISPLAY_BRIGTHNESS_0
//  }
   
   uint8_t data[] = { colorABC, colorABC, colorABC};  
   ssd1351_setCommand(SSD1351_CONTRASTABC,data,3);  
   ssd1351_setCommand(SSD1351_CONTRASTMASTER,&colorCurrContrast,1);
   if(lvl)
     ssd1351_setCommand(SSD1351_SLEEP_MODE_OFF,0,0);
   else
     ssd1351_setCommand(SSD1351_SLEEP_MODE_ON,0,0);
   return true;
}

bool isFontStyleValid(tU8 *data, tU8 dataLen) {
  for(tU8 i = 0 ; i < dataLen; i+=2)   
    if (data[i] > 3)
    return false;
  return true;
}


//* * * * * * * TextOutput * * * * * * *

void display_clearAll() {  
  display_drawFillOver(0, 0, SSD1351_WIDTH-1, SSD1351_HEIGHT-1,COLOR_BACK);
}
/* Cleaning the first two lines for 5,6 modes*/
void display_clearUpDwnLines(tU8 rowNumb) {
  /*In 5-line and 6-line mode, 
  let's clear the two empty pixel rows above the first line 
  whenever the first line is modified, and in 5-line mode, 
  let's clear that one empty pixel row below the last 
  line whenever the last line is modified
  */
  if ((rowNumb == 0) && (DisplayMode != MODE_4_LINE))   
  {
    display_drawFillOver(0, 0, SSD1351_WIDTH-1, 2-1,COLOR_BACK);
  } 
  if((DisplayMode == MODE_5_LINE) && rowNumb == 4)
     display_drawFillOver(0, SSD1351_HEIGHT-1, SSD1351_WIDTH-1, SSD1351_HEIGHT-1,COLOR_BACK);    
}

bool display_drawFormatString(tU8 rowNumb,tU8 rowLen, tU8 *rowData) {
  
  //if the line number does not correspond to the mode
  if (rowNumb != checkRowNumb(rowNumb)) return false; //NROW > NROWMAX

  rowLen = checkRowLen(rowLen);
  
  tU8 shiftX = 0;
  tU8 symbNumb;
  //If there is a unimportant type of font
  if (!isFontStyleValid(rowData,rowLen)) return false;
  
 // display_drawFillOver(0,0,SSD1351_WIDTH-1, SSD1351_HEIGHT-1,SSD1351_BLUE); 
  for(symbNumb = 0 ; symbNumb < rowLen; symbNumb++)
  {
    //get type
    FontType font = display_getFontForStyle((font_styles_t)rowData[2*symbNumb]);
 
    display_drawCharacter(rowData[OP_SIZE*symbNumb+1], shiftX, rowNumb*font.height+font.yShift, font,COLOR_FRONT,COLOR_BACK,isEnverted(rowData[2*symbNumb]));
    shiftX += font.width; 
  }

  //add space to end of str
  FontType font = display_getFontForStyle((font_styles_t)(rowData[0]&0x01));//not use undrerline
  for( ; symbNumb < 8 ; symbNumb++)
  {
    display_drawCharacter(DISPLAY_DEFAULT_SYMBOL,shiftX,rowNumb*font.height+font.yShift,font,COLOR_FRONT,COLOR_BACK,false);
    shiftX += font.width;      
  } 
  
  //Clean unnecessary pixels
  display_clearUpDwnLines(rowNumb);
  return true;
}

bool display_drawFormatAllStrings(tU8 dataLen, tU8 *rowData){
    
  dataLen = dataLen > MAX_ROW_LEN*MAX_ROW_NUMB ? MAX_ROW_LEN*MAX_ROW_NUMB : dataLen;

  tU8 rowLen = dataLen > MAX_ROW_LEN ? MAX_ROW_LEN : dataLen;
  tU8 maxRowNumb = checkRowNumb(MAX_ROW_NUMB);
  
  if (!isFontStyleValid(rowData,dataLen)) return false;
  
  
  for(tU8 rowNumb = 0; (rowNumb <= maxRowNumb) && (rowLen > 0); rowNumb++)//On all lines
  {
    display_drawFormatString(rowNumb,rowLen,&rowData[rowNumb*MAX_ROW_LEN*OP_SIZE]);
    display_clearUpDwnLines(rowNumb);    
    dataLen -= rowLen;
    rowLen = dataLen > MAX_ROW_LEN ? MAX_ROW_LEN : dataLen;
  }
  return true;
}

bool display_drawPartialString(tU8 rowNumb,tU8 colNumb, tU8 rowLen, tU8 *rowData ) {
  
  //With 0x03, we aren't getting a NACK when NCOL > 8 in the 4-line mode, 5-line mode, or 6-line mode
  if (rowLen > MAX_ROW_LEN) return false;
  
  if (colNumb > MAX_ROW_LEN-1) return false; //
  
  //If N + NCOL - 1 > 8, then the string gets truncated
  if ( rowLen + colNumb > MAX_ROW_LEN)
    rowLen =MAX_ROW_LEN - colNumb;
  
  if (rowNumb != checkRowNumb(rowNumb)) return false;
  
  rowNumb = checkRowNumb(rowNumb);
  
  FontType font = display_getFontForStyle((font_styles_t)rowData[0]);
  tU8 shiftX = colNumb*font.width; 
 
  if (!isFontStyleValid(rowData,rowLen)) return false; 
  
  for(tU8 symbNumb = 0 ;symbNumb < rowLen; symbNumb++)
  {
    FontType font = display_getFontForStyle((font_styles_t)rowData[2*symbNumb]);
 
    display_drawCharacter(rowData[OP_SIZE*symbNumb+1], shiftX, rowNumb*font.height+font.yShift, font,COLOR_FRONT,COLOR_BACK,isEnverted(rowData[2*symbNumb]));
    shiftX += font.width; 
  }  
// Clean unnecessary pixels
  display_clearUpDwnLines(rowNumb);
  return true;
} 

static inline tU16 loadDmaBuffer(const tU16 *data,tU8 height, tU8 width,ssd1351_Colors_t colorFront, ssd1351_Colors_t colorBack,bool inverse)
{
  //prepare buff
  tU32 i, b, j,k = 0;
  for(i = 0; i < height; i++) 
  {
     b = *data++;
    if(inverse) 
      b =~b;
    for(j = 0; j < width; j++) 
    {
      if((b << j) & 0x8000)//for each byte in a pack from the older to the worst
      {
        dmaDisplaylBuffer[k++] = colorFront>>8;
        dmaDisplaylBuffer[k++] = colorFront&0xFF;            
      } 
      else
      {
        dmaDisplaylBuffer[k++] = colorBack>>8;
        dmaDisplaylBuffer[k++] = colorBack&0xFF;
      }
    }
  } 
  return k;
}
void display_drawCharacter(char ch,tU8 x, tU8 y,FontType font, ssd1351_Colors_t colorFront, ssd1351_Colors_t colorBack,bool inverse) {

  tS16 strPos = font.symbMatch[ch];
  if(strPos == -1)//replace any empty to SPACE
    strPos = font.symbMatch[DISPLAY_SYMBOL_START_NUM];
  tU16 buff_size = loadDmaBuffer(&font.data[strPos * font.height],font.height,font.width,colorFront,colorBack,inverse);
  
// occurs from time to time artifacts that are associated with the display itself
  // It is decided to send each symbol 2 times.
  // Speed ??allows
  tU8 cnt = 2;
  
  while(cnt) {
    ssd1351_cs_on();     
    ssd1351_SetAddressWindow(x, y, x+font.width-1, y+font.height-1);
    ssd1351_data_on();
    dmaProcessingFull(buff_size);
    ssd1351_cs_release();
    cnt--;
  }
} 


FontType display_getFontForStyle(font_styles_t fontStyle)
{

  fontStyle =(font_styles_t) (fontStyle&0x2); //Only the second bit
  if(DisplayMode == MODE_4_LINE)
  {
    switch(fontStyle)
    {
    case STYLE_NORMAL:
      return Font_4_Normal;
    case STYLE_UNDERLINED:
       return Font_4_Underlined;
    default:
      return Font_4_Normal;
    }
  }
  if(DisplayMode == MODE_5_LINE)
  {
    switch(fontStyle)
    {
    case STYLE_NORMAL:
      return Font_5_Normal;
    case STYLE_UNDERLINED:
       return Font_5_Underlined;
    default:
      return Font_5_Normal;
    }
  }
  if(DisplayMode == MODE_6_LINE)
  {
    switch(fontStyle)
    {
    case STYLE_NORMAL:
      return Font_6_Normal;
    case STYLE_UNDERLINED:
       return Font_6_Underlined;
    default:
      return Font_6_Normal;
    }
  }
   return Font_4_Normal;
}
bool isEnverted(tU8 dataByte) {
  if (dataByte &0x01)
    return true;
  return false;
}
tU8 checkRowLen(tU8 rowLen) {
  return rowLen < MAX_ROW_LEN ? rowLen : MAX_ROW_LEN;
}
tU8 checkRowNumb(tU8 rowNumb) {
  tU8 maxRowNumb;
  if(DisplayMode == MODE_4_LINE) maxRowNumb = 4-1;
  if(DisplayMode == MODE_5_LINE) maxRowNumb = 5-1;
  if(DisplayMode == MODE_6_LINE) maxRowNumb = 6-1;
  
  return rowNumb < maxRowNumb ? rowNumb : maxRowNumb; 
}


       
       
static inline tU32 calcTotalBufferSize(uint16_t width,uint16_t height) {
  return OP_SIZE *width * height;
}
/*recalculation of the total length and current length for sending */
static inline void calcBufferSize(tU32 *totalBufferSize, tU32 *currentBufferSize,tU32 MAX_SIZE) {
  *currentBufferSize = *totalBufferSize > MAX_SIZE ? MAX_SIZE : *totalBufferSize; 
  *totalBufferSize -= *currentBufferSize;
}
/* Setting a new length of the package for sending by DMA */
tU32 dmaDisplayBufferSize = 0;
static inline void dmaSetBufferSize() {
  DMA2_Stream3->CR &= ~(tU32)DMA_SxCR_EN;//disable DMA
  DMA2->LIFCR = (tU32) (DMA_FLAG_FEIF3 | DMA_FLAG_DMEIF3 | DMA_FLAG_TEIF3 | DMA_FLAG_HTIF3 | DMA_FLAG_TCIF3);//clear flags
  DMA2_Stream3->NDTR = dmaDisplayBufferSize;//set buff size
}
/* Source change for DMA */
static inline void dmaSetBuffer() {
  DMA2_Stream3->M0AR = (tU32) dmaDisplaylBuffer; //set buff
  DMA2_Stream3->CR |= (tU32)DMA_SxCR_EN;//enable dma
}
/* Enjoyment of the end of sending DMA*/
static inline void dmaProcessing() {
  //while(!DMA_GetFlagStatus(DMA2_Stream3,DMA_FLAG_TCIF3))
  while(!(DMA2->LISR & DMA_FLAG_TCIF3))//while !TC
    ;
  DMA2->LIFCR = (tU32) (DMA_FLAG_TCIF3);//clear flag TC
}
/* Full set of actions with DMA */
static inline void dmaProcessingFull(tU32 currentBufferSize) {
  dmaDisplayBufferSize = currentBufferSize;
  dmaSetBufferSize();
  dmaSetBuffer();
  dmaProcessing();
}


/* Fill in one color */
void display_drawFillOver(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,ssd1351_Colors_t color) {
  ssd1351_cs_on();
  tU32 totalBufferSize = calcTotalBufferSize((y2-y1+1),(x2-x1+1));//
  tU32 currentBufferSize = totalBufferSize > DMA_BUFFER_SIZE ? DMA_BUFFER_SIZE: totalBufferSize;//
 // startCountDWT();
    for (tU32 i = 0; i <currentBufferSize; i += OP_SIZE)
    {
      dmaDisplaylBuffer[i] = color>>8;
      dmaDisplaylBuffer[i+1] = color&0xFF;          
    }
    ssd1351_SetAddressWindow(x1, y1, x2, y2);    
    ssd1351_data_on();
    //start dma
    while (totalBufferSize) //So far there is something to send
    {  //send data
      dmaProcessingFull( currentBufferSize);
      //recalc buff size
      calcBufferSize(&totalBufferSize,&currentBufferSize,DMA_BUFFER_SIZE);
    }    
    ssd1351_cs_release();    
    //stopCountDWT();
}





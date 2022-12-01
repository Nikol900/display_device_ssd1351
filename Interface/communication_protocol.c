#include "communication_protocol.h"
#include "string.h"
#include "crc16.h"


tU8  UsartDriver_TxBuffer[2][CP_TXBUFFSIZE]; // 2 buffs
tU16 UsartDriver_TxBufferLen[2]; //2buffs
tU8  UsartDriver_RxBuffer[CP_RXBUFFSIZE];

bool CP_NewIncomingDataFlag = false;
tU16 UsartDriver_RxSize = 0;

bool TxComplete = true;

/////////// prots
void UsartTransmit(tU8* Buffer, tU16 Cnt);
void UsartRecieve (tU8* Buffer);
tU16 UsartTxRxFinish();
/////////////


tU8 bufferNumber = 0;

tU8 cp_getFreeBufferNumber() {
  if(bufferNumber == 0)
    return 1;
  else
    return 0;
}


void CP_transmitData(tU8 *data, tU8 dataSize) { 


    //If something is being sent at the moment
    if(!TxComplete)
    {
      tU8 tmpNumb = cp_getFreeBufferNumber(); // get free buff number
      tU16 len = UsartDriver_TxBufferLen[tmpNumb]; //get the current data length

      memcpy(&UsartDriver_TxBuffer[tmpNumb][len],data,dataSize);   //copy data
      dataSize +=2;
      FrameEndCrc16(&UsartDriver_TxBuffer[tmpNumb][len], dataSize); //add crc
      UsartDriver_TxBufferLen[tmpNumb] +=dataSize; //increase length 
    }
    //шина свободна
    else {
      //тупо скопировали или не тупо??
      tU16 len = UsartDriver_TxBufferLen[bufferNumber]; //get curr buff len
      memcpy(&UsartDriver_TxBuffer[bufferNumber][len],data,dataSize);
      dataSize+=2;//+crc16
      FrameEndCrc16(&UsartDriver_TxBuffer[bufferNumber][len], dataSize); 

      UsartDriver_TxBufferLen[bufferNumber] +=dataSize; //increase length 
    }
}

void CP_processing() {
       
         
  //If bus is free
  if (TxComplete) {
    //If  something to send
    if(UsartDriver_TxBufferLen[0]) {           
      bufferNumber = 1;//While sending, we use another buffer
      UsartTransmit(&UsartDriver_TxBuffer[0][0], UsartDriver_TxBufferLen[0]);      
      UsartDriver_TxBufferLen[0] = 0;  //reset len
    }else if(UsartDriver_TxBufferLen[1]) {
      bufferNumber = 0;//While sending, we use another buffer
      UsartTransmit(&UsartDriver_TxBuffer[1][0], UsartDriver_TxBufferLen[1]);      
      UsartDriver_TxBufferLen[1] = 0;   
      }      
    } 
}

void CP_USART_IRQHandler() {
  
  static tU16 maxLen = 0;
  UsartDriver_RxSize = UsartTxRxFinish();
 
  if(UsartDriver_RxSize > maxLen) 
    maxLen = UsartDriver_RxSize;
  if (maxLen > 200)
    maxLen ++;
  
  
  if(UsartDriver_RxSize)
  {
    CP_NewIncomingDataFlag = true;
    UsartRecieve(UsartDriver_RxBuffer);    
  }

}

void CP_init() {
  UsartDriver_TxBufferLen[0] = 0;
  UsartDriver_TxBufferLen[1] = 0;
  UsartRecieve (UsartDriver_RxBuffer);   
}

bool CP_isNewIncomingData() {
  return CP_NewIncomingDataFlag;
}
  
tU16 CP_getIncomingData(tU8 *data) {  
  memcpy(data,UsartDriver_RxBuffer,UsartDriver_RxSize);
  CP_NewIncomingDataFlag = false;
  return UsartDriver_RxSize;
}



//////////////// USART Functions

void UsartTransmit(tU8* Buffer, tU16 Cnt) { 
  //set flag that the trasnefer was started
  TxComplete = false; 

  //DMA settings for data transfer to UART 
  DMA2_Stream7->CR &= ~(uint32_t)DMA_SxCR_EN;
  USART1->SR  &=  ~USART_SR_TC;   
  DMA2->HIFCR = (uint32_t) (DMA_FLAG_FEIF7 | DMA_FLAG_DMEIF7 | DMA_FLAG_TEIF7 | DMA_FLAG_HTIF7 | DMA_FLAG_TCIF7);
  DMA2_Stream7->NDTR = Cnt;
  DMA2_Stream7->M0AR = (uint32_t)Buffer;
  USART1->CR3 |=  USART_CR3_DMAT; 
  USART1->CR1 |=  USART_CR1_TE;   
  DMA2_Stream7->CR |= (uint32_t)DMA_SxCR_EN;
   
  USART1->CR1 |=  USART_CR1_TCIE; 
  
} 

void UsartRecieve (tU8* Buffer) {
  //DMA settings for data recieve  UART
  DMA2_Stream2->CR &= ~(uint32_t)DMA_SxCR_EN;
  DMA2->LIFCR = (uint32_t) (DMA_FLAG_FEIF2 | DMA_FLAG_DMEIF2 | DMA_FLAG_TEIF2 | DMA_FLAG_HTIF2 | DMA_FLAG_TCIF2);
  USART1->CR3 |=  USART_CR3_DMAR;
  DMA2_Stream2->NDTR = CP_RXBUFFSIZE;
  DMA2_Stream2->M0AR = (uint32_t)Buffer;
  DMA2_Stream2->CR |= (uint32_t)DMA_SxCR_EN;
  USART1->CR1 |=  USART_CR1_IDLEIE;
  USART1->CR1 |=  USART_CR1_RE;
}


tU16 UsartTxRxFinish()
{
  tU32 IIR;
  IIR = USART1->SR;
     
    if ((IIR & USART_SR_TC) && (USART1->CR1 & USART_CR1_TCIE)) // The transfer is over (the last byte is completely transferred to the port)
      {   
        USART1->SR  &=  ~USART_SR_TC;   
        USART1->CR1 &=  ~USART_CR1_TCIE;
        USART1->CR3 &=  ~USART_CR3_DMAT;
        DMA2_Stream7->CR &= ~(uint32_t)DMA_SxCR_EN;        
        
        TxComplete = true;
        return 0;
      }
  
    if ((IIR & USART_SR_IDLE) & (USART1->CR1 & USART_CR1_IDLEIE)) // Between the bytes during the reception, a pause was found in 1 IDLE byte
      {
       // startCountDWT();
        USART1->DR; 
        USART1->CR1 &=  ~USART_CR1_RE;    
        USART1->CR1 &=  ~USART_CR1_IDLEIE;
        USART1->CR3 &=  ~USART_CR3_DMAR;  
        DMA2_Stream2->CR &= ~(uint32_t)DMA_SxCR_EN;       
        return (CP_RXBUFFSIZE - (tU16)DMA2_Stream2->NDTR);
      }
    return 0;
}


void memcpy2(tU8 *dst, tU8 *src, tU16 len) {
  for(tU16 i = 0; i < len; i++)
    dst[i] = src[i];
}
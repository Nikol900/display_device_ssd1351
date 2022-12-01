#include "stm32f4xx.h" 
#include "crc16.h"
#include "ssd1351.h"
#include "analog_data.h"
uint16
//--------------------------------------------------------------

void GPIO_Configuration(void){
  
  GPIO_InitTypeDef GPIO_InitStructure;
  
  RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD , ENABLE);
    
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; 
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; 
//------------------------------------------------------------------------------------------------------------     
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN; 
  
  //порт А                           AIN0         AIN1
  GPIO_InitStructure.GPIO_Pin  =  GPIO_Pin_0 | GPIO_Pin_1;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
//------------------------------------------------------------------------------------------------------------
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;   
  
  //порт B                         NSS  2        
  GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_12 ;
  GPIO_Init(GPIOB, &GPIO_InitStructure);  
  
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN; 
  
  //порт B                         BIT0         BIT1         BIT2         BIT3
  GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
  GPIO_Init(GPIOB, &GPIO_InitStructure); 
  
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 
  
  //порт C                        BTN_BRIGHT    BTN_LEFT    BTN_RIGHT      BTN_UP      BTN_DWN     BTN_MENU     
  GPIO_InitStructure.GPIO_Pin  =  GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
//------------------------------------------------------------------------------------------------------------
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;

  //порт А                             D/#C        
  GPIO_InitStructure.GPIO_Pin  =   GPIO_Pin_6 ;          
  GPIO_Init(GPIOA, &GPIO_InitStructure);  
  GPIO_ResetBits(GPIOA, GPIO_Pin_6);
  
  //порт B                           #RES          LED         
  GPIO_InitStructure.GPIO_Pin  =  GPIO_Pin_0 | GPIO_Pin_10;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_ResetBits(GPIOB, GPIO_Pin_0);

  //порт B                            
  GPIO_InitStructure.GPIO_Pin  =  GPIO_Pin_13 ;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  
  //порт А                         NSS   1          
  GPIO_InitStructure.GPIO_Pin  =  GPIO_Pin_4;            
  GPIO_Init(GPIOA, &GPIO_InitStructure);

//------------------------------------------------------------------------------------------------------------   
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; 

  GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1); //SCK - OUT_SH(DIO_CLK)
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1); //MOSI - OUT_D 
  
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource9,  GPIO_AF_USART1); //Tx 
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1); //Rx

  //порт А                          SCK           MOSI        USART1_Tx    USART1_Rx
  GPIO_InitStructure.GPIO_Pin  =  GPIO_Pin_5 | GPIO_Pin_7 |  GPIO_Pin_9 | GPIO_Pin_10;  
  GPIO_Init(GPIOA, &GPIO_InitStructure);

}


//******************************************************************************
//Timer with an interruption once at 0.001 sec
void TIM6_Configuration(void){
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
   /* Time Base configuration */
  TIM_TimeBaseStructure.TIM_Prescaler = 100-1;//freq = TIMCLK(84MHz)/100 = 840KHz
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_Period = 840;//frequency of overflow 840KHz/840 = 1KHz
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

  TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
  TIM_ARRPreloadConfig(TIM6, DISABLE);
  TIM6->SR = 0;
  TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
  TIM_Cmd(TIM6, ENABLE);    
}
//******************************************************************************
//Timer with an interruption once at 0.001 sec = 1 msec
void TIM3_Configuration(void){
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
   /* Time Base configuration */
  TIM_TimeBaseStructure.TIM_Prescaler = 100-1;//freq = TIMCLK(84MHz)/100 = 840KHz
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_Period = 840;//frequency of overflow 840KHz/840 = 1KHz
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
  

  TIM3->SR = 0;
  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
  TIM_Cmd(TIM3, ENABLE);  
  
}
//******************************************************************************
//SPI setting up to work with a display
void SPI1_Configuration(void){
  SPI_InitTypeDef  SPI_InitStructure;
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_SPI1, ENABLE);
  /* SPI1 configuration */
  SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Hard;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;//
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SPI1, &SPI_InitStructure);

  /* Enable SPI1 NSS output for master mode */
  SPI_SSOutputCmd(SPI1, ENABLE);
  /* Enable SPI1  */
  //SPI_Cmd(SPI1, ENABLE);
  
  
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE); 
  SPI_Cmd(SPI1, DISABLE);
  DMA_InitTypeDef DMA_InitStructure;                                         
  
  DMA_InitStructure.DMA_Channel = DMA_Channel_3;                            
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&SPI1->DR;         
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&dmaDisplaylBuffer[0];         
  DMA_InitStructure.DMA_BufferSize = 0;                                  
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;            
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                     
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //DMA_PeripheralDataSize_HalfWord    
  DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte;   //DMA_PeripheralDataSize_HalfWord      
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;//DMA_Priority_High;                       
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;                     
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_Init(DMA2_Stream3, &DMA_InitStructure); //  SPI1_Tx
  
  
  DMA_Cmd(DMA2_Stream3, ENABLE); 
  

  //DMA_ITConfig(DMA2_Stream3,DMA_IT_TC,ENABLE); 
  SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
  
  SPI_Cmd(SPI1, ENABLE);
}

//******************************************************************************
//
void USART1_Configuration()
{
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
  DMA_InitTypeDef DMA_InitStructure;
  USART_InitTypeDef USART_InitStructure;  
  DMA_DeInit(DMA2_Stream2);
  DMA_DeInit(DMA2_Stream7); 
  USART_DeInit(USART1);
  
  
  DMA_InitStructure.DMA_Channel = DMA_Channel_4;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &(USART1->DR);
  DMA_InitStructure.DMA_BufferSize = 0;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;//DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_Init(DMA2_Stream2, &DMA_InitStructure);//USART_Rx
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_Init(DMA2_Stream7, &DMA_InitStructure);//USART_Tx  
  
  
  USART_InitStructure.USART_BaudRate = 115200;  
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity =USART_Parity_No;  
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART1, &USART_InitStructure);
  
  USART1->CR1 |=  USART_CR1_RE;
  USART1->CR1 |=  USART_CR1_TE;
  USART1->CR1 |=  USART_CR1_UE;
  
//  USART1->CR1 |=  USART_CR1_TXEIE;/*!<PE Interrupt Enable */
//  USART1->CR1 |=  USART_CR1_PEIE;/*!<PE Interrupt Enable */ 
   

  
}

//******************************************************************************

void EXTI_Config_btn()
{  
  RCC_APB2PeriphClockCmd ( RCC_APB2Periph_SYSCFG,ENABLE); 
  EXTI_InitTypeDef EXTI_InitStruct;
     
  // btns PC2 - PC7
  SYSCFG_EXTILineConfig ( EXTI_PortSourceGPIOC,EXTI_PinSource2);
  SYSCFG_EXTILineConfig ( EXTI_PortSourceGPIOC,EXTI_PinSource3);
  SYSCFG_EXTILineConfig ( EXTI_PortSourceGPIOC,EXTI_PinSource4);
  SYSCFG_EXTILineConfig ( EXTI_PortSourceGPIOC,EXTI_PinSource5);
  SYSCFG_EXTILineConfig ( EXTI_PortSourceGPIOC,EXTI_PinSource6);
  SYSCFG_EXTILineConfig ( EXTI_PortSourceGPIOC,EXTI_PinSource7);
  EXTI_InitStruct . EXTI_Line =  EXTI_Line2 | EXTI_Line3 | EXTI_Line4 | EXTI_Line5 | EXTI_Line6 | EXTI_Line7;
  EXTI_InitStruct . EXTI_LineCmd =  ENABLE ; 
  EXTI_InitStruct . EXTI_Mode =  EXTI_Mode_Interrupt ;
  EXTI_InitStruct . EXTI_Trigger =  EXTI_Trigger_Rising_Falling;
  EXTI_Init (& EXTI_InitStruct ); 
}

//******************************************************************************

#define ADC_TIM_PERIOD ((tU32) 168000000)//1 sec
void ADC_Configuration(uint32_t AinBuffAddr)
{
  
  DMA_InitTypeDef DMA_InitStructure; 
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);  
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  ADC_CommonStructInit(&ADC_CommonInitStructure);
  ADC_InitTypeDef ADC_InitStructure;
  ADC_StructInit(&ADC_InitStructure); 
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;//ADC_CLK = PCLK2/4=84MHz/4=24Mhz
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_20Cycles;//delay between samples 
  ADC_CommonInit(&ADC_CommonInitStructure);
  
 
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;//
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;//starts along the pulse front TIM8
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T8_TRGO;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
   

  DMA_InitStructure.DMA_Channel = DMA_Channel_0;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &(ADC1->DR);
  DMA_InitStructure.DMA_Memory0BaseAddr = AinBuffAddr;  
  DMA_InitStructure.DMA_BufferSize = 3;
  DMA_InitStructure.DMA_Priority = DMA_Priority_Low;//DMA_Priority_VeryHigh;
  DMA_Init(DMA2_Stream0, &DMA_InitStructure);

  DMA_ITConfig(DMA2_Stream0, DMA_IT_TC , ENABLE);

  DMA_Cmd(DMA2_Stream0, ENABLE);
  
  ADC_InitStructure.ADC_NbrOfConversion = 3;  
  ADC_Init(ADC1, &ADC_InitStructure);
  ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
  ADC_DMACmd(ADC1, ENABLE);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_0,  1, ADC_SampleTime_28Cycles);//ADC0
  ADC_RegularChannelConfig(ADC1, ADC_Channel_1,  2, ADC_SampleTime_28Cycles);//ADC1 VCOMH
  ADC_RegularChannelConfig(ADC1, ADC_Channel_Vrefint, 3, ADC_SampleTime_28Cycles);//VrefIn

  ADC_Cmd(ADC1, ENABLE);
    

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);


  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;    
  TIM_TimeBaseStructure.TIM_Prescaler = 0;//freq = TIM8CLK(168MHz)
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_Period = ADC_TIM_PERIOD-1;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure);  
  
  //Impulse configuration, i.e. Channel for comparison
  TIM_OCInitTypeDef TIM_OCInitStructure;
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;       
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;  
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OCInitStructure.TIM_Pulse = ADC_TIM_PERIOD/2;//pulse width = half of the period
  TIM_OC1Init(TIM8, &TIM_OCInitStructure);

  TIM_SelectOutputTrigger(TIM8, TIM_TRGOSource_Update);

  TIM_Cmd(TIM8, ENABLE);    
    
}


//******************************************************************************

void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

#ifdef  VECT_TAB_RAM  
  /* Set the Vector Table base location at 0x20000000 */
  NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0); 
#else  /* VECT_TAB_FLASH  */
  /* Set the Vector Table base location at 0x08000000 */ 
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);   
#endif
  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);  
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  

  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;// 
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_Init(&NVIC_InitStructure);  
    
  
  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;//
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_Init(&NVIC_InitStructure);    
  
  //BTN IRQ
  NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;//btn
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_Init(&NVIC_InitStructure);    
  NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;//btn
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_Init(&NVIC_InitStructure);   
  NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;//btn
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_Init(&NVIC_InitStructure);   
  NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;//btn
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_Init(&NVIC_InitStructure); 
  
  NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;//dma adc
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_Init(&NVIC_InitStructure); 
}




void hardwareInit (void){ 
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE); 
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE); 
  GPIO_Configuration();

  SPI1_Configuration();
  USART1_Configuration();
  EXTI_Config_btn();

  TIM3_Configuration();
  NVIC_Configuration();
  ADC_Configuration((tU32)(AnalogDataBuffer));
}

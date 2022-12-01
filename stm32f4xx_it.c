/**
  ******************************************************************************
  * @file    Project/STM32F4xx_StdPeriph_Template/stm32f4xx_it.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    30-September-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/        
#include "stm32f4xx_it.h"
#include "display.h"
#include "buttons.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/******************************************************************************/


/******************************************************************************/
#include "ssd1351.h"
#include "analog_data.h"
#include "process_handler.h"
#include "communication_protocol.h"

    
void DMA2_Stream0_IRQHandler(void)//Interrupt by readiness of ADC data by DMA
{
  DMA2->LIFCR = (uint32_t) (DMA_FLAG_FEIF0 | DMA_FLAG_DMEIF0 | DMA_FLAG_TEIF0 | DMA_FLAG_HTIF0 | DMA_FLAG_TCIF0);//clear flags dma
  DigitalSignalProcessing();
}

void USART1_IRQHandler(void)
{
  CP_USART_IRQHandler();
}   

#include "crc16.h"
void TIM3_IRQHandler(void)//general -purpose timer 1 kHz
{ 
  // startCountDWT();
  /*PH_errorSendControl();
  TIM3->SR = 0;
  buttons_timerProcessing(BUTTON_2_BRIGHT);
  buttons_timerProcessing(BUTTON_3_LEFT);
  buttons_timerProcessing(BUTTON_4_RIGHT);
  buttons_timerProcessing(BUTTON_5_UP);
  buttons_timerProcessing(BUTTON_6_DOWN);
  buttons_timerProcessing(BUTTON_7_MENU);*/

  // stopCountDWT();
  
   if ((GPIOD->ODR & GPIO_Pin_13) == GPIO_Pin_13)
  {
    GPIOD->BSRRH = GPIO_Pin_13;
  }
  else
  {
    GPIOD->BSRRL = GPIO_Pin_13;
  }

  
  
  
}

// btn EXTI IRQ
#define EXTI_LINE_IS_EBANLED(EXTI_LINE) (((EXTI->PR & EXTI_LINE) != (uint32_t)RESET) && ((EXTI->IMR & EXTI_LINE) != (uint32_t)RESET))

void EXTI2_IRQHandler(void){
  EXTI->PR = EXTI_Line2; 
  buttons_EXTIProcessing(BUTTON_2_BRIGHT);
}

void EXTI3_IRQHandler(void){
  EXTI->PR = EXTI_Line3; 
  buttons_EXTIProcessing(BUTTON_3_LEFT);
}

void EXTI4_IRQHandler(void){
  EXTI->PR = EXTI_Line4; 
  buttons_EXTIProcessing(BUTTON_4_RIGHT);
}
  
void EXTI9_5_IRQHandler(void){
  if(EXTI_LINE_IS_EBANLED(EXTI_Line5))
  {
     EXTI->PR = EXTI_Line5; 
     buttons_EXTIProcessing(BUTTON_5_UP);
  }
  
  if(EXTI_LINE_IS_EBANLED(EXTI_Line6))
  {
     EXTI->PR = EXTI_Line6; 
     buttons_EXTIProcessing(BUTTON_6_DOWN);
  }
  
  if(EXTI_LINE_IS_EBANLED(EXTI_Line7))
  {
     EXTI->PR = EXTI_Line7; 
     buttons_EXTIProcessing(BUTTON_7_MENU);
  }
}





/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
    /* Go to infinite loop when Hard Fault exception occurs */
// uint32_t ss  = 23;
//  struct 
//  {
//    uint32_t r0;
//    uint32_t r1;
//    uint32_t r2;
//    uint32_t r3;
//    uint32_t r12;
//    uint32_t lr;
//    uint32_t pc;
//    uint32_t psr;
//  }*stack_ptr; //Указатель на текущее значение стека(SP)
//
// ss+=21;
//  asm(
//      "TST lr, #4 \n" //Тестируем 3ий бит указателя стека(побитовое И)
//      "ITE EQ \n"   //Значение указателя стека имеет бит 3?
//      "MRSEQ %[ptr], MSP  \n"  //Да, сохраняем основной указатель стека
//      "MRSNE %[ptr], PSP  \n"  //Нет, сохраняем указатель стека процесса
//      : [ptr] "=r" (stack_ptr)
//      );

  while (1)
  {
  }





}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
  //TimingDelay_Decrement();
}

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 



/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

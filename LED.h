#include "stm32f4xx.h"

#define LED_ON   GPIOB->BSRRH = GPIO_Pin_10//12
#define LED_OFF  GPIOB->BSRRL = GPIO_Pin_10
#define LED_ST   GPIOB->ODR & GPIO_Pin_10

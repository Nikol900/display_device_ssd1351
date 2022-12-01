#include "stm32f4xx.h"
#include "init.h"
#include "display.h"
#include "LED.h"
#include "ssd1351.h"

#include "process_handler.h"

#include "crc16.h"
int main(void)              
{  
 // startCountDWT();
  hardwareInit(); 
  PH_init(); 
 // stopCountDWT();
  
  while (1) {   
    PH_mainProcessing();
  }
}



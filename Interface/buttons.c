#include "buttons.h"
//#include "display.h"

#define BOUNCE_TIMEOUT (46) //46

bool buttons_StateIsRise[BUTTONS_COUNT] = {false,false,false,false,false,false};
tU32 buttons_BounceCnt[BUTTONS_COUNT];
tU8 buttons_Bitmap = 0;
#define CF 0//Change Flag button 1 - new event 
#define S  1//State button 0 - fall 1- rise
tU8 buttons_States [BUTTONS_COUNT][2];//0 - change Flag 1- new State
tU8 buttons_prevStates [BUTTONS_COUNT];//0 - change Flag 1- new State


void buttons_setState(buttonsNumber_t buttonNumb, buttonsState_t state) {
  
  /*If this button has changed the condition, then generate Event*/
  if (buttons_prevStates[buttonNumb] != state) 
  {
      buttons_States[buttonNumb][CF] = 1;//set CF
      buttons_States[buttonNumb][S] = state; 
    
    if(state == BTN_PRESS)
      buttons_Bitmap |=  1 << buttonNumb;
    else
      buttons_Bitmap &= ~ (1 << buttonNumb);

   //   buttons_prevStates[buttonNumb][CF]  = buttons_States[buttonNumb][CF];
    buttons_prevStates[buttonNumb] = buttons_States[buttonNumb][S] ;
  }
}

tU8 buttons_isChange(buttonsNumber_t buttonNumb) {
  return  buttons_States[buttonNumb][CF];
}
tU8 buttons_getState(buttonsNumber_t buttonNumb) {
  buttons_States[buttonNumb][CF] = 0;//reset CF
  return  buttons_States[buttonNumb][S];
}

tU8 buttons_getBitmap(){ 
  return buttons_Bitmap;
}


tU32 buttons_getGpioPin(buttonsNumber_t buttonsNumb)
{
  switch(buttonsNumb)
  {
    case BUTTON_2_BRIGHT:   return GPIO_Pin_2;
    case BUTTON_3_LEFT:     return GPIO_Pin_3;
    case BUTTON_4_RIGHT:    return GPIO_Pin_4;
    case BUTTON_5_UP:       return GPIO_Pin_5;
    case BUTTON_6_DOWN:     return GPIO_Pin_6;
    case BUTTON_7_MENU:     return GPIO_Pin_7;
    default: return 0;    
  }      
}

void buttons_timerProcessing(buttonsNumber_t buttonsNumb)
{
   if(buttons_BounceCnt[buttonsNumb])
   {
      buttons_BounceCnt[buttonsNumb]--;
   }
   
   if(buttons_BounceCnt[buttonsNumb]==1)
   {
     //It is necessary to compare with the previous state of the button so as not to generate unnecessary events
     if(buttons_StateIsRise[buttonsNumb])
     {
//       buttons_riseEvents(buttonsNumb); //generate event
       buttons_setState(buttonsNumb,BTN_PRESS); //save state
     }
     else
     {
//       buttons_fallEvents(buttonsNumb);
       buttons_setState(buttonsNumb,BTN_RELEASE);
     }       
   }  
}
void buttons_EXTIProcessing(buttonsNumber_t buttonsNumb)
{
  /*
If an event occurs for a button
  charge the timer and maintain the condition (Rise/Fall)
  in a timer after a bounce
  as of generate an event
  */
  tU32 GPIO_Pin_x = buttons_getGpioPin(buttonsNumb);
  buttons_BounceCnt[buttonsNumb] = BOUNCE_TIMEOUT; 
  if(GPIOC->IDR & GPIO_Pin_x)//release  PullUp
  {
    buttons_StateIsRise[buttonsNumb] = false;    
  }
  else
  {
    buttons_StateIsRise[buttonsNumb] = true;  
  }
}

void buttons_init() 
{
  for(tU8 btn = 0 ; btn < BUTTONS_COUNT; btn++)
  {
      buttons_prevStates[btn] = BTN_RELEASE;
      tU32 GPIO_Pin_x = buttons_getGpioPin(btn);  
      if(GPIOC->IDR & GPIO_Pin_x)//release  PullUp
      {}
      else
      {
          buttons_setState(btn,BTN_PRESS); //Save the condition
      }
  }
  
}
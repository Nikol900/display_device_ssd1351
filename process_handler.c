//PH = Process Handler
#include "process_handler.h"
#include "display.h"
#include "buttons.h"
#include "communication_protocol.h"
#include "analog_data.h"
#include "crc16.h"
#include "string.h"

#define TIMEOUT_PARSING_MS 2

tU8 BrightnessLvl = DISPLAY_BRIGTHNESS_100;
//tU8 ParsingErrorDetection = 0;
//bool BrightnessIsChange = false;


//////// prots
//void ph_decrementBrightness();
void ph_checkButtons();
void ph_checkCommunication();

void ph_communicationGenerateResponse(CP_cmd_list_t cmd, tU8 *data, tU16 dataSize);
void ph_sendButtonEvent(buttonsNumber_t buttonNumber, buttonsState_t buttonState, tU8 buttonsBitmapState);


void ph_sendParsingError();
void ph_applyCommand(tU8 *data, tU16 dataSize);
/////////////

void PH_init() {  
  Display_init();
  Display_setBrightnessLvl(BrightnessLvl);
 
  CP_init();//communication protocol  
  buttons_init();
}

tU16 timeoutCntMs = 0;//timeout counter msec
bool EnabledTimer = false;


void ph_startTimeoutTimer() {
  timeoutCntMs = 0;
  EnabledTimer = true;
  
}
void ph_stopTimeoutTimer() {
  timeoutCntMs = 0;
  EnabledTimer = false;
}

bool ph_cmdIsValid(tU8 cmd) {
  bool retVal;
  switch (cmd) {
  case CMD_SET_DISPLAY_MODE:
  case CMD_SINGLE_ROW_DATA:
  case CMD_MULTIPLE_ROW_DATA:
  case CMD_PARTIAL_ROW_DATA:
  case CMD_CLEAR:
  case CMD_BRIGHTNESS_SET:
  case CMD_BRIGHTNESS_GET:
  case CMD_ADC_GET:
  case CMD_BUTTONS_GET:
    retVal = true;
    break;
  default:
    retVal = false;
    break;
  }  
  return retVal;    
}


tU8 parseBuffer[CP_RXBUFFSIZE];

bool StartMessage = false;


tU16 predictedCmdLen = 0; //predicted package length

void ph_shiftData(tU8 *data, tU16 start, tU16 len) {
  for(tU16 i = 0 ; i < len; i ++) {
    data[i] = data[i+start];
  }     
}
  
void ph_parseData(tU8 *data, tU16 dataSize) {

 // static tU8 prevDataSize = 0;
  static tU16 parseBufferLen = 0;
  // new data  or parseBuffer not empty or timeout
  if ( dataSize || (StartMessage) || (timeoutCntMs > TIMEOUT_PARSING_MS)) {    
    //startCountDWT();
    // no more data
    if (timeoutCntMs > TIMEOUT_PARSING_MS) { 
      parseBufferLen = 0; //Clear Parsing Buffer
      predictedCmdLen = 0; //reset cmd len
      StartMessage = false;
      ph_stopTimeoutTimer();
      ph_sendParsingError();
    }     
    //new Data has arrived
    if (dataSize){
      if(parseBufferLen == 0) {              
        //copy data to parseBuffer
        memcpy2(parseBuffer,data,dataSize);
        //store new buff len
        parseBufferLen = dataSize;  
        StartMessage = true;
      // add new data to parseBuffer        
      } else {
        //copy new data to parseBuffer
        memcpy2(&parseBuffer[parseBufferLen],data,dataSize);
        //store new buff len
        parseBufferLen += dataSize;
      }    
    }
    //have CMD
    if (StartMessage) {
      ph_startTimeoutTimer();
      //parseBuffer[0] = data[0]; // cmd
      StartMessage = false;
    }
    //have dataLen
    if ((parseBufferLen > 1) && (predictedCmdLen == 0)) {
      //parseBuffer[1] = data[1]; // num of bytes
      predictedCmdLen = parseBuffer[1] + 4;// cmd, len,crc_h,crc_l
    }
    
    //have full msg
    if ((parseBufferLen >=predictedCmdLen)  && (predictedCmdLen > 0)) {
      if (!ph_cmdIsValid(parseBuffer[0])) 
        ph_sendParsingError();      
      else 
        ph_applyCommand(parseBuffer,predictedCmdLen);
      
      parseBufferLen -= predictedCmdLen; 
      ph_shiftData(parseBuffer,predictedCmdLen, parseBufferLen);       
   
      predictedCmdLen = 0; //reset cmd len
      ph_stopTimeoutTimer();
      
      if (parseBufferLen == 0) 
      {
        StartMessage = false;        
      }
      else
      {
        StartMessage = true;    
      }
    }      
  }    
}

void PH_mainProcessing() {
 /*
  This is a permanent cycle, we’ll get everything here
  And here we interview from Main
  */
  //The order is important !!!! Only in this order nothing breaks
  ph_checkCommunication();  //checked the display - the longest
  ph_checkButtons();// Checked the buttons

  CP_processing(); //sent everything  
}

bool dataIsValid(tU8 *data, tU16 dataSize) {
 //If the expected quantity! = current
  if (data[1] == (dataSize-4))
    return true;
  return false;
}

bool isCRC16valid(tU8 *data, tU16 dataSize) {
    bool retVal = false;
    tU16Union crc16Val;
    crc16Val.I = crc16(data,dataSize-2);
    if( (crc16Val.B[0] == data[dataSize-2]) && 
        (crc16Val.B[1] == data[dataSize-1]))
    {
      retVal = true;
    }
    return retVal;
}
//* * * * * * * communication functions
void ph_applyCommand(tU8 *data, tU16 dataSize) {
  
// check CRC
  if (!isCRC16valid(data,dataSize)) {
    tU8 resp[2] = {0x01,RESP_NACK};
    ph_communicationGenerateResponse(data[0],resp,3);
    return;
  }
 
    // check type CMD
    switch(data[0])
    {
      case CMD_SET_DISPLAY_MODE:
      {//[0x00] [[0x01] [mode]] [CRC16]
        /*
        00 01 00 50 70
        00 01 01 90 B1
        00 01 02 91 F1
        */
        tU8 resp[2] = {0x01,RESP_NACK};
        
        if(display_setMode(data[2])) 
          resp[1] = RESP_ACK;      
        ph_communicationGenerateResponse(CMD_SET_DISPLAY_MODE,resp,3);
        break;
      }
      case CMD_CLEAR:
      {
        display_clearAll();
        tU8 resp[2] = {0x01,RESP_ACK};
        ph_communicationGenerateResponse(CMD_CLEAR,resp,3);
        break;
      }
          
      case CMD_SINGLE_ROW_DATA:
      {//[0x01] [[1 + N x 2] [NROW] [SYMB_0_TYPE][SYMB_0_VAL] ... [SYMB_N-1_TYPE][SYMB_N-1_VAL]] [CRC16]

        tU8 resp[2] = {0x01,RESP_NACK};
        if(dataIsValid(data,dataSize) && data[1]>1){
          tU8 rowNumb = data[2]-1;////Conclusion from 0, numbering from 1
          tU8 rowLen = (data[1] -1) / 2;
          if (display_drawFormatString(rowNumb,rowLen,&data[3]))
            resp[1] = RESP_ACK;
        }          
          ph_communicationGenerateResponse(CMD_SINGLE_ROW_DATA,resp,3);
        break;
      }
      case CMD_MULTIPLE_ROW_DATA:
      {//[0x02] [[RCNT x 8 x 2] [SYMB_00_TYPE][SYMB_00_VAL] ...[SYMB_07_TYPE][SYMB_07_VAL]... [SYMB_RCNT7_TYPE][SYMB_RCNT7_VAL]] [CRC16]
                  
        tU8 resp[2] = {0x01,RESP_NACK}; 
        if(dataIsValid(data,dataSize) && data[1]>0){          
          tU8 dataLen = (data[1]) / 2;
          if(display_drawFormatAllStrings(dataLen,&data[2]))
            resp[1] = RESP_ACK;
        }
        ph_communicationGenerateResponse(CMD_MULTIPLE_ROW_DATA,resp,3);
        break;
      }
    case CMD_PARTIAL_ROW_DATA: {
      // [0x03] [2 + 2 * N] [NROW] [NCOL] [CHAR_0_TYPE][CHAR_0_VAL] ... [CHAR_N-1_TYPE][CHAR_N-1_VAL] [CRC16]
       tU8 resp[2] = {0x01,RESP_NACK};
        if(dataIsValid(data,dataSize) && (data[1]>2) && (data[3]!=0)  && (data[2]!=0)){
          tU8 rowLen = (data[1] -2) / 2;
          tU8 rowNumb = data[2]-1;//Conclusion from 0, numbering from 1
          tU8 colNumb = data[3]-1;
          if (display_drawPartialString(rowNumb,colNumb,rowLen,&data[4]))
            resp[1] = RESP_ACK;
        }          
          ph_communicationGenerateResponse(CMD_PARTIAL_ROW_DATA,resp,3);
        break;      
    }
    case CMD_BRIGHTNESS_SET:
      {
        tU8 resp[2] = {0x01,RESP_NACK};                
        if(Display_setBrightnessLvl(data[2])) 
        {
          resp[1] = RESP_ACK;  
          BrightnessLvl = data[2];
        }
        ph_communicationGenerateResponse(CMD_BRIGHTNESS_SET,resp,3);
        break;
      }      
      case CMD_BRIGHTNESS_GET:
      {
        tU8 resp[2] = {0x01,BrightnessLvl}; 
        ph_communicationGenerateResponse(CMD_BRIGHTNESS_GET,resp,3);
        break;
      }      
      case CMD_ADC_GET:
      {
        tU16Union adcVal_1,adcVal_2;
        adcVal_1.I = analogData_getADCvalue(ADC_VALUE_CH0);
        adcVal_2.I = analogData_getADCvalue(ADC_VALUE_CH1);
        tU8 resp[5] = {0x04,adcVal_1.B[0],adcVal_1.B[1],adcVal_2.B[0],adcVal_2.B[1]}; 
        ph_communicationGenerateResponse(CMD_ADC_GET,resp,6);
        break;
      }
      case CMD_BUTTONS_GET:
      {
        tU8 buttonsBitmap = buttons_getBitmap();
        tU8 resp[2] = {0x01,buttonsBitmap}; 
        ph_communicationGenerateResponse(CMD_BUTTONS_GET,resp,3);
      }  
    } 
}

void ph_communicationGenerateResponse(CP_cmd_list_t cmd, tU8 *data, tU16 dataSize) {
  tU8 responseData[CP_TXBUFFSIZE];
  responseData[0] = cmd;
  memcpy2(&responseData[1],data,dataSize);
  CP_transmitData(responseData,dataSize);
}
void ph_checkCommunication() {
  
  tU8 data[CP_RXBUFFSIZE];
  tU16 dataSize = 0;
  if (CP_isNewIncomingData())
  {
    dataSize = CP_getIncomingData(data);     
  }
  ph_parseData(data,dataSize);  
}

void ph_sendButtonEvent(buttonsNumber_t buttonNumber, buttonsState_t buttonState, tU8 buttonsBitmapState) {
  
  tU8 data[5];  
  data[0] = 0xD0;//cmd 
  data[1] = 0x03;//data size numb
  data[2] = buttonNumber;
  data[3] = buttonState;
  data[4] = buttonsBitmapState;     
  CP_transmitData(data,5);
}

void ph_sendParsingError() {
  //clear flag and send 0xD1
//  ParsingErrorDetection = 0;
  tU8 data[2];  
  data[0] = 0xD1;//cmd 
  data[1] = 0x00;//
  CP_transmitData(data,2);
}

void PH_errorSendControl() {
  static tU16 cnt = 0;
   if(EnabledTimer)
     timeoutCntMs++;  
}

//* * * * * * * buttons control    
void ph_checkButtons() {
  
  
  if(buttons_isChange(BUTTON_2_BRIGHT))
  {    
    tU8 buttonsBitmap = buttons_getBitmap(); //Collect bitmap
     buttonsState_t buttonState = buttons_getState(BUTTON_2_BRIGHT) ;
     ph_sendButtonEvent(BUTTON_2_BRIGHT,buttonState,buttonsBitmap);
  }
   if(buttons_isChange(BUTTON_3_LEFT))
  {
    tU8 buttonsBitmap = buttons_getBitmap(); //Collect bitmap
    buttonsState_t buttonState = buttons_getState(BUTTON_3_LEFT) ;
    ph_sendButtonEvent(BUTTON_3_LEFT,buttonState,buttonsBitmap);
  }
   if(buttons_isChange(BUTTON_4_RIGHT))
  {
    tU8 buttonsBitmap = buttons_getBitmap(); //Collect bitmap
    buttonsState_t buttonState = buttons_getState(BUTTON_4_RIGHT) ;
    ph_sendButtonEvent(BUTTON_4_RIGHT,buttonState,buttonsBitmap);
  }
   if(buttons_isChange(BUTTON_5_UP))
  {
    tU8 buttonsBitmap = buttons_getBitmap(); //Collect bitmap
    buttonsState_t buttonState = buttons_getState(BUTTON_5_UP) ;
    ph_sendButtonEvent(BUTTON_5_UP,buttonState,buttonsBitmap);
  }
   if(buttons_isChange(BUTTON_6_DOWN))
  {
    tU8 buttonsBitmap = buttons_getBitmap(); //Collect bitmap
    buttonsState_t buttonState = buttons_getState(BUTTON_6_DOWN) ;
    ph_sendButtonEvent(BUTTON_6_DOWN,buttonState,buttonsBitmap);
  }
   if(buttons_isChange(BUTTON_7_MENU))
  {
    tU8 buttonsBitmap = buttons_getBitmap(); //Collect bitmap
    buttonsState_t buttonState = buttons_getState(BUTTON_7_MENU) ;
    ph_sendButtonEvent(BUTTON_7_MENU,buttonState,buttonsBitmap);
  }
}

#ifndef _COMMUNICATION_PROTOCOL_H
#define _COMMUNICATION_PROTOCOL_H
#include "stm32f4xx.h"
#include "bastypes.h"
#include "stdbool.h"


#define CP_RXBUFFSIZE  255 //
#define CP_TXBUFFSIZE  255 //


typedef enum {
  CMD_SET_DISPLAY_MODE    = 0x00,
  CMD_SINGLE_ROW_DATA     = 0x01,
  CMD_MULTIPLE_ROW_DATA   = 0x02,
  CMD_PARTIAL_ROW_DATA    = 0x03,
  CMD_CLEAR               = 0x04,
  CMD_BRIGHTNESS_SET      = 0x06,
  CMD_BRIGHTNESS_GET      = 0x16,
  CMD_ADC_GET             = 0x1A,
  CMD_BUTTONS_GET         = 0x1B,
  //EVENT_BUTTONS_STATE     = 0xD0,
}CP_cmd_list_t;

typedef enum {
  RESP_ACK = 0x00,
  RESP_NACK = 0x01
}CP_resp_t;


void CP_init();

void CP_USART_IRQHandler();

bool CP_isNewIncomingData();
tU16 CP_getIncomingData(tU8 *data);
void CP_transmitData(tU8 *data, tU8 dataSize);

void CP_processing();

void memcpy2(tU8 *dst, tU8 *scr, tU16 len);

#endif
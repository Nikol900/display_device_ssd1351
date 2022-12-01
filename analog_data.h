#ifndef __ANALOG_DATA_H__
#define __ANALOG_DATA_H__
#include "bastypes.h"

typedef enum
{
   ADC_VALUE_CH0 = 0,
   ADC_VALUE_CH1,
   VREF_IN_CH
}adc_channel_t;

extern tU16 AnalogDataBuffer[3];

void DigitalSignalProcessing(void);

//void analogData_getADCvalues(tU16 *adc0, tU16 *adc1);
tU16 analogData_getADCvalue(adc_channel_t channel);
#endif
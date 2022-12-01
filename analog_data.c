#include "analog_data.h"

tU16 AnalogDataBuffer[3];

#define VREFIN_CAL_ADDR 0x1FFF7A2A      //datasheet VREFIN_CAL addr

#define ADC_RESOLUTION  4095           // 12 bits
                         
tU16 AdcVal[3];




void DigitalSignalProcessing(void)
{
    tU16 VrefInCalVal = *((tU16*)VREFIN_CAL_ADDR);

    tF32 VrefCalibValue = 1;//VrefInCalVal / AnalogDataBuffer[VREF_IN_CH];
    
    AdcVal[ADC_VALUE_CH0] = (tU16) (AnalogDataBuffer[ADC_VALUE_CH0] * VrefCalibValue);
    AdcVal[ADC_VALUE_CH1] = (tU16) (AnalogDataBuffer[ADC_VALUE_CH1] * VrefCalibValue);
    AdcVal[VREF_IN_CH] = VrefInCalVal;

//реальные величины    
//    tU16 adc0 = 3.3 * (AnalogDataBuffer[ADC_VALUE_CH0]/ADC_RESOLUTION) * (VrefInCalVal / VrefIn);    
//    tU16 adc1 = 3.3 * (AnalogDataBuffer[ADC_VALUE_CH1]/ADC_RESOLUTION) * (VrefInCalVal / VrefIn);
}

//void analogData_getADCvalues(tU16 *adc0, tU16 *adc1)
//{
//  adc0 = &AdcVal[ADC_VALUE_CH0];
//  adc1 = &AdcVal[ADC_VALUE_CH1];
//}

tU16 analogData_getADCvalue(adc_channel_t channel)
{
  return AdcVal[channel];
}
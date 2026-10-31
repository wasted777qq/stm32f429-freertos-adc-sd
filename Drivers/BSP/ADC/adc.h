#ifndef __ADC_H
#define __ADC_H

#include "./SYSTEM/sys/sys.h"		

#define ADC_GPIO_PORT   GPIOA
#define ADC_GPIO_PIN    GPIO_PIN_4

#define ADCx            ADC1
#define ADC_CHANNEL     ADC_CHANNEL_4

void ADC_GPIO_init(void);
void ADC_Init(void);
uint16_t ADC_Get(void);

#endif


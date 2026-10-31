#include "adc.h"

static ADC_HandleTypeDef adc_handler;

void ADC_GPIO_init(void)// ADC GPIO初始化代码
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = ADC_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(ADC_GPIO_PORT, &GPIO_InitStruct);
}

void ADC_Init(void)
{
	ADC_ChannelConfTypeDef ADC_Channel_Config;
    
    ADC_GPIO_init();
    __HAL_RCC_ADC1_CLK_ENABLE();
    
    adc_handler.Instance = ADC1;
    adc_handler.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    adc_handler.Init.Resolution = ADC_RESOLUTION_12B;//12位分辨率
    adc_handler.Init.ScanConvMode = DISABLE;//单通道
    adc_handler.Init.ContinuousConvMode = DISABLE;//单次转换
    adc_handler.Init.ExternalTrigConv = ADC_SOFTWARE_START;//软件触发
    adc_handler.Init.DataAlign = ADC_DATAALIGN_RIGHT;//右对齐
    adc_handler.Init.NbrOfConversion = 1;//转换次数（只采一个通道就填 1）
    adc_handler.Init.DMAContinuousRequests = DISABLE;//DMA关闭
    adc_handler.Init.EOCSelection = ADC_EOC_SINGLE_CONV;//单次转换模式
    adc_handler.Init.DiscontinuousConvMode = DISABLE;//非不连续模式
    adc_handler.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;//无外部触发

    HAL_ADC_Init(&adc_handler);

    ADC_Channel_Config.Channel = ADC_CHANNEL;
    ADC_Channel_Config.Rank = 1;
    ADC_Channel_Config.SamplingTime = ADC_SAMPLETIME_480CYCLES;//采样时间
    ADC_Channel_Config.Offset = 0;

    HAL_ADC_ConfigChannel(&adc_handler,&ADC_Channel_Config);
}

uint16_t ADC_Get(void)
{
    uint16_t ADC_Value = 0;

    HAL_ADC_Start(&adc_handler);
    if(HAL_ADC_PollForConversion(&adc_handler, 10) == HAL_OK)
    {
        ADC_Value = HAL_ADC_GetValue(&adc_handler);
    }
    HAL_ADC_Stop(&adc_handler);

    return ADC_Value;
}


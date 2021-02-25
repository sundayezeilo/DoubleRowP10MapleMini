/*
 * my_stm32f10x_adc.h
 *
 *  Created on: May 16, 2017
 *      Author: zeilotech
 */

#ifndef ADCX_H_
#define ADCX_H_

#define ADC_GPIO_PORT GPIOA
//only two ADc channels here, not four
/*
#define ADC_ANALOG_1 GPIO_Pin_4
#define ADC_ANALOG_2 GPIO_Pin_5
#define ADC_ANALOG_3 GPIO_Pin_7*/

//#define TOTAL_ADC_CHANNELS (uint32_t)3UL

//#define ADC_ANALOG_4 GPIO_Pin_7
//#define ADC_ANALOG_3 GPIO_Pin_2
//#define ADC_ANALOG_4 GPIO_Pin_3
//#define ADC_GPIO_MASK (ADC_ANALOG_1|ADC_ANALOG_2|ADC_ANALOG_3|ADC_ANALOG_4)
//#define ADC_GPIO_MASK (ADC_ANALOG_1|ADC_ANALOG_2|ADC_ANALOG_3)
#define ADC_GPIO_MASK GPIO_Pin_7
//const uint32_t ADC_MUX_TABLE[TOTAL_ADC_CHANNELS]={4,5,6,7};
//const uint32_t ADC_MUX_TABLE[TOTAL_ADC_CHANNELS]={4,5,7};
#define ADC_SAMPLE_INTERVAL (1000/TOTAL_ADC_CHANNELS)

void enableADCIRQ(void)
{
	  ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
	  ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

void disableADCIRQ(void)
{
	  ADC_ITConfig(ADC1, ADC_IT_EOC, DISABLE);
	  ADC_SoftwareStartConvCmd(ADC1, DISABLE);
}
/*
void readADC1WithIRQ(void)
{
  ADC_RegularChannelConfig(ADC1, ADC_MUX_TABLE[ADC.index], 1, ADC_SampleTime_13Cycles5);
  ADC_ClearFlag(ADC1,ADC_FLAG_EOC);	//this is required here!!!!????
  ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}
*/
u16 readADC1NoIRQ(uint32_t channel)
{
	ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_13Cycles5);
	ADC_ClearFlag(ADC1,ADC_FLAG_EOC);	//this is required here!!!!????
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
	return ADC_GetConversionValue(ADC1);
}

uint16_t getADC(uint32_t ch)
{
	ADC_ClearFlag(ADC1,ADC_FLAG_EOC);	//this required here!!!!????
	return(readADC1NoIRQ(ch));
}

void	ADC1_2_IRQHandler(void)
{
	ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
}

void ADCRegConfigure(void)
{
	ADC_InitTypeDef  ADC_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	ADC_DeInit(ADC1);
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);
	ADC_Cmd(ADC1, ENABLE);
	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));
	disableADCIRQ();
	NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void ADCIOConfigure(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	uint16_t mask = ADC_GPIO_MASK;

	GPIO_InitStruct.GPIO_Pin = mask;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(ADC_GPIO_PORT, &GPIO_InitStruct);
}
#endif /* ADCX_H_ */

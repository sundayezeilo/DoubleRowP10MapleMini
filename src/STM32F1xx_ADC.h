#define ADC_GPIO_PORT GPIOA
#define ADC_GPIO_MASK (GPIO_Pin_7)

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

u16 readADC1NoIRQ(uint32_t channel)
{
	ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_13Cycles5);
	ADC_ClearFlag(ADC1,ADC_FLAG_EOC);	//this required here!!!!????
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

//USAGE
/*
void main(void)
{
	ADCIOConfigure();
	ADCRegConfigure();

	uint16_t x;
	while(1){
		x = getADC(7);
	}
}
*/

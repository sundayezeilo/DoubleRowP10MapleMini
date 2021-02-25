/*
 * timers_config.h
 *
 *  Created on: Sep 25, 2017
 *      Author: zeilotech
 */

#ifndef TIMERS_CONFIG_H_
#define TIMERS_CONFIG_H_

#define TIM2_PRESCALER (uint16_t)72UL
void TIM2Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef TIM2_TimeBaseInitStruct;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	TIM2_TimeBaseInitStruct.TIM_Prescaler=TIM2_PRESCALER-1;  // 1us resolution at 72MHz clock
	TIM2_TimeBaseInitStruct.TIM_CounterMode=TIM_CounterMode_Up;
	TIM2_TimeBaseInitStruct.TIM_Period=0xFFFF;
	TIM2_TimeBaseInitStruct.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM2_TimeBaseInitStruct.TIM_RepetitionCounter=0;
	TIM_TimeBaseInit(TIM2, &TIM2_TimeBaseInitStruct);
	// Clear update interrupt bit
	TIM_ClearITPendingBit(TIM2, TIM_FLAG_Update);
	//Disable update interrupt
	TIM_ITConfig(TIM2, TIM_FLAG_Update,DISABLE);

	NVIC_InitStructure.NVIC_IRQChannel=TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=7;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelCmd=DISABLE;
	NVIC_Init(&NVIC_InitStructure);
	TIM_Cmd(TIM2, ENABLE);
}
//**********************************************************************************************
//**********************************************************************************************

#endif /* TIMERS_CONFIG_H_ */

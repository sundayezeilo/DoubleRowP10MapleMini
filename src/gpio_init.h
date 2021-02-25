/*
 * gpioInit.h
 *
 *  Created on: Sep 29, 2017
 *      Author: zeilotech
 */

#ifndef GPIO_INIT_H_
#define GPIO_INIT_H_
//**********************************************************************************************
//**********************************************************************************************
void gpioInit(void)
{
	 GPIO_InitTypeDef GPIO_InitStructure;					//GPIO structure used to initialize port
	 GPIO_InitStructure.GPIO_Pin=PIN_DMD_SH_CP|PIN_DMD_ST_CP;	//|PIN_DMD_nOE //select pins to initialize
	 GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;		//select output pull-up mode. External pull-up is required
	 GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;		//highest speed available
	 GPIO_Init(DMD_CLK_GPIO,&GPIO_InitStructure);				//using same structure we will initialize button pin
	 DMD_CLK_GPIO->ODR&=~(PIN_DMD_SH_CP|PIN_DMD_ST_CP);

	 GPIO_InitStructure.GPIO_Pin=PIN_DMD_B|PIN_DMD_A;
	 GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;		//select output pull-up mode. External pull-up is required
	 GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;		//highest speed available
	 GPIO_Init(DMD_ROW_SELECT_GPIO,&GPIO_InitStructure);

	 GPIO_InitStructure.GPIO_Pin=PIN_DMD_DT1;
	 GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;		//select output pull-up mode. External pull-up is required
	 GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;		//highest speed available
	 GPIO_Init(PIN_DMD_DT1_GPIO,&GPIO_InitStructure);

	 GPIO_InitStructure.GPIO_Pin=PIN_DMD_DT2;
	 GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;		//select output pull-up mode. External pull-up is required
	 GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;		//highest speed available
	 GPIO_Init(PIN_DMD_DT2_GPIO,&GPIO_InitStructure);
}

#endif /* GPIO_INIT_H_ */

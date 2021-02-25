#define PWM_TIMER TIM1
#define PWM_GPIO GPIOA
#define PWM_CHANNEL (uint32_t)3UL	//PA11
#define F_PWM (uint16_t)1500UL		//1KHz
#define MIN_PWM (uint16_t)0UL
#define MAX_PWM (uint16_t)1000UL
#define PWM_PRESCALER (XTAL/(F_PWM*MAX_PWM))	//prescaler  = 1000

#define PWM_DEFAUL_DUTY (uint16_t)(MAX_PWM*0.5)//50% DUTY

const uint32_t PWM_PIN[]={GPIO_Pin_8,GPIO_Pin_9,GPIO_Pin_10,GPIO_Pin_11};

uint16_t readPWM(uint8_t ch)
{
	switch(ch){
	case 0:	return PWM_TIMER->CCR1;
	case 1: return PWM_TIMER->CCR2;
	case 2: return PWM_TIMER->CCR3;
	case 3: return PWM_TIMER->CCR4;
	default:	return 0;
	}
}

void writePWM(uint8_t ch, uint16_t pwm)
{
	switch(ch){
		case 0:	PWM_TIMER->CCR1=pwm;break;
		case 1: PWM_TIMER->CCR2=pwm;break;
		case 2: PWM_TIMER->CCR3=pwm;break;
		case 3: PWM_TIMER->CCR4=pwm;break;
		default:	break;
	}
}

uint16_t setResetPWM(uint16_t pwm)
{
	uint16_t temp=readPWM(PWM_CHANNEL);
	writePWM(PWM_CHANNEL,pwm);
	return temp;
}

void initTimer1PWM(uint8_t chan,uint16_t pwm)
{
	TIM_TimeBaseInitTypeDef timerInitStructure;
	TIM_OCInitTypeDef outputChannelInit;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	TIM_TimeBaseStructInit(&timerInitStructure);

	timerInitStructure.TIM_Prescaler = PWM_PRESCALER-1;	//(1 is added in the function)
	timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	timerInitStructure.TIM_Period =MAX_PWM-1;		//TIMx_ARR - max PWM count (1 is added in the function)
	timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	timerInitStructure.TIM_RepetitionCounter = 0;	//RCR = 0;
	TIM_TimeBaseInit(TIM1, &timerInitStructure);

	TIM_OCStructInit(&outputChannelInit);

	outputChannelInit.TIM_OCMode = TIM_OCMode_PWM1;
	outputChannelInit.TIM_Pulse = pwm;	//stating pulse length
	outputChannelInit.TIM_OutputState = TIM_OutputState_Enable;
	outputChannelInit.TIM_OCPolarity = TIM_OCPolarity_High;

	switch(chan&0x3){
	    case 0:
	        TIM_OC1Init(TIM1, &outputChannelInit);
	        TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
	        break;
	    case 1:
	        TIM_OC2Init(TIM1, &outputChannelInit);
	        TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);
	        break;
	    case 2:
	        TIM_OC3Init(TIM1, &outputChannelInit);
	        TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);
	        break;
	    case 3:
	        TIM_OC4Init(TIM1, &outputChannelInit);
	        TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);
	        break;
	     default:	return;
	};
	////TIM_ARRPreloadConfig(TIMx, ENABLE);
	TIM_CtrlPWMOutputs(TIM1, ENABLE);
	TIM_Cmd(TIM1, ENABLE);

	GPIO_InitTypeDef gpioStructure;
	gpioStructure.GPIO_Pin=PWM_PIN[chan];
	gpioStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	gpioStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(PWM_GPIO,&gpioStructure);
}

void testPwm(void)
{
	uint16_t tPWM=setResetPWM(1000);	//read current PW and assign to variable
	tPWM++;	//increment PWM
	writePWM(PWM_CHANNEL,tPWM);	//write to PWM channel 0
	writePWM(PWM_CHANNEL+1,tPWM);	//write to PWM channel 1
	writePWM(PWM_CHANNEL+2,tPWM);	//write to PWM channel 2
	writePWM(PWM_CHANNEL+3,tPWM);	//write to PWM channel 3
}
//**********************************************************************************************
//**********************************************************************************************

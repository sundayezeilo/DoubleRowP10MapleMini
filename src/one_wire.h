#define ONE_WIRE_GPIO GPIOB
#define ONE_WIRE_PIN GPIO_Pin_13

void initOneWire(void)
{
	 GPIO_InitTypeDef One_Wire_GPIO_InitStructure;
	 One_Wire_GPIO_InitStructure.GPIO_Pin=ONE_WIRE_PIN;	//select pins to initialize
	 One_Wire_GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_OD;		//select output pull-up mode. External pull-up is required
	 One_Wire_GPIO_InitStructure.GPIO_Speed=GPIO_Speed_2MHz;		//highest speed available
	 GPIO_Init(ONE_WIRE_GPIO,&One_Wire_GPIO_InitStructure);				//using same structure we will initialize button pin
}
//**********************************************************************************************
//**********************************************************************************************
#define ONE_WIRE_BUS_RECOVERY_TIME (uint8_t)80UL
#define ONE_WIRE_RESET_TIME (uint16_t)700UL
#define ONE_WIRE_MAX_RESET_TIME (uint8_t)250UL

uint8_t oneWireReset(void)
{
		ONE_WIRE_GPIO->BRR=ONE_WIRE_PIN;			      							//pull wire low
      TIM2->CNT=0x00;							                          			   		   //start timer
      while(TIM2->CNT<ONE_WIRE_RESET_TIME);	    							   //wait for 480us
		ONE_WIRE_GPIO->BSRR=ONE_WIRE_PIN;	//turn to input
      TIM2->CNT=0x00;							                          			   	  	   //reset timer
      while((ONE_WIRE_GPIO->IDR&ONE_WIRE_PIN)&&(TIM2->CNT<ONE_WIRE_BUS_RECOVERY_TIME));
      if(!(ONE_WIRE_GPIO->IDR&ONE_WIRE_PIN))
      {
    	  TIM2->CNT=0x00;
        while(!(ONE_WIRE_GPIO->IDR&ONE_WIRE_PIN)&&(TIM2->CNT<=ONE_WIRE_MAX_RESET_TIME));
        if(TIM2->CNT>=ONE_WIRE_MAX_RESET_TIME)return 1;
        return 0;
      }
      return 1;
}
//**********************************************************************************************
//**********************************************************************************************
#define ONE_WIRE_BIT_TIME (uint8_t)85UL	                  //chekc datasheet
#define ONE_WIRE_BIT_LOW_TIME (uint8_t)10UL
//**********************************************************************************************
//**********************************************************************************************
void writeBit(uint8_t bit)
{
	ONE_WIRE_GPIO->BRR=ONE_WIRE_PIN;
	TIM2->CNT=0x00;
	while(TIM2->CNT<ONE_WIRE_BIT_LOW_TIME);
	if(bit)ONE_WIRE_GPIO->BSRR=ONE_WIRE_PIN;
	while(TIM2->CNT<ONE_WIRE_BIT_TIME);
	ONE_WIRE_GPIO->BSRR=ONE_WIRE_PIN;
	while(TIM2->CNT<ONE_WIRE_BUS_RECOVERY_TIME);
}
//**********************************************************************************************
//**********************************************************************************************
void writeByte(uint8_t byte)
{
 	uint8_t i;

	for(i=0;i<8;i++)
	{
	 	writeBit(byte&0x01);
		byte>>=1;
	}
}
//**********************************************************************************************
//**********************************************************************************************
#define LOGIC_ONE_BIT_TIME (uint8_t)20UL
#define MAX_READ_BIT_TIME (uint8_t)150UL
//**********************************************************************************************
//**********************************************************************************************
uint8_t readBit(void)
{
 	volatile uint8_t c=0;
	ONE_WIRE_GPIO->BRR=ONE_WIRE_PIN;	//pull IO low
	TIM2->CNT=0x00;
	while(TIM2->CNT<2);
	ONE_WIRE_GPIO->BSRR=ONE_WIRE_PIN;
	while((ONE_WIRE_GPIO->IDR&ONE_WIRE_PIN)&&(TIM2->CNT<LOGIC_ONE_BIT_TIME));
	if(ONE_WIRE_GPIO->IDR&ONE_WIRE_PIN)c=1;
	while(TIM2->CNT<MAX_READ_BIT_TIME);
	return c;
}
//**********************************************************************************************
//**********************************************************************************************
uint8_t readByte(void)
{
 	uint8_t i,value = 0;

	for (i=0;i<8;i++)
	{
	 	value>>=1;
		if(readBit())value|=0x80;
	}
	return value;
}
//**********************************************************************************************
//**********************************************************************************************
#define CONV_TIME_9_BIT (uint16_t)100UL
#define CONV_TIME_10_BIT (uint16_t)200UL
#define CONV_TIME_11_BIT (uint16_t)400UL
#define CONV_TIME_12_BIT (uint16_t)750UL
#define CONFIG_BYTE_9_BIT 0x1F
#define CONFIG_BYTE_10_BIT 0x3F
#define CONFIG_BYTE_11_BIT 0x5F
#define CONFIG_BYTE_12_BIT 0x7F
#define RECALL_E 0xB8
#define CONVERT_T 0x44
#define SKIP_ROM 0xCC
#define READ_ROM 0x33
#define MATCH_ROM 0x55
#define SEARCH_ROM 0xF0
#define ALARM_SEARCH 0xEC
#define COPY_SCRATCHPAD 0x48
#define READ_SCRATCHPAD 0xBE
#define WRITE_SCRATCHPAD 0x4E
#define READ_POWER_SUPPLY 0xB4

#define DS18B20_CONFIG_BYTE CONFIG_BYTE_12_BIT
#define ONE_WIRE_CONV_TIME CONV_TIME_12_BIT
#define MAX_RESOLUTION (uint8_t)12UL
#define RESOLUTION_SHIFT (MAX_RESOLUTION-8)
//**********************************************************************************************
//**********************************************************************************************
uint8_t getTemperature(uint16_t *temp)
{
	uint8_t c,i,s[32];
	oneWireReset();
	writeByte(SKIP_ROM);
	writeByte(WRITE_SCRATCHPAD);
	writeByte(0x00);
	writeByte(0x00);
	writeByte(DS18B20_CONFIG_BYTE);
	oneWireReset();
	writeByte(SKIP_ROM);
 	writeByte(CONVERT_T);
 	delay_ms(ONE_WIRE_CONV_TIME);
 	if((c=oneWireReset()))return c;
	writeByte(SKIP_ROM);
	writeByte(READ_SCRATCHPAD);
	for(i=0;i<9;i++)s[i]=readByte();
	*temp=((uint16_t)(s[1]<<8))|s[0];
	return 0;
}
//**********************************************************************************************
//**********************************************************************************************
void padString(char *s)
{
 	char *p;
	if((p=strrchr(s,'.'))==NULL)strcat(s,".00");
	else
	{
		if(*(p+2)=='\0')strcat(s,"0");
	}
}
//**********************************************************************************************
//**********************************************************************************************
void formatTemperature(char *s,uint16_t temp)
{
	uint16_t tempFraction;
	uint8_t tempWhole,negative=0;
	float data, fraction;

  	if(temp&0x8000){negative=1; temp=~temp+1;}
	tempWhole=temp>>RESOLUTION_SHIFT;
	data=tempWhole;
	tempFraction=temp<<(4-RESOLUTION_SHIFT);
	tempFraction&=0x000F;
	fraction=tempFraction*0.0625;
	data+=fraction;
	if(negative)data=0-data;
	sprintf(s,"%.2f",data);			//change .2f between .1f and .4f for desired precision (decimal places)
	padString(s);
	strcat(&s[strlen(s)]," C");
}

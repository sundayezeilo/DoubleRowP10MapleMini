#define SERVER_USART USART2
#define DEBUG_USART USART1

#define DEBUG_BAUD_RATE (uint32_t)115200UL
#define SERVER_BAUD_RATE (uint32_t)115200UL
#define USART1_GPIO GPIOA
#define USART2_GPIO GPIOA
#define USART3_GPIO GPIOB
void DEBUG_USART_Init(void)
{
    //USART configuration structure for USART1
    USART_InitTypeDef USART1_Init_Struct;
    //Bit configuration structure for GPIOA PIN2 and PIN3
    GPIO_InitTypeDef USART1_GPIO_Init_Struct;

    //Enalbe clock for USART1, AFIO and GPIOA
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    USART_Cmd(USART1, DISABLE);

    USART1_GPIO_Init_Struct.GPIO_Pin = GPIO_Pin_9;	//Tx
    USART1_GPIO_Init_Struct.GPIO_Speed = GPIO_Speed_10MHz;
    USART1_GPIO_Init_Struct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(USART1_GPIO, &USART1_GPIO_Init_Struct);

    USART1_GPIO_Init_Struct.GPIO_Pin = GPIO_Pin_10;	//Rx
    USART1_GPIO_Init_Struct.GPIO_Speed = GPIO_Speed_10MHz;
    USART1_GPIO_Init_Struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(USART1_GPIO, &USART1_GPIO_Init_Struct);

       //Baud rate 9600, 8-bit data, One stop bit
    //No parity, Do both Rx and Tx, No HW flow control
    USART1_Init_Struct.USART_BaudRate=DEBUG_BAUD_RATE;
    USART1_Init_Struct.USART_WordLength=USART_WordLength_8b;
    USART1_Init_Struct.USART_StopBits=USART_StopBits_1;
    USART1_Init_Struct.USART_Parity=USART_Parity_No ;
    USART1_Init_Struct.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;
    USART1_Init_Struct.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
    //Configure USART1
    USART_Init(USART1, &USART1_Init_Struct);
    //Enable RXNE interrupt
    //USART_ITConfig(USART1, USART_IT_RXNE|USART_IT_TXE, ENABLE);
    //USART_ITConfig(USART1,USART_IT_TXE,ENABLE);// enable USART1 Tx IRQ
    //Enable USART1 global interrupt
    //NVIC_EnableIRQ(USART1_IRQn);
    //Enable USART1
    USART_Cmd(USART1, ENABLE);
}
//**********************************************************************************************
//**********************************************************************************************
void SERVER_USART_Init(void)
{
    //USART configuration structure for USART2
    USART_InitTypeDef USART2_Init_Struct;
    GPIO_InitTypeDef USART2_GPIO_Init_Struct;

    //Enalbe clock for USART2, AFIO and GPIOB
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
    USART_Cmd(USART2, DISABLE);

    USART2_GPIO_Init_Struct.GPIO_Pin = GPIO_Pin_2;	//Tx
    USART2_GPIO_Init_Struct.GPIO_Speed = GPIO_Speed_50MHz;
    USART2_GPIO_Init_Struct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(USART2_GPIO, &USART2_GPIO_Init_Struct);

    USART2_GPIO_Init_Struct.GPIO_Pin = GPIO_Pin_3;	//Rx
    USART2_GPIO_Init_Struct.GPIO_Speed = GPIO_Speed_50MHz;
    USART2_GPIO_Init_Struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(USART2_GPIO, &USART2_GPIO_Init_Struct);

    //Baud rate, 8-bit data, One stop bit
    //No parity, Do both Rx and Tx, No HW flow control
    USART2_Init_Struct.USART_BaudRate=SERVER_BAUD_RATE;
    USART2_Init_Struct.USART_WordLength=USART_WordLength_8b;
    USART2_Init_Struct.USART_StopBits=USART_StopBits_1;
    USART2_Init_Struct.USART_Parity=USART_Parity_No ;
    USART2_Init_Struct.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;
    USART2_Init_Struct.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
    //Configure USART2
    USART_Init(USART2, &USART2_Init_Struct);
    //Enable RXNE interrupt
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    //USART_ITConfig(USART2,USART_IT_TXE,ENABLE);// enable USART2 Tx IRQ
    //Enable USART2 global interrupt
    NVIC_EnableIRQ(USART2_IRQn);//Enable USART2 IRQ
    USART_Cmd(USART2, ENABLE);
}
//**********************************************************************************************
//**********************************************************************************************
void DEBUG_X(const char *);
void writeUart(USART_TypeDef *,uint8_t);
void writeTextUart(USART_TypeDef *,char *);

void writeUart(USART_TypeDef *USARTx,volatile uint8_t  c)
{
	while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE)==RESET);	//use this for interrupt-driven transmission
	USART_SendData(USARTx, c);
}

void writeTextUart(USART_TypeDef *USARTx,char *s)
{
 	while(*s)writeUart(USARTx,*s++);
}

void DEBUG_X(const char *s)
{
 	while(*s)writeUart(DEBUG_USART,*s++);
}
//**********************************************************************************************
//**********************************************************************************************

#include<stddef.h>
#include "stm32f10x.h"

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>

//#define SYS_DEBUG
//#define DEBUG_DS18B
//**********************************************************************************************
//***********************************************************************************************
#define SETBIT(x,y) (x->BSRR=y)
#define CLRBIT(x,y) (x->BRR=y)
#define FLIPBIT(x,y) (x->ODR^=y)
#define CHECKBIT(x,y) (x->IDR&y)
//**********************************************************************************************
//**********************************************************************************************
#define XTAL (uint32_t)72000000UL

__IO uint8_t msTick=0;
void delay_us(uint16_t);
void delay_ms(uint32_t);

#define TIME_NOT_SET (uint8_t)4UL

enum{
	FONT_INDEX_OF_OFFSET = (uint8_t)0UL,
	FONT_INDEX_OF_BYTES_PER_CHAR,
	FONT_INDEX_OF_BYTES_PER_COLUMN,
	FONT_LENGTH_OF_INFO_BYTES		//3	mind this line in case of any modification
};
//**********************************************************************************************
//**********************************************************************************************
uint32_t BCDToBin(uint32_t c)
{
 	/*uint8_t msd=(c>>4)&0x0F;
	uint8_t lsd=(c&0x0F);
	return((msd*10)+lsd);*/
	return(((c>>4)*10)+(c&0x0f));
}

uint8_t binToBCD(uint8_t c)
{
 	return(((c/10)<<4)|(c%10));
}

uint16_t bytesToInt(uint8_t *s)
{
	return(((uint16_t)(s[0]<<8))|s[1]);
}

void swapBytes(uint8_t *s)
{
	uint8_t highByte=s[0];
	s[0]=s[1];
	s[1]=highByte;
}
//**********************************************************************************************
//**********************************************************************************************
//#include "i2c.h"
#include "timers_config.h"
//#include "one_wire.h"
#include "my_stm32f10x_adc.h"
//#include "STM32F1xx_ADC.h"
#include "my_stm32f103_pwm.h"
#include "flash.h"
#include "my_stm32f103_rtc.h"
#include "usarts.h"
#include "defines.h"
#include "fonts.h"
#include "gpio_init.h"
#include "prototypes.h"
#include "structs.h"
#include "delay.h"
#include "DS18B20.h"
//***********************************************************************************************
//***********************************************************************************************
//functions prototypes
void initVars(void);
void clearDMD(void);
void runLoop(void);
uint8_t getRtcc(void);
uint16_t padMsg(char *);
uint8_t formatMsg(char *);
uint8_t cstrMatch(const char *, const char *);

void initSys(void);
void TIM2Init(void);
void disableJTAG(void);

uint8_t setClock(void);
uint8_t selectMsg(void);
void USART1_Init(void);
void USART2_Init(void);
uint8_t printTemp(char *);
void checkServerRequest(void);
static uint8_t printTime(char *);
void readFlashData(char *,uint8_t);
void writeFlashData(char *,uint8_t);
void debug(USART_TypeDef *,const char*);
void writeUart(USART_TypeDef *,uint8_t);
void writeTextUart(USART_TypeDef *,char *);

void getBrightness(void);
uint8_t formatStatic(void);
void extractText(char *);
uint8_t getSize(char *, char *);

//**********************************************************************************************
//**********************************************************************************************
///const char msg1[]="!\"#$%&'(*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
//const char *msg1[]={"Hello, zeilotech integrated systems welcomes you to embedded world!","visit www.zis.com","contact us: info@zeilotech.com","\r\n"};
//**********************************************************************************************
//**********************************************************************************************
/*******************************************************************
Important Note!!!
Once a column of a char is read, shift the bits in the contents of the virtualShiftRegRowData
and update it with the new word. Then copy the virtualShiftRegRowData to the shiftRegRowData
when it's time to shift bits. Also ensure that a refresh cycle is complete before updating
the shiftRegRowData. As well, ensure that virtualShiftRegRowData copy is complete before reading
new word into virtualShiftRegRowData. Finally, update the screenRam with contents of the shiftRegRowData
that correspond with the rows to scan, each time scan is to be done.
*******************************************************************/
//**********************************************************************************************
//**********************************************************************************************
void enableGPIO(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|
			RCC_APB2Periph_GPIOD|RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOE|RCC_APB2Periph_GPIOF|
			RCC_APB2Periph_GPIOG, ENABLE);
}
//**********************************************************************************************
//**********************************************************************************************
void shiftRowData(void)
{
    uint8_t i,j;
    for(i=0;i<boardRowHeight;i++)
    {
        for(j=0;j<(totalShiftRegCols-1);j++)
        {
            DMD.virtualShiftRegRowData[i][j]<<=1;
            DMD.virtualShiftRegRowData[i][j]|=((DMD.virtualShiftRegRowData[i][j+1]>>7)&0x01);
        }
        DMD.virtualShiftRegRowData[i][j]<<=1;
        DMD.virtualShiftRegRowData[i][j]|=(uint8_t)((DMD.currentCol>>i)&0x01);
    }
}
//**********************************************************************************************
//**********************************************************************************************
void shiftRowsDown(void)
{
	uint8_t row,col;

	for(col=0;col<totalShiftRegCols;col++)
	{
		for(row=boardRowHeight-1;row>0;row--)
		{
			DMD.shiftRegRowData[row][col]=DMD.shiftRegRowData[row-1][col];
		}
		DMD.shiftRegRowData[row][col]=DMD.virtualShiftRegRowData[boardRowHeight-1][col];
	}
	//shift down the contents of virtual Ram one row down
	for(col=0;col<totalShiftRegCols;col++)
	{
		for(row=boardRowHeight-1;row>0;row--)
		{
			DMD.virtualShiftRegRowData[row][col]=DMD.virtualShiftRegRowData[row-1][col];
		}
	}
}
//**********************************************************************************************
//**********************************************************************************************
void copyVirtualRam(void)
{
    //transfer the contents of virtualShiftRowData to shiftRegRowData
    memcpy(&DMD.shiftRegRowData[0][0], &DMD.virtualShiftRegRowData[0][0],sizeof(DMD.virtualShiftRegRowData));
}
//**********************************************************************************************
//**********************************************************************************************
/*#define row1s (uint8_t)1UL
#define row2s (uint8_t)2UL
#define row3s (uint8_t)3UL
#define row4s (uint8_t)4UL*/
enum{
	row1s = (uint8_t)1UL,
	row2s,
	row3s,
	row4s
};
//**********************************************************************************************
//**********************************************************************************************
void copyToScreenRam(uint8_t row)
{
    uint8_t i,j, rowIndex[totalShiftRegRows];
    for(i=row,j=0;j<totalShiftRegRows;i+=4,j++){rowIndex[j]=i-1;}
    for(i=0;i<totalShiftRegRows;i++)
    {
        memcpy(&DMD.screenRam[i][0],&DMD.shiftRegRowData[rowIndex[i]][0],totalShiftRegCols);
    }
}
//**********************************************************************************************
//**********************************************************************************************
#include "uncommon_functions.h"
void getFont(void);
uint8_t getUserMsg(char *);
uint8_t dispTemp(void);
uint8_t scrollMsg(void);
uint8_t slideDown(void);
uint8_t displayClk(void);
uint8_t fillLeft2Right(void);

//const char *msg[]={"hello,","have a","lovely","day!","Bye!","\r\n"};

//#define STR_DELIM "\r\n"
/*
#define CPY_RAM (uint8_t)1UL
#define END_OF_STR (uint8_t)2UL
#define END_OF_MSG (uint8_t)3UL*/
enum{
	CPY_RAM=(uint8_t)1UL,
	END_OF_STR,
	END_OF_MSG
};
#define STATIC_MSG_DLY (uint16_t)3000UL
//**********************************************************************************************
//**********************************************************************************************
enum{
	MODE1=(uint8_t)0UL,
	MODE2,
	MODE3,
	MODE4,
	MODE5
};

uint8_t (*effectPtr[])(void)={fillLeft2Right,scrollMsg,displayClk,dispTemp,slideDown};
//**********************************************************************************************
//**********************************************************************************************
uint8_t fillLeft2Right(void)
{
    uint8_t row=0,temp=0,c=0;

    //goto endOfMsg;
    if(!DMD.init)
    {
        DMD.init=1; DMD.col=0; DMD.bitCnt=7; DMD.colCnt=0;
		  memset(user.msgRam,'\0',sizeof(user.msgRam));
		  if(getUserMsg(user.msgRam))goto endOfMsg;
		  extractText(user.msgRam);
		  user.staticMsgPtr=user.msgRam;
		  getFont();
        if(formatStatic())goto endOfMsg;
        user.msgPtr=user.staticMsgRam;
        readMsg();
        //read word here
        readNextWord();
        clearDMD(); for(uint8_t i=0;i<4;i++)writeScreen();//ensure screen doesn't flicker while clearing display during static display.
        DMD.sleep=2000;
#ifdef SYS_DEBUG
	DEBUG_X("Executing \"fillLeft2Right\" function!\r\n");
#endif
        return 0;
    }
    for(row=0;row<boardRowHeight;row++)
    {
        temp=(DMD.virtualShiftRegRowData[row][DMD.col]>>DMD.bitCnt);
        temp&=~0x01;//clear the 1 previously appended to the unused bit
        temp|=(uint8_t)((DMD.currentCol>>row)&0x01);//bring in new column
        c=DMD.bitCnt;
        while(c--){temp<<=1;temp|=0x01;}//this line appends 1s to all unused bits of the byte in question
        DMD.virtualShiftRegRowData[row][DMD.col]=temp;
    }
    //wipe unused columns of the board
    for(row=0;row<boardRowHeight;row++)
    {
    	for(c=(DMD.col+1);c<totalShiftRegCols;c++){DMD.virtualShiftRegRowData[row][c]=0xff;}
    }//end of wipe unused columns of the board
    if(!DMD.bitCnt--){DMD.bitCnt=7; DMD.col++;}//increment shift reg column address after every 8 columns (8 bits)
    if((DMD.colCnt/DMD.fontPtr[FONT_INDEX_OF_BYTES_PER_COLUMN])>=DMD.usedWord)
    {
        DMD.colCnt=0;
        if(!(*user.msgPtr))
        {
        		if(!(*user.staticMsgPtr))
            {
                //DMD.sleep=STATIC_MSG_DLY;
                endOfMsg:
                	DMD.init=0;
                	DMD.mode=MODE2;
                	DMD.sleep=STATIC_MSG_DLY;
#ifdef SYS_DEBUG
	DEBUG_X("Done executing \"fillLeft2Right\" function!\r\n");
#endif
                	return END_OF_MSG;
            }
            if(formatStatic())goto endOfMsg;
            user.msgPtr=user.staticMsgRam;
            readMsg();
            DMD.col=0;
            DMD.bitCnt=7;
            //read a word here
            readNextWord();
            DMD.sleep=STATIC_MSG_DLY;
            return END_OF_STR;
        }
        readMsg();
    }
    //read word here
    readNextWord();
    DMD.sleep=5;
    return 0;
}
//**********************************************************************************************
//**********************************************************************************************
uint8_t slideDown(void)
{
	static uint8_t lastMsgRead=0;
	uint8_t c;

	//goto endOfMsg;
	if(!DMD.init)
	{
		lastMsgRead=0;
		DMD.rowCnt=boardRowHeight;
		DMD.init=1; DMD.col=0; DMD.bitCnt=7; DMD.colCnt=0;
		memset(user.msgRam,'\0',sizeof(user.msgRam));
		if(getUserMsg(user.msgRam))goto endOfMsg;
		extractText(user.msgRam);
		user.staticMsgPtr=user.msgRam;
		getFont();
		if(formatStatic())goto endOfMsg;
		user.msgPtr=user.staticMsgRam;
		readMsg();
		//read word here
		readNextWord();
		clearDMD();  for(uint8_t i=0;i<4;i++)writeScreen();//ensure screen doesn't flicker while clearing display during static display.
		DMD.sleep=2000;
#ifdef SYS_DEBUG
	DEBUG_X("Executing \"slideDown\" function!\r\n");
#endif
		return 0;
	}
	if(!DMD.rowCnt)
	{
		DMD.rowCnt=boardRowHeight;
		DMD.sleep=STATIC_MSG_DLY;
		return CPY_RAM;
	}
	if(DMD.rowCnt==boardRowHeight)
	{
		clearDMD();for(uint8_t i=0;i<4;i++)writeScreen();//ensure screen doesn't flicker while clearing display during static display.
		if(lastMsgRead)
		{
			endOfMsg:
				DMD.init=0;
				DMD.sleep=2;
				DMD.mode=MODE1;		//Mode changes to the next mode here
#ifdef SYS_DEBUG
	DEBUG_X("Done executing \"slideDown\" function!\r\n");
#endif
				return 0;
		}
		while((c=fillLeft2Right())!=END_OF_MSG){if(c==END_OF_STR)break;}
		if(c==END_OF_MSG){lastMsgRead=1;DMD.mode=MODE5;DMD.init=1;}//DMD.mode remains this mode (MODE5) here, because the returning function
	}																							//has updated Mode, while the last message has not been printed
	shiftRowsDown();
	DMD.sleep=50;
	DMD.rowCnt--;
	return CPY_RAM;
}
//**********************************************************************************************
//**********************************************************************************************
#define TIME_FMT_STR "THE TIME IS %x:%02x %s"
#define DATE_FMT_STR "TODAY IS %s, %s %x, 20%02x"
#define TIME_DATE_FMT_STR "TIME IS %x:%02x %s TODAY IS %s, %s %x, 20%02x"
const char *MERIDIAN[]={"AM","PM"};
const char *DAYS[]={"Sun", "Mon", "Tue", "Wed", "Thur", "Fri", "Sat"};
const char *MONTHS[]={"January","January", "February", "March", "April", "May", "June", "July", "August","September", "October", "November", "December"};
//**********************************************************************************************
//**********************************************************************************************
uint8_t displayClk(void)
{
	//goto endOfMsg;
	if(!DMD.init)
	{
#ifdef SYS_DEBUG
		DEBUG_X("Starting \"displayClk\"function!\r\n");
#endif
		clearDMD();
		memset(sys.clkCalMsg,'\0',sizeof(sys.clkCalMsg));	//this line is very important!!!
		memset(sys.textBuff,'\0',sizeof(sys.textBuff));	//this line is very important!!!
		getFont();

		printTime(sys.clkCalMsg);
#ifdef SYS_DEBUG
		DEBUG_X("returned from \"printTime\"function!\r\n");
#endif
		if(sys.clockStatus)
		{
			strcpy(sys.textBuff,"INCORRECT TIME AND DATE!");
			padMsg(sys.textBuff);
		}
		strcat(sys.textBuff,sys.clkCalMsg);
#ifdef SYS_DEBUG
		DEBUG_X("About to execute \"padMsg\"function!\r\n");
#endif
		padMsg(sys.textBuff);
#ifdef SYS_DEBUG
		DEBUG_X("Done executing \"padMsg\"function!\r\n");
#endif
		user.msgPtr=sys.textBuff;
#ifdef SYS_DEBUG
		DEBUG_X("About to execute \"readMsg\"function!\r\n");
#endif
		readMsg();
#ifdef SYS_DEBUG
		DEBUG_X("Done executing \"readMsg\"function!\r\n");
#endif
#ifdef SYS_DEBUG
		DEBUG_X("About to execute \"readNextWord\"function!\r\n");
#endif
		readNextWord();
#ifdef SYS_DEBUG
		DEBUG_X("Done executing \"readNextWord\"function!\r\n");
#endif
		DMD.init=1;
		DMD.sleep=2000;
		return 0;
	}
	if(scrollMsg()==END_OF_MSG)
	{//optional exit path. In case message display is not required, this label can be called on entry to this routine.
		endOfMsg:
			DMD.init=0;
			DMD.mode=MODE4;
#ifdef SYS_DEBUG
		DEBUG_X("Done executing \"displayClk\"function!\r\n");
#endif
			return END_OF_MSG;
	}
	return 0;
}
//**********************************************************************************************
//**********************************************************************************************
uint8_t dispTemp(void)
{
	//goto endOfMsg;
	uint8_t c;
	if(!DMD.init)
	{
#ifdef SYS_DEBUG
	DEBUG_X("Executing \"DisplayTemp\" function!\r\n");
#endif
		clearDMD();
		memset(sys.tempMsg,'\0',sizeof(sys.tempMsg));	//this line is very important!!!
		memset(sys.textBuff,'\0',sizeof(sys.textBuff));	//this line is very important!!!
		getFont();
		if(!(c=printTemp(sys.tempMsg)))
		{
			strcpy(sys.textBuff,"THE TEMPERATURE IS ");
			strcat(sys.textBuff,sys.tempMsg);
#ifdef SYS_DEBUG
			DEBUG_X(sys.textBuff);
			DEBUG_X("\r\n");
#endif
		}
		else{strcpy(sys.textBuff,"TEMPERATURE SENSOR ERROR!");}
		padMsg(sys.textBuff);
		user.msgPtr=sys.textBuff;
		readMsg();
		readNextWord();
		DMD.init=1;
		return 0;
	}
	if(scrollMsg()==END_OF_MSG)
	{
		endOfMsg:
			DMD.init=0;
			DMD.mode=MODE5;
#ifdef SYS_DEBUG
	DEBUG_X("Done executing \"DisplayTemp\" function!\r\n");
#endif
			return END_OF_MSG;
	}
	return 0;
}
//**********************************************************************************************
//**********************************************************************************************
uint8_t scrollMsg(void)
{
	if(!DMD.init)
    {
    	  memset(user.msgRam,'\0',sizeof(user.msgRam));
    	  getUserMsg(user.msgRam);
    	  DMD.init=1;
        clearDMD();
        getFont();

        memset(sys.textBuff,'\0',sizeof(sys.textBuff));	//this line is very important!!!
        strcpy(sys.textBuff,user.msgRam);	//replace msg1 later with user.msgRam
        formatMsg(sys.textBuff);
        user.msgPtr=sys.textBuff;
        readMsg();
        readNextWord();
        DMD.sleep=2000;
#ifdef SYS_DEBUG
	DEBUG_X("Executing \"scrollMsg\" function!\r\n");
#endif
        return 0;
    }else{DMD.sleep=50;}
    if((DMD.colCnt/DMD.fontPtr[FONT_INDEX_OF_BYTES_PER_COLUMN])>=DMD.usedWord)
    {
        DMD.colCnt=0;
        if(!(*user.msgPtr))
        {//optional exit path. In case message display is not required, this label can be called on entry to this routine.
        		endOfMsg:
					DMD.init=0;
					DMD.mode=MODE3;
#ifdef SYS_DEBUG
	DEBUG_X("Done executing \"scrollMsg\" function!\r\n");
#endif
					return END_OF_MSG;
        }
       readMsg();
    }
    shiftRowData();
    readNextWord();
    return 0;
}
//**********************************************************************************************
//**********************************************************************************************
void clearDMD(void)
{
    memset(&DMD.shiftRegRowData[0][0],0xff,sizeof(DMD.shiftRegRowData));
    memset(&DMD.virtualShiftRegRowData[0][0],0xff,sizeof(DMD.virtualShiftRegRowData));
    memset(&DMD.screenRam[0][0],0xff,sizeof(DMD.screenRam));
}
//**********************************************************************************************
//**********************************************************************************************
enum{
	MSG1 = (uint8_t)0UL,
	MSG2,
	MSG3,
	MSG4,
	USER_CONFIG_MSG_ADD
};
#define TOTAL_NO_OF_MSG (uint8_t)4UL
//**********************************************************************************************
//**********************************************************************************************
void initVars(void)
{
	clearDMD();
   DMD.init=0;
   DMD.mode=MODE1;
   DMD.fontPtr=NULL;
   DMD.row=row1s;
   sys.brightAdjust=2000;
}
//**********************************************************************************************
//**********************************************************************************************
uint8_t formatMsg(char *s1)
{
	uint16_t cnt=0,index=0;
	char s2[sizeof(sys.textBuff)];
	memset(s2,'\0',sizeof(s2));
	while((((*(s1+index))=='\n')||((*(s1+index))=='\r')||(*(s1+index))==' '))index++;	//skip any newline or space char appended before valid characters
	while(*(s1+index))
	{
			while(isprint(*(s1+index)))
			{
				s2[cnt++]=*(s1+index++);
				s2[cnt]='\0';
			}
			if(((strchr((s1+index),'\r'))!=NULL)||((strchr((s1+index),'\n'))!=NULL)){cnt=padMsg(s2);}
			while((((*(s1+index))=='\n')||((*(s1+index))=='\r')))index++;
	}
	padMsg(s2);
	strcpy(s1,s2);
#ifdef SYS_DEBUG
	uint16_t i;
	DEBUG_X(s2);
	DEBUG_X("\r\n");
	for(i=strlen(s2);s2[i-1]==' ';i--);
	i=strlen(s2)-i;
	memset(s2,'\0',sizeof(s2));
	sprintf(s2,"%i\r\n",i);
	DEBUG_X("last padding:\r\n");
	DEBUG_X(s2);
#endif
	return 0;
}
//**********************************************************************************************
//**********************************************************************************************
uint16_t padMsg(char *p)
{
    uint16_t i, x=strlen(p),c=boardColWidth/DMD.fontPtr[FONT_LENGTH_OF_INFO_BYTES];//DMD.fontPtr[3] indexes the 1st element of words for space bar

    for(i=x;p[i-1]==' ';i--);
    i=x-i;

    if(boardColWidth%DMD.fontPtr[FONT_LENGTH_OF_INFO_BYTES])c+=1;
    if(i<c)
    {
    	c-=i;
    	memset(&p[x], ' ', c+=1);
    	p[x+c]='\0';
    }
    return(strlen(p));
}
//**********************************************************************************************
//**********************************************************************************************
static uint8_t printTime(char *s)
{
 	uint8_t c=0;
 	uint8_t rtcc[RTCC_DATA_SIZE];
 	char clkString[48];
 	memset(clkString,'\0',sizeof(clkString));
 	memset(rtcc,'\0',sizeof(rtcc));


#ifdef SYS_DEBUG
	DEBUG_X("Executing \"PrintTime\" function!\r\n");
#endif
 	uint32_t secs=RTC_GetCounter();
 	makeTimeFromSeconds(rtcc,secs);
 	binToBcdArray(rtcc,sizeof(rtcc));

 	//rtcc[MONTHS_ADDRESS]&=~0x80;
 	rtcc[MONTHS_ADDRESS]=BCDToBin(rtcc[MONTHS_ADDRESS]);
 	//rtcc[HOURS_ADDRESS]=((rtcc[HOURS_ADDRESS]>>4)*10)+(rtcc[HOURS_ADDRESS]&0x0f);//BCD - to- BIN
 	rtcc[HOURS_ADDRESS]=BCDToBin(rtcc[HOURS_ADDRESS]);
 	if(!rtcc[HOURS_ADDRESS]){rtcc[HOURS_ADDRESS]=0x12;c=0;}	//this takes care of 12:00 midnight and prints 12:00 AM, instead of 00:00 AM
 	else
 	{
 		if(rtcc[HOURS_ADDRESS]<12)c=0;
	   else
	   {
		   c=1;
		   if(rtcc[HOURS_ADDRESS]>12)rtcc[HOURS_ADDRESS]-=12;
	   }
 		rtcc[HOURS_ADDRESS]=binToBCD(rtcc[HOURS_ADDRESS]);
 		//rtcc[HOURS_ADDRESS]=((rtcc[HOURS_ADDRESS]/10)<<4)|(rtcc[HOURS_ADDRESS]%10);
 	}
#ifdef SYS_DEBUG
 	char tempBuf[24];
 	sprintf(tempBuf,"%s returned for month\r\n",MONTHS[rtcc[MONTHS_ADDRESS]]);
	DEBUG_X(tempBuf);
#endif
#ifdef SYS_DEBUG
	DEBUG_X("about to execute first \"sprintf\" function!\r\n");
#endif
	sprintf(clkString,TIME_FMT_STR,rtcc[HOURS_ADDRESS],rtcc[MINUTES_ADDRESS],MERIDIAN[c]);//close this function here once "sprintf" issue is resolved!!!
#ifdef SYS_DEBUG
	DEBUG_X("Done executing first \"sprintf\" function!\r\n");
#endif

#ifdef SYS_DEBUG
	DEBUG_X("about to execute \"padMsg\" function!\r\n");
#endif
	c=padMsg(clkString);
#ifdef SYS_DEBUG
	DEBUG_X("Done executing \"padMsg\" function!\r\n");
#endif

#ifdef SYS_DEBUG
	DEBUG_X("about to execute second \"sprintf\" function!\r\n");
#endif

	sprintf(&clkString[c],DATE_FMT_STR,DAYS[rtcc[DAY_OF_WEEK_ADDRESS]],
			MONTHS[rtcc[MONTHS_ADDRESS]],rtcc[DAYS_ADDRESS],rtcc[YEARS_ADDRESS]);
#ifdef SYS_DEBUG
	DEBUG_X("Done executing second \"sprintf\" function!\r\n");
#endif
strcpy(s,clkString);
#ifdef SYS_DEBUG
	DEBUG_X(s);
	DEBUG_X("\r\nDone executing \"PrintTime\" function!\r\n");
#endif
	return 0;
}
//**********************************************************************************************
//**********************************************************************************************
uint8_t printTemp(char *s)
{
/*	uint16_t temp;
	if(getTemperature(&temp))
	{
#ifdef SYS_DEBUG
		DEBUG_X("Temperature Sensor not read\r\n");
#endif
		return 1;
	}
	formatTemperature(s,temp);
#ifdef SYS_DEBUG
	DEBUG_X("Temperature Sensor read OK!\r\n");
#endif
*/
	uint16_t data[TEMP_SENSORS_COUNT];
	ds18b20ConvertTemperatureAll();//start conversion here
	ds18b20WaitForConversion();
	if(readDS1820(data))
	{
#ifdef SYS_DEBUG
		DEBUG_X("Temperature Sensor not read\r\n");
#endif
		return 1;
	}
	formatTemperature(s,data[0]);
#ifdef SYS_DEBUG
	DEBUG_X("Temperature Sensor read OK!\r\n");
#endif
	return 0;
}
//**********************************************************************************************
//**********************************************************************************************
void USART1_IRQHandler(void)
{
    //uint8_t rxByte;
    if(USART_GetITStatus(USART1, USART_IT_RXNE)!=RESET)		//RXNE handler
    {
    	/*rxByte=*/USART_ReceiveData(USART1);
    }
    if(USART_GetITStatus(USART1, USART_IT_TXE)!=RESET)
    {
    	//USART_ClearITPendingBit(USART1, USART_IT_TXE);
    }
}
//**********************************************************************************************
//**********************************************************************************************
struct{
			__IO char txBuff[1048],rxBuff[1048];
			__IO uint16_t txPtr,rxPtr,length2Write;
			__IO uint8_t timeout,busy,writePending;
		}uart;
//**********************************************************************************************
//**********************************************************************************************
void USART2_IRQHandler(void)
{
    uint8_t rxByte;
    if(USART_GetITStatus(USART2, USART_IT_RXNE)!=RESET)		//RXNE handler
    {
			rxByte=USART_ReceiveData(USART2);
    		if(!uart.busy&&(uart.rxPtr<1048)){uart.rxBuff[uart.rxPtr++]=rxByte;}
    		uart.timeout=0;
    }
    if(USART_GetITStatus(USART2, USART_IT_TXE)!=RESET)
    {
        if(uart.txPtr<uart.length2Write){USART_SendData(USART2, uart.txBuff[uart.txPtr++]);}
    	  else
    	  {
    		  uart.busy=0;
    		  uart.txPtr=0;
    		  uart.writePending=0;
    		  uart.length2Write=0;
    		  USART_ITConfig(USART2,USART_IT_TXE, DISABLE);
    		  memset((char *)uart.txBuff,'\0',sizeof(uart.txBuff));
    	  }
    }
}
//**********************************************************************************************
//**********************************************************************************************
#define SERVER_REQ_CMD '$'

enum{
	MSG_REQ_BYTE = (uint8_t)0UL,
	CLOCK_REQ_BYTE,
	MSG_SELECT_REQ_BYTE,
	BRIGHTNESS_ADJUST_REQ_BYTE,
	DELETE_MSG_REQ_BYTE
};

enum{
	INDEX_OF_SERVER_REQ_CMD = (uint8_t)0UL,
	INDEX_OF_REQ_NO,
	INDEX_OF_RW_BYTE,
	INDEX_OF_MSG_NO,
	INDEX_OF_FONT_SELECT,
	INDEX_OF_USER_MSG
};

enum{
	FLASH_INDEX_OF_MSG_FONT,
	FLASH_INDEX_OF_USER_MSG_TEXT
};

enum{
	FLASH_INDEX_OF_MSG_SELECT,
	FLASH_INDEX_OF_BRIGHTNESS_ADJUST
};

//#define LENGTH_OF_USR_SET_BYTES (uint8_t)1UL

struct{
	__IO uint8_t readPending,writePending,msgNo;
	__IO uint16_t startAdd,length2Read,length2Write,addPtr;
}flash;

#define DEVICE_READ (uint8_t)0UL
#define DEVICE_WRITE (uint8_t)1UL

void readFlashData(char *dest, uint8_t startAdd)
{
	char s[FLASH_PAGE_SIZE];

	memset(s,'\0',sizeof(s));
	//Note: introduce a check here!!!
	readMessage(s,sizeof(s),MSG1_PAGE_ADDRESS-startAdd);
	memcpy(dest,s,sizeof(s));
}
//**********************************************************************************************
//**********************************************************************************************
void writeFlashData(char *buff,uint8_t startAdd)
{
	char s[FLASH_PAGE_SIZE];

	memset(s,'\0',sizeof(s));
	memcpy(s,buff,sizeof(s));
	uint16_t size=alignOnLong(FLASH_PAGE_SIZE);			//(strlen(s));
	//Note: introduce a check here!!!
	if(size>FLASH_PAGE_SIZE)size=FLASH_PAGE_SIZE;
	writeMessage(s,size,MSG1_PAGE_ADDRESS-startAdd);
}
//**********************************************************************************************
//**********************************************************************************************
#define LENGTH_OF_MSG_INFO_BYTES (uint8_t)2UL
#define LENGTH_OF_SERVER_REQ_CMD_BYTES (uint8_t)3UL

void printStatus(void)
{
	memcpy((char *)uart.txBuff,(char *)uart.rxBuff,sizeof(uart.txBuff));
	memset((char *)uart.rxBuff,'\0',sizeof(uart.rxBuff));
	uart.rxPtr=0;
	uart.txPtr=0;
	uart.busy=0;
	uart.writePending=1;
	uart.length2Write+=LENGTH_OF_SERVER_REQ_CMD_BYTES;
}
//**********************************************************************************************
//**********************************************************************************************
void rtcc(volatile char *p,uint8_t rw)
{
	uint8_t s[RTCC_DATA_SIZE];
	if(rw==DEVICE_WRITE)
	{
		memcpy((char*)s,(char*)p,sizeof(s));
		adjustClock(s);
		BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);		//update backup register on user access
		sys.clockStatus=0;
	}
	else if(rw==DEVICE_READ)
	{
		uint32_t secs=RTC_GetCounter();
		makeTimeFromSeconds(s,secs);
		binToBcdArray(s,RTCC_DATA_SIZE);
		memcpy((char *)p,s,sizeof(s));
	}
}
//**********************************************************************************************
//**********************************************************************************************
void deviceRead(void)
{
	char tempBuff[1024];
	memset(tempBuff,'\0',sizeof(tempBuff));
	switch(uart.rxBuff[INDEX_OF_REQ_NO]-'0')
	{
		case MSG_REQ_BYTE:
			if(((flash.startAdd=uart.rxBuff[INDEX_OF_MSG_NO]-'0'))<TOTAL_NO_OF_MSG)
			{
				readFlashData((char *)&uart.rxBuff[INDEX_OF_FONT_SELECT],flash.startAdd);
				if(!isprint(uart.rxBuff[INDEX_OF_USER_MSG]))
				{
					if((uart.rxBuff[INDEX_OF_USER_MSG]!='\r')&&(uart.rxBuff[INDEX_OF_USER_MSG]!='\n'))
					{
						uart.rxBuff[INDEX_OF_USER_MSG]='\0';
						uart.rxBuff[INDEX_OF_FONT_SELECT]='\0';
					}
				}
				/* if((uart.rxBuff[INDEX_OF_FONT_SELECT]>MAX_FONT_NO)||(uart.rxBuff[INDEX_OF_FONT_SELECT]<MIN_FONT_NO))
				{
					uart.rxBuff[INDEX_OF_FONT_SELECT]=DEFAULT_FONT;
				}*/
				if(uart.rxBuff[INDEX_OF_FONT_SELECT])uart.rxBuff[INDEX_OF_FONT_SELECT]+='0';		//comment out soon!!
				uart.length2Write=LENGTH_OF_MSG_INFO_BYTES+strlen((char *)&uart.rxBuff[INDEX_OF_USER_MSG]);
				printStatus();
			}
			else{memset((char *)uart.rxBuff,'\0',sizeof(uart.rxBuff));}
			break;
		case CLOCK_REQ_BYTE:
			uart.length2Write=7;
			rtcc(&uart.rxBuff[LENGTH_OF_SERVER_REQ_CMD_BYTES],DEVICE_READ);
			printStatus();
			break;
		case MSG_SELECT_REQ_BYTE:
			uart.length2Write=1;	//msg_select_byte is 1 byte
			flash.startAdd=USER_CONFIG_MSG_ADD;
			readFlashData(tempBuff,flash.startAdd);	//read 2-byte config info
			uart.rxBuff[LENGTH_OF_SERVER_REQ_CMD_BYTES]=tempBuff[FLASH_INDEX_OF_MSG_SELECT];//+'0';//browser sends ascii
			printStatus();
			break;
		case BRIGHTNESS_ADJUST_REQ_BYTE:
			uart.length2Write=1;
			flash.startAdd=USER_CONFIG_MSG_ADD;
			readFlashData(tempBuff,flash.startAdd);
			uart.rxBuff[LENGTH_OF_SERVER_REQ_CMD_BYTES]=tempBuff[FLASH_INDEX_OF_BRIGHTNESS_ADJUST];	//+'0';
			printStatus();
#ifdef SYS_DEBUG
			DEBUG_X("read Brightness from FLASH SUCCESS:\r\n");
			writeUart(DEBUG_USART,tempBuff[FLASH_INDEX_OF_BRIGHTNESS_ADJUST]);//Note: uart.rxBuff has been reset in printStatus function, hence not suitable here!!!
			DEBUG_X("\r\nreturn Brightness to server ok!\r\n");
#endif
			break;
		default: break;
	}
}
//**********************************************************************************************
//**********************************************************************************************
void deviceWrite(void)
{
	char tempBuff[1024];
	memset(tempBuff,'\0',sizeof(tempBuff));
	uart.length2Write=0;
	switch(uart.rxBuff[INDEX_OF_REQ_NO]-'0')
	{
		case MSG_REQ_BYTE:
			if(((flash.startAdd=uart.rxBuff[INDEX_OF_MSG_NO]-'0'))<TOTAL_NO_OF_MSG)
			{
				uart.rxBuff[INDEX_OF_FONT_SELECT]-='0';		//comment out soon!!
				writeFlashData((char *)&uart.rxBuff[INDEX_OF_FONT_SELECT],flash.startAdd);
				printStatus();
			}
			else{memset((char *)uart.rxBuff,'\0',sizeof(uart.rxBuff));}
			break;
		case CLOCK_REQ_BYTE:
			rtcc(&uart.rxBuff[LENGTH_OF_SERVER_REQ_CMD_BYTES],DEVICE_WRITE);
			sys.clockStatus=0;
			printStatus();
			break;
		case MSG_SELECT_REQ_BYTE:
			flash.startAdd=USER_CONFIG_MSG_ADD;
			readFlashData(tempBuff,flash.startAdd);
			tempBuff[FLASH_INDEX_OF_MSG_SELECT]=uart.rxBuff[LENGTH_OF_SERVER_REQ_CMD_BYTES];//-'0'server sends ascii
			writeFlashData(tempBuff,flash.startAdd);
			printStatus();
			break;
		case BRIGHTNESS_ADJUST_REQ_BYTE:
			flash.startAdd=USER_CONFIG_MSG_ADD;
			readFlashData(tempBuff,flash.startAdd);
			tempBuff[FLASH_INDEX_OF_BRIGHTNESS_ADJUST]=uart.rxBuff[LENGTH_OF_SERVER_REQ_CMD_BYTES];	//-'0';
#ifdef SYS_DEBUG
			DEBUG_X("received Brightness from server:\r\n");
			writeUart(DEBUG_USART,uart.rxBuff[LENGTH_OF_SERVER_REQ_CMD_BYTES]);
			DEBUG_X("\r\n");
#endif
			writeFlashData(tempBuff,flash.startAdd);
			printStatus();
#ifdef SYS_DEBUG
			DEBUG_X("write Brightness to flash SUCCESS!\r\nresponse to server sent ok!\r\n");
#endif
			break;
		default:	break;
	}
}
//**********************************************************************************************
//**********************************************************************************************//**********************************************************************************************
//**********************************************************************************************
void checkServerRequest(void)
{
	if((!uart.busy)&&(uart.rxPtr>=LENGTH_OF_SERVER_REQ_CMD_BYTES)&&(uart.timeout>20))
   {
		if(uart.rxBuff[INDEX_OF_SERVER_REQ_CMD]==SERVER_REQ_CMD)
		{
			if((uart.rxBuff[INDEX_OF_RW_BYTE]-'0')==DEVICE_READ){deviceRead();}
			else
			{
				if((uart.rxBuff[INDEX_OF_RW_BYTE]-'0')==DEVICE_WRITE){deviceWrite();}
				else if((uart.rxBuff[INDEX_OF_RW_BYTE]-'0')==DELETE_MSG_REQ_BYTE)
				{
					uart.rxBuff[INDEX_OF_RW_BYTE]=DEVICE_WRITE+'0';	//
					deviceWrite();
					uart.txBuff[INDEX_OF_RW_BYTE]=DELETE_MSG_REQ_BYTE+'0';	//data to be sent to the server have been transferred to txBuff in the printStatus routine, before this line
				}
			}
		}//reset buffIndex if irrelevant bytes are received
    	uart.rxPtr=0;
    }
	 else
	 {//reset buffIndex if irrelevant bytes are received
		 if((!uart.busy)&&(uart.rxPtr<LENGTH_OF_SERVER_REQ_CMD_BYTES)&&(uart.timeout>20))uart.rxPtr=0;
	 }
    if(!uart.busy&&uart.writePending)
    {
		uart.busy=1;
		USART_SendData(SERVER_USART,uart.txBuff[uart.txPtr++]);
		USART_ITConfig(SERVER_USART,USART_IT_TXE,ENABLE);//start uart write and enable Tx IRQ
	 }
}
//**********************************************************************************************
//**********************************************************************************************
char asciiToBin(char c)
{
	if((c>='0')&&(c<='9'))c-='0';
	else
	{
		if((c>='A')&&(c<='F'))c-='7';
	}
	return c;
}
//**********************************************************************************************
//**********************************************************************************************

void initDS18B20(void)
{
	ds18b20Init();
	searchOneWire();
}


void initSys(void)
{
	enableGPIO();//note that this function must have been executed before calling disableJTAG()
	disableJTAG();
	TIM2Init();
	gpioInit();
	SysTick_Config(SystemCoreClock/1000);
	DEBUG_USART_Init();
	SERVER_USART_Init();
	ADCIOConfigure();
	ADCRegConfigure();
   initTimer1PWM(PWM_CHANNEL,MAX_PWM*0);
   initDS18B20();
	initVars();
	sys.clockStatus=initRTC();
}
//**********************************************************************************************
//**********************************************************************************************
//#define GPIO_Remap_SWJ_JTAGDisable  ((uint32_t)0x00300200)
void disableJTAG(void)
{
	AFIO->MAPR&=(uint32_t)~AFIO_MAPR_SWJ_CFG;
	AFIO->MAPR|=AFIO_MAPR_SWJ_CFG_DISABLE;
	//GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	//GPIO_PinRemapConfig (GPIO_Remap_SWJ_Disable, ENABLE);
}
//**********************************************************************************************
//**********************************************************************************************
int main(void)
{
	initSys();
	delay_ms(10);
	sys.refreshScreen=1;
	DMD.sleep=10;
	printTemp(sys.tempMsg);			//discard initial value of DS18B20
   runLoop();
	NVIC_SystemReset();
	while(1);
   //return EXIT_SUCCESS;
}
//**********************************************************************************************
//**********************************************************************************************
const char *MSG[]={"MESSAGE-1","MESSAGE-2","MESSAGE-3","MESSAGE-4"};

uint8_t getUserMsg(char *dest)
{
	char s[FLASH_PAGE_SIZE],c;
	memset(s,'\0',sizeof(s));

	if(!DMD.msgSelect)
	{
		DMD.msgCnt=0;
		readFlashData(s,USER_CONFIG_MSG_ADD);
		DMD.msgSelect=asciiToBin(s[FLASH_INDEX_OF_MSG_SELECT]);
		DMD.msgSelect&=0x0F;
		if(!DMD.msgSelect)
		{
			strcat(dest," NO MESSAGE SELECTED TO DISPLAY!");
			padMsg(dest);
			strcat(dest," EDIT AND SELECT MESSAGE TO DISPLAY!");
			return 1;
		}
	}
	selectMsg();
	readFlashData(s,DMD.msgToDisplay);
	///////////////////////////////////////////////////////////////////////////
	/*readFlashData(s,USER_CONFIG_MSG_ADD);	//get configuration info
	if(s[FLASH_INDEX_OF_MSG_SELECT]>=TOTAL_NO_OF_MSG)return 1;
	readFlashData(s,s[FLASH_INDEX_OF_MSG_SELECT]);*/
	///////////////////////////////////////////////////////////////////////////
	c=s[FLASH_INDEX_OF_USER_MSG_TEXT];
	if(!isprint(c))
	{
		if((c!='\r')&&(c!='\n'))
		{
			strcpy(dest,MSG[DMD.msgToDisplay]);
			strcat(dest," SELECTED BUT EMPTY!");
			return 1;
		}
	}
	c=s[FLASH_INDEX_OF_MSG_FONT];
	if((c>MAX_FONT_NO)||(c<MIN_FONT_NO))DMD.fontSelect=DEFAULT_FONT;	//select default font, if invalid font is returned
	else{DMD.fontSelect=s[FLASH_INDEX_OF_MSG_FONT];}	//select the returned font, if valid
	strcpy(dest,&s[FLASH_INDEX_OF_USER_MSG_TEXT]);		//skip index of message font and copy user text
	return 0;
}
//**********************************************************************************************
//**********************************************************************************************
void runLoop(void)
{
    uint8_t c;
    while(1)
    {
    	if(sys.refreshScreen)
		{
			sys.refreshScreen=0;
			writeScreen();
		}
		if(DMD.scanComplete&&DMD.updateMsg)
		{
			if((c=(*effectPtr[DMD.mode])())!=CPY_RAM){copyVirtualRam();}
			DMD.msgUpdateTick=0;
			DMD.updateMsg=0;
		}
		checkServerRequest();
		if(!sys.brightAdjust)
		{
			sys.brightAdjust=2000;
			getBrightness();
		}
    }
}
//**********************************************************************************************
//**********************************************************************************************
#define screenRefreshInterval (uint8_t)2UL  //refresh screen in 2 ms
void SysTick_Handler(void)
{
	static uint8_t refreshCnt=0;
	msTick=1;
	if(++refreshCnt>=screenRefreshInterval)
	{
		refreshCnt=0;
	   sys.refreshScreen=1;
	}
	if(++DMD.msgUpdateTick>=DMD.sleep)//scrollInterval
	{
		DMD.updateMsg=1;
	   DMD.msgUpdateTick=0;
	}
	if(uart.timeout<255)uart.timeout++;
	if(sys.brightAdjust)sys.brightAdjust--;
}
//**********************************************************************************************
//**********************************************************************************************
uint8_t selectMsg(void)
{
	for(; DMD.msgCnt<4; DMD.msgCnt++,DMD.msgSelect>>=1)
	{
		if(DMD.msgSelect&0x01)
		{
			DMD.msgCnt++;
			DMD.msgSelect>>=1;
			break;
		}
	}
	DMD.msgToDisplay=DMD.msgCnt-1;
	if(DMD.msgCnt>3)
	{
		DMD.msgCnt=0;
		DMD.msgSelect=0;
	}
	return 0;
}
//**********************************************************************************************
//**********************************************************************************************
void arrayShiftRight(char *txt)
{
	for(uint16_t length=strlen(txt);length>0;length--)
	{
		*(txt+length)=*(txt+length-1);
	}
}
//**********************************************************************************************
//**********************************************************************************************
uint16_t getMsgWidth(char *p)
{
	uint16_t c,size=0;

	while(*p)
	{
		c=(*p-DMD.fontPtr[FONT_INDEX_OF_OFFSET])*DMD.fontPtr[FONT_INDEX_OF_BYTES_PER_CHAR]+FONT_LENGTH_OF_INFO_BYTES;
		size+=DMD.fontPtr[c];				//DMD.fontPtr[c] refers to no. of used columns
		p++;
	}
	return size;
}
//**********************************************************************************************
//**********************************************************************************************
void centreText(char *txt)
{
	uint16_t width=getMsgWidth(txt);
	uint8_t spaces;
	if(width<boardColWidth)
	{
		spaces=(boardColWidth-width)/DMD.fontPtr[FONT_LENGTH_OF_INFO_BYTES];
		for(uint16_t i=0;i<(spaces/2);i++)
		{
			arrayShiftRight(txt);
			*txt=' ';
		}
	}
}
//**********************************************************************************************
//**********************************************************************************************
void extractText(char *dest)
{
	char s[1024];
	uint16_t i=0,j=0;
	memset(s,'\0',sizeof(s));
	strcpy(s,dest);
	memset(dest,'\0',sizeof(user.msgRam));
#ifdef SYS_DEBUG
	DEBUG_X("extracting text...\r\n");
#endif
	while(s[i])
	{
		if(isprint(s[i]))dest[j++] = s[i];
		else
		{
			if(dest[j-1]!=' ')strcat(dest," ");		//append a space in case of newline encountered while maintain only single space character, even if more than one newline is encountered
		}
		while((s[i]==' ')&&(s[i+1]==' '))i++;	//avoid double spacing
		i++;
	}
	if(dest[j-1]!=' ')strcat(dest," ");

#ifdef SYS_DEBUG
	DEBUG_X("extracting text...OK\r\n");
	DEBUG_X("text:\r\n");
	DEBUG_X(dest);
	DEBUG_X("\r\n");
#endif
}
//**********************************************************************************************
//**********************************************************************************************
uint8_t getSize(char *s1,char *s2)
{
	uint16_t c,size=0;
	char s3[48], *p;

	memset(s3,'\0',sizeof(s3));
	p=s3;
	strcpy(p,s1);
	strcat(p,s2);
#ifdef SYS_DEBUG
	DEBUG_X("checking size...\r\n");
#endif
	while(*p)
	{	//check if the text length can fit in the display width
		c=(*p-DMD.fontPtr[FONT_INDEX_OF_OFFSET])*DMD.fontPtr[FONT_INDEX_OF_BYTES_PER_CHAR]+FONT_LENGTH_OF_INFO_BYTES;
		size+=DMD.fontPtr[c];				//DMD.fontPtr[c] refers to no. of used columns
		if(size>boardColWidth)
		{
#ifdef SYS_DEBUG
	DEBUG_X("text width error!\r\n");
#endif
			return 1;
		}
		p++;
	}
#ifdef SYS_DEBUG
	DEBUG_X("checking size...OK\r\n");
#endif
	return 0;
}
//**********************************************************************************************
//**********************************************************************************************
uint8_t formatStatic(void)
{
	char *p, s[MAX_BOARD_CHAR_WIDTH],init=1;
	uint16_t index=0,cnt;

	memset(s,'\0',sizeof(s));
	memset(user.staticMsgRam,'\0',sizeof(user.staticMsgRam));
	while(*user.staticMsgPtr==' ')user.staticMsgPtr++;		//skip space char at the beginning of the string
	while(*user.staticMsgPtr)
	{
		cnt=0;
		if((p=strchr(user.staticMsgPtr,' '))!=NULL)
		{
			while(p!=(user.staticMsgPtr+cnt))
			{
				s[index++]=*(user.staticMsgPtr+cnt);
				cnt++;
				s[index]='\0';
			}
			if(!init)
			{
				arrayShiftRight(s);
				s[0]=' ';				//append space between strings
				strcat(s,"\0");
			}
			init=0;
			if(!getSize(user.staticMsgRam,s))
			{
				strcat(user.staticMsgRam,s);
				index=0;
				memset(s,'\0',sizeof(s));
				user.staticMsgPtr=p+1;
			}
			else
			{
				if(!user.staticMsgRam[0])
				{
					index=0;init=1;
					memset(s,'\0',sizeof(s));
					user.staticMsgPtr=p+1;
					continue;
				}
				goto EXIT;
			}
		}
	}
	EXIT:
		if(user.staticMsgRam[0])
		{
			centreText(user.staticMsgRam);
#ifdef SYS_DEBUG
			DEBUG_X(user.staticMsgRam);
			DEBUG_X("\r\n");
#endif
		}
		else
		{
#ifdef SYS_DEBUG
			DEBUG_X("No message to print\r\n");
#endif
			return 1;
		}
		return 0;
}
//**********************************************************************************************
//**********************************************************************************************
#define BRIGHTNESS_CTRL_ADC_CHN (uint32_t)7UL			//Channel 0 (GPIOA 7)
#define ADC_FSD (float)4096.0
#define ADC_VREF (float)3.30
const float brightScale[10]={0.10,0.20,0.30,0.40,0.50,0.60,0.70,0.80,0.90,1.00};

void getBrightness(void)
{
	char s[FLASH_PAGE_SIZE];
	uint8_t c;
	uint16_t adcVal;
	float analogVin;
	memset(s,'\0',sizeof(s));

	readFlashData(s,USER_CONFIG_MSG_ADD);
	c=(s[FLASH_INDEX_OF_BRIGHTNESS_ADJUST]);

	if(!(c-='0'))
	{
		adcVal=getADC(BRIGHTNESS_CTRL_ADC_CHN);
		analogVin=(float)(adcVal*ADC_VREF/ADC_FSD);
		writePWM(PWM_CHANNEL,MAX_PWM*analogVin);
	}
	else
	{
		writePWM(PWM_CHANNEL,MAX_PWM*brightScale[c]);
	}
}
//**********************************************************************************************
//**********************************************************************************************

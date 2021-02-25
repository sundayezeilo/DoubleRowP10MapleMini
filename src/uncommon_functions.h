//**********************************************************************************************
//**********************************************************************************************
/*
 * uncommon_functions.h
 *
 *  Created on: Sep 29, 2017
 *      Author: zeilotech
 */
#ifndef UNCOMMON_FUNCTIONS_H_
#define UNCOMMON_FUNCTIONS_H_
void writeScreen(void)
{
    uint8_t i,j;
    copyToScreenRam(DMD.row);
    for(j=0;j<(totalShiftRegCols);j++)
    {
        for(i=(totalShiftRegRows/boardArrayHeight);i>0;i--)
        	{
        		dmdWriteByte(DMD.screenRam[i-1][j],DMD.screenRam[i-1+shiftRegRowsPerModule][j]);
        	}
    }
    enableOutput(DMD.row++);
    if(DMD.row>(totalShiftRegRows/2))DMD.row=row1s;
}
//**********************************************************************************************
//**********************************************************************************************

void dmdWriteByte(uint8_t byte1,uint8_t byte2)//byte1 corresponds to data for the upper board row, and byte2 for the lower
{
    uint8_t bitCnt;
    for(bitCnt=0;bitCnt<8;bitCnt++)
    {
    	  if(byte1&0x80)SETBIT(PIN_DMD_DT1_GPIO,PIN_DMD_DT1);
    	  else {CLRBIT(PIN_DMD_DT1_GPIO,PIN_DMD_DT1);}

    	  if(byte2&0x80)SETBIT(PIN_DMD_DT2_GPIO,PIN_DMD_DT2);
    	  else {CLRBIT(PIN_DMD_DT2_GPIO,PIN_DMD_DT2);}
        //toggle PIN_DMD_SH_CP of 74HC595 shift register LOW-HIGH-LOW
    	  SHIFT_DMD_REG_DATA();
        byte1<<=1; byte2<<=1;
    }
}

/*
void dmdWriteByte(uint8_t byte1,uint8_t byte2)
{
   uint8_t bitCnt;
	uint32_t tempODR, DTWord=0;
	for(bitCnt=0;bitCnt<8;bitCnt++)
	{
		if(byte1&0x80)DTWord|= 1<<DT1_SHIFT_LENGTH;
		else{DTWord&= ~(1<<DT1_SHIFT_LENGTH);}

		if(byte2&0x80)DTWord|= 1<<DT2_SHIFT_LENGTH;
		else{DTWord&= ~(1<<DT2_SHIFT_LENGTH);}

		tempODR = PIN_DMD_DATA_GPIO->ODR;
		tempODR&= ~(PIN_DMD_DT1|PIN_DMD_DT2);
		PIN_DMD_DATA_GPIO->ODR=tempODR|DTWord;
		//toggle PIN_DMD_SH_CP of 74HC595 shift register LOW-HIGH-LOW
  	   SHIFT_DMD_REG_DATA();
		byte1<<=1; byte2<<=1;
	}
}
*/
//**********************************************************************************************
//**********************************************************************************************
void enableOutput(uint8_t row)
{
	//static uint8_t firstScan=1;
	switch(row)
	{
		case row1s:
			LIGHT_DMD_ROW_1s();
			DMD.scanComplete=0;break;
	   case row2s:LIGHT_DMD_ROW_2s();break;
	   case row3s:LIGHT_DMD_ROW_3s();break;
	   case row4s:
		   LIGHT_DMD_ROW_4s();
		   DMD.scanComplete=1;break;
	   default:break;
	}
	//delay_us(5);
	LATCH_DMD_SHIFT_REG_TO_OUTPUT();	//latch data at output of 74HC595
	//if(firstScan){firstScan=0;writePWM(PWM_PIN[0],MAX_PWM*0.01);}//adjust pwm duty
}
//**********************************************************************************************
//**********************************************************************************************
void readMsg(void)
{
    uint8_t i;
    uint16_t c=(*user.msgPtr-DMD.fontPtr[FONT_INDEX_OF_OFFSET])*DMD.fontPtr[FONT_INDEX_OF_BYTES_PER_CHAR]+
    		FONT_LENGTH_OF_INFO_BYTES;
    user.msgPtr++;
    DMD.usedWord=DMD.fontPtr[c];
    for(i=0;i<DMD.fontPtr[FONT_INDEX_OF_BYTES_PER_CHAR];i++){DMD.loadedCharFont[i]=~(DMD.fontPtr[++c]);}//the bits are inverted here, since the fonts codes were generated for 'active high', and the shift regs write 'active low' data to the board
    //each inner array of the char fonts contains 33 elements
}
//**********************************************************************************************
//**********************************************************************************************
//#define bytesPerChar (uint8_t)4UL
void readNextWord(void)
{
	uint8_t i=0;

	for(i=DMD.fontPtr[FONT_INDEX_OF_BYTES_PER_COLUMN];i>0;i--)//DMD.fontPtr[2] refers to number of bytes per column
	{
		DMD.currentCol=((DMD.currentCol<<8)|DMD.loadedCharFont[DMD.colCnt+i-1]);
	}
	DMD.colCnt+=DMD.fontPtr[FONT_INDEX_OF_BYTES_PER_COLUMN];
}

//**********************************************************************************************
//**********************************************************************************************
void getFont(void)
{
    switch(DMD.fontSelect)
    {
    		case ARIAL_FONT:	DMD.fontPtr=Arial_Black33x32;break;
    		case VERDANA_FONT: DMD.fontPtr=Verdana39x32;break;
    		default:	DMD.fontPtr=Arial_Black33x32;break;	//default font select
    }
}
//**********************************************************************************************
//**********************************************************************************************
/*void getScrollFont(void)
{
    switch(DMD.fontSelect)
    {
    		case ARIAL_FONT:	DMD.fontPtr=Arial_Black33x32;break;
    		case VERDANA_FONT: DMD.fontPtr=Verdana39x32;break;
    		default:	DMD.fontPtr=Verdana39x32;break;	//default font select
    }
}*/

#endif /* UNCOMMON_FUNCTIONS_H_ */

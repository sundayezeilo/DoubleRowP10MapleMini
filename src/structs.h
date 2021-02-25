/*
 * structs.h
 *
 *  Created on: Oct 8, 2017
 *      Author: zeilotech
 */

#ifndef STRUCTS_H_
#define STRUCTS_H_


//**********************************************************************************************
//**********************************************************************************************
struct{
    uint8_t shiftRegRowData[boardRowHeight][totalShiftRegCols];
    uint8_t virtualShiftRegRowData[boardRowHeight][totalShiftRegCols];
    uint8_t screenRam[totalShiftRegRows][totalShiftRegCols];
    //use virtualScreenRam temporarily for shifting operation,
    //then transfer the contents to screenRam when shifting is done
    uint8_t copyVirtualRamDone,colCnt,row,usedWord,scanComplete;
    uint8_t loadedCharFont[(maxFontWidth*numberOfBytesPerCol)];
    volatile uint8_t updateMsg;
    uint32_t currentCol;		//note uint32 for double row
    uint16_t col;
    uint8_t bitCnt,init,mode,rowCnt,fontSelect,msgToDisplay,msgSelect,msgCnt;
    const uint8_t *fontPtr;
    volatile uint16_t sleep,msgUpdateTick;
}DMD;
//**********************************************************************************************
//**********************************************************************************************
struct{
    __IO uint8_t msTick,refreshScreen, reboot,clockStatus,clockType;
    __IO uint16_t brightAdjust;
    char msgRam[48],textBuff[(userMsgMaxLength+300)],*memPtr,clkCalMsg[48],tempMsg[48];
}sys;
//**********************************************************************************************
//**********************************************************************************************
struct{
    char msgRam[(userMsgMaxLength)],*msgPtr,
	*staticMsgPtr,staticMsgRam[48];//extra two bytes for message delimiters (null or CR+LF)
}user;

#endif /* STRUCTS_H_ */

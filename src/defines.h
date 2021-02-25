/*
 * defines.h
 *
 *  Created on: Sep 29, 2017
 *      Author: zeilotech
 */

#ifndef DEFINES_H_
#define DEFINES_H_


//**********************************************************************************************
//**********************************************************************************************
/*
definition of symbolic constants
*/
#define DMD_CLK_GPIO GPIOB	//Not used!!!

#define PIN_DMD_SH_CPGPIO GPIOB
#define PIN_DMD_SH_CP GPIO_Pin_5

#define PIN_DMD_ST_CPGPIO GPIOB
#define PIN_DMD_ST_CP GPIO_Pin_4


//#define PIN_DMD_DATA_GPIO GPIOB

#define PIN_DMD_DT1_GPIO GPIOB
#define PIN_DMD_DT1 GPIO_Pin_3

//#define DT1_SHIFT_LENGTH (uint8_t)3UL

#define PIN_DMD_DT2_GPIO GPIOA
#define PIN_DMD_DT2 GPIO_Pin_15

//#define DT2_SHIFT_LENGTH (uint8_t)15UL

#define PIN_DMD_OE_GPIO PWM_GPIO
#define PIN_DMD_OE GPIO_Pin_11

#define PIN_DMD_A GPIO_Pin_7
#define PIN_DMD_B GPIO_Pin_6
#define DMD_ROW_SELECT_GPIO GPIOB

//#define DMD_GPIO_CLK RCC_APB2Periph_GPIOB

//**********************************************************************************************
//**********************************************************************************************
#define LIGHT_DMD_ROW_1s() {CLRBIT(DMD_ROW_SELECT_GPIO,PIN_DMD_B);CLRBIT(DMD_ROW_SELECT_GPIO,PIN_DMD_A);}
#define LIGHT_DMD_ROW_2s() {CLRBIT(DMD_ROW_SELECT_GPIO,PIN_DMD_B);SETBIT(DMD_ROW_SELECT_GPIO,PIN_DMD_A);}
#define LIGHT_DMD_ROW_3s() {SETBIT(DMD_ROW_SELECT_GPIO,PIN_DMD_B);CLRBIT(DMD_ROW_SELECT_GPIO,PIN_DMD_A);}
#define LIGHT_DMD_ROW_4s() {SETBIT(DMD_ROW_SELECT_GPIO,PIN_DMD_B);SETBIT(DMD_ROW_SELECT_GPIO,PIN_DMD_A);}
#define LATCH_DMD_SHIFT_REG_TO_OUTPUT() {SETBIT(PIN_DMD_ST_CPGPIO,PIN_DMD_ST_CP);CLRBIT(PIN_DMD_ST_CPGPIO,PIN_DMD_ST_CP);}
#define SHIFT_DMD_REG_DATA() {SETBIT(PIN_DMD_SH_CPGPIO,PIN_DMD_SH_CP);CLRBIT(PIN_DMD_SH_CPGPIO,PIN_DMD_SH_CP);}
//**********************************************************************************************
//**********************************************************************************************
#define shiftRegRowsPerModule (uint8_t)4UL                          //4 shift reg rows per board
#define shiftRegColsPerModule (uint8_t)4UL              //4 shift reg cols per board
#define rowsPerModule (uint8_t)16UL
#define colsPerModule (uint8_t)32UL
#define boardArrayHeight (uint8_t)2UL		/*modify this value if display size changes*/
#define boardArrayWidth (uint8_t)8UL		/*modify this value if display size changes*/
#define boardRowHeight (rowsPerModule*boardArrayHeight)
#define boardColWidth (uint16_t)(colsPerModule*boardArrayWidth)
#define totalShiftRegRows (shiftRegRowsPerModule*boardArrayHeight)
#define totalShiftRegCols (shiftRegColsPerModule*boardArrayWidth)

#define MAX_BOARD_CHAR_WIDTH (uint8_t)40UL
#define maxFontWidth (uint8_t)40UL		//font width should not exceed 40 columns
#define userMsgMaxLength (uint16_t)1024UL
#define numberOfBytesPerCol (uint8_t)4UL


#define VERDANA_FONT (uint8_t)4UL
#define ARIAL_FONT (uint8_t)5UL
#define MAX_FONT_NO ARIAL_FONT
#define MIN_FONT_NO VERDANA_FONT
#define DEFAULT_FONT ARIAL_FONT


#endif /* DEFINES_H_ */

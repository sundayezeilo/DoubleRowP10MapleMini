/*
 * prototypes.h
 *
 *  Created on: Sep 29, 2017
 *      Author: zeilotech
 */

#ifndef PROTOTYPES_H_
#define PROTOTYPES_H_

void readMsg(void);
void writeScreen(void);
void shiftRowData(void);
void shiftRowsDown(void);
void readNextWord(void);
void copyVirtualRam(void);
void enableOutput(uint8_t);
void dmdWriteByte(uint8_t,uint8_t);
void copyToScreenRam(uint8_t);
void gpioInit(void);

void swapBytes(uint8_t *);
uint32_t BCDToBin(uint32_t);
uint16_t bytesToInt(uint8_t *);
void formatTemperature(char *,uint16_t);
#endif /* PROTOTYPES_H_ */

/*
 * my_stm32f103_rtc.h
 *
 *  Created on: Oct 11, 2017
 *      Author: zeilotech
 */

#ifndef MY_STM32F103_RTC_H_
#define MY_STM32F103_RTC_H_

#define RTCC_DATA_SIZE 8UL
enum{
			secondsAddress=0,
			minutesAddress,
			hoursAddress,
			dayOfWeekAddress,
			daysAddress,
			monthsAddress,
			yearsAddress,
			controlAddress,
	};

#define defaultDay 0x01
#define defaultMonth 0x01
#define defaultYear 0x16
#define defaultHour 0x00
#define defaultDayOfWeek 0x01
#define defaultMinutes 0x00
#define defaultSeconds 0x00
#define controlMask 0x10

__IO uint8_t  timeDisplay = 0;
const unsigned char defaultClockTable[]={
		defaultSeconds,
		defaultMinutes,
		defaultHour,
		defaultDayOfWeek,
		defaultDay,
		defaultMonth,
		defaultYear,
		controlMask
};
//**********************************************************************************************
//**********************************************************************************************
const uint8_t monthDays[]={0,31,28,31,30,31,30,31,31,30,31,30,31};
#define LEAP_YEAR(year) ((((year%4)==0)&&((year%100)!=0))||((year%400)==0))
#define SECONDS_ADDRESS secondsAddress
#define MINUTES_ADDRESS minutesAddress
#define HOURS_ADDRESS hoursAddress
#define DAY_OF_WEEK_ADDRESS dayOfWeekAddress
#define DAYS_ADDRESS daysAddress
#define MONTHS_ADDRESS monthsAddress
#define YEARS_ADDRESS yearsAddress
#define CONTROL_ADDRESS controlAddress
//**********************************************************************************************
//**********************************************************************************************
void makeTimeFromSeconds(uint8_t *date,uint32_t seconds)
{
	uint32_t epoch=seconds;

	unsigned int year;
	unsigned char month, monthLength;
	unsigned long days;

	date[SECONDS_ADDRESS]=epoch%60;
	epoch/=60;
	date[MINUTES_ADDRESS]=epoch%60;
	epoch/=60;
	date[HOURS_ADDRESS]=epoch%24;
	epoch/=24;
	date[DAY_OF_WEEK_ADDRESS]=(epoch+6)%7;
	year=0;	//2000 here
	days=0;
	while((days += (LEAP_YEAR(year) ? 366 : 365)) <= epoch)year++;
	date[YEARS_ADDRESS]=year;
	days -= LEAP_YEAR(year) ? 366 : 365;
	epoch -= days; // now it is days in this year, starting at 0

	days=0;
	month=0;
	monthLength=0;
	for(month=1; month<=12; month++){
		if (month==2){
			if (LEAP_YEAR(year)){
				monthLength=29;
			} else{monthLength=28;}
		}else {monthLength = monthDays[month];}

		if (epoch>=monthLength){epoch-=monthLength;}
				else{ break;}
	}
	date[MONTHS_ADDRESS]=month;
	date[DAYS_ADDRESS]=epoch+1;
}

uint32_t secondsSince2000(uint8_t *date)
{
		uint32_t seconds,temp;
		uint16_t i;

		seconds=date[YEARS_ADDRESS];
		seconds*=(60*60*24L*365);

		for (i=0; i<date[YEARS_ADDRESS]; i++){
			if (LEAP_YEAR(i)) {seconds+= 60*60*24L;}
		}

		for (i=0; i<date[MONTHS_ADDRESS]; i++) {
			if (i==1 && LEAP_YEAR(date[YEARS_ADDRESS])){
				seconds+= 60*60*24L*29;
			} else {
				//seconds+= 60*60*24L*monthDays[i];
				   temp=monthDays[i];
				   temp*=60*60*24L;
				   seconds+=temp;
			}
		}

	temp=date[DAYS_ADDRESS]-1;
	temp*=60*60*24L;
	seconds+=temp;
	//seconds+= (date[DAYS_ADDRESS]-1)*60*60*24L;
	temp=date[HOURS_ADDRESS];
	temp*=60*60;
	seconds+=temp;
	//seconds+= date[HOURS_ADDRESS]*60*60L;
	//seconds+= date[MINUTES_ADDRESS]*60;
	temp=date[MINUTES_ADDRESS];
	temp*=60;
	seconds+=temp;
	//seconds+= date[SECONDS_ADDRESS];
	temp=date[SECONDS_ADDRESS];
	seconds+=temp;
	return seconds;
}

uint32_t minutesSince2000(uint8_t *date)
{
	uint32_t minutes=date[YEARS_ADDRESS];
	uint32_t temp;
	uint16_t i;

	minutes*=(60*24*365);
	for(i=0;i<date[YEARS_ADDRESS];i++){
		if(LEAP_YEAR(i)){minutes+=(60*24L);}
	}

	if(date[MONTHS_ADDRESS]>1){
		for(i=1;i<date[MONTHS_ADDRESS];i++){
			if((i==2)&&(LEAP_YEAR(date[YEARS_ADDRESS]))){
				minutes+=(60*24L*29);
			}
			else{
					temp=monthDays[i];
					temp*=60*24L;
					minutes+=temp;
			}
		}
	}
	temp=date[DAYS_ADDRESS]-1;
	temp*=60*24L;
	minutes+=temp;
	temp=date[HOURS_ADDRESS];
	temp*=60;
	minutes+=temp;
	temp=date[MINUTES_ADDRESS];
	minutes+=temp;
	return minutes;
}

uint8_t makeTimeFromMinutes(uint8_t *date,uint32_t minutes)
{
	uint32_t epoch=minutes;

	unsigned int year;
	unsigned char month, monthLength;
	unsigned long days;

	date[SECONDS_ADDRESS]=0;	//always set this o ZERO!!!
	date[MINUTES_ADDRESS]=epoch%60;
	epoch/=60;
	date[HOURS_ADDRESS]=epoch%24;
	epoch/=24;
	date[DAY_OF_WEEK_ADDRESS]=(epoch+4)%8;
	year=0;	//2000 here
	days=0;
	while((days += (LEAP_YEAR(year) ? 366 : 365)) <= epoch)year++;
	date[YEARS_ADDRESS]=year;
	days -= LEAP_YEAR(year) ? 366 : 365;
	epoch -= days; // now it is days in this year, starting at 0

	days=0;
	month=0;
	monthLength=0;
	for(month=1; month<=12; month++){
    if (month==2){
		if (LEAP_YEAR(year)){
			monthLength=29;
		} else{monthLength=28;}
    }else {monthLength = monthDays[month];}

    if (epoch>=monthLength){epoch-=monthLength;}
	else{ break;}
  }
  date[MONTHS_ADDRESS]=month;
  date[DAYS_ADDRESS]=epoch+1;
   return 0;
}

unsigned char bcdToBin(unsigned char c)
{
 	unsigned char msd=(c>>4)&0x0F;
	unsigned char lsd=(c&0x0F);
	return((msd*10)+lsd);
}
unsigned char binToBcd(unsigned char c)
{
 	return(((c/10)<<4)|(c%10));
}

void binToBcdArray(uint8_t *dest,uint8_t size)
{
 	 uint8_t c,i;

	 for(i=0;i<size;i++){
	 		c=dest[i];
	 		dest[i]=binToBcd(c);
	}
}

void bcdToBinArray(uint8_t *dest,uint8_t size)
{
 	 uint8_t c,i;

	 for(i=0;i<size;i++){
	 		c=dest[i];
	 		dest[i]=bcdToBin(c);
	}
}

void adjustClock(uint8_t *rtcc)
{
    RTC_WaitForLastTask();
    bcdToBinArray(rtcc,RTCC_DATA_SIZE);
    uint32_t t=secondsSince2000(rtcc);
    RTC_SetCounter(t);
    RTC_WaitForLastTask();
    //BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
}

void writeDefaultTime(void)
{
    uint8_t rtcc[RTCC_DATA_SIZE];
    memcpy(rtcc,defaultClockTable,RTCC_DATA_SIZE);
    adjustClock(rtcc);
}

void RTC_Configuration(void);

void RTC_IRQHandler(void)
{
	if (RTC_GetITStatus(RTC_IT_SEC) != RESET){
		RTC_ClearITPendingBit(RTC_IT_SEC);
		timeDisplay=1;
		RTC_WaitForLastTask();
	}
}

void RTC_Configuration(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR|RCC_APB1Periph_BKP, ENABLE);
	PWR_BackupAccessCmd(ENABLE);
	BKP_DeInit();
	RCC_LSEConfig(RCC_LSE_ON);
	while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET){}
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
	RCC_RTCCLKCmd(ENABLE);
	RTC_WaitForSynchro();
	RTC_WaitForLastTask();
	RTC_ITConfig(RTC_IT_SEC, ENABLE);
	RTC_WaitForLastTask();
	RTC_SetPrescaler(32767); //RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1)
	RTC_WaitForLastTask();
}


void initIRQ_RTC(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

uint8_t initRTC(void)
{
	uint8_t status=0;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR|RCC_APB1Periph_BKP, ENABLE);
	PWR_BackupAccessCmd(ENABLE);
	initIRQ_RTC();
	if(BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5){
		RTC_Configuration();
		writeDefaultTime();
		//BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
		status=TIME_NOT_SET;
	}
	else{
		if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET){}
		else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET){}
		RTC_WaitForSynchro();
		RTC_ITConfig(RTC_IT_SEC, ENABLE);
		RTC_WaitForLastTask();
	}
	RCC_ClearFlag();
	return status;
}
//to set time, call adjustRTCC() with the time/date info packed in DS1307-style register sets
//**********************************************************************************************
//**********************************************************************************************

#endif /* MY_STM32F103_RTC_H_ */

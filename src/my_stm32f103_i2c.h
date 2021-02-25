#include<stddef.h>
#include "stm32f10x.h"
//***********************************************************************************************
//***********************************************************************************************
/*Start of I2C routines*/
//pass the address to the function and specify if the address type is 8-bit or 16-bit depending on other devices
//on the bus.
//to use DS1307 on the same bus as a 24LC512, pass 0xD0 as slave address, set the 8-bit address, and set addressType to address8
//this will not work for device greater than 64KB except the slave address is modified before passing to the uint8_t i2c(uint8_t slaveAddress....) function
#define READ (uint8_t)0UL
#define WRITE (uint8_t)1UL
#define READ_MASK (uint8_t)1UL		//obtained from datasheet
#define WRITE_MASK (uint8_t)0UL	//obtained from datasheet
#define I2C_READ_ERROR (uint8_t)1UL
#define I2C_WRITE_ERROR (uint8_t)2UL
#define NVM_SIZE (uint32_t)65536

#define ADDRESS_8 (uint8_t)0UL
#define ADDRESS_16 (uint8_t)1UL
#define PAGE_SIZE (uint8_t)128UL


void printWriteError(char *s,uint8_t c)
{
	switch(c)
	{
		case I2C_READ_ERROR: strcpy(s,"I2C DEVICE WRITE FAILED");break;
		case I2C_WRITE_ERROR: strcpy(s,"I2C DEVICE READ FAILED");break;
		//case TIME_NOT_SET: strcpy(s,"TIME & DATE NOT SET!");break;
		default: strcpy(s,"CLOCK ERROR");break;
	}
}
//***********************************************************************************************
//***********************************************************************************************
#define I2C_GPIO GPIOB
#define SDA GPIO_Pin_11
#define SCL GPIO_Pin_10
#define TWI_DELAY (uint8_t)10UL
#define ACK 0
#define NACK 1
#define OFF (uint8_t)0UL
#define ON (uint8_t)1UL
//***********************************************************************************************
//***********************************************************************************************
void I2C_Delay(void){delay_us(TWI_DELAY);}
//***********************************************************************************************
//***********************************************************************************************
void i2cWriteClock(uint8_t level)
{
	I2C_Delay();
	if(level)I2C_GPIO->BSRR=SCL;
	else I2C_GPIO->BRR=SCL;
	I2C_Delay();
}
//***********************************************************************************************
//***********************************************************************************************
void i2cStart(void)
{
	I2C_GPIO->BSRR=SCL|SDA;
	I2C_Delay();
	I2C_GPIO->BRR=SDA;
	I2C_Delay();
	I2C_GPIO->BRR=SCL;
	I2C_Delay();
}
//***********************************************************************************************
//***********************************************************************************************
void i2cStop(void)
{
	I2C_GPIO->BSRR=SDA;
	I2C_Delay();
	I2C_GPIO->BRR=SDA;
	I2C_Delay();
	I2C_GPIO->BSRR=SCL;
	I2C_Delay();
	I2C_GPIO->BSRR=SDA;
}
//***********************************************************************************************
//***********************************************************************************************
uint8_t I2C_Ackn(void)
{
	uint8_t ackn=0;

	I2C_GPIO->BRR=SCL;
	I2C_GPIO->BSRR=SDA;
	i2cWriteClock(1);
	if((I2C_GPIO->IDR&SDA))ackn=1;
	i2cWriteClock(0);
	return ackn;
}
//***********************************************************************************************
//***********************************************************************************************
void i2cWriteBit(uint8_t bit)
{
	if(bit)I2C_GPIO->BSRR=SDA;
	else I2C_GPIO->BRR=SDA;
	i2cWriteClock(1);
	i2cWriteClock(0);
}
//***********************************************************************************************
//***********************************************************************************************
uint8_t i2cWriteByte(uint8_t byte)
{
	uint8_t i;

	I2C_GPIO->BRR=SCL;
	I2C_Delay();
	for(i=0;i<8;i++){i2cWriteBit(byte&0x80);byte<<=1;}
	return(I2C_Ackn());
}
//***********************************************************************************************
//***********************************************************************************************
uint8_t i2cReadByte(uint8_t ack)
{
	uint8_t i,c=0;

	I2C_GPIO->BSRR=SDA;
	for(i=0;i<8;i++)
	{
		c<<=1;
		i2cWriteClock(1);
		if((I2C_GPIO->IDR&SDA))c|=0x01;
		i2cWriteClock(0);
	}
	if(!ack)I2C_GPIO->BRR=SDA;
	else I2C_GPIO->BSRR=SDA;
	i2cWriteClock(1);
	i2cWriteClock(0);
	return c;
}
//***********************************************************************************************
//***********************************************************************************************
#define I2C_ACK_WAIT_TIME (uint8_t)20UL
uint8_t i2cWriteAck(uint8_t sAddress)
{
	uint8_t z=I2C_ACK_WAIT_TIME;
	while(z)
	{
		i2cStart();
		if(!i2cWriteByte(sAddress))return 0;
		delay_us(1000);	//2 ms
		z--;
	}
	return 1;
}
//***********************************************************************************************
//***********************************************************************************************
uint8_t writePage(uint8_t slaveAddress,char *s,uint16_t address,uint8_t type,uint16_t count)
{
	uint16_t nADDRESS_16,pageCount;//remainder;
	uint16_t byteCount;

	nADDRESS_16=address%NVM_SIZE;
	pageCount=count/PAGE_SIZE;
	while(pageCount)
	{
		byteCount=PAGE_SIZE;
		i2cStart();
		if(i2cWriteByte(slaveAddress|WRITE_MASK))return I2C_WRITE_ERROR;
		if(type==ADDRESS_16){if(i2cWriteByte(nADDRESS_16>>8))return I2C_WRITE_ERROR;}
		if(i2cWriteByte(nADDRESS_16))return I2C_WRITE_ERROR;
		while(byteCount)
		{
			if(i2cWriteByte(*s++))return I2C_WRITE_ERROR;
			byteCount--;
		}
		i2cStop();
		if(i2cWriteAck(slaveAddress|WRITE_MASK))return I2C_WRITE_ERROR;
		nADDRESS_16+=PAGE_SIZE;
		pageCount--;
	}
	return 0;
}
//***********************************************************************************************
//***********************************************************************************************
uint8_t writeBytesInPage(uint8_t slaveAddress,char *s,uint16_t address,uint8_t type,uint16_t count)	//write remainder here now
{
	while(count--)
	{
		i2cStart();
		if(i2cWriteByte(slaveAddress|WRITE_MASK))return I2C_WRITE_ERROR;
		if(type==ADDRESS_16){if(i2cWriteByte(address>>8))return I2C_WRITE_ERROR;}
		if(i2cWriteByte(address++))return I2C_WRITE_ERROR;
		if(i2cWriteByte(*s++))return I2C_WRITE_ERROR;
		i2cStop();
		if(i2cWriteAck(slaveAddress|WRITE_MASK))return I2C_WRITE_ERROR;
	}
	return 0;
}
//***********************************************************************************************
//***********************************************************************************************
uint8_t i2c(uint8_t slaveAddress,char *s,uint16_t address,uint8_t type,uint16_t count, uint8_t mode)
{
	uint16_t pageCount,temp;
	uint16_t remainder;
	uint8_t c;

	if(!count)return 0;

	if(mode==READ)
	{
		i2cStart();
		if(i2cWriteByte(slaveAddress|WRITE_MASK))return I2C_READ_ERROR;
		if(type==ADDRESS_16){if(i2cWriteByte(address>>8))return I2C_READ_ERROR;}
		if(i2cWriteByte(address))return I2C_READ_ERROR;
		i2cStart();
		if(i2cWriteByte(slaveAddress|READ_MASK))return I2C_READ_ERROR;
		while(count>1){
			*s++=i2cReadByte(0);
			count--;
		}
		*s=i2cReadByte(1);
		i2cStop();
		return 0;
	}
	/*if(mode==WRITE){
			while(count){
				i2cStart();
				if(i2cWriteByte(slaveAddress|WRITE_MASK))return I2C_WRITE_ERROR;
				if(type==ADDRESS_16){if(i2cWriteByte(address>>8))return I2C_WRITE_ERROR;}
				if(i2cWriteByte(address++))return I2C_WRITE_ERROR;
				if(i2cWriteByte(*s++))return I2C_WRITE_ERROR;
				i2cStop();
				if(i2cWriteAck(slaveAddress|WRITE_MASK))return I2C_WRITE_ERROR;
				count--;
			}
			return 0;
	}*/
	if(mode==WRITE)
	{
		//if(slaveAddress==ADD_DS1307){goto PAGE_NOT_FULL;}
		if(!(address%PAGE_SIZE))
		{
			SKIP_I2C:
		 	if((count)&&(count<PAGE_SIZE)){	goto PAGE_NOT_FULL;}
			pageCount=count/PAGE_SIZE;
			remainder=count%PAGE_SIZE;
			temp=PAGE_SIZE*pageCount;
			if((c=writePage(slaveAddress,s,address,type,temp)))return c;
			if(!remainder)return 0;
			address+=temp;
			s+=temp;
			count=remainder;
			PAGE_NOT_FULL:
			return(writeBytesInPage(slaveAddress,s,address,type,count));	//write remainder here now
		}
		else
		{
			pageCount=count/PAGE_SIZE;
			remainder=count%PAGE_SIZE;
			temp=PAGE_SIZE*pageCount;
			if((c=writeBytesInPage(slaveAddress,s,address,type,remainder)))return c;
			address+=remainder;
			s+=remainder;
			count-=remainder;
			goto SKIP_I2C;
		}
	}
	return I2C_WRITE_ERROR;		//what to return here: bus error
}
//***********************************************************************************************
//***********************************************************************************************
void initTWIO(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
   GPIO_InitStructure.GPIO_Pin=SCL|SDA;
   GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_OD;
   GPIO_InitStructure.GPIO_Speed=GPIO_Speed_10MHz;
   GPIO_Init(I2C_GPIO, &GPIO_InitStructure);
   I2C_GPIO->BSRR=SDA|SCL;
   I2C_Delay();
}
/*End of I2C routines*/
//***********************************************************************************************
//***********************************************************************************************

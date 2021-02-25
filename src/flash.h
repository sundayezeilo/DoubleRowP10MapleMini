#define FLASH_START_ADDRESS 0x8000000
#define FLASH_MEM_SIZE (uint32_t)131072UL						//65536UL for 64 KB flash version
#define FLASH_PAGE_SIZE    ((uint16_t)0x400)
#define TOTAL_PAGE_COUNT (FLASH_MEM_SIZE/FLASH_PAGE_SIZE)
typedef enum{PASSED =0,FAILED} TestStatus;

#define MSG1_PAGE_ADDRESS TOTAL_PAGE_COUNT- 1	//page 63
#define MSG2_PAGE_ADDRESS MSG1_PAGE_ADDRESS - 1	//page 62
#define MSG3_PAGE_ADDRESS MSG1_PAGE_ADDRESS - 2	//page 61
#define MSG4_PAGE_ADDRESS MSG1_PAGE_ADDRESS -3	//page 60

#define FLASH_DATA_OFFSET (uint32_t)125952UL								//131072 - 5x1024 (5k)

void ULongToBytes(uint8_t *s,uint32_t val32)
{
	union{
		uint32_t val;
		uint8_t bytes[sizeof(long)];
	}this;
	this.val=val32;
	memcpy(s, this.bytes,sizeof(this.bytes));
}

uint32_t bytesToULong(char *s)
{
	union{
		uint32_t val;
		uint8_t bytes[sizeof(long)];
	}this;
	memcpy(this.bytes,s,sizeof(this.bytes));
	return(this.val);
}

//reads <size> of data from <page>
uint8_t readFLASH(uint32_t address,uint8_t *data,uint16_t byteCount)
{
	uint8_t *tData = data;
	uint32_t endAddress = address + byteCount;
	//volatile TestStatus MemoryProgramStatus = PASSED;
	FLASH_LockBank1();
	uint32_t startAddress = address;

	while(startAddress < endAddress){
		uint32_t tempData = 	(*(__IO uint32_t*) startAddress);
		ULongToBytes(tData,tempData);
		startAddress+=sizeof(long);
		tData+=sizeof(long);
	}
	return 0;
}


uint8_t readMessage(char *s,uint16_t size,uint16_t page)
{
	uint32_t startAddress = FLASH_START_ADDRESS + (page*FLASH_PAGE_SIZE);
	return(readFLASH(startAddress,s,size));
}

uint8_t writeFLASH(uint32_t address,char *data,uint16_t byteCount)
{
	volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;
	//volatile TestStatus MemoryProgramStatus = PASSED;
	uint32_t startAddress = address;
	uint32_t endAddress = address + byteCount;
	char *tData = data;

 	FLASH_UnlockBank1();
	FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);
	FLASHStatus = FLASH_ErasePage(address);
	while((startAddress < endAddress) && (FLASHStatus == FLASH_COMPLETE)){
		uint32_t pData = bytesToULong(tData);
		tData+=sizeof(long);
		FLASHStatus = FLASH_ProgramWord(startAddress, pData);
		startAddress += sizeof(long);
	}
	FLASH_LockBank1();
	return FLASHStatus;
}

uint8_t writeMessage(char *s,uint16_t size,uint16_t page)
{
	uint32_t startAddress = FLASH_START_ADDRESS + (page*FLASH_PAGE_SIZE);
	return(writeFLASH(startAddress,s,size));
}

uint16_t alignOnLong(uint16_t size)
{
	while(size%sizeof(long))size++;
	return size;
}
/*
int testFlash(void)
{
	char s[FLASH_PAGE_SIZE];

	memset(s,'\0',sizeof(s));
	strcpy(s,"AaBbCcDdEeFfg");
	uint16_t size=alignOnLong(strlen(s));
	//Note: introduce a check here!!!
	if(size>FLASH_PAGE_SIZE)size=FLASH_PAGE_SIZE;
	uint8_t c=writeMessage(s,size,MSG1_PAGE_ADDRESS);
	c=writeMessage(s,size,MSG2_PAGE_ADDRESS);
	c=writeMessage(s,size,MSG3_PAGE_ADDRESS);
	c=writeMessage(s,size,MSG4_PAGE_ADDRESS);
	readMessage(s,sizeof(s),MSG1_PAGE_ADDRESS);
	readMessage(s,sizeof(s),MSG2_PAGE_ADDRESS);
	readMessage(s,sizeof(s),MSG3_PAGE_ADDRESS);
	readMessage(s,sizeof(s),MSG4_PAGE_ADDRESS);
	//while(1);
}
*/

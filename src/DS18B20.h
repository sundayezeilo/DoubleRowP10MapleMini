/*
 * DS18B20.h
 *
 *  Created on: Oct 20, 2017
 *      Author: zeilotech
 */
#ifndef ONE_WIRE_H_
#define ONE_WIRE_H_

/*
#define DS18B20_ERROR (uint8_t)1UL
#define DS18B20_READ_ERROR (uint8_t)2UL
#define DS18B20_NOT_FOUND_ERROR (uint8_t)3UL
#define OW_CRC_ERROR (uint8_t)4UL
*/

enum{
	DS18B20_ERROR = (uint8_t)1UL,
	DS18B20_READ_ERROR,
	DS18B20_NOT_FOUND_ERROR,
	OW_CRC_ERROR
};

#define TEMP_SENSORS_COUNT (uint32_t)1UL

//extern void delay_us(uint16_t);
//extern void swapBytes(uint8_t *);
//extern uint16_t bytesToInt(uint8_t *);
uint32_t ds18b20DecodeTemperatureRaw(uint16_t *raw);
void ds18b20SetPrecision(uint8_t p);

#define OW_PORT GPIOB
#define OW_GPIO GPIO_Pin_13

typedef enum {
    ONE_WIRE_SLAVE_PRESENT,
    ONE_WIRE_ERROR
}OWState;

typedef struct {
    uint8_t address[8];
}OWDevice;

uint8_t precision = 1;			//between 0 and 3 for 9-bit to 12-bit
OWDevice *OWListOfDevices;
uint8_t OWDevicesListSize = 2;
uint32_t OneWireCount;

uint32_t OWReset(void);
void OWWrite1(void);
void OWWrite0(void);
void OWWriteBit(uint8_t bit);
uint8_t OWReadBit(void);
void OWWriteByte(uint8_t data);
uint8_t OWReadByte(void);
OWDevice * OWSearchROM(uint8_t *devices);
uint8_t OWMatchROM(OWDevice device);
uint8_t OWGetCRC(void);
uint8_t OWCRC(uint8_t data);
void OWResetCRC(void);

static OWDevice OWDevices[10];
static uint8_t OWDeviceCount = 0;

static OWState state;
// global search state
static uint8_t lastMismatch;
static uint8_t lastDeviceFlag;
static uint8_t crc8;
static uint8_t ROM_NO[8];

typedef struct {
    int8_t integer;
    uint16_t fractional;
    uint8_t is_valid;
} simple_float;

// Structure for returning list of devices on one wire
typedef struct{
    uint8_t size;
    OWDevice *devices;
} ds18b20_devices;

void ds18b20WaitForConversion(void);
simple_float ds18b20DecodeTemperature(void);
simple_float* ds18b20ReadTemperatureAll(void);
simple_float ds18b20ReadTemperature(OWDevice device);
simple_float ds18b20ReadTemperatureSimple(void);
void ds18b20SetPrecision(uint8_t p);
ds18b20_devices ds18b20GetDevices(uint32_t scan);

const uint8_t OW_CRC_TABLE[] = {
    0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31, 65,
    157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130, 220,
    35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60, 98,
    190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161, 255,
    70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89, 7,
    219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196, 154,
    101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122, 36,
    248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185,
    140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205,
    17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80,
    175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238,
    50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115,
    202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139,
    87, 9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22,
    233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168,
    116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53
};

void OWInit(void)
{
	state = ONE_WIRE_ERROR;

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Pin = OW_GPIO;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(OW_PORT, &GPIO_InitStructure);
}
#define ONE_WIRE_RESET_PULSE (uint32_t)500UL
uint32_t OWResetPulse(void)
{
    // Pull bus down for 500 us (min 480 us)
#ifdef USE_CRITICAL
   	startCritical();
#endif
	OW_PORT->BRR = OW_GPIO;
	TIM2->CNT  = 0;
    while(TIM2->CNT<ONE_WIRE_RESET_PULSE);
    OW_PORT->BSRR = OW_GPIO;

    // Wait 70 us, bus should be pulled up by resistor and then
    // pulled down by slave (15-60 us after detecting rising edge)
    TIM2->CNT = 0;
    while(TIM2->CNT<70);
    uint32_t bit = OW_PORT->IDR&OW_GPIO;

#ifdef USE_CRITICAL
	stopCritical();
#endif
    if(!bit){
    	//DEBUG_X("\r\n__OW_SENSOR_OK");
        state = ONE_WIRE_SLAVE_PRESENT;
    }else {
        state = ONE_WIRE_ERROR;
    	//DEBUG_X("\r\n__OW_NO_SENSOR");
        return DS18B20_ERROR;
    }
    // Wait additional 430 us until slave keeps bus down (total 500 us, min. 480 us)
    TIM2->CNT = 0;
    while(TIM2->CNT<430);
    return 0;
}

void OWWrite1(void)
{
    // Pull bus down for 15 us
    OW_PORT->BRR = OW_GPIO;
    TIM2->CNT = 0;
    while(TIM2->CNT<15);
    OW_PORT->BSRR = OW_GPIO;
    // Wait until end of timeslot (60 us) + 5 us for recovery
    TIM2->CNT = 0;
     while(TIM2->CNT<50);
}

void OWWrite0(void)
{
    // Pull bus down for 60 us
    OW_PORT->BRR = OW_GPIO;
    TIM2->CNT = 0;
    while(TIM2->CNT<60);
    OW_PORT->BSRR = OW_GPIO;
    // Wait until end of timeslot (60 us) + 5 us for recovery
    TIM2->CNT = 0;
    while(TIM2->CNT<5);
}

void OWWriteBit(uint8_t bit)
{
    if (bit) {
        OWWrite1();
    } else {
        OWWrite0();
    }
}

uint8_t OWReadBit(void)
{
    // Pull bus down for 5 us
    OW_PORT->BRR = OW_GPIO;
    TIM2->CNT = 0;
    while(TIM2->CNT<5);
    OW_PORT->BSRR = OW_GPIO;
    // Wait 5 us and check bus state
    TIM2->CNT = 0;
    while(TIM2->CNT<5);

    uint32_t bit;
    bit = OW_PORT->IDR&OW_GPIO;
    // Wait until end of time slot (60 us) + 5 us for recovery
    TIM2->CNT = 0;
    while(TIM2->CNT<55);
    if (bit)return 1;
    return 0;
}

void OWWriteByte(uint8_t data)
{
    uint8_t i;
    for (i = 0; i < 8; ++i) {
        if ((data >> i) & 1) {
            OWWrite1();
        } else {
            OWWrite0();
        }
    }
}

uint8_t OWReadByte(void)
{
    uint8_t i;
    uint8_t data = 0;
    uint8_t bit;
    for (i = 0; i < 8; ++i) {
        bit = OWReadBit();
        data |= bit << i;
    }
    return data;
}

void OWResetCRC(void)
{
    crc8 = 0;
}

uint8_t OWGetCRC(void)
{
    return crc8;
}

uint8_t OWCRC(uint8_t data)
{
    crc8 = OW_CRC_TABLE[crc8 ^ data];
    return crc8;
}

void OWReadROM(void)
{
    OWResetPulse();
    OWWriteByte(0x33);

    uint32_t x;

    for(x = 0; x<8; x++){
    	OWReadByte();
    }
}

int OWSearch(void)
{
    uint8_t id_bit_number;
    int last_zero, rom_byte_number, search_result;
    uint8_t id_bit, cmp_id_bit;
    uint8_t rom_byte_mask, search_direction;

    // initialize for search
    id_bit_number = 1;
    last_zero = 0;
    rom_byte_number = 0;
    rom_byte_mask = 1;
    search_result = 0;
    crc8 = 0;
    // if the last call was not the last one
    if (!lastDeviceFlag){
        // 1-Wire reset
    	if(OWResetPulse()) {
            // reset the search
            lastMismatch = 0;
            lastDeviceFlag = 0;
#ifdef DEBUG_DS18B
            DEBUG_X("OW_RESET_ERROR\r\n");
#endif
            return 0;
        }
        // issue the search command
        OWWriteByte(0xF0);
        // loop to do the search
        do{
            // read a bit and its complement
            id_bit = OWReadBit();
            cmp_id_bit = OWReadBit();
            // check for no devices on 1-wire
            if ((id_bit == 1) && (cmp_id_bit == 1))
                break; // no devices

            // all devices coupled have 0 or 1
            if (id_bit != cmp_id_bit)
                search_direction = id_bit; // bit write value for search
            else{
                // if this discrepancy if before the Last Discrepancy
                // on a previous next then pick the same as last time
                if (id_bit_number < lastMismatch)
                    search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
                else
                    // if equal to last pick 1, if not then pick 0
                    search_direction = (id_bit_number == lastMismatch);
                // if 0 was picked then record its position in LastZero
                if (search_direction == 0){
                    last_zero = id_bit_number;
                }
            }
            // set or clear the bit in the ROM byte rom_byte_number
            // with mask rom_byte_mask
            if (search_direction == 1)
                ROM_NO[rom_byte_number] |= rom_byte_mask;
            else
                ROM_NO[rom_byte_number] &= ~rom_byte_mask;
            // serial number search direction write bit
            OWWriteBit(search_direction);
            // increment the byte counter id_bit_number
            // and shift the mask rom_byte_mask
            id_bit_number++;
            rom_byte_mask <<= 1;
            // if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
            if (rom_byte_mask == 0){
                OWCRC(ROM_NO[rom_byte_number]); // accumulate the CRC
                rom_byte_number++;
                rom_byte_mask = 1;
            }
        }
        while(rom_byte_number < 8); // loop until through all ROM bytes 0-7
        // if the search was successful then
        if (!((id_bit_number < 65) || (crc8 != 0))){
            // search successful so set last_mismatch, last_device_flag, search_result
            lastMismatch = last_zero;
            // check for last device
            if (lastMismatch == 0)
                lastDeviceFlag = 1;

            search_result = 1;
        }
    }
    // if no device found then reset counters so next 'search' will be like a first
    if (!search_result || !ROM_NO[0]){
        lastMismatch = 0;
        lastDeviceFlag = 0;
        search_result = 0;
    }
    return search_result;
}

uint8_t OWMatchROM(OWDevice device)
{
    int i;
    OWResetPulse();
    OWWriteByte(0x55); // Match ROM command
    for (i = 0; i < 8; ++i) {
        OWWriteByte(device.address[i]);
    }
    return 0;
}

int OWFirst(void)
{
    // reset the search state
    lastMismatch = 0;
    lastDeviceFlag = 0;
    return OWSearch();
}

int OWNext(void)
{
    // leave the search state alone
    return OWSearch();
}

OWDevice* OWSearchROM(uint8_t *devices)
{
    int result, i;
    OWDeviceCount = 0;
    result = OWFirst();
//    char buffer[10];
    while (result){
        OWDevice device;
        // print device found - CRC, ID, Family
        for (i = 7; i >= 0; i--) {
//            sprintf(buffer, "%02X", ROM_NO[i]);
//            usart2_print(buffer);
        }
        for (i = 7; i >= 0; i--) {
            device.address[i] = ROM_NO[i];
        }
        OWDevices[OWDeviceCount++] = device;
        result = OWNext();
    }
    *devices = OWDeviceCount;
    return OWDevices;
}

void ds18b20Init(void)
{
	OWInit();
	OWListOfDevices = OWSearchROM(&OWDevicesListSize);
	ds18b20SetPrecision(3);
	//DEBUG_X("\r\nOW_COUNT: %lu",OWDevicesListSize);
}

uint8_t ds18b20GetPrecision(void)
{
	return precision;
}

ds18b20_devices ds18b20GetDevices(uint32_t scan)
{
    if (scan) {
        OWDevicesListSize = 0;
        OWListOfDevices = OWSearchROM(&OWDevicesListSize);
    }
    ds18b20_devices ret;
    ret.size = OWDevicesListSize;
    ret.devices = OWListOfDevices;
    return ret;
}

void ds18b20SetPrecision(uint8_t p)
{
	precision = p;
	OWResetPulse();
	OWWriteByte(0xCC); // Skip ROM
	OWWriteByte(0x4E); // Write scratchpad

	OWWriteByte(0x4B);
	OWWriteByte(0x46);
	// set precision
	OWWriteByte(0x1F | (precision << 5));
}

void ds18b20ConvertTemperature(OWDevice device)
{
	//OWResetPulse();
	OWMatchROM(device); // Match ROM
	OWWriteByte(0x44); // Convert temperature
}

void ds18b20ConvertTemperatureAll(void)
{
	uint8_t i = 0;
	for (i = 0; i < OWDevicesListSize; ++i) {
		ds18b20ConvertTemperature(OWListOfDevices[i]);
	}
}

uint32_t ds18b20DecodeTemperatureRaw(uint16_t *raw)
{
#ifdef DEBUG_DS18B
	DEBUG_X("ds18b20DecodeTemperatureRaw()\r\n");
#endif
	int i;
	uint8_t crc;
	uint8_t data[9];
	OWResetCRC();

	for (i = 0; i < 9; ++i) {
		data[i] = OWReadByte();
		crc = OWCRC(data[i]);
	}
	if (crc != 0)return OW_CRC_ERROR;
	swapBytes(data);
	*raw =bytesToInt(data);
    return 0;
}

uint32_t ds18b20ReadTemperatureRaw(OWDevice device,uint16_t *dest)
{
#ifdef DEBUG_DS18B
	DEBUG_X("ds18b20ReadTemperatureRaw()\r\n");
#endif
	OWResetPulse();
	OWMatchROM(device); // Match ROM
	OWWriteByte(0xBE); // Read scratchpad
	return ds18b20DecodeTemperatureRaw(dest);
}

uint32_t ds18b20ReadTemperatureAllRaw(uint16_t data[3],uint32_t *mask)
{
	uint8_t i = 0;
	uint32_t x = OW_CRC_ERROR;
	uint32_t tmp = 0;

#ifdef DEBUG_DS18B
	DEBUG_X("ds18b20ReadTemperatureAllRaw()\r\n");
#endif
	for (i = 0; i < OWDevicesListSize; ++i) {
		if((x = ds18b20ReadTemperatureRaw(OWListOfDevices[i],&data[i])))tmp|=(1<<i);
	}
	*mask = tmp;
	return x;
}

void ds18b20WaitForConversion(void)
{
	switch(precision){
		case 0:	delay_ms(95);break;
		case 1:	delay_ms(190);break;
		case 2:	delay_ms(380);break;
		case 3:	delay_ms(750);
		default:	break;
	}
}

uint32_t searchOneWire(void)
{
	//LCD(1,2,"Searching OW Sensors");
#ifdef SYS_DEBUG
		DEBUG_X("Searching OW Sensors\r\n");
#endif

	OneWireCount = OWSearch();
	/*if(!(OneWireCount))LCD(0,3,"None found");
	else{LCDPrintInt(3,"%lu sensors detected",OneWireCount);}*/
#ifdef SYS_DEBUG
	if(!(OneWireCount))
	{
		DEBUG_X("None found\r\n");
	}
	else
	{
		char s[32];
		sprintf(s,"%lu sensors detected\r\n",OneWireCount);
		DEBUG_X(s);
	}
#endif
	delay_ms(700);
	return 0;
}

uint32_t readDS1820(uint16_t data[TEMP_SENSORS_COUNT])
{
	uint32_t mask = 0;
	uint32_t c = DS18B20_NOT_FOUND_ERROR;

	memset(data,0,sizeof(data));
	if(OneWireCount)
	{
		if((c = ds18b20ReadTemperatureAllRaw(data,&mask)))
		{
#ifdef DEBUG_DS18B
			DEBUG_X("Error reading One-wire sensors\r\n");
#endif
			return DS18B20_READ_ERROR;
		}
	}
	return c;
}

#define MAX_RESOLUTION (uint32_t)12UL
#define RESOLUTION_SHIFT (MAX_RESOLUTION-8)

void padString(char *s)
{
 	char *p;
	if((p=strrchr(s,'.'))==NULL)strcat(s,".00");
	else
	{
		if(*(p+2)=='\0')strcat(s,"0");
	}
}

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
	strcat(&s[strlen(s)],"C");
}

//usage
/*
void main(void)
{
	uint16_t data[TEMP_SENSORS_COUNT];

	ds18b20Init();
	searchOneWire();
	while(1){
				ds18b20ConvertTemperatureAll();//start converison here
				ds18b20WaitForConversion();
				readDS1820(data);
	}
}
*/
#endif /* DS18B20_H_ */

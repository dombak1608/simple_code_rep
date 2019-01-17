#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

void Read8(uint8_t* ptrBuffer, uint8_t* ptrValue);
void Read16(uint8_t* ptrBuffer, uint16_t* ptrValue);
void Read32(uint8_t* ptrBuffer, uint32_t* ptrValue);
void Write8(uint8_t* ptrBuffer, uint8_t* ptrValue);
void Write16(uint8_t* ptrBuffer, uint16_t* ptrValue);
void Write32(uint8_t* ptrBuffer, uint32_t* ptrValue);

int main()
{
	uint8_t value1, value5;
	uint16_t value2, value4;
	uint32_t value3;
	uint8_t buffer1[10] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x12 };
	uint8_t buffer2[10];

	Read8(&(buffer1[0]), &value1);
	Read16(&(buffer1[1]), &value2);
	Read32(&(buffer1[3]), &value3);
	Read16(&(buffer1[7]), &value4);
	Read8(&(buffer1[9]), &value5);

	Write16(&(buffer2[0]), &value2);
	Write8(&(buffer2[2]), &value1);
	Write16(&(buffer2[3]), &value4);
	Write32(&(buffer2[5]), &value3);
	Write8(&(buffer2[9]), &value5);
	return 0;
}

void Read8(uint8_t* ptrBuffer, uint8_t* ptrValue)
{
	(*ptrValue) = (uint8_t)(ptrBuffer[0]);
}

void Read16(uint8_t* ptrBuffer, uint16_t* ptrValue)
{
	(*ptrValue) = (uint16_t)(ptrBuffer[0]);
	(*ptrValue) <<= 8;
	(*ptrValue) |= (uint16_t)(ptrBuffer[1]);
}

void Read32(uint8_t* ptrBuffer, uint32_t* ptrValue)
{
	(*ptrValue) = (uint32_t)(ptrBuffer[0]);
	(*ptrValue) <<= 8;
	(*ptrValue) |= (uint32_t)(ptrBuffer[1]);
	(*ptrValue) <<= 8;
	(*ptrValue) |= (uint32_t)(ptrBuffer[2]);
	(*ptrValue) <<= 8;
	(*ptrValue) |= (uint32_t)(ptrBuffer[3]);
}

void Write8(uint8_t* ptrBuffer, uint8_t* ptrValue)
{
	ptrBuffer[0] = (uint8_t)(ptrValue[0]);
}

void Write16(uint8_t* ptrBuffer, uint16_t* ptrValue)
{
	ptrBuffer[1] = (uint8_t)((*ptrValue) & 0x00FF);
	ptrBuffer[0] = (uint8_t)(((*ptrValue)>>8) & 0x00FF);
}

void Write32(uint8_t* ptrBuffer, uint32_t* ptrValue)
{
	ptrBuffer[3] = (uint8_t)((*ptrValue) & 0x000000FF);
	ptrBuffer[2] = (uint8_t)(((*ptrValue) >> 8) & 0x000000FF);
	ptrBuffer[1] = (uint8_t)(((*ptrValue) >> 16) & 0x000000FF);
	ptrBuffer[0] = (uint8_t)(((*ptrValue) >> 24) & 0x000000FF);
}
#include <stdio.h>
#include <stdint.h>

//varijable
#define brCasa 6
#define buffSize 15
typedef struct filterS
{
	uint8_t oblici;
	uint8_t boje;
	uint8_t masaMin;
	uint8_t masaMax;
}filter;
void filterParseIzBuffer(void);
void punjenjeBuffera(void);
void pomicanjePolja(uint8_t brBit);
uint8_t maskaF(uint8_t brBit);
uint8_t rx_buffer[buffSize];
filter filteriP[brCasa];

filter filteriSlanje[brCasa - 1];	//5*24 bita = 120 bit = 15 B; 6. casa je poznata u stm32-zapisano (nerazvrstano-neprepoznato)
uint8_t bufferSlanje[buffSize];
//kraj

int main()
{
	filteriSlanje[0].oblici = 0b10001;
	filteriSlanje[0].boje = 0b10000;
	filteriSlanje[0].masaMin = 0b1001000;
	filteriSlanje[0].masaMax = 0b0011001;

	filteriSlanje[4].oblici = 0b10001;
	filteriSlanje[4].boje = 0b10000;
	filteriSlanje[4].masaMin = 0b1001000;
	filteriSlanje[4].masaMax = 0b0011001;

	punjenjeBuffera();
	uint8_t i;
	for (i = 0; i < buffSize; i++)
	{
		rx_buffer[i] = bufferSlanje[i];
	}
	filterParseIzBuffer();
	
	
	return 0;
}
//pocetak def funkc
void punjenjeBuffera(void)
{
	uint8_t i, j;
	for (i = 0, j = 0; i < brCasa - 1; i++, j+=3)
	{
		bufferSlanje[j] |= filteriSlanje[i].oblici & maskaF(5);
		bufferSlanje[j] |= (filteriSlanje[i].boje & maskaF(3)) << 5;
		bufferSlanje[j + 1] |= (filteriSlanje[i].boje & 0x18) >> 3;
		bufferSlanje[j + 1] |= (filteriSlanje[i].masaMin & maskaF(6)) << 2;
		bufferSlanje[j + 2] |= (filteriSlanje[i].masaMin & 0x40) >> 6;
		bufferSlanje[j + 2] |= (filteriSlanje[i].masaMax & maskaF(7)) << 1;
	}
}

void filterParseIzBuffer(void)
{
	uint8_t i;

	for (i = 0; i < brCasa - 1; i++)
	{
		filteriP[i].oblici = rx_buffer[0] & maskaF(5);	//uzimanje 5 bitova iz polja i pomicanje cijelog polja
		pomicanjePolja(5);

		filteriP[i].boje = rx_buffer[0] & maskaF(5);	//uzimanje 5 bitova iz polja i pomicanje cijelog polja
		pomicanjePolja(5);

		filteriP[i].masaMin = rx_buffer[0] & maskaF(7);	//uzimanje 7 bitova iz polja i pomicanje cijelog polja
		pomicanjePolja(7);

		filteriP[i].masaMax = rx_buffer[0] & maskaF(7);	//uzimanje 7 bitova iz polja i pomicanje cijelog polja
		pomicanjePolja(7);
	}
	//if sve ok sa filterima
	//poznatiFilteri = 1;
}
void pomicanjePolja(uint8_t brBit)
{
	uint8_t i;
	for (i = 0; i < buffSize; i++)
	{
		rx_buffer[i] >>= brBit;
		if (i < buffSize - 1)
		{
			rx_buffer[i] |= (rx_buffer[i + 1] & maskaF(brBit)) << (8 - brBit);
		}
	}
}
uint8_t maskaF(uint8_t brBit)
{
	int i;
	uint8_t maska = 0;
	for (i = 0; i < brBit; i++)
	{
		maska <<= 1;
		maska |= 1;
	}
	return maska;
}
//kraj
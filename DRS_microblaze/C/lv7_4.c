#include <stdio.h>
#include "xparameters.h"
#include "xgpio.h"
#include "xil_types.h"

void print(char *str);
void rotation(int rotationDirection);
int i=1;

int main()
{
    u8 data;
    while(1)
    {
    	while(1)
		{
			xil_printf("unesi slovo: ");
			data = XUartLite_RecvByte(XPAR_UARTLITE_1_BASEADDR);
			if(data == 13) break;
			i++;
			Xil_Out32LE(XPAR_MICRON_RAM_MEM0_BASEADDR + (i*4), data);
		}
		Xil_Out32LE(XPAR_MICRON_RAM_MEM0_BASEADDR, i);
		xil_printf("uneseseno je %d znakova",i);
		rotation(0); //0 lijevo, 1 desno
    }
    return 0;
}

void rotation(int rotationDirection)
{
	int pomocnaAdr = XPAR_MICRON_RAM_MEM0_BASEADDR + 4;
	int vrijednost;
	int vrijednost2;
	int j;
	if(!rotationDirection)
	{
		for(j=0;j<i;j++)
		{
			vrijednost = Xil_In32LE(XPAR_MICRON_RAM_MEM0_BASEADDR + ((j+2)*4));
			vrijednost2 = Xil_In32LE(XPAR_MICRON_RAM_MEM0_BASEADDR + ((j+3)*4));
			Xil_Out32LE(pomocnaAdr, vrijednost);
			Xil_Out32LE(XPAR_MICRON_RAM_MEM0_BASEADDR + ((j+2)*4), vrijednost2);
		}
	}
	else
	{
		for(j=i;j>=0;j--)
		{
			vrijednost = Xil_In32LE(XPAR_MICRON_RAM_MEM0_BASEADDR + ((j+2)*4));
			vrijednost2 = Xil_In32LE(XPAR_MICRON_RAM_MEM0_BASEADDR + ((j+3)*4));
			Xil_Out32LE(pomocnaAdr, vrijednost);
			Xil_Out32LE(XPAR_MICRON_RAM_MEM0_BASEADDR + ((j+2)*4), vrijednost2);
		}
	}
}

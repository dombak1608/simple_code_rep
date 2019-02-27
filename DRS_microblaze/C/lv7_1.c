#include <stdio.h>
#include "xparameters.h"

void print(char *str);

int main()
{
    int i;
    int address;
    int data;

    for(i = 0; i < 14; i++)
    {
    	address = XPAR_MICRON_RAM_MEM0_BASEADDR + (i*4);
    	Xil_Out32LE(address, i);
    }
	
    for(i = 0; i < 14; i++)
    {
    	address = XPAR_MICRON_RAM_MEM0_BASEADDR + (i*4);
    	data = Xil_In32LE(address);

        xil_printf("Na memorijskoj adresi 0x%x spremljen je podatak %d\r\n", address, data);
    }

    return 0;
}

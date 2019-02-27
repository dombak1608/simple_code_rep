#include <stdio.h>
#include "xparameters.h"
#include "xgpio.h"
#include "xil_types.h"

void print(char *str);

int main()
{
    u8 data;
    int i=0;

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
    }
    return 0;
}

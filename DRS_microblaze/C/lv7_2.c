#include <stdio.h>
#include "xparameters.h"
#include "xgpio.h"
#include "xil_types.h"

void print(char *str);

int main()
{
    u8 data;
    XGpio switches, leds;
	int switchesValue;

	XGpio_Initialize(&switches, XPAR_DIP_SWITCHES_8BITS_DEVICE_ID);
	XGpio_SetDataDirection(&switches, 1, 1);
	XGpio_Initialize(&leds, XPAR_LEDS_8BITS_DEVICE_ID);
	XGpio_SetDataDirection(&leds, 1, 0);

    while(1)
    {
    	switchesValue = XGpio_DiscreteRead(&switches, 1);
    	XGpio_DiscreteWrite(&leds, 1, switchesValue);

    	xil_printf("unesi slovo: ");
        data = XUartLite_RecvByte(XPAR_UARTLITE_1_BASEADDR);

		Xil_Out32LE(XPAR_MICRON_RAM_MEM0_BASEADDR + (switchesValue*4), data);

        xil_printf("Primljeni podatak je: %d, a ispisan kao character: %c\r\n", data, data);
    }

    return 0;
}

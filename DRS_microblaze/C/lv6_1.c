#include "xtft.h"
#include "xparameters.h"

#define TFT_FRAME_ADDR XPAR_MICRON_RAM_MEM0_BASEADDR
#define FGCOLOR_VALUE 0x00000000
#define BGCOLOR_VALUE 0x00FFFFFF
#define RED_COLOR 0x00FF0000
#define GREEN_COLOR 0x0000FF00
#define BLUE_COLOR 0x000000FF

int TftInitialize(u32 TftDeviceId);
int TftWriteString(XTft *InstancePtr, const u8 *CharValue);

static XTft TftInstance;

int main()
{
    int Status;
    u8 Znak;

    Status = TftInitialize(XPAR_VGA_CONTROLLER_DEVICE_ID);
    if ( Status != XST_SUCCESS) {
        return XST_FAILURE;
    }
	/*
    XTft_ClearScreen(&TftInstance);

    XTft_SetColor(&TftInstance, FGCOLOR_VALUE, BGCOLOR_VALUE);

    XTft_Write(&TftInstance, 'n');
    XTft_Write(&TftInstance, 'o');
    XTft_Write(&TftInstance, 'o');
    XTft_Write(&TftInstance, 'b');
    XTft_Write(&TftInstance, 'a');
    XTft_Write(&TftInstance, 'r');
    XTft_Write(&TftInstance, 'e');
    XTft_Write(&TftInstance, '!');
    */


    //XTft_ClearScreen(&TftInstance);
    //TftWriteString(&TftInstance,"nananna");
	
    XTft_ClearScreen(&TftInstance);
    XTft_SetColor(&TftInstance, FGCOLOR_VALUE, BGCOLOR_VALUE);
    int i,j;
    for(i=30;i<91;i+=20)
    {
    	for(j=30;j<450;j++)
    	{
    		XTft_SetPixel(&TftInstance,j,i,0x00FFFF00);
    	}
    }
    return XST_SUCCESS;
}

int TftInitialize(u32 TftDeviceId){

    int Status;
    XTft_Config *TftConfigPtr;
	
    TftConfigPtr = XTft_LookupConfig(TftDeviceId);

    if (TftConfigPtr == (XTft_Config *)NULL) {
        return XST_FAILURE;
    }

    Status = XTft_CfgInitialize(&TftInstance, TftConfigPtr, TftConfigPtr->BaseAddress);

    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    //Èekanje da Vsync status bit bude postavljen u 1 kako bi bili sigurni da je
    //prethodni frame prikazan
    while (XTft_GetVsyncStatus(&TftInstance) !=
                    XTFT_IESR_VADDRLATCH_STATUS_MASK);

    XTft_SetFrameBaseAddr(&TftInstance, TFT_FRAME_ADDR);

    return XST_SUCCESS;
}

//funkcija piše znak po znak na ekran sve dok ne doðe do null znaka ili kraja stringa
int TftWriteString(XTft *InstancePtr, const u8 *CharValue)
{

    while (*CharValue != 0) {
        XTft_Write(InstancePtr, *CharValue);
        CharValue++;
    }

    return XST_SUCCESS;
}

#include "xstatus.h"
#include "stdio.h"
#include "xps2.h"
#include "xtft.h"
#include "xparameters.h"

#define KEYBOARD_ACK        0xFA
#define TFT_FRAME_ADDR XPAR_MICRON_RAM_MEM0_BASEADDR
#define FGCOLOR_VALUE 0x00000000
#define BGCOLOR_VALUE 0x00FFFFFF
#define RED_COLOR 0x00FF0000
#define GREEN_COLOR 0x0000FF00
#define BLUE_COLOR 0x000000FF

int Ps2Initialize(u16 Ps2DeviceId);
int Ps2SendData(u8 data);
int Ps2ReceiveData();

int TftInitialize(u32 TftDeviceId);
int TftWriteString(XTft *InstancePtr, const u8 *CharValue);

static XPs2 Ps2Inst;
u8 SendCODE = 0xED;
u8 RxBuffer;
u8 Trash;

static XTft TftInstance;

int main(void)
{
    int Status, i, j;

    Status = Ps2Initialize(XPAR_PS2_0_DEVICE_ID);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }
	Status = TftInitialize(XPAR_VGA_CONTROLLER_DEVICE_ID);
	if ( Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

    /*print("Sending data to keyboard\r\n");
    for(i=0; i<8; i++){


        Status = Ps2SendData(SendCODE);
        Status = Ps2SendData(i);

        for(j=0; j<66670000/12; j++);
    }
*/
	XTft_ClearScreen(&TftInstance);
	XTft_SetColor(&TftInstance, FGCOLOR_VALUE, BGCOLOR_VALUE);
    print("Receiving data from keyboard:\r\n");
    while(1){

        Status = Ps2ReceiveData();
        if (Status != XST_SUCCESS){
            return XST_FAILURE;
            print("Data receiving FAILED!\r\n");
        }
        else{
        	if(RxBuffer == 0xF0) Ps2ReceiveData();
        	else
        	{
        		xil_printf("KEY_CODE: %x\r\n", RxBuffer);
        		if(RxBuffer == 0x1C) TftWriteString(&TftInstance,"a");
        		if(RxBuffer == 0x1B) TftWriteString(&TftInstance,"s");
        		if(RxBuffer == 0x23) TftWriteString(&TftInstance,"d");
        		if(RxBuffer == 0x2B) TftWriteString(&TftInstance,"f");
        	}

			/*
			XTft_ClearScreen(&TftInstance);
			XTft_SetColor(&TftInstance, FGCOLOR_VALUE, BGCOLOR_VALUE);
			TftWriteString(&TftInstance,"nananna");
			*/
        }
    }

    return XST_SUCCESS;
}

int Ps2Initialize(u16 Ps2DeviceId)
{
    XPs2_Config *Ps2ConfigPtr;

    Ps2ConfigPtr = XPs2_LookupConfig(Ps2DeviceId);
    if (Ps2ConfigPtr == NULL) {
        print("Config Lookup FAILED!\r\n");
        return XST_FAILURE;
    }
    else
        print("Config Lookup SUCCESS!\r\n");

    XPs2_CfgInitialize(&Ps2Inst, Ps2ConfigPtr, Ps2ConfigPtr->BaseAddress);

    print("PS2 Initialize SUCCESS!\r\n");
    return XST_SUCCESS;
}

int Ps2SendData(u8 data){

    u32 StatusReg;
    u32 BytesSent;
    u32 BytesReceived;
    int ACK_WAIT = 10000;

    BytesSent = XPs2_Send(&Ps2Inst, &data, 1);
    if( BytesSent != 1) {
        print("Data Send FAILED!\r\n");
        return XST_FAILURE;
    }

    do {
        StatusReg = XPs2_GetStatus(&Ps2Inst);
        ACK_WAIT--;
    }while((StatusReg & XPS2_STATUS_RX_FULL) == 0 && ACK_WAIT > 0);

    BytesReceived = XPs2_Recv(&Ps2Inst, &RxBuffer, 1);
	
    //Provjeriti jel primljen ACK byte
    if (RxBuffer != KEYBOARD_ACK) {
        print("Nije primljen ACK byte!\r\n");
        return XST_FAILURE;
    }
    else
        xil_printf("Data %x Send SUCESS!\r\n", data);

    return XST_SUCCESS;
}

int Ps2ReceiveData(){

    u32 StatusReg;
    u32 BytesReceived;

    do {
        StatusReg = XPs2_GetStatus(&Ps2Inst);
    }while((StatusReg & XPS2_STATUS_RX_FULL) == 0);
    BytesReceived = XPs2_Recv(&Ps2Inst, &RxBuffer, 1);
	
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

int TftWriteString(XTft *InstancePtr, const u8 *CharValue)
{

    while (*CharValue != 0) {
        XTft_Write(InstancePtr, *CharValue);
        CharValue++;
    }

    return XST_SUCCESS;
}

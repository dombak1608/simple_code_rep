#include "xps2.h"
#include "xparameters.h"
#include "xstatus.h"
#include "stdio.h"

#define KEYBOARD_ACK 0xFA

int Ps2Initialize(u16 Ps2DeviceId);
int Ps2SendData(u8 data);
int Ps2ReceiveData();

static XPs2 Ps2Inst;
u8 SendCODE = 0xED;
u8 RxBuffer;

int main(void)
{
    int Status, i, j;

    Status = Ps2Initialize(XPAR_PS2_0_DEVICE_ID);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    print("Sending data to keyboard\r\n");
    for(i=0; i<8; i++){
    	Ps2SendData(SendCODE);
    	Ps2SendData(i);
        //delay od cca 1s
        for(j=0; j<66670000/12; j++);
    }

    print("Receiving data from keyboard:\r\n");
    while(1){
    	Status = Ps2ReceiveData();

        if (Status != XST_SUCCESS){
            return XST_FAILURE;
            print("Data receiving FAILED!\r\n");
        }
        else
        	xil_printf("Podatak: 0x%x\n\r",RxBuffer);
    }
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
	
	XPs2_CfgInitialize(&Ps2Inst,&Ps2ConfigPtr,Ps2ConfigPtr->BaseAddress);
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

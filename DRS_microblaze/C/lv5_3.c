#include "stdio.h"
#include "xil_exception.h"
#include "xstatus.h"
#include "xparameters.h"
#include "xintc.h"
#include "xps2.h"
#include "xgpio.h"

#define PS2_DEVICE_ID 0
#define INTC_DEVICE_ID 0
#define INTR_ID XPAR_XPS_INTC_0_PS2_MOUSE_KEYBOARD_IP2INTC_IRPT_1_INTR
#define printf xil_printf

int Ps2IntrExample(XIntc* IntcInstPtr, XPs2* Ps2InstPtr, u16 Ps2DeviceId, u8 Ps2IntrId);

static void Ps2IntrHandler(void *CallBackRef, u32 Event, u32 EventData);

static int Ps2SetupIntrSystem(XIntc* IntcInstPtr, XPs2 *Ps2Ptr, u8 IntrId);

XPs2 Ps2Inst;
XIntc IntcInst;
XGpio ledice;
int lediceCounter=1;
int zadnjiF0=0;

//Varijable koje se koriste u prekidnoj rutini
volatile static int RxDataRecv = FALSE;     /* Flag to indicate Receive Data */
volatile static int RxError = FALSE;        /* Flag to indicate Receive Error */
volatile static int RxOverFlow = FALSE;     /* Flag to indicate Receive Overflow */
volatile static int TxDataSent = FALSE;     /* Flag to indicate Tx Data sent */
volatile static int TxNoAck = FALSE;        /* Flag to indicate Tx No Ack */
volatile static int TimeOut = FALSE;        /* Flag to indicate Watchdog Timeout */
volatile static int TxNumBytes = 0;         /* Number of bytes transmitted */
volatile static int RxNumBytes = 0;         /* Number of bytes received */

u8 RxBuffer;

int main(void)
{
    int Status;

    XGpio_Initialize(&ledice,0);
    XGpio_SetDataDirection(&ledice,1,0);
	Status = Ps2IntrExample(&IntcInst, &Ps2Inst, PS2_DEVICE_ID, INTR_ID);

    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    return XST_SUCCESS;

}

/****************************************************************************/
/**
*
* Ova funkcija je primjer inicijalizacije i komunikacije s tipkovnicom u interrupt naèinu rada.
* APIs.
*
*
* @param    IntcInstPtr je pokazivaè na varijablu tipa XIntc.
* @param    Ps2InstPtr je pokazivaè na varijablu tipa XPs2.
* @param    Ps2DeviceId je device id XPS PS2 kontrolera iz datoteke xparameters.h.
* @param    Ps2IntrId je vrijednost konstante XPAR_<INTC_instance>_<PS2_instance>_VEC_ID iz datoteke xparameters.h.
* @return
*       - XST_SUCCESS ako je funkcija završio uspješno.
*       - XST_FAILURE ako je funkcija nije završila uspješno.
*
* @note     Funkcija nikada neæe vratiti XST_SUCCESS ukoliko se ne implementira prekid beskonaène petlje.
****************************************************************************/
int Ps2IntrExample(XIntc* IntcInstPtr, XPs2* Ps2InstPtr,
            u16 Ps2DeviceId, u8 Ps2IntrId)
{
    int Status;
    XPs2_Config *ConfigPtr;

    ConfigPtr = XPs2_LookupConfig(Ps2DeviceId);
    if (ConfigPtr == NULL) {
        return XST_FAILURE;
    }

    XPs2_CfgInitialize(Ps2InstPtr, &ConfigPtr, ConfigPtr->BaseAddress);

    //Self Test PS/2 ureðaja
    Status = XPs2_SelfTest(Ps2InstPtr);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    Status = Ps2SetupIntrSystem(IntcInstPtr, Ps2InstPtr, INTR_ID);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    //Postavljanje prekidne rutine
    XPs2_SetHandler(Ps2InstPtr, (XPs2_Handler)Ps2IntrHandler, Ps2InstPtr);

    XPs2_IntrEnable(Ps2InstPtr, XPS2_IPIXR_RX_ALL);
    XPs2_IntrGlobalEnable(Ps2InstPtr);

    printf("\r\n Press Keys on the keyboard \r\n");

    //Beskonaèna petlja
    while(1)
    {
    	/*if(RxBuffer == 0x1C) XGpio_DiscreteWrite(&ledice,1,0b00001000);
    	if(RxBuffer == 0x1B) XGpio_DiscreteWrite(&ledice,1,0b00000100);
    	if(RxBuffer == 0x23) XGpio_DiscreteWrite(&ledice,1,0b00000010);
    	if(RxBuffer == 0x2B) XGpio_DiscreteWrite(&ledice,1,0b00000001);
    	*/
    	//XGpio_DiscreteWrite(&ledice,1,lediceCounter);


    	XGpio_DiscreteWrite(&ledice,1,lediceCounter);
    }

    //Onemoguæavanje prekida za prijem i globalno onemoguæavanje prekida u XPS PS2 kontroleru
    //XPs2_IntrDisable(&Ps2Inst, XPS2_IPIXR_ALL);
    //XPs2_IntrGlobalDisable(&Ps2Inst);

    return XST_SUCCESS;
}

/*****************************************************************************/
/**
* Prekidna rutina koja se poziva od strane procesora u sluèajevima kada XPS PS2
* kontroler generira jedan od sljedeæih prekida:
*   - Primanje podataka     (Receive Data Interrupt)
*   - Primanje pogreške     (Receive Error Interrupt)
*   - Primanje prelijevanja (Receive Overflow Interrupt)
*   - Slanje podataka       (Transmit Data Interrupt)
*   - Ne primanje ACK       (Transmit No ACK Interrupt)
*   - Watchdog timeout      (Watchdog Timeout Interrupt)
*
* @param    CallBackRef je callback referenca koja se prosljeðuje u prekidnu
*           rutinu.
* @param    IntrMask je maska koja govori koji je dogaðaj izazvao prekid.
*           U datoteci xps2_l.h se nalaze konstante XPS2_IPIXR_* kojima se
*           objašnjavaju dogaðaji koji su izazvali prekid:
*       - XPS2_IPIXR_RX_FULL za Primanje podataka
*       - XPS2_IPIXR_RX_ERR za Primanje pogreške
*       - XPS2_IPIXR_RX_OVF za Primanje prelijevanja
*       - XPS2_IPIXR_TX_ACK za Slanje podataka
*       - XPS2_IPIXR_TX_NOACK za Ne primanje ACK
*       - XPS2_IPIXR_WDT_TOUT za Watchdog timeout
* @param    ByteCount sadrži broj bytova koji su primljeni ili poslani u
*           trenutku poziva.
*
* @return   None.
*
* @note     None.*
******************************************************************************/
static void Ps2IntrHandler(void *CallBackRef, u32 IntrMask, u32 ByteCount)
{

	ByteCount = XPs2_Recv(&Ps2Inst, &RxBuffer, 1);
	if(RxBuffer == 0xF0) zadnjiF0=1;
	if(RxBuffer == 0x1C && !zadnjiF0)
	{
		if(lediceCounter == 128)
			lediceCounter=1;
		else
			lediceCounter*=2;
		zadnjiF0=0;
	}
	if(RxBuffer == 0x1B && !zadnjiF0)
	{
		if(lediceCounter == 1) lediceCounter=128;
		else lediceCounter/=2;
		zadnjiF0=0;
	}
    //Podatak je primljen.
    if (IntrMask & XPS2_IPIXR_RX_FULL) {

        RxDataRecv = TRUE;
        RxNumBytes = ByteCount;

        printf("%x\r\n", RxBuffer);
        //printf ("Interrupt mask: XPS2_IPIXR_RX_FULL\r\n");
    }

    //Primljena je pogreška.
    if (IntrMask & XPS2_IPIXR_RX_ERR) {

        RxError = TRUE;
        //printf ("Interrupt mask: XPS2_IPIXR_RX_ERR\r\n");
    }

    //Primljen prelijev.
    if (IntrMask & XPS2_IPIXR_RX_OVF) {

        RxOverFlow = TRUE;
        //printf ("Interrupt mask: XPS2_IPIXR_RX_OVF\r\n");
    }

    //Transmission of the specified data is completed.
    if (IntrMask & XPS2_IPIXR_TX_ACK) {

        TxDataSent = TRUE;
        TxNumBytes = ByteCount;
        //printf ("Interrupt mask: XPS2_IPIXR_TX_ACK\r\n");
    }

    //Nije poslan ACK.
    if (IntrMask & XPS2_IPIXR_TX_NOACK) {

        TxNoAck = TRUE;
        //printf ("Interrupt mask: XPS2_IPIXR_TX_NOACK\r\n");
    }

    //Timeout slanaja.
    if (IntrMask & XPS2_IPIXR_WDT_TOUT) {

        TimeOut = TRUE;
        //printf ("Interrupt mask: XPS2_IPIXR_WDT_TOUT\r\n");
    }
}

/****************************************************************************/
/**
* Funkcija za postaljanje sustava prekida tako da se oni mogu dogoditi za
* XPS PS2 kontroler.
*
* @param    IntcInstPtr je pokazivaè na varijablu tipa XIntc.
* @param    Ps2Ptr je pokazivaè na varijablu tipa XPs2 koja æe biti povezana
*           s upravljaèem prekida.
* @param    IntrId je konstanta XPAR_<INTC_instance>_<PS2_instance>_VEC_ID iz
*           datoteke xparameters.h.
*
* @return   XST_SUCCESS ako je uspješno završena ili XST_FAILURE u suprotnom.
*
* @note     None.
*
****************************************************************************/
static int Ps2SetupIntrSystem(XIntc* IntcInstPtr, XPs2 *Ps2Ptr, u8 IntrId )
{
    int Status;

    Status = XIntc_Initialize(IntcInstPtr, IntrId);

    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    //Povezivanje kontrolera s prekidnom rutinom
    Status = XIntc_Connect(IntcInstPtr,
                IntrId,
                (XInterruptHandler) XPs2_IntrHandler,
                Ps2Ptr);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }
    XIntc_Start(IntcInstPtr, XIN_REAL_MODE);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    XIntc_Enable(IntcInstPtr, IntrId);

    //Inicijalizacija iznimaka
    Xil_ExceptionInit();

    //Povezivanje prekidne rutine s iznimkama
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
                (Xil_ExceptionHandler) XIntc_InterruptHandler,
                IntcInstPtr);

    //Omoguæavanje iznimki
    Xil_ExceptionEnable();

    return XST_SUCCESS;
}

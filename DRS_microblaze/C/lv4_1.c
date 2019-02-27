#include <stdio.h>
#include <xparameters.h>
#include <xtmrctr.h>
#include <xintc.h>
#include <xgpio.h>

#define TIMER_CNTR_0 0
#define RESET_VALUE 49999998

int TmrCtrIntrInit(XIntc* IntcInstancePtr,
 XTmrCtr* TmrInstancePtr,
 u16 TimerDeviceId,
 u16 IntcDeviceId,
 u16 IntrSourceId,
 u8 TmrCtrNumber);

void TimerCounterHandler(void *CallBackRef, u8 TmrCtrNumber);

XIntc InterruptControler;
XTmrCtr TimerCounterInst;
XGpio leds;

volatile int TimerExpired;

int main(void)
{
    print("-- Start of the program! --\r\n");

    int Status;

    XGpio_Initialize(&leds,XPAR_LEDS_8BITS_DEVICE_ID);
    XGpio_SetDataDirection(&leds,1,0x00000000);
	
    Status = TmrCtrIntrInit(&InterruptControler,
                            &TimerCounterInst,
                            XPAR_DELAY_DEVICE_ID,
                            XPAR_INTC_0_DEVICE_ID,
                            XPAR_INTC_0_TMRCTR_0_VEC_ID,
                            TIMER_CNTR_0);

    if (Status != XST_SUCCESS){
        return XST_FAILURE;
    }

    XTmrCtr_Start(&TimerCounterInst,XPAR_TMRCTR_0_DEVICE_ID);

    while (1) {

    }

    print("-- End of the program! --\r\n");
    return XST_SUCCESS;
}

/********************************************************************/
/**
* Inicijalizacija timera i upravljaèa prekidima.
* Funkcija prima sljedeæe parametre:
*
* @paramIntcInstancePtr - pokazivaè na varijablu tipa XIntc,
* @paramTmrCtrInstancePtr - pokazivaè na varijablu tipa XTmrCtr,
* @paramTimerDeviceId - vrijednost konstante XPAR_<TmrCtr_instance>_DEVICE_ID iz datoteke xparameters.h,
* @paramIntcDeviceId - vrijednost konstante XPAR_<Intc_instance>_DEVICE_ID iz datoteke xparameters.h,
* @paramIntrSourceId - vrijednost konstante XPAR_<INTC_instance>_<TmrCtr_instance>_INTERRUPT_INTR iz datoteke xparameters.h,
* @paramTmrCtrNumber - redni broj timera koji se inicijalizira.
*
* @returnXST_SUCCESS ako je inicijalizacija uspješna, a u suprotno funkcija vraæa XST_FAILURE
*
*********************************************************************/
int TmrCtrIntrInit(XIntc* IntcInstancePtr,
 XTmrCtr* TmrCtrInstancePtr,
 u16 TimerDeviceId,
 u16 IntcDeviceId,
 u16 IntrSourceId,
 u8 TmrCtrNumber)
{
    int Status;

    print("Init STARTED\r\n");
    Status=XTmrCtr_Initialize(TmrCtrInstancePtr,TimerDeviceId);
    if (Status != XST_SUCCESS) {
        print("Timer Initialize FAILED\r\n");
        return XST_FAILURE;
    }
    print("Timer Initialize SUCCESS\r\n");

    Status=XIntc_Initialize(IntcInstancePtr,IntcDeviceId);
    if (Status != XST_SUCCESS) {
        print("Intc Initialize FAILED\r\n");
        return XST_FAILURE;
    }
    print("Intc Initialize SUCCESS\r\n");

    /*
     * Povezivanje upravljaèa prekida s rukovateljem prekida koji se
     * poziva kada se dogodi prekid. Rukovatelj prekida obavlja
     * specifiène zadatke vezane za rukovanje prekidima.
     */
    Status = XIntc_Connect(IntcInstancePtr, IntrSourceId,
     (XInterruptHandler)XTmrCtr_InterruptHandler,
     (void *)TmrCtrInstancePtr);

    if (Status != XST_SUCCESS) {
        print("Intc Connect FAILED\r\n");
        return XST_FAILURE;
    }
    print("Intc Connect SUCCESS\r\n");

    Status=XIntc_Start(IntcInstancePtr,XIN_REAL_MODE);

    if (Status != XST_SUCCESS) {
        print("Intc Start FAILED\r\n");
        return XST_FAILURE;
    }
    print("Intc Start SUCCESS\r\n");

    XIntc_Enable(IntcInstancePtr,IntcDeviceId);


    //Omoguæavanje microblaze prekida.
    microblaze_enable_interrupts();

    /*
     * Postavljanje prekidne rutine koja æe biti pozvana kada se dogodi prekid
     * od strane timera. Kao parametri predaju se pokazivaè na komponentu za
     * koju se postavlja prekidna rutina, naziv prekidne rutine (funkcije)
     * i pointer na timer, kako bi prekidna rutina mogla pristupiti timeru.
     */
    XTmrCtr_SetHandler(TmrCtrInstancePtr,
     TimerCounterHandler,
     TmrCtrInstancePtr);
	 
    XTmrCtr_SetOptions(TmrCtrInstancePtr,TmrCtrNumber,XTC_INT_MODE_OPTION|XTC_AUTO_RELOAD_OPTION|XTC_DOWN_COUNT_OPTION);

    XTmrCtr_SetResetValue(TmrCtrInstancePtr,TmrCtrNumber,RESET_VALUE);

    print("Init FINISHED\r\n");
    return XST_SUCCESS;
}

/*
 * Prekidna rutina koja se poziva kada timer generira prekid.
 * Funkcija prima pokazivaè na void parametar CallBackRef
 * koji se cast-a na pokazivaè tipa XTmrCtr.
 * Ovaj parametar je napravljen kako bi se pokazao naèin na
 * koji se unutar prekidne rutine može pristupiti timer
 * komponenti i njenim funkcijama.
*/
void TimerCounterHandler(void *CallBackRef, u8 TmrCtrNumber)
{
    print("Interrupt Handler!\r\n");

    XTmrCtr *InstancePtr = (XTmrCtr *)CallBackRef;

    int timeMS;
    uint uCount;

    TimerExpired++;
    XGpio_DiscreteWrite(&leds,1,TimerExpired);
    uCount=XTmrCtr_GetValue(&TimerCounterInst,TIMER_CNTR_0);
    timeMS=(uCount+2)/50000;

    xil_printf("Elapsed time: %d ms\tcount: %d\r\n", timeMS, uCount);
}

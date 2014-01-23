#include <cstdio>
#include "miosix.h"
#include "spi_driver.h"
#include "nRF24L01P.h"
#include "arch/cortexM4_stm32f4/common/CMSIS/stm32f4xx.h"
#include <miosix/kernel/scheduler/scheduler.h>
#define TRANSFER_SIZE 4

using namespace std;
using namespace miosix;

bool trasmission;
int num_step=0;
static Thread *waiting=0;
char *data; //data received from air


typedef Gpio<GPIOD_BASE,12> greenLed;
typedef Gpio<GPIOA_BASE,1> IRQ;
typedef Gpio<GPIOD_BASE,14> redLed;

void invia(int num_passi){
    trasmission=true;
}


void __attribute__((naked)) EXTI1_IRQHandler(){
    saveContext();
    asm volatile("bl _Z16EXTI1HandlerImplv");
    restoreContext();
}

void __attribute__((used))EXTI1HandlerImpl(){
    EXTI->PR=EXTI_PR_PR1;
    redLed::high();
    printf("Sono nell'interrupt");
    if(waiting==0) return;
    waiting->IRQwakeup(); 
    if(waiting->IRQgetPriority()>Thread::IRQgetCurrentThread()->IRQgetPriority())
        Scheduler::IRQfindNextThread();
    waiting=0;
}

void waitForModule(){
    FastInterruptDisableLock dLock; 
    waiting=Thread::IRQgetCurrentThread();
    while(waiting)
    {
        Thread::IRQwait(); 
        FastInterruptEnableLock eLock(dLock); 
        Thread::yield(); 
    }
}

void configureModuleInterrupt()
{
    IRQ::mode(Mode::INPUT_PULL_UP);
    SYSCFG->EXTICR[1] = SYSCFG_EXTICR1_EXTI1_PA;
    EXTI->IMR |= EXTI_IMR_MR1; /*Periferica che gestisce gli external interrupt, è per il gpio 1*/
    EXTI->RTSR &= ~EXTI_RTSR_TR1; /*Vado a verificare durante il fronte di salita*/
    EXTI->FTSR |= EXTI_FTSR_TR1;
    NVIC_EnableIRQ(EXTI1_IRQn); /*Abilitano il controller dell'interrupt, passando il nome e poi la priorità*/
    NVIC_SetPriority(EXTI1_IRQn,15); //Low priority
    
}
void *wifi_start(void *arg)
{
    nRF24L01P *my_nrf24l01p = new nRF24L01P();

    char txData[TRANSFER_SIZE]="cia";
    int txDataCnt = 4;
 
    my_nrf24l01p->powerUp();
 
    // Display the (default) setup of the nRF24L01+ chip
    printf( "nRF24L01+ Frequency    : %d MHz\r\n",  my_nrf24l01p->getRfFrequency() );
    printf( "nRF24L01+ Output power : %d dBm\r\n",  my_nrf24l01p->getRfOutputPower() );
    printf( "nRF24L01+ Data Rate    : %d kbps\r\n", my_nrf24l01p->getAirDataRate() );
    printf( "nRF24L01+ TX Address   : 0x%010llX\r\n", my_nrf24l01p->getTxAddress() );
    printf( "nRF24L01+ RX Address   : 0x%010llX\r\n", my_nrf24l01p->getRxAddress() );
 
    printf( "Type keys to test transfers:\r\n  (transfers are grouped into %d characters)\r\n", TRANSFER_SIZE );
 
     my_nrf24l01p->setTransferSize( TRANSFER_SIZE );
 
    my_nrf24l01p->setReceiveMode();
    my_nrf24l01p->enable();

    while(1){
        my_nrf24l01p->write( NRF24L01P_PIPE_P0, txData, txDataCnt );
        usleep(1000000);
}
   
    printf("Hello world, write your application here\n");
    
}

void *wifi_receive(void *arg){
    nRF24L01P *my_nrf24l01p = new nRF24L01P();

    char rxData[TRANSFER_SIZE];
    int rxDataCnt = 4;
 
    my_nrf24l01p->powerUp();
 
    // Display the (default) setup of the nRF24L01+ chip
    printf( "nRF24L01+ Frequency    : %d MHz\r\n",  my_nrf24l01p->getRfFrequency() );
    printf( "nRF24L01+ Output power : %d dBm\r\n",  my_nrf24l01p->getRfOutputPower() );
    printf( "nRF24L01+ Data Rate    : %d kbps\r\n", my_nrf24l01p->getAirDataRate() );
    printf( "nRF24L01+ TX Address   : 0x%010llX\r\n", my_nrf24l01p->getTxAddress() );
    printf( "nRF24L01+ RX Address   : 0x%010llX\r\n", my_nrf24l01p->getRxAddress() );
 
    printf( "Type keys to test transfers:\r\n  (transfers are grouped into %d characters)\r\n", TRANSFER_SIZE );
 
     my_nrf24l01p->setTransferSize( TRANSFER_SIZE );
 
    my_nrf24l01p->setReceiveMode();
    my_nrf24l01p->enable();
    while(1){
        waitForModule();
        rxDataCnt = my_nrf24l01p->read( NRF24L01P_PIPE_P0, rxData, sizeof( rxData ) );
        for ( int i = 0; rxDataCnt > 0; rxDataCnt--, i++ ) {
 
                printf("%c\n", rxData[i] );
            }
    
}
}

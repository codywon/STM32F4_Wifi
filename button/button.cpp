
#include "button.h"
#include <miosix.h>
#include <miosix/kernel/scheduler/scheduler.h>

using namespace miosix;

typedef Gpio<GPIOA_BASE,0> button;

static Thread *waiting=0; /**/

void __attribute__((naked)) EXTI0_IRQHandler()
{
    saveContext();
    asm volatile("bl _Z16EXTI0HandlerImplv");
    restoreContext();
}

void __attribute__((used)) EXTI0HandlerImpl()
{
    EXTI->PR=EXTI_PR_PR0; 
    if(waiting==0) return;
    waiting->IRQwakeup();
    if(waiting->IRQgetPriority()>Thread::IRQgetCurrentThread()->IRQgetPriority())
		Scheduler::IRQfindNextThread();
    waiting=0;
}

void configureButtonInterrupt()
{
    button::mode(Mode::INPUT_PULL_DOWN);
    EXTI->IMR |= EXTI_IMR_MR0; 
    EXTI->RTSR |= EXTI_RTSR_TR0; 
    NVIC_EnableIRQ(EXTI0_IRQn); 
    NVIC_SetPriority(EXTI0_IRQn,15);

}

void waitForButton() 
{
    FastInterruptDisableLock dLock; 
    waiting=Thread::IRQgetCurrentThread();
    while(waiting)
    {
        Thread::IRQwait(); 
        FastInterruptEnableLock eLock(dLock); 
        Thread::yield(); 
    }
}

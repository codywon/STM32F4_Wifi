
#include <cstdio>
#include "miosix.h"
#include "./nrf24l01p/wifi_module.h"
#include "./pedometer/pedometer.h"
#include "./button/button.h"
#include "./audio/slice-and-play.h"
#include "util/lcd44780.h"


#define LCD_ROW         2
#define LCD_COL         8


using namespace std;
using namespace miosix;

typedef Gpio<GPIOE_BASE,7>  rs;         //pin4
typedef Gpio<GPIOE_BASE,8>  e;          //pin6
typedef Gpio<GPIOE_BASE,9>  d4;         //pin11
typedef Gpio<GPIOE_BASE,10>  d5;        //pin12
typedef Gpio<GPIOE_BASE,11>  d6;        //pin13
typedef Gpio<GPIOE_BASE,12>  d7;        //pin14

static pthread_t stepsThread;
static Thread *pedometer_t;




void *steps_thread(void *arg){
    int steps;
    configureButtonInterrupt();
    for(;;){
        waitForButton();
        steps = Pedometer::instance().getSteps();
        if(steps != 0){
                ring::instance().play_n_of_step(steps,100);
        }
        usleep(100000);
        
    }
}

void *pedometerTask(void *arg) {
    Pedometer::instance().init();
    Pedometer::instance().start();
}

void initCore(){
    pedometer_t = Thread::create(pedometerTask, 2048, 2, NULL, Thread::JOINABLE);
    pthread_create(&stepsThread,NULL,&steps_thread,NULL);
    init();
}


int main()
{
    int steps;
    char stepsCod[32];
		
    Lcd44780 lcd(rs::getPin(), e::getPin(), d4::getPin(), d5::getPin(), d6::getPin(), d7::getPin(), LCD_ROW, LCD_COL);
    while(1){
        usleep(500000);
        steps = Pedometer::instance().getSteps();
        if(steps != 0){
            sprintf(stepsCod, "%d", steps);
            send(stepsCod);
        }
        lcd.clear();
        lcd.go(0,0);
        lcd.printf("%d", steps);
        lcd.go(0,1);
        switch(Pedometer::instance().getMode()) {
            case Pedometer::MODE_STEADY:
                lcd.printf("STEADY");
                break;
            case Pedometer::MODE_WALK:
                lcd.printf("WALK");
                break;            
            case Pedometer::MODE_RUN:
                lcd.printf("RUN");
                break; 
            default: lcd.printf("");
        }

    }
     
       
}



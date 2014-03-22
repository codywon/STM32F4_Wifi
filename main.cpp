/**
 * File:   main.cpp
 *  \brief     This file contains integration code.
 *  \author    Andrea Corna
 *  \author    Lorenzo Fontana
 *  \version   1.0
 *  \date      20/03/2014
 */
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



/**
 * This function is implemented by a thread in order to listen how many steps you have done
 * pushing on blue button
 * @param arg
 * @return 
 */
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

/**
 * Function to init and start pedometer core. It should be implemented by a thread
 * @param arg
 * @return 
 */
void *pedometerTask(void *arg) {
    Pedometer::instance().init();
    Pedometer::instance().start();
}

/**
 * Initialize all threads
 */
void initCore(){
    pedometer_t = Thread::create(pedometerTask, 2048, 2, NULL, Thread::JOINABLE);
    pthread_create(&stepsThread,NULL,&steps_thread,NULL);
    init();
}

/**
 * Main function
 * @return -1 if something goes wrong
 */
int main()
{
    int steps;
    char stepsCod[32];
    float calories = 0;
		
    Lcd44780 lcd(rs::getPin(), e::getPin(), d4::getPin(), d5::getPin(), d6::getPin(), d7::getPin(), LCD_ROW, LCD_COL);
    while(1){
        usleep(500000);
        steps = Pedometer::instance().getSteps();
        if(steps != 0){
            sprintf(stepsCod, "%d", steps);
            send(stepsCod);
        }
        calories = Pedometer::instance().getCalories();
        lcd.clear();
        lcd.go(0,0);
        lcd.printf("%.2f", calories);
        lcd.go(0,1);
        lcd.printf("%d", steps);
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
    return -1; 
       
}



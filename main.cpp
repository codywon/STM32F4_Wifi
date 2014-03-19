
#include <cstdio>
#include "miosix.h"
#include "./nrf24l01p/wifi_module.h"
#include "./pedometer/pedometer.h"
#include "./button/button.h"
#include "./audio/slice-and-play.h"


using namespace std;
using namespace miosix;


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


int main()
{
    int steps;
    init();
    char stepsCod[32];
		
    pthread_t stepsThread;
    Thread *pedometer_t;
    pedometer_t = Thread::create(pedometerTask, 2048, 2, NULL, Thread::JOINABLE);
    pthread_create(&stepsThread,NULL,&steps_thread,NULL);
    while(1){
        usleep(500000);
        steps = Pedometer::instance().getSteps();
        if(steps != 0){
            sprintf(stepsCod, "%d", steps);
            send(stepsCod);
        }

    }
     
       
}



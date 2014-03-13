
#include <cstdio>
#include "miosix.h"
#include "./nrf24l01p/wifi_module.h"
#include "./pedometer/pedometer.h"
#include "./button/button.h"
#include "./audio/slice-and-play.h"


using namespace std;
using namespace miosix;


void *steps_thread(void *arg){
    configureButtonInterrupt();
    for(;;){
        waitForButton();
        int steps = 1123;
        //int steps = Pedometer::instance().getSteps();
        ring::instance().play_n_of_step(steps,100);
        usleep(100000);
        
    }
}


int main()
{
    init();
    char stepsCod[32];
		
    pthread_t stepsThread;
    pthread_create(&stepsThread,NULL,&steps_thread,NULL);
    while(1){
        usleep(500000);
        int steps = 300;
        sprintf(stepsCod, "%d", steps);
        //int steps = Pedometer::instance().getSteps();
        send(stepsCod);

    }
     
       
}



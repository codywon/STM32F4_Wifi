
#include <cstdio>
#include "miosix.h"
#include "wifi_module.h"


using namespace std;
using namespace miosix;



int main()
{
    char transmit[32];
    char received[960];
    pthread_t wifi_thread;
    pthread_t thread_receive;
    init();
    while(1){
    
        scanf("%s", transmit);
        send(transmit);
        receive(received);

    }
    pthread_join(wifi_thread,NULL);
    pthread_join(thread_receive,NULL);
    
       
}
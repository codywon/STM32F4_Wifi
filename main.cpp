
#include <cstdio>
#include "miosix.h"
#include "wifi_module.h"


using namespace std;
using namespace miosix;



int main()
{
    char transmit[32];
    char received[960];
    init();
    while(1){
    
        scanf("%s", transmit);
        send(transmit);
        receive(received);

    }
     
       
}
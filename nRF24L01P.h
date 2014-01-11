/* 
 * File:   nRF24L01P.h
 * Author: r0rshark
 *
 * Created on 3 gennaio 2014, 17.22
 */
#include "spi_driver.h"

#ifndef NRF24L01P_H
#define	NRF24L01P_H


class nRF24L01P {
public:
    nRF24L01P();
    nRF24L01P(const nRF24L01P& orig);
    virtual ~nRF24L01P();    
    void power_up();
    void power_down();
    void set_transmit_mode();
    void set_receive_mode();
    void transmit(int num_passi);
    int receive(int pipe,char *data,int count);
    void test(); 

private:
    void CE_enable();
    void CE_disable();
    void set_register(int addr_registro,int data_registro);
    int get_register(int registro);
    bool packet_in_pipe(int pipe);
    int get_status_register();
    spi_driver *spi;
    int mode;

};

#endif	/* NRF24L01P_H */


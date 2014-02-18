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
    int transmit(int count, char* data);
    int receive(char *data,int count);
    int get_frequency();
    int get_air_data_rate(); 
    int get_output_power();
    int get_crc_width();
    void disable_auto_ack();
    void disable_auto_retransmit();
    unsigned long long get_tx_address();
    unsigned long long get_rx_address();
    int get_register_status();
    void reset_interrupt();
    bool packet_in_pipe();



private:
    void clear_pending_interrupt();
    void disable_tx_interrupt();
    void CE_enable();
    void CE_disable();
    void set_register(int addr_registro,int data_registro);
    int get_register(int registro);
    void CE_restore(int old_ce);
    void setup_Gpio();
    void set_frequency(int frequency);
    void set_power_output(int power);
    void set_air_data_rate(int rate);
    void set_tx_address(int number);
    void flush_tx();
    void set_crc_width(int width);
    void set_tx_address(unsigned long long address, int width);
    void set_rx_address(unsigned long long address, int width);
    void set_transfer_size(int size);
    int mode;
    spi_driver *spi;

};

#endif	/* NRF24L01P_H */


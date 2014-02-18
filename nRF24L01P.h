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
    void set_crc_width(int width);
    void set_tx_address(unsigned long long address, int width);
    void set_rx_address_pipe0(unsigned long long address, int width);
    void set_frequency(int frequency);
    void set_power_output(int power);
    void set_air_data_rate(int rate);
    void set_tx_address(int number);
    void set_transfer_size(int size);
    int get_frequency();
    int get_air_data_rate(); 
    int get_output_power();
    int get_crc_width();
    unsigned long long get_tx_address();
    unsigned long long get_rx_address_pipe0();
    int get_register_status();
    void reset_interrupt();
    bool packet_in_pipe0();
    void disable_auto_ack();
    void disable_auto_retransmit();
    void disable_tx_interrupt();
    int transmit(int count, char* data);
    int receive(char *data,int count);
    

private:
    /**
     * The method reset all interrupt bits
     */
    void clear_pending_interrupt();
    
    /**
     * The function set high CE gpio in order to communicate to module to accept the
     * configuration given by spi lines.
     */
    void CE_enable();
    
    /**
     * The function set low CE gpio in order to start to communicate the new configuration 
     * to the module.
     */
    void CE_disable();
    
    /**
     * The function allowes us to set a register with a specific value
     * @param addr_register - address of the register to set
     * @param data_register - data to set into the register
     */
    void set_register(int addr_register,int data_register);
    
    /**
     * The function return the data of the register
     * @param reg - the register witch we want to know the data
     * @return data of the register
     */
    int get_register(int reg);
    
    /**
     * The function restore the old value of the CE gpio
     * @param old_ce - old value
     */
    void CE_restore(int old_ce);
    
    /**
     * The function setup all the gpio in order to create a spi communication beetween the board and the module.
     * We use:
     * - GPIOB 11 CE;
     * - GPIOB 12 CS;
     * - GPIOB 13 SCK;
     * - GPIOB 14 MISO;
     * - GPIOB 15 MOSI;
     * - GPIOA 1 IRQ;
     * We use the SPI2 interface
     */
    void setup_Gpio();
    
    void flush_tx();
    
    
    int mode;
    /**
     * Driver spi variable
     */
    spi_driver *spi;
    
    /**
     * Enums of the possibly states of the module
     */
    typedef enum {
        NRF24L01P_UNKNOWN_MODE,
        NRF24L01P_POWER_DOWN_MODE,
        NRF24L01P_STANDBY_MODE,
        NRF24L01P_RX_MODE,
        NRF24L01P_TX_MODE,
    } NRF24L01P_mode;   

};

#endif	/* NRF24L01P_H */


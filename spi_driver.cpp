/* 
 * File:   spi_driver.cpp
 * Author: r0rshark
 * 
 * Created on 3 gennaio 2014, 17.10
 */

#include "spi_driver.h"
#include <miosix.h>
#include <miosix/kernel/scheduler/scheduler.h>

using namespace miosix;

/*Spi Gpio*/
typedef Gpio<GPIOB_BASE,11> CE;
typedef Gpio<GPIOB_BASE,12> CS;
typedef Gpio<GPIOB_BASE,13> SCK;
typedef Gpio<GPIOB_BASE,14> MISO;
typedef Gpio<GPIOB_BASE,15> MOSI;
typedef Gpio<GPIOA_BASE,1> IRQ;

/*Led Gpio*/
typedef Gpio<GPIOD_BASE,12> greenLed;



spi_driver::spi_driver() {
     RCC->APB1ENR |= RCC_APB1ENR_SPI2EN; /*attivo il clock*/
    
    MISO::mode(Mode::ALTERNATE);
    MISO::alternateFunction(5);
    MOSI::mode(Mode::ALTERNATE); 
    MOSI::alternateFunction(5);
    
    IRQ::mode(Mode::INPUT);
   
    greenLed::mode(Mode::OUTPUT);
    greenLed::high(); /*test*/
}

spi_driver::spi_driver(const spi_driver& orig) {
}

spi_driver::~spi_driver() {
}

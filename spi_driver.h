/* 
 * File:   spi_driver.h
 * Author: r0rshark
 *
 * Created on 3 gennaio 2014, 17.10
 */

#ifndef SPI_DRIVER_H
#define	SPI_DRIVER_H

class spi_driver {
public:
    spi_driver();
    spi_driver(const spi_driver& orig);
    virtual ~spi_driver();
    int write(int command);
    int spi_Receive();
private:

};

#endif	/* SPI_DRIVER_H */


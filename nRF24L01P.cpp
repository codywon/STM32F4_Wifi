/* 
 * File:   nRF24L01P.cpp
 * Author: r0rshark
 * 
 * Created on 3 gennaio 2014, 17.22
 */

#include "nRF24L01P.h"
#include <cstdio>
#include "miosix.h"

//NRF24L01P Macro 
//rf register 
 
#define NRF24L01P_RF_HIGH_BIT       (1 << 3)
#define NRF24L01P_RF_LOW_BIT        (1 << 5)
#define NRF24L01P_RF_MASK           (NRF24L01P_RF_LOW_BIT| NRF24L01P_RF_HIGH_BIT)
#define NRF24L01P_RF_250KBPS        (NRF24L01P_RF_LOW_BIT)
#define NRF24L01P_RF_1MBPS          (0)
#define NRF24L01P_RF_2MBPS          (NRF24L01P_RF_HIGH_BIT)

//pipe number for multiceiver
#define NRF24L01P_PIPE_NO_0     0
#define NRF24L01P_PIPE_NO_1     1
#define NRF24L01P_PIPE_NO_2     2
#define NRF24L01P_PIPE_NO_3     3
#define NRF24L01P_PIPE_NO_4     4
#define NRF24L01P_PIPE_NO_5     5
#define NRF24L01P_REG_RX_PW_P0              0x11

//size buffers
#define NRF24L01P_RX_BUFFER_SIZE       32


//Command
#define NRF24L01P_CMD_RD_REG                    0x00
#define NRF24L01P_CMD_WT_REG                    0x20
#define NRF24L01P_CMD_WR_TX_PAYLOAD             0xa0
#define NRF24L01P_CMD_NOP        0xff
#define NRF24L01P_R_RX_PAY       0x61
#define NRF24L01P_SPI_CMD_FLUSH_TX          0xe1

//bitmask and register address
#define NRF24LO1P_REG_ADDR_BITMASK              0x1f
#define NRF24L01P_REG_CONF                      0x00
#define NRF24L01P_REG_STATUS                    0x07
#define NRF24L01P_REG_RF_CH                     0x05
#define NRF24L01P_REG_RF_SETUP                  0x06
#define NRF24L01P_REG_AA                        0x01
#define NRF24L01P_REG_SETUP_AW                  0x03
#define NRF24L01P_REG_TX_ADDR                   0x10
#define NRF24L01P_REG_RX_ADDR_P0            0x0a
#define NRF24L01P_REG_SETUP_RETR            0x04
#define NRF24L01P_REG_EN_RXADDR             0x02

#define NRF24L01P_PRIM_RX                       (1<<0)
#define NRF24L01P_PWR_UP                        (1<<1)
#define NRF24L01P_STATUS_TX_DS                  (1<<5)
#define NRF24L01P_STATUS_MAX_RT                 (1<<4)
#define NRF24L01P_STATUS_RX_DR                  (1<<6)
#define NRF24L01P_RF_SETUP_RF_PWR_MASK          (0x3<<1)
#define NRF24L01P_RF_SETUP_RF_DR_MASK           (40<<0)
#define NRF24L01P_RF_SETUP_PWR_0DBM             (0x3<<1)
#define NRF24L01P_RF_SETUP_PWR_MINUS_6DBM       (0x2<<1)
#define NRF24L01P_RF_SETUP_PWR_MINUS_12DBM      (0x1<<1)
#define NRF24L01P_RF_SETUP_PWR_MINUS_18DBM      (0x0<<1)
#define NRF24L01P_RF_DR_250KBPS                 (1<<5)
#define NRF24L01P_RF_DR_1MBPS                   (0)
#define NRF24L01P_RF_DR_2MBPS                   (1<<3)
#define NRF24L01P_STATUS_DR_RX                  (1<<6)         //set if data register full write 1 to clear
#define NRF24L01P_STATUS_RX_P_NO                (0x7<<1)
#define NRF24L01P_ENAA_P0                       (1<<0)
//CRC
#define NRF24L01P_CONFIG_CRC0                   (1<<2)
#define NRF24L01P_CONFIG_EN_CRC                 (1<<3)
#define NRF24L01P_CONFIG_CRC_MASK       (NRF24L01P_CONFIG_EN_CRC| NRF24L01P_CONFIG_CRC0)
#define NRF24L01P_CONFIG_CRC_NONE       (0)
#define NRF24L01P_CONFIG_CRC_8BIT       (NRF24L01P_CONFIG_EN_CRC)
#define NRF24L01P_CONFIG_CRC_16BIT      (NRF24L01P_CONFIG_EN_CRC|NRF24L01P_CONFIG_CRC0)
#define NRF24L01P_CRC_NONE               0
#define NRF24L01P_CRC_8_BIT              8
#define NRF24L01P_CRC_16_BIT            16

// SETUP_AW register:
#define NRF24L01P_SETUP_AW_AW_MASK      (0x3<<0)
#define NRF24L01P_SETUP_AW_AW_3BYTE     (0x1<<0)
#define NRF24L01P_SETUP_AW_AW_4BYTE     (0x2<<0)
#define NRF24L01P_SETUP_AW_AW_5BYTE     (0x3<<0)
//time
#define NRF24L01P_TPD2STBY                      2000  //2mS
#define NRF24L01P_TPECE2CSN                     4  //4uS
#define NRF24L01P_TPECETR                       10
#define NRF24L01P_TPRCV                         130

//size
#define NRF24L01P_TX_FIFO_SIZE                  32
#define NRF24L01P_RX_FIFO_SIZE                  32
#define NRF24L01P_MIN_RF_FREQUENCY              2400
#define NRF24L01P_MAX_RF_FREQUENCY              2525
#define NRF24L01P_TX_PWR_ZERO_DB                 0
#define NRF24L01P_TX_PWR_MINUS_6_DB             -6
#define NRF24L01P_TX_PWR_MINUS_12_DB            -12
#define NRF24L01P_TX_PWR_MINUS_18_DB            -18
#define NRF24L01P_DATARATE_250KBPS              250
#define NRF24L01P_DATARATE_1MBPS                1000
#define NRF24L01P_DATARATE_2MBPS                2000
#define NRF24L01P_EN_AA_NONE                   0
#define NRF24L01P_EN_RXADDR_NONE        0
#define NRF24L01P_SETUP_RETR_NONE       0
#define NRF24L01P_ADDRESS_DEFAULT   ((unsigned long long) 0xE7E7E7E7E7 )
#define NRF24L01P_ADDRESS_DEFAULT_WIDTH  5

#define NRF24L01P_RX_PW_Px_MASK         0x3F


typedef enum {
    NRF24L01P_UNKNOWN_MODE,
    NRF24L01P_POWER_DOWN_MODE,
    NRF24L01P_STANDBY_MODE,
    NRF24L01P_RX_MODE,
    NRF24L01P_TX_MODE,
} NRF24L01P_mode;       

using namespace miosix;


/*Spi Gpio*/
typedef Gpio<GPIOB_BASE,11> CE;
typedef Gpio<GPIOB_BASE,12> CS;
typedef Gpio<GPIOB_BASE,13> SCK;
typedef Gpio<GPIOB_BASE,14> MISO;
typedef Gpio<GPIOB_BASE,15> MOSI;
typedef Gpio<GPIOA_BASE,1> IRQ;


nRF24L01P::nRF24L01P() {
    spi = new spi_driver();
    setup_Gpio();
    //power_down();
    
    set_register(NRF24L01P_REG_STATUS, NRF24L01P_STATUS_TX_DS | NRF24L01P_STATUS_MAX_RT |
                                NRF24L01P_STATUS_RX_DR);/*clear every pending interrupt bits*/
    set_register(NRF24L01P_REG_EN_RXADDR, NRF24L01P_EN_RXADDR_NONE);
    set_tx_address(5);
    set_frequency(NRF24L01P_MIN_RF_FREQUENCY+2);
    set_power_output(NRF24L01P_TX_PWR_ZERO_DB);
    set_air_data_rate(NRF24L01P_DATARATE_1MBPS);
    set_crc_width(NRF24L01P_CRC_8_BIT);
    setTxAddress(NRF24L01P_ADDRESS_DEFAULT, NRF24L01P_ADDRESS_DEFAULT_WIDTH);
    setRxAddress(NRF24L01P_ADDRESS_DEFAULT, NRF24L01P_ADDRESS_DEFAULT_WIDTH,NRF24L01P_PIPE_NO_0);
    disable_auto_ack();
    disable_auto_retransmit();
    setTransferSize(4,NRF24L01P_PIPE_NO_0);
    //set_register(NRF24L01P_REG_AA,NRF24L01P_ENAA_P0);
    printf("Frequency %d\n",get_frequency());
    printf("Output power %d\n",get_output_power());
    printf("Air data rate %d\n",get_air_data_rate());
    printf("Crc %d\n",get_crc_width());
    printf("SETUP_AW %d\n",get_register(NRF24L01P_REG_SETUP_AW));
    printf("tx register 0x%010llX\n",get_tx_address());
    printf("rx address 0x%010llX\n",get_rx_address(NRF24L01P_PIPE_NO_0));
    showInternal();
      
}

nRF24L01P::nRF24L01P(const nRF24L01P& orig) {
}

nRF24L01P::~nRF24L01P() {
     
}

/**
 * power up the module
 */
void nRF24L01P::power_up() {
    //I get the current config and I add the power up bit then I write it back 
    int current_config = get_register(NRF24L01P_REG_CONF); 
    current_config |= NRF24L01P_PWR_UP;
    set_register(NRF24L01P_REG_CONF,current_config);
    usleep(NRF24L01P_TPD2STBY);
    //set_tx_address(5);
    //set_register(NRF24L01P_REG_TX_ADDR , 0xE7E7E7E701);
    //set_register(NRF24L01P_REG_RX_ADDR_P0 ,0xE7E7E7E7E7);
    printf("SETUP_AW %d\n",get_register(NRF24L01P_REG_SETUP_AW));
    printf("tx register 0x%010llX\n",get_tx_address());
    printf("rx address 0x%010llX\n",get_rx_address(NRF24L01P_PIPE_NO_0));
    mode=NRF24L01P_STANDBY_MODE;
    flushTx();
    showInternal();
}

void nRF24L01P::power_down() {
    int current_config = get_register(NRF24L01P_REG_CONF);
    current_config &= ~NRF24L01P_PWR_UP;
    set_register(NRF24L01P_REG_CONF,current_config);
    usleep(NRF24L01P_TPD2STBY);
    mode = NRF24L01P_POWER_DOWN_MODE;
}

void nRF24L01P::set_receive_mode(){
    
    if (mode==NRF24L01P_POWER_DOWN_MODE){
        power_up();
    }
    int cur_config = get_register(NRF24L01P_REG_CONF);
    cur_config |= NRF24L01P_PRIM_RX;
    set_register(NRF24L01P_REG_CONF,cur_config);
    /*if (CE::value()==0){
    CE_enable();
    }*/
    CE::high();
    usleep(NRF24L01P_TPRCV);    
    
    mode = NRF24L01P_RX_MODE;
   
}

void nRF24L01P::set_transmit_mode(){
    printf("Inizio transmit \n");
    if (mode==NRF24L01P_POWER_DOWN_MODE){
        power_up();
    }
    int cur_config = get_register(NRF24L01P_REG_CONF);
    cur_config &= ~NRF24L01P_PRIM_RX;
    set_register(NRF24L01P_REG_CONF,cur_config);
    
        CE_enable();
    
    mode = NRF24L01P_TX_MODE;
    printf("Fine transmit \n");
    
}
/**
 * function allowes to transmit a data with the nRF24L01P module
 * @param count dimension of data
 * @param data data to send
 * @return number of bits sent
 */
int nRF24L01P::transmit(int count, char* data){
    int old_ce = CE::value();
    if( count < 0)
        return 0;
    if( count > NRF24L01P_TX_FIFO_SIZE)
        count = NRF24L01P_TX_FIFO_SIZE;
    CE_disable();
    set_register(NRF24L01P_REG_STATUS, NRF24L01P_STATUS_TX_DS); /*clear bit interrupt data sent tx fifo*/
    CS::low();
    spi->spi_write(NRF24L01P_CMD_WR_TX_PAYLOAD); //command to start write from payload TX
    for( int i=0; i<count; i++){
        printf("char %c\n",*data);
        spi->spi_write(*data++);
    }
    //spi->spi_write(12);
    CS::high();
    int old_mode = mode;
    set_transmit_mode();
    CE::high();
    usleep(NRF24L01P_TPECETR);  
    CE_disable();
    printf("Before polling \n");
    printf("Get register status %d\n",get_register_status());
    while( !( get_register_status() & NRF24L01P_STATUS_TX_DS)){
        
    } //polling waiting for transfert complete
    printf("After polling\n");
    set_register(NRF24L01P_REG_STATUS, NRF24L01P_STATUS_TX_DS); /*clear bit data sent tx fifo*/
    if( old_mode == NRF24L01P_RX_MODE){              //reset the state before
        set_receive_mode();
    }
    CE_restore(old_ce);
    return count;
    
}

int nRF24L01P::receive(int pipe,char *data,int count){
    if(mode!=NRF24L01P_RX_MODE){
        printf("Before receive set up in recieve_mode\n");
        return -1;
    }
    if (pipe<NRF24L01P_PIPE_NO_0 || pipe>NRF24L01P_PIPE_NO_5){
        printf("Error number of pipe must be between 0 and 5 not %d\n",pipe);
        return -1;
    }
    if (count<=0) {
        return 0;
    }
    if (count>NRF24L01P_RX_BUFFER_SIZE){
        count= NRF24L01P_RX_BUFFER_SIZE;
    }
    if(packet_in_pipe(pipe)){
        //NB----I skip the phase of check the lenght of the packet
        
        CS::low();
        spi->spi_write(NRF24L01P_R_RX_PAY);
        
        for(int i=0;i<count;i++){
            *data = spi->spi_Receive();
            data++;
        }
        CS::low();
        //clear RX_DR status bit
        set_register(NRF24L01P_REG_STATUS,NRF24L01P_STATUS_DR_RX);
        return count;
    }
    else{
        printf("Pipe chosen is empty\n");
        return 0;
    }
    return 0;
}

void nRF24L01P::CE_restore(int old_ce){
    old_ce ? CE::high():CE::low();      //wifi_module.cpp:75:34: error: no matching function for call to 'nRF24L01P::receive()'restore old ce value
    usleep(NRF24L01P_TPECE2CSN);    //sleep to apply ce value change
}

void nRF24L01P::CE_enable(){
    CE::high();
    usleep(NRF24L01P_TPECE2CSN);        //sleep to apply ce value change
}

void nRF24L01P::CE_disable(){
    CE::low();
}

/**
 * function allowes to set a register to a particular value
 * @param addr_registro address of the register
 * @param data_registro data to set the register
 */
void nRF24L01P::set_register(int addr_registro,int data_registro){
        int old_ce =CE::value();  //save the CE value    
        CE_disable(); //in order to change value of register the module has to be in StandBy1 mode
        CS::low();
        spi->spi_write(NRF24L01P_CMD_WT_REG |(addr_registro & NRF24LO1P_REG_ADDR_BITMASK)); //command to write the at correct address of register
        spi->spi_write(data_registro & NRF24L01P_CMD_NOP);    //data used to set the register
        CS::high();
        CE_restore(old_ce);

}

int  nRF24L01P::get_register(int registro){
    int command = NRF24L01P_CMD_RD_REG | (registro & NRF24LO1P_REG_ADDR_BITMASK);
    int result;
    CS::low();
    spi->spi_write(command);   
    result = spi->spi_Receive();
    CS::high();
    return result;   
}

bool nRF24L01P::packet_in_pipe(int pipe){
    if ((pipe<NRF24L01P_PIPE_NO_0) || (pipe> NRF24L01P_PIPE_NO_5)){
        return false;
    }
    printf("Status register in packet_inpipe %\n",get_register_status());
    int status=get_register_status();
    //& is bitwise (it returns 01001100) && is and (return 0 or 1))
    if((status & NRF24L01P_STATUS_DR_RX)&&((status & NRF24L01P_STATUS_RX_P_NO)>>1)==(pipe & 0x7)){
        return true;
    }
    return false;
}

/**
 * Function to get the status register
 * @return status register
 */
int nRF24L01P::get_register_status(){
    CS::low();
    int status = spi->spi_Receive();    //the module send status bit every time is sent a command
    CS::high();
    return status;
}

void nRF24L01P::test_receive(){
    power_up();
    printf("Config register at power up %d\n",get_register(NRF24L01P_REG_CONF));
    printf("Status register before receive %d\n",get_register_status());
    set_receive_mode();
    printf("Config register at receive %d\n",get_register(NRF24L01P_REG_CONF));
    printf("Status register after receive %d\n",get_register_status());

    printf("default value");
    printf( "nRF24L01+ Frequency    : %d MHz\r\n",  get_frequency() );
   // printf( "nRF24L01+ Output power : %d dBm\r\n",  my_nrf24l01p.getRfOutputPower() );
    printf( "nRF24L01+ Data Rate    : %d kbps\r\n", get_air_data_rate());
   // printf( "nRF24L01+ RX Address   : 0x%010llX\r\n", my_nrf24l01p.getRxAddress() );
    char *data;
    while(true){
    usleep(3000000);
    printf("Status register %d\n",get_register_status());
    printf("Config register %d\n",get_register(NRF24L01P_REG_CONF));
    printf("ricevuto da pipe 0 %d\n",receive(NRF24L01P_PIPE_NO_0,data,1));
    printf("ho ricevuto %s\n",data);    
    printf("ricevuto da pipe 1 %d\n",receive(NRF24L01P_PIPE_NO_1,data,1));
    printf("ho ricevuto %s\n",data);


    }
}

void nRF24L01P::setup_Gpio(){
    
    MISO::mode(Mode::ALTERNATE);
    MISO::alternateFunction(5);
    MOSI::mode(Mode::ALTERNATE); 
    MOSI::alternateFunction(5);
    IRQ::mode(Mode::INPUT);
    SCK::mode(Mode::ALTERNATE);
    SCK::alternateFunction(5);
    CS::mode(Mode::OUTPUT);
    CS::high();
    CE::mode(Mode::OUTPUT);
    CE::high();
    
}

void nRF24L01P::set_frequency(int frequency){
    printf("Begin set frequency\n");
    if ((frequency < NRF24L01P_MIN_RF_FREQUENCY) | (frequency > NRF24L01P_MAX_RF_FREQUENCY)){
        printf("Error frequency module %d\n",frequency);
        return;
    }
    int channel = (frequency - NRF24L01P_MIN_RF_FREQUENCY) & 0x7F;  /*from manual RF_freq = frequency - NRF24L01P_MIN_RF_FREQUENCY)*/
    set_register(NRF24L01P_REG_RF_CH, channel);
    printf("end set frequency\n");
}
int nRF24L01P::get_frequency(){
    int freq = get_register(NRF24L01P_REG_RF_CH);
    return (freq + NRF24L01P_MIN_RF_FREQUENCY);

}

void nRF24L01P::set_power_output(int power){
    
    int rf_config = get_register(NRF24L01P_REG_RF_SETUP) & ~NRF24L01P_RF_SETUP_RF_PWR_MASK; /*get rf config except for the power bits*/
    printf("Start set power config %d\n", rf_config);
    switch (power){                                     /*set the power*/
        case NRF24L01P_TX_PWR_ZERO_DB:
            rf_config |= NRF24L01P_RF_SETUP_PWR_0DBM;
            break;
        case NRF24L01P_TX_PWR_MINUS_6_DB:
            rf_config |= NRF24L01P_RF_SETUP_PWR_MINUS_6DBM;
            break;
        case NRF24L01P_TX_PWR_MINUS_12_DB:
            rf_config |= NRF24L01P_RF_SETUP_PWR_MINUS_12DBM;
            break;
        case NRF24L01P_TX_PWR_MINUS_18_DB:
            rf_config |= NRF24L01P_RF_SETUP_PWR_MINUS_18DBM;
            break;
        default:
            printf("Error power module %d\n",power);
            return;
    }
    set_register(NRF24L01P_REG_RF_SETUP, rf_config);    /*set the rf setup register*/
    printf("End set power config %d\n",rf_config);
}

void nRF24L01P::set_air_data_rate(int rate){
    printf("Start set air rate\n");
    int air_config = get_register(NRF24L01P_REG_RF_SETUP) & ~NRF24L01P_RF_SETUP_RF_DR_MASK; /*get rf config except rf_dr_low and rf_dr_high*/
    switch (rate){
        case NRF24L01P_DATARATE_250KBPS:
            air_config |= NRF24L01P_RF_DR_250KBPS;
            break;
        case NRF24L01P_DATARATE_1MBPS:
            air_config |= NRF24L01P_RF_DR_1MBPS;
            break;
        case NRF24L01P_DATARATE_2MBPS:
            air_config |= NRF24L01P_RF_DR_2MBPS;
            break;
        default:
            printf("Error air data rate %d\n",rate);
            return;
    }
    set_register(NRF24L01P_REG_RF_SETUP, air_config);
}

int nRF24L01P::get_air_data_rate() {
 
    int rate = get_register(NRF24L01P_REG_RF_SETUP) & NRF24L01P_RF_SETUP_RF_DR_MASK;
 
    switch ( rate ) {
 
        case NRF24L01P_RF_250KBPS:
            return NRF24L01P_DATARATE_250KBPS;
 
        case NRF24L01P_RF_1MBPS:
            return NRF24L01P_DATARATE_1MBPS;
 
        case NRF24L01P_RF_2MBPS:
            return NRF24L01P_DATARATE_2MBPS;
 
        default:
            printf( "nRF24L01P: Unknown Air Data Rate value %d\n", rate );
            return 0;

    }
}

int nRF24L01P::get_output_power() {
 
    int power = get_register(NRF24L01P_REG_RF_SETUP) & NRF24L01P_RF_SETUP_RF_PWR_MASK;
 
    switch ( power ) {
 
        case NRF24L01P_RF_SETUP_PWR_0DBM:
            return NRF24L01P_TX_PWR_ZERO_DB;
 
        case NRF24L01P_RF_SETUP_PWR_MINUS_6DBM:
            return NRF24L01P_TX_PWR_MINUS_6_DB;
 
        case NRF24L01P_RF_SETUP_PWR_MINUS_12DBM:
            return NRF24L01P_TX_PWR_MINUS_12_DB;
 
        case NRF24L01P_RF_SETUP_PWR_MINUS_18DBM:
            return NRF24L01P_TX_PWR_MINUS_18_DB;
 
        default:
            printf( "nRF24L01P: Unknown RF Output Power value %d\n", power );
            return 0;
 
    }
}

int nRF24L01P::get_crc_width() {
 
    int crcWidth = get_register(NRF24L01P_REG_CONF) & NRF24L01P_CONFIG_CRC_MASK;
 
    switch ( crcWidth ) {
 
        case NRF24L01P_CONFIG_CRC_NONE:
            return NRF24L01P_CRC_NONE;
 
        case NRF24L01P_CONFIG_CRC_8BIT:
            return NRF24L01P_CRC_8_BIT;
 
        case NRF24L01P_CONFIG_CRC_16BIT:
            return NRF24L01P_CRC_16_BIT;
 
        default:
            printf( "nRF24L01P: Unknown CRC Width value %d\n", crcWidth );
            return 0;
 
    }
}

void nRF24L01P::test_transmit(){
    power_down();
    printf("Config register at power down %d\n",get_register(NRF24L01P_REG_CONF));
    power_up();
    printf("Config register at power up %d\n",get_register(NRF24L01P_REG_CONF));
    set_transmit_mode();
    printf("Config register at transmit %d\n",get_register(NRF24L01P_REG_CONF));
    
}

void nRF24L01P::set_tx_address(int number){
    int num_bit = number -2;
    set_register(NRF24L01P_REG_SETUP_AW, num_bit);
}

void nRF24L01P::flushTx()
{
    printf("ORA FLUSH TX\n");
  CS::low();
  spi->spi_write( NRF24L01P_SPI_CMD_FLUSH_TX  );  //svuoto coda TX
  CS::high();
  
}

void nRF24L01P::set_crc_width(int width) {
 
    int config = (NRF24L01P_REG_CONF) & ~NRF24L01P_CONFIG_CRC_MASK;
 
    switch ( width ) {
 
        case NRF24L01P_CRC_NONE:
            config |= NRF24L01P_CONFIG_CRC_NONE;
            break;
 
        case NRF24L01P_CRC_8_BIT:
            config |= NRF24L01P_CONFIG_CRC_8BIT;
            break;
 
        case NRF24L01P_CRC_16_BIT:
            config |= NRF24L01P_CONFIG_CRC_16BIT;
            break;
 
        default:
            printf( "nRF24L01P: Invalid CRC Width setting %d\n", width );
            return;
 
    }
 
    set_register(NRF24L01P_REG_CONF, config);
 
}

void nRF24L01P::disable_auto_ack(){
    set_register(NRF24L01P_REG_AA, NRF24L01P_EN_AA_NONE);// deactivate wait for ack*/
}

void nRF24L01P::disable_auto_retransmit() {
 
    set_register(NRF24L01P_REG_SETUP_RETR, NRF24L01P_SETUP_RETR_NONE);
 
}

unsigned long long nRF24L01P::get_tx_address() {
 
    int setupAw = get_register(NRF24L01P_REG_SETUP_AW) & NRF24L01P_SETUP_AW_AW_MASK;
 
    int width;
 
    switch ( setupAw ) {
 
        case NRF24L01P_SETUP_AW_AW_3BYTE:
            width = 3;
            break;
 
        case NRF24L01P_SETUP_AW_AW_4BYTE:
            width = 4;
            break;
 
        case NRF24L01P_SETUP_AW_AW_5BYTE:
            width = 5;
            break;
 
        default:
            printf( "nRF24L01P: Unknown getTxAddress width value %d\n", setupAw );
            return 0;
 
    }
 
    int cn = (NRF24L01P_CMD_RD_REG | (NRF24L01P_REG_TX_ADDR & NRF24LO1P_REG_ADDR_BITMASK));
 
    unsigned long long address = 0;
 
    CS::low();
 
    spi->spi_write(cn);
 
    for ( int i=0; i<width; i++ ) {
 
        //
        // LSByte first
        //
        address |= ( ( (unsigned long long)( spi->spi_Receive() & 0xFF ) ) << (i*8) );
 
    }
 
    CS::high();
 
    return address;
}

unsigned long long nRF24L01P::get_rx_address(int pipe) {
 
    if ( ( pipe < NRF24L01P_PIPE_NO_0 ) || ( pipe > NRF24L01P_PIPE_NO_5 ) ) {
 
        printf( "nRF24L01P: Invalid setRxAddress pipe number %d\n", pipe );
        return 0;
 
    }
 
    int width;
 
    if ( ( pipe == NRF24L01P_PIPE_NO_0 ) || ( pipe == NRF24L01P_PIPE_NO_1 ) ) {
 
        int setupAw = get_register(NRF24L01P_REG_SETUP_AW) & NRF24L01P_SETUP_AW_AW_MASK;
 
        switch ( setupAw ) {
 
            case NRF24L01P_SETUP_AW_AW_3BYTE:
                width = 3;
                break;
 
            case NRF24L01P_SETUP_AW_AW_4BYTE:
                width = 4;
                break;
 
            case NRF24L01P_SETUP_AW_AW_5BYTE:
                width = 5;
                break;
 
            default:
                printf( "nRF24L01P: Unknown getRxAddress width value %d\n", setupAw );
                return 0;
 
        }
 
    } else {
 
        width = 1;
 
    }
 
    int rxAddrPxRegister = NRF24L01P_REG_RX_ADDR_P0 + ( pipe - NRF24L01P_PIPE_NO_0 );
 
    int cn = (NRF24L01P_CMD_RD_REG | (rxAddrPxRegister & NRF24LO1P_REG_ADDR_BITMASK));
 
    unsigned long long address = 0;
 
    CS::low();
 
    spi->spi_write(cn);
 
    for ( int i=0; i<width; i++ ) {
 
        //
        // LSByte first
        //
        address |= ( ( (unsigned long long)( spi->spi_Receive() & 0xFF ) ) << (i*8) );
 
    }
 
    CS::high();
 
    if ( !( ( pipe == NRF24L01P_PIPE_NO_0 ) || ( pipe == NRF24L01P_PIPE_NO_1 ) ) ) {
 
        address |= ( get_rx_address(NRF24L01P_PIPE_NO_1) & ~((unsigned long long) 0xFF) );
 
    }
 
    return address;
 
}

void nRF24L01P::setTxAddress(unsigned long long address, int width) {
 
    int setupAw = get_register(NRF24L01P_REG_SETUP_AW) & ~NRF24L01P_SETUP_AW_AW_MASK;
 
    switch ( width ) {
 
        case 3:
            setupAw |= NRF24L01P_SETUP_AW_AW_3BYTE;
            break;
 
        case 4:
            setupAw |= NRF24L01P_SETUP_AW_AW_4BYTE;
            break;
 
        case 5:
            setupAw |= NRF24L01P_SETUP_AW_AW_5BYTE;
            break;
 
        default:
            printf( "nRF24L01P: Invalid setTxAddress width setting %d\n", width );
            return;
 
    }
 
    set_register(NRF24L01P_REG_SETUP_AW, setupAw);
 
    int cn = (NRF24L01P_CMD_WT_REG   | (NRF24L01P_REG_TX_ADDR & NRF24LO1P_REG_ADDR_BITMASK));
 
    CS::low();
 
    spi->spi_write(cn);
 
    while ( width-- > 0 ) {
 
        //
        // LSByte first
        //
        spi->spi_write((int) (address & 0xFF));
        address >>= 8;
 
    }
 
    CS::high();
 
}

void nRF24L01P::setRxAddress(unsigned long long address, int width, int pipe) {
 
    if ( ( pipe < NRF24L01P_PIPE_NO_0 ) || ( pipe > NRF24L01P_PIPE_NO_5 ) ) {
 
        printf( "nRF24L01P: Invalid setRxAddress pipe number %d\n", pipe );
        return;
 
    }
 
    if ( ( pipe == NRF24L01P_PIPE_NO_0 ) || ( pipe == NRF24L01P_PIPE_NO_1 ) ) {
 
        int setupAw = get_register(NRF24L01P_REG_SETUP_AW) & ~NRF24L01P_SETUP_AW_AW_MASK;
    
        switch ( width ) {
    
            case 3:
                setupAw |= NRF24L01P_SETUP_AW_AW_3BYTE;
                break;
    
            case 4:
                setupAw |= NRF24L01P_SETUP_AW_AW_4BYTE;
                break;
    
            case 5:
                setupAw |= NRF24L01P_SETUP_AW_AW_5BYTE;
                break;
    
            default:
                printf( "nRF24L01P: Invalid setRxAddress width setting %d\n", width );
                return;
    
        }
    
        set_register(NRF24L01P_REG_SETUP_AW, setupAw);
 
    } else {
    
        width = 1;
    
    }
 
    int rxAddrPxRegister = NRF24L01P_REG_RX_ADDR_P0 + ( pipe - NRF24L01P_PIPE_NO_0 );
 
    int cn = (NRF24L01P_CMD_WT_REG | (rxAddrPxRegister & NRF24LO1P_REG_ADDR_BITMASK));
 
    CS::low();
 
    spi->spi_write(cn);
 
    while ( width-- > 0 ) {
 
        //
        // LSByte first
        //
        spi->spi_write((int) (address & 0xFF));
        address >>= 8;
 
    }
 
    CS::high();;
 
    int enRxAddr = get_register(NRF24L01P_REG_EN_RXADDR);
 
    enRxAddr |= (1 << ( pipe - NRF24L01P_PIPE_NO_0) );
 
    set_register(NRF24L01P_REG_EN_RXADDR, enRxAddr);
}

void nRF24L01P::setTransferSize(int size, int pipe) {
 
    if ( ( pipe < NRF24L01P_PIPE_NO_0) || ( pipe > NRF24L01P_PIPE_NO_5 ) ) {
 
        printf( "nRF24L01P: Invalid Transfer Size pipe number %d\n", pipe );
        return;
 
    }
 
    if ( ( size < 0 ) || ( size > NRF24L01P_RX_FIFO_SIZE ) ) {
 
        printf( "nRF24L01P: Invalid Transfer Size setting %d\n", size );
        return;
 
    }
 
    int rxPwPxRegister = NRF24L01P_REG_RX_PW_P0 + ( pipe - NRF24L01P_PIPE_NO_0 );
 
    set_register(rxPwPxRegister, ( size & NRF24L01P_RX_PW_Px_MASK ) );
 
}

int nRF24L01P::readRegister(int regAddress) //arriva su 1 byte
{
    // request = regAddress; tanto questo è in | bit a bit con 0x00 ( READ ) e in & bit a bit con una MASK 0x1f
    int value;
    
    //every spi commands must starts with a high to low cs signal
    CS::low();
    
    spi->spi_write(regAddress);
    value = spi->spi_Receive();
    
    CS::high();
    
    return value;
}

void nRF24L01P::showInternal()
{   
    int result;
    
    result = this->readRegister(0);
    printf("<<CONFIG REGISTER>> è: %d\n" , result);
    
    result = this->readRegister(1);
    printf("EN_AA REGISTER è: %d\n" , result);
    
    result = this->readRegister(2);
    printf("EN_RXADDR REGISTER è: %d\n" , result);
    
    result = this->readRegister(3);
    printf("SETUP_AW REGISTER è: %d\n" , result);
    
    result = this->readRegister(4);
    printf("SETUP_RETR REGISTER è: %d\n" , result);
    
    result = this->readRegister(5);
    printf("RF_CH REGISTER è: %d\n" , result);
    
    result = this->readRegister(6);
    printf("RF_SETUP REGISTER è: %d\n" , result);
    
    result = this->readRegister(7);
    printf("<<STATUS REGISTER>> è: %d\n" , result);
    
    result = this->readRegister(8);
    printf("OBSERVE_TX REGISTER è: %d\n" , result);
    
    result = this->readRegister(9);
    printf("RPD REGISTER è: %d\n" , result);
    
    result = this->readRegister(10);
    printf("RX_ADDR_P0 REGISTER è: %d\n" , result);
    
     result = this->readRegister(11);
    printf("RX_ADDR_P1 REGISTER è: %d\n" , result);
    
     result = this->readRegister(12);
    printf("RX_ADDR_P2 REGISTER è: %d\n" , result);
    
     result = this->readRegister(13);
    printf("RX_ADDR_P3 REGISTER è: %d\n" , result);
    
     result = this->readRegister(14);
    printf("RX_ADDR_P4 REGISTER è: %d\n" , result);
    
     result = this->readRegister(15);
    printf("RX_ADDR_P5 REGISTER è: %d\n" , result);
            
     result = this->readRegister(16);
    printf("TX_ADDR REGISTER è: %d\n" , result);
    
    result = this->readRegister(17);
    printf("RX_PW_P0 REGISTER è: %d\n" , result);
    
    result = this->readRegister(18);
    printf("RX_PW_P1 REGISTER è: %d\n" , result);
    
    result = this->readRegister(19);
    printf("RX_PW_P2 REGISTER è: %d\n" , result);
    
    result = this->readRegister(20);
    printf("RX_PW_P3 REGISTER è: %d\n" , result);
   
    result = this->readRegister(21);
    printf("RX_PW_P4 REGISTER è: %d\n" , result);
    
    result = this->readRegister(22);
    printf("RX_PW_P5 REGISTER è: %d\n" , result);
    
    result = this->readRegister(23);
    printf("<<FIFO_STATUS>> REGISTER è: %d\n" , result);
    
    result = this->readRegister(29);
    printf("FEATURE REGISTER è: %d\n" , result);

}
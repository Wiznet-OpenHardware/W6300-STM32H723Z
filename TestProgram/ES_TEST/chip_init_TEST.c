
/**
 * Copyright (c) 2023 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * ----------------------------------------------------------------------------------------------------
 * Includes
 * ----------------------------------------------------------------------------------------------------
 */
#include "wizchip_conf.h"
#include "chip_init_TEST.h"

/**
 * ----------------------------------------------------------------------------------------------------
 * Macros
 * ----------------------------------------------------------------------------------------------------
 */


/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */
/**
 * ----------------------------------------------------------------------------------------------------
 *  common Functions
 * ----------------------------------------------------------------------------------------------------
 */


uint8_t ES_get_default_value(uint16_t addr){// 방법론적으로 어떻게 받아오는지 확인해보기 
    uint8_t default_Value = 0;

    return default_Value;
}

uint8_t ES_check_default(uint16_t addr ){
    uint8_t temp_data, default_Value = 0;
    temp_data = WIZCHIP_READ(addr);
    default_Value = get_default_value(addr);
    printf("[%04x]Read data = 0x%02X || Defaultdata = 0x%02X\r\n", temp_data, default_Value);
    if(temp_data == default_Value){
        printf("---->default value\r\n");
        return 0;
    }
    else{
        printf("---->default value\r\n");
        return 1;
    }
}

void Chip_lock(void){
    CHIPLOCK();
}
void Chip_unlock(void){
    CHIPUNLOCK();
}
void Chip_lock_status(void){
   getCHPLCKR();
}


/**
 * ----------------------------------------------------------------------------------------------------
 *  ES_CHIP INITAILIZE Functions
 * ----------------------------------------------------------------------------------------------------
 */




void ES_SW_Reset(void){    //Chip HW Reset
    printf("========== SW Reset ==========\r\n");
    //chip sw reset
    CHIPUNLOCK();
    setSYCR0(SYCR0_RST);
    HAL_Delay(100);
    //delay
    NETUNLOCK();
}

void ES_HW_Reset(void){    //Chip HW Reset
    printf("========== HW Reset ==========\r\n");
    //chip hw reset
    chip_hw_reset();
}

void ES_Set_Clk_25Mhz(void){    //Clock switching
    uint8_t temp_data = 0;
    printf("========== Set W6300  Operation clk 25 Mhz  ==========\r\n");
    temp_data = getSYCR1() | SYCR1_CLKSEL;

    Chip_unlock();
    HAL_Delay(10);

    setSYCR1(temp_data );
    HAL_Delay(10);

    Chip_lock();


    if( (getSYCR1() & 0x01)  == 1 ){
        printf_GREEN("\t\t\t\t->!!! Change W6300  Operation clk 25 Mhz Success!!!\r\n");
    }
    else{
        printf_RED("\t\t\t\t->!!! Change W6300  Operation clk 25 Mhz Success fail!!!,%d\r\n");
        printf("SYCR1 = 0x%02X\r\n", getSYCR1());
    }
}

void ES_Set_Clk_100Mhz(void){    //Clock switching
    uint8_t temp_data = 0;
    printf_RED("==========[!!!Need improve]Set W6300  Operation clk 100 Mhz  lock ==========\r\n");
    

    temp_data = getSYCR1() & 0xFE;


    HAL_Delay(10);

    while(getCHPLCKR()){
        Chip_unlock();
        printf("LOCK status  = 0x%02X\r\n", getCHPLCKR());
        HAL_Delay(10);
    }
    printf("temp_data = 0x%02X\r\n", temp_data);
    setSYCR1(temp_data);

    HAL_Delay(1000);
    printf("result = 0x%02X\r\n", getSYCR1());

    Chip_lock();  


    if( (getSYCR1() & 0x01) == 0 ){
        printf_GREEN("\t\t\t\t->!!! Change W6300  Operation clk 100 Mhz Success!!!\r\n");
        printf("_SYSR_ = 0x%02X\r\n", getSYCR1());

    }
    else{
        printf_RED("\t\t\t\t ->!!! Change W6300  Operation clk 100 Mhz Success fail!!! %d \r\n");
        printf("_SYSR_ = 0x%02X\r\n", getSYCR1());


    }
}   

/**
 * ----------------------------------------------------------------------------------------------------
 *  ES Host Interface Functions
 * ----------------------------------------------------------------------------------------------------
 */

void ES_common_register_0x0000_read(void){  //Common Reg ID 0x0000~ 0x0001 
    uint16_t temp_data = 0;
    temp_data = getCIDR();
    printf("CIDR = 0x%04X\r\n", temp_data);
}


void ES_common_register_0x0002_read(void){  //Common Reg VER 0x0002~ 0x0003 
    uint16_t temp_data = 0;
    temp_data = getVER();
    printf("VER = 0x%04X\r\n", temp_data);
}

void ES_common_register_0x2000_read(void){  //Common Reg RTL 0x2000
    uint8_t temp_data = 0;
    temp_data = getSYSR();
    printf("SYCR1 = 0x%02X\r\n", temp_data);
}


void ES_SOCKET_buffer_write(uint8_t offset , uint16_t addr, uint8_t *wizdata, uint8_t len ){
    uint32_t addrsel =  offset;
    WIZCHIP_WRITE_BUF(addrsel, wizdata, len);
}


void ES_SOCKET_buffer_read(uint8_t offset , uint16_t addr, uint8_t *wizdata, uint16_t len ){

    uint32_t addrsel = offset ;
    WIZCHIP_READ_BUF(addrsel, wizdata, len);
    for (uint8_t i = 0; i <= len; i++)
    {
        printf("0x%02X ", wizdata[i]);
        if( i % 16 == 15)
        {
            printf("\r\n");
        }
    }
    printf("\r\n");
}

uint8_t ES_SOCKET_buffer_write_read (uint8_t offset , uint8_t *wizdata, uint16_t len){

    char str[3][3] = {"RX", "TX"}; 
    int socketTX_RX = offset % 2;
  


    wiz_delay_ms(5);

    uint32_t addrsel =  offset;
    uint8_t readData[len]    ;

    WIZCHIP_WRITE_BUF(addrsel,wizdata, len);
    
    wiz_delay_ms(10);

    WIZCHIP_READ_BUF(addrsel,readData, len); 
    wiz_delay_ms(10);

#if 1//for debug Message
    for (uint8_t i = 0; i < len; i++)
    {
        printf("0x%02X ", wizdata[i]);
        if( i % 16 == 15)
        {
            printf("\r\n");
        }
    }
    printf("\r\n");
#endif 


#if 1//for debug Message
    for (uint8_t i = 0; i < len; i++)
    {
        printf("0x%02X ", readData[i]);
        if( i % 16 == 15)
        {
            printf("\r\n");
        }
    }
    printf("\r\n");
#endif 

    int result  = memcmp(wizdata, readData, len);

    if(result != 0)
    {
        printf_RED("\t\t\t\t->!!! Buffer Write Read TEST fail!!!\r\n");
    }
    else
    {
        printf_GREEN("\t\t\t\t->!!! Buffer Write Read TEST OK!!!\r\n");
    }
    return result;
}




/**
 * ----------------------------------------------------------------------------------------------------
 *  ES phy Interface Functions
 * ----------------------------------------------------------------------------------------------------
 */


uint8_t phy_mode_check(uint8_t temp) 
{

    temp = ( temp >> 3 ) & 0x07;
    switch (temp)
    {
    case PHYMODE_100_FDX:
        printf("PHY MODE = 100_FULL DX");
        break;
    case PHYMODE_100_HDX:
        printf("PHY MODE = 100 Half DX");
        break;
    case PHYMODE_10_FDX:
        printf("PHY MODE = 10 FULL DX");
        break; 
    case PHYMODE_10_HDX:    
        printf("PHY MODE = 10 Half DX");
        break;
    default:
        printf("PHY MODE = PHYMODE_AUTO[%d] " , temp);
        break;
    }
    printf("\r\n") ; 
    return temp;
}

void ES_LINK_STATUS(void){   // Link check
    printf("========== PHY LINK STATUS ==========  \r\n");
    uint8_t temp = 0;
    temp = getPHYSR();
    if(temp & 0x01){
        printf("Link up\r\n");
    }
    else{
        printf("Link down\r\n");
    }
}

uint8_t ES_GET_PHY_MODE(void){  //get Fixed Mode
 printf("========== PHY get PHY MODE  ========== \r\n");
    uint8_t temp = 0;
    printf("PHY MODE =  \r\n");
    phy_mode_check(getPHYSR());

    return temp;
}


uint8_t ES_SET_PHY_MODE(uint8_t data){   //set Fixed Mode 
    printf("========== PHY SET PHY MODE  ========== \r\n");
    printf("before PHY MODE = ");
    uint8_t phy_mode =  phy_mode_check(getPHYSR());
    setPHYCR0(data);    
    printf("after PHY MODE = ");
    phy_mode =  phy_mode_check(getPHYSR());
    if (phy_mode == data)
    {
        printf_GREEN("PHY MODE SET OK\r\n");
    }
    else
    {
        printf_RED("PHY MODE SET FAIL\r\n");
    }
    //TODO: how to verify phy mode???
    return 0 ;
}

void ES_PHY_HW_RESET(void){   //Phy HW Reset - TODO Define 
    printf_RED("==========PHY reset-- TODO Define==========  \r\n");
    // HAL_GPIO_WritePin(RSTn_GPIO_Port, RSTn_Pin, GPIO_PIN_RESET);
    // HAL_Delay(500);
    // HAL_GPIO_WritePin(RSTn_GPIO_Port, RSTn_Pin, GPIO_PIN_SET);
    // HAL_Delay(500);
    printf("PHY reset complete!\r\n");
}
void ES_PHY_SW_RESET(void){   //Phy SW Reset 
    printf("========== PHY SW reset ==========\r\n");
    setPHYCR1(1);
    printf("PHY reset complete!\r\n");
}

void ES_GET_PHY_POWER_DOWN(void){   //Phy Power Down
    printf("========== PHY GET Power Down ==========\r\n");
    uint8_t powerDownMode = (getPHYCR1() & 0x20) >> 5;
    if(powerDownMode == 1){
        printf("PHY Power Down Mode == 25 Mhz\r\n");
    }
    else{
        printf("PHY Normal Mode  == 100 Mhz\r\n");
    }

}

void ES_SET_PHY_POWER_DOWN(uint8_t data){   //Phy Power Down
    uint8_t tmp =  getPHYCR1() ;
    printf("========== PHY SET Power Down ==========\r\n");

    if( tmp & 0x20 ){
        printf("Power Down Mode  ---> ");
    }
    else{
        printf("PHY Normal Mode  ---> ");
    }

    tmp = (tmp & ~0x20) |( data << 5);
    setPHYCR1( tmp ); 

    tmp =  getPHYCR1() ;
    if(tmp & 0x20){
        printf("Power Down Mode \r\n");
    }
    else{
        printf("PHY Normal Mode \r\n ");
    }


}

/**
 * ----------------------------------------------------------------------------------------------------
 *  ES Network TEST Functions
 * ----------------------------------------------------------------------------------------------------
 */


void Ping(void){
    uint8_t pingDestIP[4]= {192 , 168 , 11 , 2};
    setPINGIDR(0x6300); // Ping ID
    setPINGSEQR(getPINGSEQR() + 1); // Ping SEQR +1 
    while ((getSLCR()  & 0x20) == 1 )   //Wait for ping ready
    {
         wiz_delay_ms(100);
    }
    setSLCR( (getSLCR() | 0x20) ) ;     //Send ping cmd
}



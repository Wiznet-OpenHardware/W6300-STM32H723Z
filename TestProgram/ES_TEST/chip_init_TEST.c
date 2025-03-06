
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
#include "wizchip_conf.h"
#include "socket.h"
/**
 * ----------------------------------------------------------------------------------------------------
 * Macros
 * ----------------------------------------------------------------------------------------------------
 */

	#define DATA_BUF_SIZE			2048

/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */
static uint8_t sock_state[8] = {0,};
static uint16_t any_port = 	50000;
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

uint8_t writeDummyData(void){
    uint8_t set_ip_table[4] ={192,168,11,55};
    uint8_t ip_table_result[4] ={0,};

    setSIPR(set_ip_table);  
    HAL_Delay(10);
    getSIPR(ip_table_result);
    
    int result  = memcmp(set_ip_table, ip_table_result, 4);
    return result ;
}

uint8_t checkDefualtData(void){
    uint8_t ipTableDefault[4] ={0,0,0,0};
    uint8_t ipTableResult[4] ={0,};
    getSIPR(ipTableResult);
    int result  = memcmp(ipTableDefault, ipTableResult, 4);
    return result ; 
}


/**
 * ----------------------------------------------------------------------------------------------------
 *  ES_CHIP INITAILIZE Functions
 * ----------------------------------------------------------------------------------------------------
 */




void ES_SW_Reset(void){    //Chip HW Reset
    PRINT_TEST_NAME();

    //chip sw reset
    CHIPUNLOCK();
    NETUNLOCK();
    
    writeDummyData() ; 

    HAL_Delay(10);
    setSYCR0(SYCR0_RST);
    HAL_Delay(100);

    //delay
    uint8_t result = checkDefualtData(); 

    PRINT_RESULT(result);
}

void ES_HW_Reset(void){    //Chip HW Reset
   PRINT_TEST_NAME();
    //chip hw reset

    CHIPUNLOCK();
    NETUNLOCK();
    writeDummyData() ; 

    HAL_Delay(10);
    chip_hw_reset();
    HAL_Delay(100);

    uint8_t result = checkDefualtData(); 

    PRINT_RESULT(result);

}

void ES_Set_Clk_25Mhz(void){    //Clock switching
    PRINT_TEST_NAME();
    uint8_t result ;
    uint8_t temp_data = 0;
    temp_data = getSYCR1() | SYSCLK_25MHZ;

    Chip_unlock();

    HAL_Delay(10);
    setSYCR1(temp_data );
    HAL_Delay(10);

    if((getSYCR1() & 0x01)  == SYSCLK_25MHZ ){
        result = SUCCESS; 
    }else{
        result = FAIL; 
    }

    PRINT_RESULT(result) ;
}

void ES_Set_Clk_100Mhz(void){    //Clock switching
    PRINT_TEST_NAME();
    uint8_t result ;
    uint8_t temp_data = 0;
    printf_RED("==========[!!!Need improve]Set W6300  Operation clk 100 Mhz  lock ==========\r\n");
    
    temp_data = getSYCR1() | SYSCLK_100MHZ;

    while(getCHPLCKR()){
        Chip_unlock();
        printf("LOCK status  = 0x%02X\r\n", getCHPLCKR());
        HAL_Delay(10);
    }
    // printf("temp_data = 0x%02X\r\n", temp_data);
    HAL_Delay(10); 
    setSYCR1(temp_data);
    HAL_Delay(10); 

    // printf("result = 0x%02X\r\n", getSYCR1());

    if( (getSYCR1() & 0x01) == SYSCLK_100MHZ ){
        //printf("_SYSR_ = 0x%02X\r\n", getSYCR1());
        result = SUCCESS ; 
    }
    else{
        //printf("_SYSR_ = 0x%02X\r\n", getSYCR1());
        result = FAIL ; 
    }

    PRINT_RESULT(result) ;
}   

/**
 * ----------------------------------------------------------------------------------------------------
 *  ES Host Interface Functions
 * ----------------------------------------------------------------------------------------------------
 */


uint8_t register_read_compare(uint16_t addr , uint16_t value ){  //Common Reg ID 0x0000~ 0x0001 
 
    uint8_t result ;
    uint16_t read_data = 0;
    
    const uint16_t default_value = value; 
    read_data = WIZCHIP_READ((_W6300_IO_BASE_ + (addr << 8) + WIZCHIP_CREG_BLOCK));
        
    if (default_value == read_data )
    {
        result = 0;
        printf("\t\tRead_value[0x%04X] = 0x%04X\r\n", addr,  read_data );
    }
    else
    {
        result = 1;
        printf("\t\t\033[0;34m addr[0x%04X] = 0x%04X\033[0m\r\n", addr,  read_data );
    }
    return result;
}




void ES_TEST_common_register_read(void){  //Common Reg ID 0x0000~ 0x0001 
    PRINT_TEST_NAME();
    uint8_t result ;
    uint16_t temp_data = 0;

    temp_data +=   register_read_compare(0x0000 , 0x61) ;
    temp_data +=   register_read_compare(0x0001 , 0x00) ;
    temp_data +=   register_read_compare(0x0002 , 0x46) ;
    temp_data +=   register_read_compare(0x0003 , 0x61) ;
    temp_data +=   register_read_compare(0x0004 , 0x11) ;
    temp_data +=   register_read_compare(0x2000 , 0x01) ;

    if(temp_data == 0 )
    {
        result = SUCCESS ; 
    }
    else
    {
        result = FAIL ; 
    }
    PRINT_RESULT(result); 
}





uint8_t ES_TEST_BUFFER_TEST_write_read (uint8_t offset , uint8_t *wizdata, uint32_t len){
    #define ES_SOCKET_buffer_write_read_DEBUG 0
    uint8_t result ;
    char str[3][3] = {"RX", "TX"}; 
    int socketTX_RX = offset % 2;

    wiz_delay_ms(5);

    uint32_t addrsel =  offset;
    const uint32_t lenValue = len ; 
    uint8_t readData[lenValue] ;

    WIZCHIP_WRITE_BUF(addrsel,wizdata, lenValue);
    HAL_Delay(10);
    WIZCHIP_READ_BUF(addrsel,readData, lenValue); 
    HAL_Delay(10);

    result = memcmp(wizdata, readData, lenValue);

#if ES_SOCKET_buffer_write_read_DEBUG //for debug Message
    if (result != 0){
        for (uint32_t i = 0; i < len; i++)
        {
            printf("0x%02X ", wizdata[i]);
            if( i % 16 == 15)
            {
                printf("\r\n");
            }
        }
        printf("\r\n");
        for (uint32_t i = 0; i < len; i++)
        {
            printf("0x%02X ", readData[i]);
            if( i % 16 == 15)
            {
                printf("\r\n");
            }
        }
        printf("\r\n");
    }
    PRINT_RESULT_noWhile(result);
#endif 
    
    return result;
}


uint8_t ES_TEST_BUFFER_TEST(uint32_t len_max){
    PRINT_TEST_NAME(); 
    uint8_t result ; 
    
    //#define len_max 16
    const uint32_t const_len_max  = len_max ; 

    uint8_t buffer[const_len_max];

    /* TX TEST */
    #define socket_nums 8
    printf("\t\t->TX_buffer TEST start<-\r\n");
    for(int i = 0; i < socket_nums; i++)
    {
        printf("\r\n");
        for(int len =1; len <= const_len_max; len=len*2)
        {   
            if (len > 1024)   
            {   
                printf("\t\tTX_SOCKET[%d]- %d KByte Read/Write\r", i , len /1024);
            }   
            else  
            {
                printf("\t\tTX_SOCKET[%d]- %d Byte  Read/Write\r", i , len );
            }
            memset(buffer, (len >> 8) + (len & 0xff) , len);
            result = ES_TEST_BUFFER_TEST_write_read( WIZCHIP_TXBUF_BLOCK(i), buffer, len);
            if(result == FAIL){
                printf("\r\n") ; 
                PRINT_RESULT(result); 
               // return result ; 
            }
            HAL_Delay (10);
        }
    }
    printf("\r\n") ; 

    /* RX TEST */
    printf("\t\t->RX_buffer TEST start<-\r\n");
    for(int i = 0; i < socket_nums; i++)
    {
        printf("\r\n");
        for(int len =1; len <= const_len_max; len=len*2)
        {
            if (len > 1024)
            {
                printf("\t\tRX_SOCKET[%d]- %d KByte Read/Write\r", i , len /1024);
            }
            else
            {
                printf("\t\tRX_SOCKET[%d]- %d Byte  Read/Write\r", i , len );
            }
            memset(buffer, (len >> 8) + (len & 0xff) , len);
            result = ES_TEST_BUFFER_TEST_write_read( WIZCHIP_RXBUF_BLOCK(i), buffer, len);
            if(result == FAIL){
                printf("\r\n") ; 
                PRINT_RESULT(result); 
               // return result ; 
            }
            HAL_Delay (10);
        }
    }
    printf("\r\n") ; 
    PRINT_RESULT(SUCCESS) ;
    return  result ; 
}




/**
 * ----------------------------------------------------------------------------------------------------
 *  ES phy Interface Functions
 * ----------------------------------------------------------------------------------------------------
 */

void ES_LINK_STATUS(void){   // Link check
    PRINT_TEST_NAME();
    uint8_t result ; 
    uint8_t cnt = 0 ; 
        
    while (1){
        uint8_t  link_status = getPHYSR() & 0x01;
        if(link_status == PHY_LINK_OFF)
        {
            if (cnt < 20  )
            {
                HAL_Delay(100) ;
                continue;
            }
            else
            {
                printf("\t\tLink down\r\n");
                result = FAIL; 
                break;
            }
        }
        else
        {
            printf("\t\tLink up\r\n");
            result = SUCCESS ; 
            break;
        }
    }
    
     PRINT_RESULT(result) ; 
}


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

uint8_t ES_GET_PHY_MODE(void){  //get Fixed Mode
    PRINT_TEST_NAME();
    uint8_t result ; 
    uint8_t temp = 0;
    printf("\t\t" );
    phy_mode_check(getPHYSR());

    return temp;
}


void ES_SET_PHY_MODE(uint8_t data){   //set Fixed Mode 
    PRINT_TEST_NAME();
    uint8_t result ; 

    printf("\t\tbefore ");
    uint8_t phy_mode =  phy_mode_check(getPHYSR());
    setPHYCR0(data);    
    
    printf("\t\tafter ");
    phy_mode =  phy_mode_check(getPHYSR());

    if (phy_mode == data)
       result = SUCCESS; 
    else
       result = FAIL; 

    PRINT_RESULT(result);
}


void ES_PHY_SW_RESET_TEST3(void){   //Phy SW Reset 
    PRINT_TEST_NAME();
    uint8_t result ; 
    printf_RED("TODO: need improve....\r\n");
    printf_RED("data was not changed\r\n");

    printf("\t\tbefore ");
    phy_mode_check(getPHYSR());
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    ES_PHY_MDIO_READ_TEST(0x0001) ;
    setPHYCR0(PHYMODE_10_FDX);     
    printf("\t\tafter ");
    phy_mode_check(getPHYSR());
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    ES_PHY_MDIO_READ_TEST(0x0001) ;
    // // setPHYCR0(PHYMODE_AUTO);    
    // printf("\t\tafter ");
    // phy_mode_check(getPHYSR());
    // ES_PHY_MDIO_READ_TEST(0x0000) ;
    // ES_PHY_MDIO_READ_TEST(0x0001) ;

    while(getCHPLCKR()){
        Chip_unlock();
        printf("LOCK status  = 0x%02X\r\n", getCHPLCKR());
        HAL_Delay(10);
    }

    //  setPHYCR1(getPHYCR1() | 0x01);
    //wiz_mdio_write(PHYRAR_BMCR, wiz_mdio_read(PHYRAR_BMCR) | BMCR_RST);
    
    wiz_mdio_write(PHYRAR_BMCR, wiz_mdio_read(PHYRAR_BMCR) | BMCR_RST);
    wiz_mdio_write(PHYRAR_BMCR, wiz_mdio_read(PHYRAR_BMCR) | BMCR_SPD );
    printf("\t\t----set  BCMR_SPD \r\n");
    HAL_Delay(1000);
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    wiz_mdio_write(PHYRAR_BMCR, wiz_mdio_read(PHYRAR_BMCR) & ~BMCR_RST);
    HAL_Delay(1000);
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    HAL_Delay(1000);
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    HAL_Delay(1000);
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    HAL_Delay(1000);
    HAL_Delay(1000);
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    HAL_Delay(1000);
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    HAL_Delay(1000);
 
 
    wiz_mdio_write(PHYRAR_BMCR, wiz_mdio_read(PHYRAR_BMCR) | BMCR_RST);
    wiz_mdio_write(PHYRAR_BMCR, wiz_mdio_read(PHYRAR_BMCR) & ~BMCR_SPD );
    printf("\t\t----clear  BCMR_SPD \r\n");
    HAL_Delay(1000);
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    wiz_mdio_write(PHYRAR_BMCR, wiz_mdio_read(PHYRAR_BMCR) & ~BMCR_RST);
    printf("\t\t----clear  BCMR_SPD \r\n");
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    HAL_Delay(1000);
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    HAL_Delay(1000);
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    HAL_Delay(1000);
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    HAL_Delay(1000);
    wiz_mdio_write(PHYRAR_BMCR, wiz_mdio_read(PHYRAR_BMCR) | BMCR_RST);
    HAL_Delay(1000);
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    HAL_Delay(1000);
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    HAL_Delay(1000);
    // ES_HW_Reset();

    wiz_mdio_write(PHYRAR_BMCR, wiz_mdio_read(PHYRAR_BMCR) | BMCR_RST);
    wiz_mdio_write(PHYRAR_BMCR, wiz_mdio_read(PHYRAR_BMCR) | BMCR_SPD );
    printf("\t\t----set  BCMR_SPD \r\n");
    HAL_Delay(1000);
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    wiz_mdio_write(PHYRAR_BMCR, wiz_mdio_read(PHYRAR_BMCR) & ~BMCR_RST);
    HAL_Delay(1000);
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    HAL_Delay(1000);
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    HAL_Delay(1000);
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    HAL_Delay(1000);
    HAL_Delay(1000);
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    HAL_Delay(1000);
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    HAL_Delay(1000);
 



    ES_HW_Reset();
    printf("HW-RESET\r\n");
    HAL_Delay(200);   
     HAL_Delay(200);

    ES_PHY_MDIO_READ_TEST(0x0000) ;

        HAL_Delay(200);

    ES_PHY_MDIO_READ_TEST(0x0000) ;
        HAL_Delay(200);

    ES_PHY_MDIO_READ_TEST(0x0000) ;
    // }


    uint8_t phy_mode =  phy_mode_check(getPHYSR());
    
    if (phy_mode == PHYMODE_AUTO)
        result = SUCCESS;
    else
        result = FAIL;
    
    // PRINT_RESULT(result);
    PRINT_RESULT(SUCCESS);
}
void ES_PHY_SW_RESET_TEST(void){   //Phy SW Reset 
    PRINT_TEST_NAME();
    uint8_t result ; 
    printf_RED("TODO: need improve....\r\n");
    printf_RED("data was not changed\r\n");
    printf("\t\twait Phy link up \r\n");

//case 1 
    ES_HW_Reset(); 
    HAL_Delay(1000);

    printf("set setPHYCR0 = PHYMODE_10_FDX\r\n"); 
    while(getCHPLCKR()){
        Chip_unlock();
        printf("LOCK status  = 0x%02X\r\n", getCHPLCKR());
        HAL_Delay(10);
    }
    printf( "before  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    HAL_Delay(1000);
    setPHYCR0(PHYMODE_10_FDX);     
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    
    printf("MDIO[0x0000] |= 0x80  \r\n");
    wiz_mdio_write(PHYRAR_BMCR, wiz_mdio_read(PHYRAR_BMCR) | BMCR_RST);
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));


//case 1-2
    ES_HW_Reset(); 
    HAL_Delay(1000);

    printf("set setPHYCR0 = PHYMODE_10_FDX\r\n"); 
    while(getCHPLCKR()){
        Chip_unlock();
        printf("LOCK status  = 0x%02X\r\n", getCHPLCKR());
        HAL_Delay(10);
    }
    printf( "before  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    HAL_Delay(1000);
    // setPHYCR0(PHYMODE_10_FDX);     
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    
    printf("MDIO[0x0000] |= 0x80  \r\n");
    wiz_mdio_write(PHYRAR_BMCR, wiz_mdio_read(PHYRAR_BMCR) | BMCR_RST);
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));




     
//case 2 
    ES_HW_Reset(); 
    HAL_Delay(1000);

    printf("set setPHYCR0 = PHYMODE_10_FDX"); 
    while(getCHPLCKR()){
        Chip_unlock();
        printf("LOCK status  = 0x%02X\r\n", getCHPLCKR());
        HAL_Delay(10);
    }
    printf( "before  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    HAL_Delay(1000);
    setPHYCR0(PHYMODE_10_FDX);     

    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));

    printf("MDIO[0x0000] |= 0x80  \r\n");
    //wiz_mdio_write(PHYRAR_BMCR, wiz_mdio_read(PHYRAR_BMCR) | BMCR_RST);
    setPHYCR1(getPHYCR1() | 0x01);
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);   
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));

    setPHYCR1(getPHYCR1() | 0x01);
    printf("setPHYCR1(getPHYCR1() | 0x01)\r\n") ;
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));



     
//case 3
    ES_HW_Reset(); 
    HAL_Delay(1000);

    printf("set setPHYCR0 = PHYMODE_10_FDX"); 
    while(getCHPLCKR()){
        Chip_unlock();
        printf("LOCK status  = 0x%02X\r\n", getCHPLCKR());
        HAL_Delay(10);
    }
    printf( "before  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    HAL_Delay(1000);
    // setPHYCR0(PHYMODE_10_FDX);     
    wiz_mdio_write(PHYRAR_BMCR, wiz_mdio_read(PHYRAR_BMCR) & ~(uint16_t)BMCR_SPD);

    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));

    printf("MDIO[0x0000] |= 0x8000  \r\n");
    wiz_mdio_write(PHYRAR_BMCR, wiz_mdio_read(PHYRAR_BMCR) | BMCR_RST);
    // setPHYCR1(getPHYCR1() | 0x01);
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);   
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));

    setPHYCR1(getPHYCR1() | 0x01);
    printf("setPHYCR1(getPHYCR1() | 0x01)\r\n") ;
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));
    HAL_Delay(1000);
    printf("wait 1 sec \r\n");
    printf( "after  value = 0x%04x \r\n",wiz_mdio_read(PHYRAR_BMCR));



    PRINT_RESULT(SUCCESS);
}
void ES_PHY_SW_RESET_TEST2(void){   //Phy SW Reset 
    PRINT_TEST_NAME();
    uint8_t result ; 
    printf_RED("TODO: need improve....\r\n");
    printf_RED("data was not changed\r\n");

    printf("\t\tbefore ");
    phy_mode_check(getPHYSR());
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    ES_PHY_MDIO_READ_TEST(0x0001) ;
    setPHYCR0(PHYMODE_10_FDX);     
    printf("\t\tafter ");
    phy_mode_check(getPHYSR());
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    ES_PHY_MDIO_READ_TEST(0x0001) ;
    // // setPHYCR0(PHYMODE_AUTO);    
    // printf("\t\tafter ");
    // phy_mode_check(getPHYSR());
    // ES_PHY_MDIO_READ_TEST(0x0000) ;
    // ES_PHY_MDIO_READ_TEST(0x0001) ;

    while(getCHPLCKR()){
        Chip_unlock();
        printf("LOCK status  = 0x%02X\r\n", getCHPLCKR());
        HAL_Delay(10);
    }

     setPHYCR1(getPHYCR1() | 0x01);
    //wiz_mdio_write(PHYRAR_BMCR, wiz_mdio_read(PHYRAR_BMCR) | BMCR_RST);
    // wiz_mdio_write(PHYRAR_BMCR, wiz_mdio_read(PHYRAR_BMCR) | );

    printf("\t\t----reset  \r\n");

    ES_PHY_MDIO_READ_TEST(0x0000) ;
    HAL_Delay(1000);
    printf("\t\t----reset_delay\r\n");
    ES_PHY_MDIO_READ_TEST(0x0000) ;

    HAL_Delay(1000);
    // printf("SYSR1 = 0x%02X\r\n", getSYSR());
    // while((getPHYSR() & 0x01)==0 ){
    //     HAL_Delay(1000);
        ES_PHY_MDIO_READ_TEST(0x0000) ;
    HAL_Delay(1000);
        ES_PHY_MDIO_READ_TEST(0x0000) ;
    HAL_Delay(1000);
        ES_PHY_MDIO_READ_TEST(0x0000) ;
        // printf("PHYSR = 0x%02X\r\n", getPHYSR() );
        ES_PHY_MDIO_READ_TEST(0x0000) ;
    setPHYCR0(PHYMODE_100_FDX);  
    HAL_Delay(1000);
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    HAL_Delay(1000);
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    HAL_Delay(1000);
    HAL_Delay(1000);
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    HAL_Delay(1000);
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    HAL_Delay(1000);
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    HAL_Delay(1000);
    ES_PHY_MDIO_READ_TEST(0x0000) ;
     setPHYCR1(getPHYCR1() | 0x01);
    //   wiz_mdio_write(PHYRAR_BMCR, wiz_mdio_read(PHYRAR_BMCR) | BMCR_RST);
    HAL_Delay(1000);
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    HAL_Delay(1000);
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    HAL_Delay(1000);
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    HAL_Delay(1000);
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    HAL_Delay(1000);
    ES_PHY_MDIO_READ_TEST(0x0000) ;
    HAL_Delay(1000);
    ES_PHY_MDIO_READ_TEST(0x0000) ;

    // }

    ES_SW_Reset();
    // ES_HW_Reset();
    printf("SW-RESET\r\n");
    HAL_Delay(200);

    ES_PHY_MDIO_READ_TEST(0x0000) ;

    HAL_Delay(200);

    ES_PHY_MDIO_READ_TEST(0x0000) ;
    HAL_Delay(200);

    ES_PHY_MDIO_READ_TEST(0x0000) ;
    // }

    ES_HW_Reset();
    printf("HW-RESET\r\n");
    HAL_Delay(200);   
     HAL_Delay(200);

    ES_PHY_MDIO_READ_TEST(0x0000) ;

        HAL_Delay(200);

    ES_PHY_MDIO_READ_TEST(0x0000) ;
        HAL_Delay(200);

    ES_PHY_MDIO_READ_TEST(0x0000) ;
    // }


    uint8_t phy_mode =  phy_mode_check(getPHYSR());
    
    if (phy_mode == PHYMODE_AUTO)
        result = SUCCESS;
    else
        result = FAIL;
    
    // PRINT_RESULT(result);
    PRINT_RESULT(SUCCESS);
}


void ES_PHY_SW_RESET(void){   //Phy HW Reset - TODO Define 
    PRINT_TEST_NAME();
    uint8_t result ; 
    uint32_t cnt = 0; 
    printf_RED("TODO: will be improve Funtion... \r\n");

    while((getPHYSR() & 0x01) == PHY_LINK_OFF){
        HAL_Delay(100);
    }
    
    chip_hw_reset();

    while((getPHYSR() & 0x01) == PHY_LINK_OFF){
        HAL_Delay(50);
        //printf("link off\r\n");          
        cnt ++;
    }

    if (result>0){
        result = SUCCESS;
    }
    PRINT_RESULT(result);
}


void ES_PHY_HW_RESET(void){   //Phy HW Reset - TODO Define 
    PRINT_TEST_NAME();
    uint8_t result ; 
    uint32_t cnt = 0; 
    printf_RED("TODO: will be improve Funtion... \r\n");

    while((getPHYSR() & 0x01) == PHY_LINK_OFF){
        HAL_Delay(100);
    }
    
    chip_hw_reset();
    uint16_t value_0000 =  ES_PHY_MDIO_READ_TEST(0x0000);
    uint16_t value_0001 =  ES_PHY_MDIO_READ_TEST(0x0001);

    while((getPHYSR() & 0x01) == PHY_LINK_OFF){
        HAL_Delay(1000);
        cnt ++;
        if (cnt > 5){
            printf_RED("PHY Link Off, Please Check the RJ45\r\n");
            PRINT_RESULT(FAIL);
            break;
        }
    }

    if (value_0000 == 0x3100 && value_0001 == 0x7809){
        result = SUCCESS; 
    }
    else{
        result = FAIL; 
    }
    PRINT_RESULT(result);
}

void ES_SET_PHY_POWER_DOWN(uint8_t data){   //Phy Power Down
    PRINT_TEST_NAME();
    uint8_t result ; 

    uint8_t currunt_power_mode =  getPHYCR1() & 0x20  ;
 
    if( currunt_power_mode ){
        printf("\t\t Power Down Mode  ---> ");
    }
    else{
        printf("\t\t PHY Normal Mode  ---> ");
    }

    uint8_t power_mode = ( getPHYCR1() & ~0x20) | ( data << 5);
    setPHYCR1( power_mode ); 
    HAL_Delay(200); 
    power_mode =  (getPHYCR1() & 0x20) >> 5  ;



    if(power_mode){
        printf("Power Down Mode \r\n");
    }
    else{
        printf("PHY Normal Mode \r\n");
    }
    
    
    if( data == power_mode )
        result =SUCCESS ; 
    else 
        result = FAIL;
    PRINT_RESULT(result);
}

void ES_GET_PHY_POWER_DOWN(void){   //Phy Power Down
   PRINT_TEST_NAME();

    uint8_t powerDownMode = (getPHYCR1() & 0x20) >> 5;
    if(powerDownMode == 1){
        printf("\t\tPHY Power Down Mode == 25 Mhz\r\n");
    }
    else{
        printf("\t\tPHY Normal Mode  == 100 Mhz\r\n");
}

}


uint16_t ES_PHY_MDIO_READ_TEST(uint16_t addr ){
    setPHYRAR(addr);
    setPHYACR(0x02); // set phy control Register
    while( getPHYACR() != 0 ){
      HAL_Delay(50);      
    }
    uint16_t test = getPHYDOR(); 
    printf ("\t\tRead_value[0x%04x]= 0x%04x \r\n",addr , test) ; 

    if(addr == 0x16){
        if (test != 0x4706){
            printf("\t\t!!\033[0;31m PHY register [0x0016]Value is 0x4706 != [%02x]!! \033[0m\r\n", test );
        }
    }
    return test ;
}

void ES_PHY_CMD_READ_TEST(uint16_t addr){
    PRINT_TEST_NAME();

    setPHYRAR(addr);
    setPHYACR(0x02); // set phy control Register
    while( getPHYACR() != 0 ){
      HAL_Delay(50);      
    }
    uint16_t test = getPHYDOR(); 
    printf ("MDIO_VALUE = %04x \r\n", test) ; 


}


/**
 * ----------------------------------------------------------------------------------------------------
 *  ES Network TEST Functions
 * ----------------------------------------------------------------------------------------------------
 */


uint8_t Ping(uint8_t *dest_ip){
    uint8_t result ; 

    uint8_t pingDestIP[4]= {192 , 168 , 11 , 2};
    setPINGIDR(0x6300); // Ping ID
    setSLDIPR(dest_ip); //Set destnation IP
    setPINGSEQR(getPINGSEQR() + 1); // Ping SEQR +1 
    while ((getSLIR()  & 0x80) == 1 )   //Wait for ping ready
    {
        printf_RED ("\t\t\t\t alraedy PING_timeout_error \r\n") ; 
        setSLIRCLR(0x80);
        wiz_delay_ms(100);
    }

    while ((getSLCR()  & 0x20) == 1 )   //Wait for ping ready
    {
         wiz_delay_ms(100);
    }


    setSLCR( (getSLCR() | 0x20) ) ;     //Send ping cmd

    uint8_t timeout_cnt = 0  ;
    uint16_t delay_time_ms  = 200 ;
    uint16_t timeout_ms  =  3000 ;

    while (1){
        HAL_Delay(delay_time_ms);
         timeout_cnt++; 

        if(timeout_ms < (timeout_cnt * delay_time_ms ) ){
            result = FAIL;
            break;
        }
        if(getSLIR() & 0x20){ // did not Recieve reply
            setSLIRCLR(0x20);
            result = SUCCESS; 

            break;
        }else if(getSLIR() & 0x80) { // Time out error 
            setSLIRCLR(0x80);
            result = FAIL;
            printf_RED ("\t\t\t\t PING_timeout_error \r\n") ; 
            break;
        }
    }

    return result ; 
}



void set_phy_loopback_mode_MDIO (void){
    PRINT_TEST_NAME();

    printf ("\t\t-getPHYCR1= %04x \r\n" ,  getPHYCR1());
    printf("\t\t-wiz_mdio_read(0x0000)=%04x \r\n" ,wiz_mdio_read(0x0000));
    wiz_mdio_write(0x0000, 0x6100);

    printf("\t\t------------after--------------------\r\n");

    printf ("\t\t-getPHYCR1= %04x \r\n" ,  getPHYCR1());
    printf("\t\t -wiz_mdio_read(0x0000)=%d \r\n" ,wiz_mdio_read(0x0000));

}


void set_phy_loopback_mode_CMD (void){
    PRINT_TEST_NAME();
    
    printf ("\t\t-getPHYCR1= %04x \r\n" ,  getPHYCR1());
    printf("\t\t -wiz_mdio_read(0x0000)=%04x \r\n" ,wiz_mdio_read(0x0000));
    setPHYCR1(getPHYCR1() | 0x10);
    // wiz_mdio_write(0x0000, wiz_mdio_read(0x0000)  & ~0x4000);
    HAL_Delay(200);

    printf("\t\t------------after--------------------\r\n");
    printf ("\t\t-getPHYCR1= %04x \r\n" ,  getPHYCR1());
    printf("\t\t -wiz_mdio_read(0x0000)=%04x \r\n" ,wiz_mdio_read(0x0000));
    setPHYCR1(getPHYCR1() | 0x01);
    
    HAL_Delay(200);
    printf("\t\t------------after--------------------\r\n");
    printf ("\t\t-getPHYCR1= %04x \r\n" ,  getPHYCR1());
    printf("\t\t -wiz_mdio_read(0x0000)=%04x \r\n" ,wiz_mdio_read(0x0000));
}

int32_t ES_LOOPBACK_TEST_CLIENT(uint8_t sn, uint8_t* buf, uint8_t* destip, uint16_t destport)
{
    check_loopback_mode_W6x00();
    uint8_t  loopback_mode = AS_IPV4; 
    int32_t ret; // return value for SOCK_ERRORs
    uint16_t sentsize=0;
    uint8_t status,inter,addr_len;
    uint16_t received_size;
    uint8_t tmp = 0;
    uint8_t arg_tmp8;
    wiz_IPAddress destinfo;
    uint8_t* TEST_buf = "hello!__I_am_W6300_Thank_you$0d$0a";
    static uint8_t send_state = 0 ;
    static uint8_t send_cnt = 0 ;

#if 1
	// 20231018 taylor
	uint8_t sn_status;
#endif

    // Socket Status Transitions
    // Check the W6100 Socket n status register (Sn_SR, The 'Sn_SR' controlled by Sn_CR command or Packet send/recv status)
    getsockopt(sn,SO_STATUS,&status);
    switch(status)
    {
    case SOCK_ESTABLISHED :
        ctlsocket(sn,CS_GET_INTERRUPT,&inter);
        if(inter & Sn_IR_CON)	// Socket n interrupt register mask; TCP CON interrupt = connection with peer is successful
        {
            arg_tmp8 = Sn_IR_CON;
            ctlsocket(sn,CS_CLR_INTERRUPT,&arg_tmp8);// this interrupt should be write the bit cleared to '1'
        }
        //////////////////////////////////////////////////////////////////////////////////////////////
        // Data Transaction Parts; Handle the [data receive and send] process
        //////////////////////////////////////////////////////////////////////////////////////////////
        getsockopt(sn, SO_RECVBUF, &received_size);
        if(received_size == 0 && send_state == 0) // Sn_RX_RSR: Socket n Received Size Register, Receiving data length
        {
            ret = send(sn, TEST_buf, strlen(TEST_buf) - 10);
            if(ret < 0) // Send Error occurred (sent data length < 0)
            {
                printf("close_%d \r\n",ret);
                close(sn); // socket close
                return ret;
            }
            send_state = 1; 
            HAL_Delay(100);
        }
        else if(received_size != 0 ) // Sn_RX_RSR: Socket n Received Size Register, Receiving data length
        {
            printf("\t\t[%02d] loopback recieve\r\n", send_cnt);
            
            if(received_size > DATA_BUF_SIZE) received_size = DATA_BUF_SIZE; // DATA_BUF_SIZE means user defined buffer size (array)
            ret = recv(sn, buf, received_size); // Data Receive process (H/W Rx socket buffer -> User's buffer)

            if(ret <= 0) return ret; // If the received data length <= 0, receive failed and process end
            received_size = (uint16_t) ret;
            // Data sentsize control
        }else if(received_size == 0 && send_state == 1 ) {
            if (memcmp(TEST_buf ,buf , strlen(TEST_buf) - 10) == 0){
                printf("\t\t[%02d] loopback OK\r\n", send_cnt);
                send_state = 0; 
                send_cnt++;
            }else{
                printf("send data = %s \r\n ",TEST_buf) ;  
                printf("send data = %s \r\n ",buf) ;  
                printf("\t\t[%02d] memcmp errror \r\n", send_cnt);
                send_state = 0; 
            }
            if (send_cnt >=10 ){
                return -999;
            }
            HAL_Delay(100);
        }
        //////////////////////////////////////////////////////////////////////////////////////////////
        break;

    case SOCK_CLOSE_WAIT :
        #ifdef _LOOPBACK_DEBUG_
            printf("%d:CloseWait\r\n",sn);
        #endif
        getsockopt(sn, SO_RECVBUF, &received_size);

        if((received_size = getSn_RX_RSR(sn)) > 0) // Sn_RX_RSR: Socket n Received Size Register, Receiving data length
        {
            if(received_size > DATA_BUF_SIZE) received_size = DATA_BUF_SIZE; // DATA_BUF_SIZE means user defined buffer size (array)
            ret = recv(sn, buf, received_size); // Data Receive process (H/W Rx socket buffer -> User's buffer)

            if(ret <= 0) return ret; // If the received data length <= 0, receive failed and process end
            received_size = (uint16_t) ret;
            sentsize = 0;
#if 1
            // 20231018 taylor
            #ifdef _LOOPBACK_DEBUG_
			getsockopt(sn,SO_EXTSTATUS, &sn_status);
			if(sn_status & TCPSOCK_MODE)
			{
				printf("Socket %d Received %d bytes from %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x port %d : \r\n",
											sn, received_size,
											destip[0], destip[1], destip[2], destip[3],
											destip[4], destip[5], destip[6], destip[7],
											destip[8], destip[9], destip[10], destip[11],
											destip[12], destip[13], destip[14], destip[15],
											destport);
			}
			else
			{
				printf("Socket %d Received %d bytes from %d.%d.%d.%d port %d : \r\n", sn, received_size, destip[0], destip[1], destip[2], destip[3], destport);
			}

			int i;
			for(i=0; i<received_size; i++)
			{
				printf("%c", buf[i]);
			}
			printf("\r\n");
            #endif
#endif

            // Data sentsize control
            while(received_size != sentsize)
            {
                ret = send(sn, buf+sentsize, received_size-sentsize); // Data send process (User's buffer -> Destination through H/W Tx socket buffer)
                if(ret < 0) // Send Error occurred (sent data length < 0)
                {
                    close(sn); // socket close
                    return ret;
                }
                sentsize += ret; // Don't care SOCKERR_BUSY, because it is zero.
            }
#if 1
            // 20231018 taylor
            #ifdef _LOOPBACK_DEBUG_
			if(sn_status & TCPSOCK_MODE)
			{
				printf("Socket %d Sent back %d bytes from %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x port %d : \r\n",
											sn, sentsize,
											destip[0], destip[1], destip[2], destip[3],
											destip[4], destip[5], destip[6], destip[7],
											destip[8], destip[9], destip[10], destip[11],
											destip[12], destip[13], destip[14], destip[15],
											destport);
			}
			else
			{
				printf("Socket %d Sent back %d bytes from %d.%d.%d.%d port %d : \r\n", sn, sentsize, destip[0], destip[1], destip[2], destip[3], destport);
			}

			int j;
			for(j=0; j<sentsize; j++)
			{
				printf("%c", buf[j]);
			}
			printf("\r\n");
            #endif
#endif
        }
        if((ret=disconnect(sn)) != SOCK_OK) return ret;
            #ifdef _LOOPBACK_DEBUG_
                printf("%d:Socket Closed\r\n", sn);
            #endif
        break;

    case SOCK_INIT :
        #ifdef _LOOPBACK_DEBUG_
            if(loopback_mode == AS_IPV4)
                printf("%d:Try to connect to the %d.%d.%d.%d, %d\r\n", sn, destip[0], destip[1], destip[2], destip[3], destport);
            else if(loopback_mode == AS_IPV6)
            {
                printf("%d:Try to connect to the %04X:%04X", sn, ((uint16_t)destip[0] << 8) | ((uint16_t)destip[1]),
                    ((uint16_t)destip[2] << 8) | ((uint16_t)destip[3]));
                printf(":%04X:%04X", ((uint16_t)destip[4] << 8) | ((uint16_t)destip[5]),
                    ((uint16_t)destip[6] << 8) | ((uint16_t)destip[7]));
                printf(":%04X:%04X", ((uint16_t)destip[8] << 8) | ((uint16_t)destip[9]),
                    ((uint16_t)destip[10] << 8) | ((uint16_t)destip[11]));
                printf(":%04X:%04X,", ((uint16_t)destip[12] << 8) | ((uint16_t)destip[13]),
                    ((uint16_t)destip[14] << 8) | ((uint16_t)destip[15]));
                printf("%d\r\n", destport);
            }
        #endif

        if(loopback_mode == AS_IPV4)
          ret = connect(sn, destip, destport, 4); /* Try to connect to TCP server(Socket, DestIP, DestPort) */
        else if(loopback_mode == AS_IPV6)
          ret = connect(sn, destip, destport, 16); /* Try to connect to TCP server(Socket, DestIP, DestPort) */

        printf("SOCK Status: %d\r\n", ret);

        if( ret != SOCK_OK) return ret;	//	Try to TCP connect to the TCP server (destination)
        break;

    case SOCK_CLOSED:
        switch(loopback_mode)
        {
        case AS_IPV4:
            tmp = socket(sn, Sn_MR_TCP4, any_port++, SOCK_IO_NONBLOCK);
            break;
        case AS_IPV6:
            tmp = socket(sn, Sn_MR_TCP6, any_port++, SOCK_IO_NONBLOCK);
            break;
        case AS_IPDUAL:
            tmp = socket(sn, Sn_MR_TCPD, any_port++, SOCK_IO_NONBLOCK);
            break;
        default:
            break;
        }

        if(tmp != sn){    /* reinitialize the socket */
            #ifdef _LOOPBACK_DEBUG_
                printf("%d : Fail to create socket.\r\n",sn);
            #endif
            return SOCKERR_SOCKNUM;
        }
        printf("%d:Socket opened[%d]\r\n",sn, getSn_SR(sn));
        sock_state[sn] = 1;

        break;
    default:
        break;
    }
    return 1;
}

/**
 * ----------------------------------------------------------------------------------------------------
 *  TEST Functions
 * ----------------------------------------------------------------------------------------------------
 */

void ES_PING_TEST(uint8_t *dest_ip, uint8_t cnt) {
    PRINT_TEST_NAME();
    uint8_t result = 0; 

 
    uint8_t success_times = 0 ; 
    for(uint8_t i = 0 ; i < cnt ; i ++ ){
           if ( Ping(dest_ip) == SUCCESS)
           {
            success_times ++ ;
           }
    }
    if ( cnt ==  success_times)
    {
        result = SUCCESS ; 
    }
    else
    {
        result = FAIL ; 
    }
    printf ("\t\tSuccessTimes / excute Times = %d  /  %d \r\n" ,success_times , cnt );
    PRINT_RESULT(result);
}




void ESTEST(void) {
    printf_GREEN("\r\n===================================\r\n");
    printf_GREEN("==========ES_TEST_start============\r\n");
    printf_GREEN("===================================\r\n");

    printf("\r\n===================================\r\n");
    printf("========Chip Initial start==========\r\n");
    printf("===================================\r\n\n");

  

    ES_SW_Reset();
    ES_HW_Reset();
    HAL_Delay(20);
    // ES_Set_Clk_25Mhz();
    // ES_Set_Clk_100Mhz();

    printf("\r\n===================================\r\n");
    printf("=======Host Interface start======\r\n");
    printf("===================================\r\n\n");



    ES_TEST_common_register_read() ;

    #if 1
        ES_TEST_BUFFER_TEST(0x7FFF - 1);
    #endif 
    printf("\r\n===================================\r\n");
    printf("========= Internel Phy  start======\r\n");
    printf("===================================\r\n\n");

    /* power down Mode and Reset TEST*/
    
    ES_LINK_STATUS();
    ES_GET_PHY_MODE(); 

    ES_PHY_MDIO_READ_TEST(0x0000);
    ES_PHY_MDIO_READ_TEST(0x0001);
    ES_PHY_MDIO_READ_TEST(0x0016);

    //ES_SET_PHY_MODE(PHYMODE_10_FDX);
    ES_SET_PHY_MODE(PHYMODE_100_HDX);
    ES_SET_PHY_POWER_DOWN(PHY_POWER_DOWN);
    ES_SET_PHY_POWER_DOWN(PHY_POWER_NORM);

    // ES_PHY_MDIO_READ_TEST(0x0000);
    // ES_PHY_MDIO_READ_TEST(0x0001);
    // ES_PHY_MDIO_READ_TEST(0x0016);

    /*TODO: need improve->data was not changed*/
    ES_PHY_HW_RESET();
    ES_PHY_SW_RESET(); //TODO :이거 값의 변화가 없는데, MDIO로 값을 읽어와서 실제로 그러한지 확인해보기.

    ES_GET_PHY_MODE(); 
    ES_GET_PHY_POWER_DOWN(); 

    // set_phy_loopback_mode_MDIO();
    // set_phy_loopback_mode_CMD();


    // ES_PHY_MDIO_READ_TEST(0x0000);
    // ES_PHY_MDIO_READ_TEST(0x0001);
    // ES_PHY_MDIO_READ_TEST(0x0016);
/////////////////////////



}

void ES_NET_PING_TEST( uint8_t *dest_ip){
    printf("\r\n===================================\r\n");
    printf(    "========= Network TEST start=======\r\n");
    printf(    "===================================\r\n\n");


    ES_PING_TEST(dest_ip,10);
} 

#define ETHERNET_BUF_MAX_SIZE_ES (1024 * 32)
static uint8_t g_udp_buf_main[ETHERNET_BUF_MAX_SIZE_ES * 2 ] = {
    0,
};


void ES_NET_LOOPBACK_TEST( uint8_t *dest_ip){

    PRINT_TEST_NAME();
    uint8_t result; 
    int retval = 0 ;
    
    while (1)
    {
        if ((retval = ES_LOOPBACK_TEST_CLIENT(0, g_udp_buf_main, dest_ip, 5010)) < 0)
        {
            if (retval == -999){
                result = SUCCESS;
                break; 
            }else{
                printf(" loopback_udps error : %d\n", retval);
                result = FAIL;
                break;

            }   
            // while (1)
            //     ;
        }
    }
    PRINT_RESULT(result);
    printf_GREEN("\r\n===================================\r\n");
    printf_GREEN(    "=========== ES TEST finish=========\r\n");
    printf_GREEN(    "===================================\r\n\n");
}
#include "W6300_TestProcess.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "loopback.h"
#include "wizchip_conf.h"
#include "socket.h"
//#include "wiznetlogo.h"

#define record_cnt  10000
uint8_t inner_reg[record_cnt]={0,};
uint16_t reg_time[record_cnt]={0,};
uint16_t recv_size_record[record_cnt]={0,};
uint32_t start_record_time=0;
uint16_t record_index = 0;
//0:RO, 1:WO, 2:RW, 3:RW-AC
//64
//                                    0,     1,      2,      3,      4,      5,      6,      7,      8,      9,     10,     11,     12,     13,     14,     15,     16,     17,     18,     19,     20,     21,     22,     23,     24,     25,     26,     27,     28,     29,     30,     31,     32,     33,     34,     35,     36,     37,     38,     39,     40,     41,     42,     43,     44,     45,     46,     47,     48,     49,     50,     51,     52,     53,     54,     55,     56,     57,     58,     59,     60,     61,     62,     63,     64
const uint8_t Common_reg_name[65][7]={"CIDR",  "VER", "SYSR", "SYCR","TCNTR"};
const uint16_t Common_reg_offset[65]={0x0000, 0x0002, 0x2000, 0x2004, 0x2005, 0x2016, 0x2020, 0x2100, 0x2101, 0x2104, 0x2108, 0x2114, 0x2124, 0x2128, 0x212c, 0x2130, 0x3000, 0x3008, 0x300c, 0x3010, 0x3014, 0x3018, 0x301c, 0x301d, 0x4000, 0x4004, 0x4008, 0x4009, 0x4100, 0x4104, 0x4108, 0x4110, 0x4114, 0x4120, 0x4130, 0x4134, 0x4138, 0x4140, 0x4150, 0x4160, 0x4170, 0x4180, 0x418c, 0x4190, 0x4198, 0x419c, 0x41a0, 0x41a4, 0x41b0, 0x41c0, 0x41c5, 0x41d0, 0x41d4, 0x41d8, 0x41dc, 0x41e0, 0x41f0, 0x41f4, 0x41f5, 0x41f6, 0x4200, 0x4204, 0x4208, 0x420c, 0x420f};
const uint8_t Common_reg_rw[65]={          0,      0,      0,      1,      2,      0,      1,      0,      0,      2,      1,      2,      2,      1,      2,      3,      0,      2,      2,      0,      3,      2,      0,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      0,      2,      2,      0,      0,      0,      0,      2,      0,      0,      0,      0,      0,      2,      1,      1,      1,      2,      2,      2,      2,      2};
const uint8_t Common_reg_len[65]={         2,      2,      1,      1,      1,      2,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      2,      2,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      6,      2,      2,      6,      4,      4,      4,     16,     16,     16,     16,     16,      4,      6,      2,      2,      4,      2,     16,      2,      2,      1,      1,      4,      4,     16,      1,      1,      1,      1,      2,      1,      2,      1,      1};
const uint8_t Common_reg_mask[65]={        0,      0,      0,      0,   0x81,      0,      0,      0,      0,   0x97,      0,      0,      0,      0,   0x03,      0,      0,   0x1F,      0,      0,      0,   0x01,      0,   0x29,   0x0F,   0x0F,   0x37,   0x81,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,   0x1F,      0,      0,      0,      0,      0,      0,      0,      0};
//uint8_t Common_reg_MAX = 65;
//                                   0,      1,      2,      3,      4,      5,      6,      7,      8,      9,     10,     11,     12,     13,     14,     15,     16,     17,     18,     19,     20,     21,     22,     23,     24,     25,     26,     27,     28,     29,     30,     31,     32,     33,     34,     35,     36,     37,     38,     39,     40,     41,     42,     43,     44,     45,     46,     47,     48,     49,     50,     51,     52,     53,     54,     55,     56,     57,     58,     59,     60,     61,     62,     63,     64
const uint16_t Socket_reg_offset[30]={0x0000, 0x0004, 0x0010, 0x0020, 0x0024, 0x0028, 0x0030, 0x0031, 0x0100, 0x0104, 0x0108, 0x010c, 0x0110, 0x0114, 0x0118, 0x0120, 0x0130, 0x0140, 0x0144, 0x0180, 0x0184, 0x0188, 0x0200, 0x0204, 0x0208, 0x020c, 0x0220, 0x0224, 0x0228,  0x022c};
const uint8_t Socket_reg_rw[30] = {        2,      2,      3,      0,      2,      1,      0,      0,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      0,      0,      2,      2,      0,      2,       0};
const uint8_t Socket_reg_len[30] = {       1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      2,      2,      2,      6,      4,     16,      2,      1,      2,      1,      1,      1,      2,      2,      2,      1,      2,      2,       2};
const uint8_t Socket_reg_mask[30] = {      0,   0x03,      0,   0x1F,   0x1F,   0x1F,      0,   0x07,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,   0x03,      0,      0,      0,   0x1F,      0,      0,      0,   0x1F,      0,      0,       0};
const uint8_t Socket_reg_saf[30] = {       0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      1,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,       0};
//uint8_t Socket_reg_MAX = 30;

//op_code = 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
//           Mod  | WR|    sock   | sel
//mode 00: single 01: dual 10:quad
//sel  00: common 01:socket 10:TX 11:RX

void print_help_menu(void)
{
  printf("CMD List\r\n");
  printf("help : show CMD list\r\n");
  printf("recmd : show input key on/off\r\n"); // recmd
  printf("       ex)>recmd 1<LF> -> [0: ON, 1:OFF] default : 0\r\n");
  printf("checkclk : QSPI clock display\r\n");
  printf("setclk <CLK> : QSPI clock setting Mhz\r\n");
  printf("       ex)>setclk 40<CR><LF> -> 40Mhz setting\r\n");
  printf("sett : format,dummy cycle,Instruction Data,Addr Data\r\n");
  printf("       [format=quad, dummy cycle 2, instruction AB, Addr = 1234 ]\r\n");
  printf("       >sett q,2,AB,1234<LF>  \r\n");
  printf("send : send string data \r\n");
  printf("hexs : send hex data \r\n");
  printf("recv : recv count number \r\n");
  printf("reset: FPGA reset \r\n");
  printf("creg <mode>, <burst><LF> : common resister test \r\n");
  printf("       ex)>creg s, 1<LF> -> [s:single spi mode d:dual q:quad][0: byte, 1:burst]\r\n");
  printf("sreg <mode>, <SOK NUM><LF>: socket resister test \r\n");
  printf("       ex)>sreg s, 0<LF> -> [s:single spi mode d:daul q:quad][0: socket number 0~7]\r\n");
  printf("stxmd <mod>, <socket>, <burst>, <test size>: socket tx memory dump test \r\n");
  printf("       ex)>stxmd s, 0, 1, 512<LF> -> [s:single spi mode d:daul q:quad][0: socket number 0~7][0:byte, 1:burst][byte size]\r\n");
  printf("stxmt <mod>, <socket>, <burst>, <test size>: socket tx memory time test \r\n");
  printf("       ex)>stxmt s, 0, 1, 512<LF> -> [s:single spi mode d:daul q:quad][0: socket number 0~7][0:byte, 1:burst][byte size]\r\n");
  printf("stxm <mod>, <socket>, <burst>, <test size>: socket tx memory test \r\n");
  printf("       ex)>stxm s, 0, 1, 512<LF> -> [s:single spi mode d:daul q:quad][0: socket number 0~7][0:byte, 1:burst][byte size]\r\n");
  printf("echos <mod>, <socketnum>, <port><LF>: loopback server test \r\n <mod == 5> test loop back <mod == 6> udp loopback\r\n");
  printf("       ex)>echos s, 1, 5000<LF> -> [s:single spi mode d:daul q:quad][0: socket number 0~3][port number]\r\n");
  printf("sclos : socket close\r\n");
  printf("ping4 <ip><LF> : ping \r\n");
  printf("       ex)>ping4 192.168.15.2<LF>\r\n");
  printf("smset <soket mem 0~7><LF> : socket memory setting test \r\n");
  printf("       ex)>smset 2, 2, 2, 2, 2, 2, 2, 2<LF>\r\n");
  printf("iperfclos<LF>: iperf server exit \r\n");
  printf("iperfs <mod>, <socketnum>, <port><LF>: iperf server test \r\n");
  printf("       ex)>iperfs s, 1, 5000<LF> -> [s:single spi mode d:daul q:quad][0: socket number 0~7][port number]\r\n");
  printf("iperfc <mod>, <socketnum>, <IP>, <PORT>, <size>, <count><LF>: iperf Client test \r\n");
  printf("       ex)>iperfc s, 1, 192.168.15.2, 5000, 2048, 10<LF> -> [s:single spi mode d:daul q:quad][0: socket number 0~7][IP address][port number][send data size][send count]\r\n");
  printf("tcpccon <mod>, <socketnum>, <IP>, <PORT>, <size>, <count><LF>: TCP Client test \r\n");
  printf("       ex)>tcpccon s, 1, 192.168.15.2, 5000, 2048, 10<LF> -> [s:single spi mode d:daul q:quad][0: socket number 0~7][IP address][port number][send data size][send count]\r\n");
  printf("reg loop test <mod>, <sel><LF>: TCP Client test \r\n");
  printf("       ex)>reg loop test s, <LF> -> [s:single spi mode d:daul q:quad][1: MAC, 2: GW, 3: SN, 4: IP]\r\n");
  printf("chip reset test<LF> : ping \r\n");
  printf("       ex)>chip reset test<LF>\r\n");
  printf("get netinfo<LF> : ping \r\n");
  printf("       ex)>get netinfo<LF>\r\n");
  printf("get_ver <mod><LF>: chip version print <mod> = [s:single spi mode d:daul q:quad]  \r\n");
  printf("       ex)>get_ver s<LF> -> single mode version read \r\n");
  printf("smemb 0, 0, 1024 //socket TX/RX buf size  socket mem test\r\n");
}
char Hex2Char(char const* szHex, unsigned char *rch)
{
    if(*szHex >= '0' && *szHex <= '9')
            *rch = *szHex - '0';
    else if(*szHex >= 'A' && *szHex <= 'F')
            *rch = *szHex - 55; //-'A' + 10
    else if(*szHex >= 'a' && *szHex <= 'f')
            *rch = *szHex - 87; //-'a' + 10
    else
        //Is not really a Hex string
        return 1;
    szHex++;
    if(*szHex >= '0' && *szHex <= '9')
        *rch = (*rch << 4) | *szHex - '0';
    else if(*szHex >= 'A' && *szHex <= 'F')
        *rch = (*rch << 4) | *szHex - 55; //-'A' + 10;
    else if(*szHex >= 'a' && *szHex <= 'f')
        *rch = (*rch << 4) | *szHex - 87; //-'a' + 10
    else
        //Is not really a Hex string
        return 2;
    return 0;
}

char qspi_set_parse(char *r_data, QSPI_Set_Data *init_data)
{
    int seq = 0;
    int temp = 0;
    char ret = 0;
    unsigned char tempHex = 0;
    char *ptr = strtok(r_data, ",");
    while(ptr != NULL)
    {
        switch(seq)
        {
            case 0: //spi format mode
                seq++;
                printf("spi mode : %c \r\n", *ptr);
                if((*ptr=='s')||(*ptr=='d')||(*ptr=='q'))
                {
                    init_data->mode = *ptr;
                }
                else
                {
                    printf("parse spi mode error \r\n");
                    return 1;
                }
                break;
            case 1: //dummy cycle
                seq++;
                temp = atoi((const char *)ptr);
                printf("dumy cycle : %s, %d\r\n", ptr, temp);
                if((temp < 0)||(temp > 31))
                {
                    printf("dummy cycle value error \r\n");
                    return 2;
                }
                init_data->dummy = temp;
                break;
            case 2: //Instruction data
                seq++;
                temp = strlen(ptr);
                //printf("instruction data : %s[%d]\r\n", ptr, temp);
                ret = Hex2Char(ptr, &tempHex);
                if(ret != 0)
                {
                    printf("instruction data value error %02X \r\n", tempHex);
                    return 3;
                }
                printf("instruction data : %02X \r\n", tempHex);
                init_data->instruction = tempHex;
                break;
            case 3: //Address data
                seq++;
                temp = strlen(ptr);
                //printf("Address data : %s[%d]\r\n", ptr, temp);
                ret = Hex2Char(ptr, &tempHex);
                if(ret != 0)
                {
                    printf("address data value error %02X \r\n", tempHex);
                    return 3;
                }
                temp = (int)tempHex;
                ret = Hex2Char(ptr+2, &tempHex);
                if(ret != 0)
                {
                    printf("address data value error %02X \r\n", tempHex);
                    return 3;
                }
                temp = temp << 8 | tempHex;
                printf("address data : %04X \r\n", temp);
                init_data->addr = temp;
                break;
        }
        ptr = strtok(NULL, ",");
    }
    return 0;
}


uint8_t reg_WR_return_data(uint8_t op_code, uint16_t reg_addr, uint16_t len, uint8_t *tx, uint8_t *rx)
{
    uint8_t ret=0;
	w6300_qspi_write_buf(op_code, reg_addr, tx, len);
	ret = w6300_qspi_read_buf(op_code, reg_addr, rx, len);
	return ret;
}
uint8_t reg_WR_buf_S_Test(uint8_t op_code, uint16_t reg_addr, uint16_t len)
{
	uint8_t *temp_tx_buf;
	uint8_t *temp_rx_buf;
    uint16_t j = 0;
    uint16_t i = 0, k = 0;
    uint8_t ret = 0;
    uint8_t t_ret = 0;

    temp_tx_buf = (uint8_t *)calloc(len + 1, sizeof(uint8_t));
    temp_rx_buf = (uint8_t *)calloc(len + 1, sizeof(uint8_t));
    printf("\r\n  NUM ");
    for(i=0; i<len; i++)
    {
    	printf("%04X ", reg_addr + i);
    }
    printf(" RET \r\n");
    for(i=0;i<=0xff;i++)
    {
        memset(temp_tx_buf, i, sizeof(uint8_t)*len );
        memset(temp_rx_buf, 0x00, sizeof(uint8_t)*len );
        if(((i+1)%10)==0)
            printf("%03d ...\r\n",i);
    
        for(j=0; j<(len/2); j++)
        {
            *(temp_tx_buf + j * 2) = 0xAE;
        }
        
        memset(temp_rx_buf, 0x00, sizeof(uint8_t)*len );
        ret = 0;
        reg_WR_return_data(op_code, reg_addr, len, temp_tx_buf, temp_rx_buf);
        
        ret = memcmp(temp_tx_buf, temp_rx_buf, sizeof(uint8_t)*len);
        if(ret != 0)
        {
            printf(" 0x%02X ",i);
            for(j=0; j<len; j++)
            {
                printf("  %02X ",temp_rx_buf[j]);
            }
            printf(" Fail\r\n");
            t_ret |= 1;
        }
        
    }
    free(temp_tx_buf);
    free(temp_rx_buf);
	return t_ret;
}

uint8_t reg_WR_buf_Test(uint8_t op_code, uint16_t reg_addr, uint16_t len, uint8_t mode)
{
	uint8_t *temp_tx_buf;
	uint8_t *temp_rx_buf;
    uint16_t j = 0;
    uint16_t i = 0, k = 0;
    uint8_t ret = 0;
    uint8_t t_ret = 0;

    temp_tx_buf = (uint8_t *)calloc(len + 1, sizeof(uint8_t));
    temp_rx_buf = (uint8_t *)calloc(len + 1, sizeof(uint8_t));
    printf("\r\n  NUM ");
    for(i=0; i<len; i++)
    {
    	printf("%04X ", reg_addr + i);
    }
    printf(" RET \r\n");
    for(i=0;i<=0xff;i++)
    {
    	memset(temp_tx_buf, i, sizeof(uint8_t)*len );
    	//memset(temp_rx_buf, 0x00, sizeof(uint8_t)*len );
    	if(((i+1)%10)==0)
            printf("%03d ...\r\n",i);
    	#if 0
        for(j=0; j<(len/2); j++)
        {
            *(temp_tx_buf + j * 2) = i;
        }
        #endif
        for(k=0; k<=0x0ff; k++)
        {
            for(j=0; j<(len/2); j++)
            {
                *(temp_tx_buf + j * 2  + 1) = k;
            }
            memset(temp_rx_buf, 0x00, sizeof(uint8_t)*len );
            ret = 0;
            if(mode < 3)
            {
                reg_WR_return_data(op_code, reg_addr, len, temp_tx_buf, temp_rx_buf);
            }
            else
            {
                bus_Write_buf(op_code, reg_addr, temp_tx_buf, len);
                bus_Read_buf(op_code, reg_addr, temp_rx_buf, len);
            }
            
            ret = memcmp(temp_tx_buf, temp_rx_buf, sizeof(uint8_t)*len);
            if(ret != 0)
            {
                printf(" 0x%02X ",i);
                for(j=0; j<len; j++)
                {
                    printf("  %02X ",temp_rx_buf[j]);
                }
                printf(" Fail\r\n");
                t_ret |= 1;
            }
        }
    }
    free(temp_tx_buf);
    free(temp_rx_buf);
	return t_ret;
}
uint8_t reg_WR_buf_T_Test(uint8_t op_code, uint16_t reg_addr, uint16_t len)
{
	uint8_t *temp_tx_buf;
	uint8_t *temp_rx_buf;
    uint8_t ret = 0, temp_ret = 0;
    uint32_t pre_time = 0, now_time = 0;
    char ret_msg[2][5]={"Pass", "Fail"};

    temp_tx_buf = (uint8_t *)calloc(len + 1, sizeof(uint8_t));
    temp_rx_buf = (uint8_t *)calloc(len + 1, sizeof(uint8_t));
	memset(temp_tx_buf, 0xAE, sizeof(uint8_t)*len );
	memset(temp_rx_buf, 0x00, sizeof(uint8_t)*len );
    //pre_time = get_time();
    pre_time = get_us_time();
	temp_ret = reg_WR_return_data(op_code, reg_addr, len, temp_tx_buf, temp_rx_buf);
    //now_time = get_time();
    now_time = get_us_time();
    ret = memcmp(temp_tx_buf, temp_rx_buf, sizeof(uint8_t)*len);
    printf("socket mem W/R Burst Time : %12dus, result: %s spimode:%02X\r\n", now_time - pre_time, ret_msg[ret], temp_ret);
    free(temp_tx_buf);
    free(temp_rx_buf);
	return ret;
}

uint8_t reg_WR_byte_Test(uint8_t op_code, uint16_t reg_addr, uint16_t len, uint8_t mode)
{
	uint8_t temp_rx_buf=0;
    uint8_t temp_save[16]={0,0};
    uint16_t j = 0;
    uint16_t i = 0;
    uint8_t ret = 0;
    uint8_t t_ret = 0;

    printf("\r\n  NUM  ");
    for(i=0; i<len; i++)
    {
    	printf("%04X ", reg_addr + i);
    }
    printf(" RET \r\n");
    for(i=0;i<=0xff;i++)
    {
        ret = 0;
    	//reg_WR_return_data(op_code, reg_addr, len, temp_tx_buf, temp_rx_buf);
    	//w6300_qspi_write_buf(op_code, reg_addr, temp_tx_buf, len);
    	//w6300_qspi_read_buf(op_code, reg_addr, temp_rx_buf, len);
    	printf(" 0x%02X ",i);
    	for(j=0; j<len; j++)
    	{
    	    if(mode < 3)
            {   
                w6300_qspi_write_buf(op_code, reg_addr + j, &i, 1);
                w6300_qspi_read_buf(op_code, reg_addr + j, &temp_rx_buf, 1);
            }
            else
            {
                bus_Write_buf(op_code, reg_addr + j, &i, 1);
                bus_Read_buf(op_code, reg_addr + j, &temp_rx_buf, 1);
            }
            temp_save[j] = temp_rx_buf;
            if(temp_rx_buf != i)
                ret = 1;
    	}
        if(ret == 0)
        {
            printf(" Pass\r\n");
        }
        else
        {
            for(j=0; j<len; j++)
                printf("  %02X ",temp_save[j]);
            printf(" Fail\r\n");
            t_ret |= 1;
        }
    }
	return t_ret;
}
uint8_t reg_WR_byte_T_Test(uint8_t op_code, uint16_t reg_addr, uint16_t len)
{
	uint8_t temp_rx_buf=0;
    uint16_t j = 0;
    uint8_t i = 0xAE;
    uint8_t ret = 0, temp_ret = 0;
    uint32_t pre_time = 0, now_time = 0;
    char ret_msg[2][5]={"Pass", "Fail"};
    pre_time = get_time();
	for(j=0; j<len; j++)
	{
    	w6300_qspi_write_buf(op_code, reg_addr + j, &i, 1);
    	temp_ret = w6300_qspi_read_buf(op_code, reg_addr + j, &temp_rx_buf, 1);
		//printf("  %02X ",temp_rx_buf);
        if(temp_rx_buf != i)
            ret = 1;
	}
    now_time = get_time();
    printf("socket mem W/R Byte Time : %8dms, result: %s spimode : %02X\r\n", now_time - pre_time, ret_msg[ret], temp_ret);
	return ret;
}

uint8_t reg_RO_buf_Test(uint8_t op_code, uint16_t reg_addr, uint8_t len, uint8_t mode)
{
	uint8_t temp_rx_buf[16]={0,};
    uint16_t i = 0;

    printf("\r\n");
    for(i=0; i<len; i++)
    {
    	printf("%04X ", reg_addr + i);
    }
    printf("\r\n");
    //w6300_qspi_write_buf(op_code, reg_addr, temp_tx_buf, len);
    if(mode < 3)    //SPI MODe
    {
        w6300_qspi_read_buf(op_code, reg_addr, temp_rx_buf, len);
    }
    else
    {
        bus_Read_buf(op_code, reg_addr, temp_rx_buf, len);
    }
    for(i=0; i<len; i++)
    {
    	printf("  %02X ",temp_rx_buf[i]);
    }
	return 0;
}
void bus_Write_buf(uint8_t op_code, uint16_t reg_addr, uint8_t *data, uint16_t len)
{
    TransUInt temp_addr;
    uint16_t i;
    uint8_t *pbuf = data;
    temp_addr.Data16 = reg_addr;
    *(__IO uint8_t *)((uint32_t)(0x60000000)) = (uint8_t)(temp_addr.Data8[1]);
    *(__IO uint8_t *)((uint32_t)(0x60000001)) = (uint8_t)(temp_addr.Data8[0]);
    *(__IO uint8_t *)((uint32_t)(0x60000002)) = (uint8_t)(op_code);
    for( i=0; i<len; i++)
    {
        *(__IO uint8_t *)((uint32_t)(0x60000003)) = *pbuf++;
        #if 0
        printf("%02x ", *pbuf);
        if(((i+1)%16)==0)
        {
            printf("\r\n");
        }
        #endif
    }
}
void bus_Read_buf(uint8_t op_code, uint16_t reg_addr, uint8_t *data, uint16_t len)
{
    TransUInt temp_addr;
    uint16_t i;
    uint8_t *pbuf = data;
    temp_addr.Data16 = reg_addr;
    *(__IO uint8_t *)((uint32_t)(0x60000000)) = (uint8_t)(temp_addr.Data8[1]);
    *(__IO uint8_t *)((uint32_t)(0x60000001)) = (uint8_t)(temp_addr.Data8[0]);
    *(__IO uint8_t *)((uint32_t)(0x60000002)) = (uint8_t)(op_code);
    for( i=0; i<len; i++)
    {
        *pbuf++ = *(__IO uint8_t *)((uint32_t)(0x60000003));
        #if 0
        printf("%02x ", *(pbuf-1));
        if(((i+1)%16)==0)
        {
            printf("\r\n");
        }
        #endif
    }
}
uint8_t reg_WR_buf_Bus_Test(uint8_t op_code, uint16_t reg_addr, uint16_t len)
{
    uint8_t *temp_tx_buf;
	uint8_t *temp_rx_buf;
    uint8_t *temp_in_tx_buf;
    uint8_t ret = 0;
    uint16_t cnt = 0;
    uint32_t pre_time = 0, now_time = 0;
    uint16_t total_time = 0;
    char ret_msg[2][5]={"Pass", "Fail"};

    temp_tx_buf = (uint8_t *)calloc(len + 1, sizeof(uint8_t));
    temp_rx_buf = (uint8_t *)calloc(len + 1, sizeof(uint8_t));
	memset(temp_tx_buf, 0xAE, sizeof(uint8_t)*(len));
	memset(temp_rx_buf, 0x00, sizeof(uint8_t)*len );
    temp_in_tx_buf = temp_tx_buf;
    for(cnt = 0; cnt<(len/2);cnt++)
    {
        temp_in_tx_buf++;
        *temp_in_tx_buf++ = 0x00;
    }
    temp_in_tx_buf = temp_tx_buf;
    #if 0 //debug print
    printf("tx data print\r\n");
    for(cnt = 0; cnt<len;cnt++)
    {
        printf("%02x ", *temp_in_tx_buf++);
        if(((cnt+1)%16)==0)
        {
            printf("\r\n");
        }
    }
    printf("\r\n");
    #endif
    pre_time = get_us_time();
    printf("\r\n");
    bus_Write_buf(op_code, reg_addr, temp_tx_buf, len);
    printf("\r\n");
    bus_Read_buf(op_code, reg_addr, temp_rx_buf, len);
    printf("\r\n");
    now_time = get_us_time();
    ret = memcmp(temp_tx_buf, temp_rx_buf, sizeof(uint8_t)*len);
    total_time = (uint16_t)(now_time - pre_time);
    ret = ret ==0? 0: 1;
    printf("socket mem W/R Burst Time : %d us, result: %s\r\n", total_time, ret_msg[ret]);
    free(temp_tx_buf);
    free(temp_rx_buf);
    return ret;
}

uint8_t reg_WR_interval_comm(uint8_t op_code, uint8_t *save_num_data, uint8_t index_data, uint8_t burst, uint8_t *result_data, uint8_t mode)
{
    uint8_t  h = 0;
    uint16_t j = 0;
    uint8_t temp_tx_data[2]={0,};
    uint8_t temp_rx_data[2]={0,};
    uint8_t ret  = 0, temp_ret = 0;
    uint8_t temp_recv[20] ={0,};
    uint16_t recv_cnt = 0;
    char ret_msg[2][5]={"Pass", "Fail"};
    ret_msg[0][4]=0;
    ret_msg[1][4]=0;
    printf("\r\n  NUM  ");
    for(j=0;j<index_data;j++)
    {
        if(Common_reg_len[save_num_data[j]] == 2)  //len  = 2
        {
            printf("%04X %04X ",Common_reg_offset[save_num_data[j]], Common_reg_offset[save_num_data[j]]  + 1);
        }
        else
        {
            printf("%04X ",Common_reg_offset[save_num_data[j]]);
        }
    }
    printf(" RET \r\n");
    for(j=0;j<=0xff;j++)
    {
        memset(temp_tx_data, j, sizeof(uint8_t)*2);
        ret = 0;
        //printf("0x%02X : ",j);
        temp_ret = 0;
        recv_cnt = 0;
        for(h=0; h<index_data; h++)
        {
            if(burst ==1)
            {
                if(mode <3)
                {
                    reg_WR_return_data(op_code, Common_reg_offset[save_num_data[h]], Common_reg_len[save_num_data[h]], temp_tx_data, temp_rx_data);
                }
                else
                {
                    bus_Write_buf(op_code, Common_reg_offset[save_num_data[h]], temp_tx_data, Common_reg_len[save_num_data[h]]);
                    bus_Read_buf(op_code, Common_reg_offset[save_num_data[h]], temp_rx_data, Common_reg_len[save_num_data[h]]);
                }
                //ret = memcmp(temp_tx_data, temp_rx_data, sizeof(uint8_t)*Common_reg_len[temp_save_num[h]]);
                //printf("  %02X ", temp_rx_data[0]);
                temp_recv[recv_cnt++] = temp_rx_data[0];
                if(temp_rx_data[0] != j)
                {
                    ret = 1;
                    temp_ret = 1;
                    result_data[save_num_data[h]] = 1;
                }
                if(Common_reg_len[save_num_data[h]] > 1)
                {
                    //printf("  %02X ", temp_rx_data[1]);
                    temp_recv[recv_cnt++] = temp_rx_data[1];
                    if(temp_rx_data[1] != j)
                    {
                        ret = 1;
                        temp_ret = 1;
                        result_data[save_num_data[h]] = 1;
                    }
                }
            }
            else
            {
                if(mode < 3)
                {
                    w6300_qspi_write_buf(op_code, Common_reg_offset[save_num_data[h]], &j, 1);
                    w6300_qspi_read_buf(op_code, Common_reg_offset[save_num_data[h]], temp_rx_data, 1);
                }
                else
                {
                    bus_Write_buf(op_code, Common_reg_offset[save_num_data[h]], &j, 1);
                    bus_Read_buf(op_code, Common_reg_offset[save_num_data[h]], temp_rx_data, 1);
                }
                //printf("  %02X ", temp_rx_data[0]);
                temp_recv[recv_cnt++] = temp_rx_data[0];
                if(temp_rx_data[0] != j)
                {
                    ret = 1;
                    temp_ret = 1;
                    result_data[save_num_data[h]] = 1;
                }
                if(Common_reg_len[save_num_data[h]] > 1)
                {
                    if(mode < 3)
                    {
                        w6300_qspi_write_buf(op_code, Common_reg_offset[save_num_data[h]]+1, &j, 1);
                        w6300_qspi_read_buf(op_code, Common_reg_offset[save_num_data[h]]+1, temp_rx_data, 1);
                    }
                    else
                    {
                        bus_Write_buf(op_code, Common_reg_offset[save_num_data[h]]+1, &j, 1);
                        bus_Read_buf(op_code, Common_reg_offset[save_num_data[h]]+1, temp_rx_data, 1);
                    }
                    //printf("  %02X ", temp_rx_data[0]);
                    temp_recv[recv_cnt++] = temp_rx_data[0];
                    if(temp_rx_data[0] != j)
                    {
                        ret = 1;
                        temp_ret = 1;
                        result_data[save_num_data[h]] = 1;
                    }
                }
            }
        }
        if(temp_ret == 1)
        {
            //data print
            printf("0x%02X : ",j);
            for(h=0; h<recv_cnt; h++)
            {
                printf("  %02X ",temp_recv[h]);
            }
            printf(" %s\r\n",ret_msg[ret]);
        }
    }
    return ret;
}
uint8_t reg_R0_interval_comm(uint8_t op_code, uint8_t *save_num_data, uint8_t index_data, uint8_t mode)
{
    uint8_t  h = 0;
    uint16_t j = 0;
    uint8_t temp_tx_data[2]={0,};
    uint8_t temp_rx_data[2]={0,};
    uint8_t ret  = 0;
    printf("\r\n");
    for(j=0;j<index_data;j++)
    {
        if(Common_reg_len[save_num_data[j]] == 2)  //len  = 2
        {
            printf("%04X %04X ",Common_reg_offset[save_num_data[j]], Common_reg_offset[save_num_data[j]]  + 1);
        }
        else
        {
            printf("%04X ",Common_reg_offset[save_num_data[j]]);
        }
    }
    printf("\r\n");
    for(h=0; h<index_data; h++)
    {
        if(mode < 3)
        {
            w6300_qspi_read_buf(op_code, Common_reg_offset[save_num_data[h]], temp_rx_data, Common_reg_len[save_num_data[h]]);
        }
        else
        {
            bus_Read_buf(op_code, Common_reg_offset[save_num_data[h]], temp_rx_data, Common_reg_len[save_num_data[h]]);
        }
        printf("  %02X ", temp_rx_data[0]);
        if(Common_reg_len[save_num_data[h]] > 1)
        {
            printf("  %02X ", temp_rx_data[1]);
        }
    }
    printf("\r\n");
    return 0;
}
uint8_t reg_WR_interval_sock(uint8_t op_code, uint8_t *save_num_data, uint8_t index_data, uint8_t *result_data)
{
    uint8_t  h = 0;
    uint16_t j = 0;
    uint8_t temp_tx_data[2]={0,};
    uint8_t temp_rx_data[2]={0,};
    uint8_t ret  = 0;
    uint8_t t_ret = 0;
    char ret_msg[2][4]={"Pass", "Fail"};
    #if 0
    printf("save Num test = ");
    for(j=0;j<index_data;j++)
    {
        printf("%d:%04X ", save_num_data[j], Socket_reg_len[save_num_data[j]]);
    }
    printf("\r\n");
    #endif
    printf("  NUM  ");
    for(j=0;j<index_data;j++)
    {
        if(Socket_reg_len[save_num_data[j]] == 2)  //len  = 2
        {
            printf("%04X %04X ",Socket_reg_offset[save_num_data[j]], Socket_reg_offset[save_num_data[j]]  + 1);
        }
        else
        {
            printf("%04X ",Socket_reg_offset[save_num_data[j]]);
        }
    }
    printf(" RET \r\n");
    for(j=0;j<=0xff;j++)
    {
        memset(temp_tx_data, j, sizeof(uint8_t)*2);
        ret = 0;
        printf("0x%02X : ",j);
        for(h=0; h<index_data; h++)
        {
#if 0
            switch(save_num_data[h])
            {
                case 12: //0110 Sn_MSSR
                case 19: //0180 Sn_RTR
                case 20: //0184 Sn_PCR
                    w6300_qspi_write_buf(op_code, Socket_reg_offset[save_num_data[h]], temp_tx_data, Socket_reg_len[save_num_data[h]]);
                    w6300_qspi_write_buf(op_code, 0x0010, 0x01, 1);
                    w6300_qspi_read_buf(op_code, 0x0010, temp_rx_data, 1);
                    while(temp_rx_data[0] != 0x00)
                    {
                        w6300_qspi_read_buf(op_code, 0x0010, temp_rx_data, 1);
                    }
                    w6300_qspi_read_buf(op_code, Socket_reg_offset[save_num_data[h]], temp_rx_data, Socket_reg_len[save_num_data[h]]);
                    break;
                case 25: //020C Sn_TX_WR
                    w6300_qspi_write_buf(op_code, Socket_reg_offset[save_num_data[h]], temp_tx_data, Socket_reg_len[save_num_data[h]]);
                    w6300_qspi_write_buf(op_code, 0x0010, 0x20, 1);
                    w6300_qspi_read_buf(op_code, 0x0010, temp_rx_data, 1);
                    while(temp_rx_data[0] != 0x00)
                    {
                        w6300_qspi_read_buf(op_code, 0x0010, temp_rx_data, 1);
                    }
                    w6300_qspi_read_buf(op_code, Socket_reg_offset[save_num_data[h]], temp_rx_data, Socket_reg_len[save_num_data[h]]);
                    break;
                case 28: //0228 Sn_RX_RD
                    w6300_qspi_write_buf(op_code, Socket_reg_offset[save_num_data[h]], temp_tx_data, Socket_reg_len[save_num_data[h]]);
                    w6300_qspi_write_buf(op_code, 0x0010, 0x40, 1);
                    w6300_qspi_read_buf(op_code, 0x0010, temp_rx_data, 1);
                    while(temp_rx_data[0] != 0x00)
                    {
                        w6300_qspi_read_buf(op_code, 0x0010, temp_rx_data, 1);
                    }
                    w6300_qspi_read_buf(op_code, Socket_reg_offset[save_num_data[h]], temp_rx_data, Socket_reg_len[save_num_data[h]]);
                    break;
                default :
                    reg_WR_return_data(op_code, Socket_reg_offset[save_num_data[h]], Socket_reg_len[save_num_data[h]], temp_tx_data, temp_rx_data);
                    break;
            }
#else
            reg_WR_return_data(op_code, Socket_reg_offset[save_num_data[h]], Socket_reg_len[save_num_data[h]], temp_tx_data, temp_rx_data);
#endif
            
            //ret = memcmp(temp_tx_data, temp_rx_data, sizeof(uint8_t)*Common_reg_len[temp_save_num[h]]);
            printf("  %02X ", temp_rx_data[0]);
            if(temp_rx_data[0] != j)
            {
                ret = 1;
                t_ret |= 1;
                result_data[save_num_data[h]] =1;
            }
            if(Socket_reg_len[save_num_data[h]] > 1)
            {
                printf("  %02X ", temp_rx_data[1]);
                if(temp_rx_data[1] != j)
                {
                    ret = 1;
                    t_ret |= 1;
                    result_data[save_num_data[h]] =1;
                }
            }
        }
        printf(" %s\r\n", ret_msg[ret]);
    }
    return t_ret;
}


// mode -> spi mode
uint8_t reg_WR_test_comm(uint8_t mode, uint8_t burst)
{
	uint8_t temp_save_num[16]={0,};
	uint8_t temp_save_index = 0;
	uint8_t temp_save_count = 0;
	uint8_t i = 0, j = 0, h = 0;
	uint8_t temp_opmode = mode<<6;
    uint8_t temp_tx_data[2]={0,};
    uint8_t temp_rx_data[2]={0,};
    uint8_t temp_result[Common_reg_MAX] ={0,};
    char ret_msg[2][5]={"Pass", "Fail"};
    int ret =0;
    ret_msg[0][4] = 0;
    ret_msg[1][4] = 0;
    if(mode == 3)
    {
        temp_opmode = 0x00;
    }
	printf("opcode : 0x%02x, Mode : %d\r\n", temp_opmode, mode);
    printf("Commen Read Only resister test\r\n");
    for(i=0; i<Common_reg_MAX; i++)
	{
		if(Common_reg_rw[i]==0)     //rw
		{
			if(Common_reg_len[i]<4)     // len 4 under
			{
				temp_save_num[temp_save_index++] = i;
				temp_save_count += Common_reg_len[i];
				if(temp_save_count > 14)
				{
                    #if 1
                    printf("temp_save_NUM ");
                    for(j=0; j<temp_save_index;j++)
                    {
                        printf("%02d ",temp_save_num[j]);
                    }
                    printf("\r\n");
                    #endif
                    reg_R0_interval_comm(temp_opmode, temp_save_num, temp_save_index, mode);
                    temp_save_index = 0;
                    temp_save_count = 0;
				}
			}
			else  //long data
			{
				reg_RO_buf_Test(temp_opmode, Common_reg_offset[i], Common_reg_len[i], mode);
			}
		}
	}
    temp_save_index = 0;
    temp_save_count = 0;
    printf("\r\nCommen Write / Read resister test\r\n", temp_opmode);
	for(i=0; i<Common_reg_MAX; i++)
	{
		if((Common_reg_rw[i]==2) && (Common_reg_mask[i]==0))     //rw
		{
			if(Common_reg_len[i]<4)     // len 4 under
			{
				temp_save_num[temp_save_index++] = i;
				temp_save_count += Common_reg_len[i];
				if(temp_save_count > 14)
				{
                    #if 0
                    printf("temp_save_NUM ");
                    for(j=0; j<temp_save_index;j++)
                    {
                        printf("%02d ",temp_save_num[j]);
                    }
                    printf("\r\n");
                    #endif
                    ret |= reg_WR_interval_comm(temp_opmode, temp_save_num, temp_save_index, burst, temp_result, mode);
                    //reg_WR_interval_test(temp_opmode, temp_save_num, temp_save_index, Common_reg_offset, Common_reg_len);
                    temp_save_index = 0;
                    temp_save_count = 0;
				}
			}
			else  //long data
			{
			    if(burst == 1)
                {         
                    temp_result[i] = reg_WR_buf_Test(temp_opmode, Common_reg_offset[i], Common_reg_len[i], mode);
                    ret |= temp_result[i];
                }
                else
                {
                    temp_result[i] = reg_WR_byte_Test(temp_opmode, Common_reg_offset[i], Common_reg_len[i], mode);
                    ret |= temp_result[i];
                }
			}
		}
	}
    if(temp_save_index > 0)
    {
        ret |= reg_WR_interval_comm(temp_opmode, temp_save_num, temp_save_index, burst, temp_result, mode);
        //reg_WR_interval_test(temp_opmode, temp_save_num, temp_save_index, Common_reg_offset, Common_reg_len);
    }
    printf("common reg result \r\n");
    for(i=0; i<Common_reg_MAX; i++)
    {
        printf(" %04X : %s\r\n", Common_reg_offset[i], ret_msg[temp_result[i]]);
    }
    printf("total result : %s\r\n", ret_msg[ret]);
    return ret;
}
uint8_t reg_WR_test_socket(uint8_t mode, uint8_t socketNUM)
{
    uint8_t temp_save_num[16]={0,};
    uint8_t temp_save_index = 0;
    uint8_t temp_save_count = 0;
    uint8_t i = 0, j = 0, h = 0;
    uint8_t temp_opmode = ((mode & 0x03) << 6) | 0x01 | ((socketNUM & 0x07) << 2);
    uint8_t temp_tx_data[2]={0,};
    uint8_t temp_rx_data[2]={0,};
    uint8_t temp_result[Socket_reg_MAX] ={0,};
    char ret_msg[2][5]={"Pass", "Fail"};
    int ret =0;
    ret_msg[0][4]=0;
    ret_msg[1][4]=0;
    printf("opcode : 0x%02x \r\n", temp_opmode);
    printf("Socket Read only resister test\r\n", temp_opmode);
    for(i=0; i<Socket_reg_MAX; i++)
	{
		if(Socket_reg_rw[i]==0)     //rw
		{
			if(Socket_reg_len[i]<4)     // len 4 under
			{
				temp_save_num[temp_save_index++] = i;
				temp_save_count += Socket_reg_len[i];
				if(temp_save_count > 14)
				{
                    #if 0
                    printf("temp_save_NUM ");
                    for(j=0; j<temp_save_index;j++)
                    {
                        printf("%02d ",temp_save_num[j]);
                    }
                    printf("\r\n");
                    #endif
                    reg_R0_interval_comm(temp_opmode, temp_save_num, temp_save_index, mode);
                    temp_save_index = 0;
                    temp_save_count = 0;
				}
			}
			else  //long data
			{
				reg_RO_buf_Test(temp_opmode, Socket_reg_offset[i], Socket_reg_len[i], mode);
			}
		}
	}
    temp_save_index = 0;
    temp_save_count = 0;
    printf("\r\nSocket Write / Read resister test\r\n", temp_opmode);
    #if 0
    w6300_qspi_write_buf(temp_opmode, 0x0144, 0x02, 1);
    w6300_qspi_write_buf(temp_opmode, 0x0000, 0x02, 1);
    w6300_qspi_read_buf(temp_opmode, 0x0144,temp_rx_data, 1);
    printf("Sn_MR2 : %02X\r\n", temp_rx_data[0]);
    w6300_qspi_read_buf(temp_opmode, 0x0000,temp_rx_data, 1);
    printf("Sn_MR : %02X\r\n", temp_rx_data[0]);
    #endif
    for(i=0; i<Socket_reg_MAX; i++)
    {
        if((Socket_reg_rw[i]==2)&&(Socket_reg_mask[i]==0))     //rw
        {
            if(Socket_reg_len[i]<4)     // len 4 under
            {
                temp_save_num[temp_save_index++] = i;
                temp_save_count += Socket_reg_len[i];
                if(temp_save_count > 14)
                {
                    ret |= reg_WR_interval_sock(temp_opmode, temp_save_num, temp_save_index, temp_result);
                    //reg_WR_interval_test(temp_opmode, temp_save_num, temp_save_index, Socket_reg_offset, Socket_reg_len);
                    temp_save_index = 0;
                    temp_save_count = 0;
                }
            }
            else  //long data
            {
                temp_result[i] = reg_WR_buf_Test(temp_opmode, Socket_reg_offset[i], Socket_reg_len[i], mode);
                ret |= temp_result[i];
            }
        }
    }
    if(temp_save_index > 0)
    {
        ret |= reg_WR_interval_sock(temp_opmode, temp_save_num, temp_save_index, temp_result);
        //reg_WR_interval_test(temp_opmode, temp_save_num, temp_save_index, Socket_reg_offset, Socket_reg_len);
    }
    printf("socket reg result \r\n");
    for(i=0; i<Socket_reg_MAX; i++)
    {
        printf(" %04X : %s\r\n", Socket_reg_offset[i], ret_msg[temp_result[i]]);
    }
    printf("total result : %s\r\n", ret_msg[ret]);
    return ret;
}
uint8_t get_str_if_mode(uint8_t mode)
{
    switch(mode)
    {
    case 's':
        printf("single spi mode \r\n");
        return 0;
        break;
    case 'd':
        printf("dual spi mode \r\n");
        return 1;
        break;
    case 'q':
        printf("quad spi mode \r\n");
        return 2;
        break;
    case 'b':
        printf("Indicet Bus mode \r\n");
        return 0x04;
        break;
    default :
        printf("not match mode -> single spi mode \r\n");
        break;
    }
    return 0;
}
uint8_t mem_WR_sock_test(uint8_t mode, uint8_t socket_NUM, uint16_t sock_size, uint8_t burst, uint8_t time_mode)
{

    uint8_t temp_opmode = ((mode & 0x03) << 6) | 0x02 | ((socket_NUM & 0x07) << 2);
    uint16_t i=0 , temp_addr = 0;
    uint8_t total_ret = 0;
    char ret_msg[2][5]={"Pass", "Fail"};
    //uint16_t mem_size = sock_size * 1024;
    //uint8_t temp_rx[16] = {0, }, temp_tx[16] = {0,};
    printf("op code : 0x%02X \r\n", temp_opmode);
    printf("socket: %d, size : %d , burst : %d\r\n", socket_NUM, sock_size, burst);
    
    if(sock_size == 0)
    {
        printf("memsize error \r\n");
        return 1;
    }
    if(time_mode ==-0)
    {
        if(burst == 1)
            total_ret = reg_WR_buf_Test(temp_opmode, temp_addr, sock_size, mode);
        else
        {
            total_ret = reg_WR_byte_Test(temp_opmode, temp_addr, sock_size, mode);
        }
        printf("\r\ntotal result : %s\r\n", ret_msg[total_ret]);
    }
    else
    {
        if(burst == 1)
            total_ret = reg_WR_buf_T_Test(temp_opmode, temp_addr, sock_size);
        else
        {
            total_ret = reg_WR_byte_T_Test(temp_opmode, temp_addr, sock_size);
        }
        printf("\r\ntotal result : %s\r\n", ret_msg[total_ret]);
    }
    
    return total_ret;
}
uint8_t mem_WR_sock_Bus_test(uint8_t mode, uint8_t socket_NUM, uint16_t sock_size)
{

    uint8_t temp_opmode = ((socket_NUM & 0x07) << 5)|(0x02 << 3); //default TX
    uint16_t i=0 , temp_addr = 0;
    uint8_t total_ret = 0;
    char ret_msg[2][5]={"Pass", "Fail"};
    //uint16_t mem_size = sock_size * 1024;
    //uint8_t temp_rx[16] = {0, }, temp_tx[16] = {0,};
    if(mode == 1) //RX opcode
        temp_opmode = 0xE0;
    
    printf("op code : 0x%02X \r\n", temp_opmode);
    printf("socket: %d, size : %d \r\n", socket_NUM, sock_size);
    
    if(sock_size == 0)
    {
        printf("memsize error \r\n");
        return 1;
    }
    total_ret = reg_WR_buf_Bus_Test(temp_opmode, temp_addr, sock_size);
    printf("\r\ntotal result : %s\r\n", ret_msg[total_ret]);    
    return total_ret;
}
uint8_t mem_WR_sock_dump_test(uint8_t mode, uint8_t socket_NUM, uint16_t sock_size, uint8_t burst)
{
    uint8_t temp_opmode[4]; //= ((mode & 0x03) << 6) | 0x02 | ((socket_NUM & 0x07) << 2);
    uint16_t i=0 , j = 0, h = 0;
    uint8_t total_ret[4] = {0,}, ret[4] = {0,};
    char ret_msg[2][5]={"Pass", "Fail"};
    char burst_msg[2][5]={"Byte", "Burst"};
    uint8_t *temp_tx_buf = NULL;
    uint8_t *temp_rx_buf = NULL;
    uint8_t temp_socket[4]= {0,1,2,3};
    printf("op code : 0x%02X \r\n", temp_opmode);
    printf("socket: %d, size : %d , burst : %d\r\n", socket_NUM, sock_size, burst);

    if(sock_size == 0)
    {
        printf("memsize error \r\n");
        return 1;
    }

    temp_opmode[0] = ((mode & 0x03) << 6) | 0x02 | ((socket_NUM & 0x07) << 2);
    temp_socket[0] =socket_NUM;
    switch(socket_NUM)
    {
        case 0:
            temp_opmode[1] = ((mode & 0x03) << 6) | 0x02 | ((1 & 0x07) << 2);
            temp_opmode[2] = ((mode & 0x03) << 6) | 0x02 | ((2 & 0x07) << 2);
            temp_opmode[3] = ((mode & 0x03) << 6) | 0x02 | ((3 & 0x07) << 2);
            break;
        case 1:
            temp_opmode[1] = ((mode & 0x03) << 6) | 0x02 | ((0 & 0x07) << 2);
            temp_opmode[2] = ((mode & 0x03) << 6) | 0x02 | ((2 & 0x07) << 2);
            temp_opmode[3] = ((mode & 0x03) << 6) | 0x02 | ((3 & 0x07) << 2);
            temp_socket[1] = 0;
            break;
        case 2:
            temp_opmode[1] = ((mode & 0x03) << 6) | 0x02 | ((1 & 0x07) << 2);
            temp_opmode[2] = ((mode & 0x03) << 6) | 0x02 | ((0 & 0x07) << 2);
            temp_opmode[3] = ((mode & 0x03) << 6) | 0x02 | ((3 & 0x07) << 2);
            temp_socket[2] = 0;
            break;
        case 3:
            temp_opmode[1] = ((mode & 0x03) << 6) | 0x02 | ((1 & 0x07) << 2);
            temp_opmode[2] = ((mode & 0x03) << 6) | 0x02 | ((2 & 0x07) << 2);
            temp_opmode[3] = ((mode & 0x03) << 6) | 0x02 | ((0 & 0x07) << 2);
            temp_socket[3] = 0;
            break;
        default:
            temp_opmode[0] = ((mode & 0x03) << 6) | 0x02 | ((0 & 0x07) << 2);
            temp_opmode[1] = ((mode & 0x03) << 6) | 0x02 | ((1 & 0x07) << 2);
            temp_opmode[2] = ((mode & 0x03) << 6) | 0x02 | ((2 & 0x07) << 2);
            temp_opmode[3] = ((mode & 0x03) << 6) | 0x02 | ((3 & 0x07) << 2);
            temp_socket[0] = 0;
            break;
    }
    printf("temp socket 0:%02d, 1:%02d, 2:%02d, 3:%02d\r\n", temp_socket[0], temp_socket[1], temp_socket[2], temp_socket[3]);
    printf("socket 0:%02X, 1:%02X, 2:%02X, 3:%02X\r\n", (temp_opmode[0]>>2)&0x07, (temp_opmode[1]>>2)&0x07, (temp_opmode[2]>>2)&0x07, (temp_opmode[3]>>2)&0x07);
    temp_tx_buf = (uint8_t *)calloc(sock_size + 1, sizeof(uint8_t));
    temp_rx_buf = (uint8_t *)calloc(sock_size + 1, sizeof(uint8_t));
    #if 1 //test 0xFF00 test
    w6300_qspi_write_buf(temp_opmode[0], 0x0000, temp_tx_buf, sock_size);
    //printf("hex input data : \r\n");
    for(h=0; h<sock_size; h++)
    {
        if(h%2 == 0)
        {
            temp_tx_buf[h] = 0xff;
        }
        else
            temp_tx_buf[h] = 0x00;
            /*
        printf(" %02x", temp_tx_buf[h]);
        if( (h+1)%16  == 0)
            printf("\r\n");*/

    }
    //printf("\r\n");
    w6300_qspi_write_buf(temp_opmode[0], 0x0000, temp_tx_buf, sock_size);
    w6300_qspi_read_buf(temp_opmode[0], 0x0000, temp_rx_buf, sock_size);
    ret[0] = memcmp(temp_tx_buf, temp_rx_buf, sizeof(uint8_t)*sock_size);
    printf("0xFF00 repeat test  : %s \r\n", (ret[0]==0?"pass":"fail"));
    #endif
    

    for(i=1; i< sock_size; i++)
    {
        if((i%1000)==0)
            printf("%d: %d:%s %d:%s %d:%s %d:%s\r\n", i, temp_socket[0], ret_msg[ret[0]], temp_socket[1], ret_msg[ret[1]], temp_socket[2], ret_msg[ret[2]], temp_socket[3], ret_msg[ret[3]]);
        
        memset(temp_tx_buf, 0x00, sizeof(uint8_t)*sock_size);
        w6300_qspi_write_buf(temp_opmode[0], 0x0000, temp_tx_buf, sock_size);
        memset(temp_tx_buf, 0xAE, sizeof(uint8_t)*i );        
    	memset(temp_rx_buf, 0x00, sizeof(uint8_t)*sock_size);
        if(burst == 1)
        {
            w6300_qspi_write_buf(temp_opmode[0], 0x0000, temp_tx_buf, i);
            for(h= 0; h<4; h++)
            {
                w6300_qspi_read_buf(temp_opmode[h], 0x0000, temp_rx_buf, sock_size);
                ret[h] = memcmp(temp_tx_buf, temp_rx_buf, sizeof(uint8_t)*sock_size);
            }
            
        }
        else
        {
            //printf("Write : ");
            for(j= 0; j<i; j++)
            {
                //printf("%02X ", temp_tx_buf[j]);
                w6300_qspi_write_buf(temp_opmode[0], j, temp_tx_buf + j, 1);
            }
            //printf("\r\n read : ");
            for(h=0; h<4; h++)
            {
                for(j= 0; j<sock_size; j++)
                {
                    w6300_qspi_read_buf(temp_opmode[h], j, temp_rx_buf + j, 1);
                    //printf("%02X ", temp_rx_buf[j]);
                }
                ret[h] = memcmp(temp_tx_buf, temp_rx_buf, sizeof(uint8_t)*sock_size);
            }
            //printf("\r\n");
            
        }
        ///ret = memcmp(temp_tx_buf, temp_rx_buf, sizeof(uint8_t)*sock_size);
        //printf("%d: ret:%d \r\n", i, ret);
        for(h= 0; h<4; h++)
        {
            if(ret[h] != 0)
                total_ret[h] |= 1;
        }
        
    }

    for(h= 0; h<4; h++)
    {
        if(total_ret[h]>1)
            total_ret[h] = 1;
    }
    
    printf("socket mem W/R %s TEST\r\n",burst_msg[burst]);
    for(h= 0; h<4; h++)
    {
        printf("%d : dump result: %s\r\n", temp_socket[h], ret_msg[total_ret[h]]);
    }
    free(temp_tx_buf);
    free(temp_rx_buf);
    return total_ret;
}

uint8_t ping4_test(char *r_data)
{
    uint8_t temp_ip[4] ={0,};
    uint8_t temp_cnt = 0; 
    uint8_t temp_reg = 0;
    uint32_t n_time = 0, set_time = 0;
    char *ptr = strtok(r_data, ".");
    while(ptr != NULL)
    {
        if(temp_cnt > 3)
        {
            printf("parameter Error %d [%s]-%d", temp_cnt, __func__, __LINE__);
            break;
        }
        temp_ip[temp_cnt++] = atoi((const char *)ptr);
        ptr = strtok(NULL, ".");
    }
    printf("set ip : %d.%d.%d.%d \r\n", temp_ip[0], temp_ip[1], temp_ip[2], temp_ip[3] );
    //SLDIPR : 0x418c
    w6300_qspi_write_buf(0x00, 0x418c, temp_ip, 4);
    w6300_qspi_read_buf(0x00, 0x418c, temp_ip, 4);
    printf("read ip : %d.%d.%d.%d \r\n", temp_ip[0], temp_ip[1], temp_ip[2], temp_ip[3] );
    temp_reg = 0x20;
    w6300_qspi_write_buf(0x00, 0x2130, temp_reg, 1);
    set_time = HAL_GetTick();
    while(temp_reg != 0x00)
    {
        w6300_qspi_read_buf(0x00, 0x2130, temp_reg, 1);
        n_time = HAL_GetTick();
        if((set_time +10000) < n_time)
        {
            printf("PING4 Request Time out %02X %d\r\n", temp_reg, n_time - set_time);
            return 2;
        }
    }
    printf("PING4 requset packet\r\n");
    set_time = HAL_GetTick();
    temp_reg =0x00;
    //SLIR : 0x2102 SLIRCLR : 0x2128
    //ping0x20 tout 0x80
    while(temp_reg == 0)
    {
        w6300_qspi_read_buf(0x00, 0x2102, temp_reg, 1);
        if(temp_reg != 0x00)
        {
            w6300_qspi_write_buf(0x00, 0x2128, temp_reg, 1);
            if(temp_reg & 0x20) //seccess
            {
                printf("PING4 recv success\r\n");
                return 1;
            }
            else if(temp_reg & 0x80) // time out
            {
                printf("PING4 recv time out reg\r\n");
                return 4;
            }
        }
        n_time = HAL_GetTick();
        if((set_time +10000) < n_time)
        {
            printf("PING4 recv Time out %d\r\n", n_time - set_time);
            return 3;
        }
    }
    return 0;
}

uint8_t Socket_mem_setting_test(char *r_data)
{
    //uint8_t temp_mem_set[2][8]={0,};
    uint8_t temp_in_set[8]={0,};
    uint8_t temp_cnt = 0;
    uint16_t i = 0;
    uint8_t temp_op_code = 0;
    uint8_t temp_result = 0;
    char *ptr = strtok(r_data, ",");
    while(ptr != NULL)
    {
        if(temp_cnt > 7)
        {
            printf("parameter Error %d [%s]-%d", temp_cnt, __func__, __LINE__);
            break;
        }
        temp_in_set[temp_cnt++] = atoi((const char *)ptr);
        ptr = strtok(NULL, ",");
    }
    printf("mem set input : ");
    for(i=0; i<temp_cnt; i++)
    {
        printf("%2d ", temp_in_set[i]);
        if(!((temp_in_set[i] == 0)|(temp_in_set[i] == 1)|(temp_in_set[i] == 2)|(temp_in_set[i] == 4)|(temp_in_set[i] == 8)|(temp_in_set[i] == 16)))
        {
            temp_in_set[i] = 2;
            printf("->default %d ", temp_in_set[i]);
        }
    }
    printf("\r\n");
    
    //sn_TX_BSR : 0x0200, sn_RX_BSR : 0x0220
    for(i=0; i<temp_cnt; i++)
    {
        #if 0
        temp_op_code = (i<<2) | 0x01;
        w6300_qspi_write_buf(temp_op_code, 0x0200, &temp_in_set[i], 1);
        w6300_qspi_write_buf(temp_op_code, 0x0220, &temp_in_set[i], 1);
        #else
        setSn_TX_BSR(i,temp_in_set[i]);
        setSn_RX_BSR(i,temp_in_set[i]);
        #endif
    }
    printf("TX BSR : ");
    for(i=0; i<temp_cnt; i++)
    {
        #if 0
        temp_op_code = (i<<2) | 0x01;
        temp_result = 0;
        w6300_qspi_read_buf(temp_op_code, 0x0200, &temp_result, 1);
        #else
        temp_result = 0;
        temp_result = getSn_TX_BSR(i);
        #endif
        printf("%2d ", temp_result);
    }
    printf("\r\nRX BSR : ");
    for(i=0; i<temp_cnt; i++)
    {
        temp_result = 0;
        #if 0
        temp_op_code = (i<<2) | 0x01;
        w6300_qspi_read_buf(temp_op_code, 0x0220, &temp_result, 1);
        #else 
        temp_result = getSn_RX_BSR(i);
        #endif
        printf("%2d ", temp_result);
    }
    printf("\r\nnum : ");
    for(i=0;i<8;i++)
    {
        printf("%4d ", i);
    }
    printf("\r\nmem : ");
    for(i=0;i<8;i++)
    {
        printf("%4d ",getSn_TxMAX(i));
    }
    printf("\r\n");
    return 0;
}
void print_record_reg(void)
{
    uint16_t temp_cnt = 0;
    printf(" cnt |  time  | reg |  size |\r\n");
    for(temp_cnt = 0; temp_cnt < record_index ; temp_cnt++)
    {
        printf("%04d | %06d |  %02x | %05d \r\n", temp_cnt, reg_time[temp_cnt], inner_reg[temp_cnt], recv_size_record[temp_cnt]);
    }
}
int8_t tcps_status(uint8_t sn, uint16_t port, uint8_t TCP_mode)
{
    int32_t ret;
    datasize_t sentsize=0;
    int8_t status,inter;
    uint8_t tmp = 0;
    datasize_t received_size;
    uint8_t arg_tmp8;
    uint8_t* mode_msg;
    uint32_t pre_time = 0, now_time = 0;
    
    uint8_t* msg_v4 = "IPv4 mode";
    uint8_t* msg_v6 = "IPv6 mode";
    uint8_t* msg_dual = "Dual IP mode";

    uint8_t buf[2000];

    if(TCP_mode == AS_IPV4)
    {
       mode_msg = msg_v4;
    }else if(TCP_mode == AS_IPV6)
    {
       mode_msg = msg_v6;
    }else
    {
       mode_msg = msg_dual;
    }
    #ifdef _LOOPBACK_DEBUG_
        uint8_t dst_ip[16], ext_status;
        uint16_t dst_port;
    #endif
        //getsockopt(sn, SO_STATUS, &status);
        status = getSn_SR(sn);
        switch(status)
        {
        case SOCK_ESTABLISHED :
            //ctlsocket(sn,CS_GET_INTERRUPT,&inter);
            inter = getSn_IR(sn);
            if(inter & Sn_IR_CON)
            {
                start_record_time = get_time();
                record_index = 0;
            #ifdef _LOOPBACK_DEBUG_
                getsockopt(sn,SO_DESTIP,dst_ip);
                getsockopt(sn,SO_EXTSTATUS, &ext_status);
                if(ext_status & TCPSOCK_MODE){
                    //IPv6
                    printf("%d:Peer IP : %04X:%04X", sn, ((uint16_t)dst_ip[0] << 8) | ((uint16_t)dst_ip[1]),
                            ((uint16_t)dst_ip[2] << 8) | ((uint16_t)dst_ip[3]));
                    printf(":%04X:%04X", ((uint16_t)dst_ip[4] << 8) | ((uint16_t)dst_ip[5]),
                            ((uint16_t)dst_ip[6] << 8) | ((uint16_t)dst_ip[7]));
                    printf(":%04X:%04X", ((uint16_t)dst_ip[8] << 8) | ((uint16_t)dst_ip[9]),
                            ((uint16_t)dst_ip[10] << 8) | ((uint16_t)dst_ip[11]));
                    printf(":%04X:%04X, ", ((uint16_t)dst_ip[12] << 8) | ((uint16_t)dst_ip[13]),
                            ((uint16_t)dst_ip[14] << 8) | ((uint16_t)dst_ip[15]));
                }else
                {
                    //IPv4
                    //getSn_DIPR(sn,dst_ip);
                    printf("%d:Peer IP : %.3d.%.3d.%.3d.%.3d, ",
                            sn, dst_ip[0], dst_ip[1], dst_ip[2], dst_ip[3]);
                }
                getsockopt(sn,SO_DESTPORT,&dst_port);
                printf("Peer Port : %d\r\n", dst_port);
            #endif
                arg_tmp8 = Sn_IR_CON;
                ctlsocket(sn,CS_CLR_INTERRUPT,&arg_tmp8);
            }
            #if 1
            //getsockopt(sn,SO_RECVBUF,&received_size);
            received_size = getSn_RX_RSR(sn);
            
            if(received_size > 0){
                #if TIME_CHECK
                pre_time = get_time();
                #endif
                if(received_size > DATA_BUF_SIZE) received_size = DATA_BUF_SIZE;
                if(status & Sn_IR_RECV)
                {
                    arg_tmp8 = Sn_IR_RECV;
                    ctlsocket(sn,CS_CLR_INTERRUPT,&arg_tmp8);
                }
                
                ret = recv(sn, buf, received_size);
                //reg record
                if(record_index < record_cnt)
                {
                    pre_time = get_time();
                    reg_time[record_index] = (uint16_t)(pre_time - start_record_time);
                    recv_size_record[record_index] = (uint16_t)received_size;
                    inner_reg[record_index++] = get_temp_reg(sn);
                }

                if(ret <= 0) return ret;      // check SOCKERR_BUSY & SOCKERR_XXX. For showing the occurrence of SOCKERR_BUSY.
                received_size = (uint16_t) ret;
                sentsize = 0;
                #if 1
                #if TIME_CHECK
                now_time = get_time();
                printf("R : %8d t:%8d\r\n", now_time - pre_time, received_size);
                #endif

                while(received_size != sentsize)
                {
                    ret = send(sn, buf+sentsize, received_size-sentsize);
                    if(ret < 0)
                    {
                        close(sn);
                        return ret;
                    }
                    sentsize += ret; // Don't care SOCKERR_BUSY, because it is zero.
                }
                #if TIME_CHECK
                now_time = get_time();
                printf("R/S : %8d\r\n", now_time - pre_time);
                #endif
                #endif
            }
            #endif
            break;
        case SOCK_CLOSE_WAIT :
            #ifdef _LOOPBACK_DEBUG_
                printf("%d:CloseWait\r\n",sn);
            #endif
            getsockopt(sn, SO_RECVBUF, &received_size);
            if(received_size > 0) // Don't need to check SOCKERR_BUSY because it doesn't not occur.
            {
                if(received_size > DATA_BUF_SIZE) received_size = DATA_BUF_SIZE;
                ret = recv(sn, buf, received_size);

                if(ret <= 0) return ret;      // check SOCKERR_BUSY & SOCKERR_XXX. For showing the occurrence of SOCKERR_BUSY.
                received_size = (uint16_t) ret;
                sentsize = 0;

                while(received_size != sentsize)
                {
                    ret = send(sn, buf+sentsize, received_size-sentsize);
                    if(ret < 0)
                    {
                        close(sn);
                        return ret;
                    }
                    sentsize += ret; // Don't care SOCKERR_BUSY, because it is zero.
                }
            }

            if((ret = disconnect(sn)) != SOCK_OK) return ret;
                #ifdef _LOOPBACK_DEBUG_
                    printf("%d:Socket Closed\r\n", sn);
                #endif
            break;
        case SOCK_INIT :
            if( (ret = listen(sn)) != SOCK_OK) return ret;
            if(record_index != 0) print_record_reg();
                #ifdef _LOOPBACK_DEBUG_
                    printf("%d:Listen, TCP server loopback, port [%d] as %s\r\n", sn, port, mode_msg);
                #endif
                    printf("%d:Listen, TCP server loopback, port [%d] as %s\r\n", sn, port, mode_msg);
            break;
        case SOCK_CLOSED:
            #ifdef _LOOPBACK_DEBUG_
                printf("%d:TCP server loopback start\r\n",sn);
            #endif
                switch(TCP_mode)
                {
                case AS_IPV4:
//#if NDA
#if 0
 			  tmp = socket(sn, Sn_MR_TCP4, port, SOCK_IO_NONBLOCK|SF_TCP_NODELAY);
 #else
                    tmp = socket(sn, Sn_MR_TCP4, port, SOCK_IO_NONBLOCK);
#endif
                    break;
                case AS_IPV6:
                    tmp = socket(sn, Sn_MR_TCP6, port, SOCK_IO_NONBLOCK);
                    break;
                case AS_IPDUAL:
                    tmp = socket(sn, Sn_MR_TCPD, port, SOCK_IO_NONBLOCK);
                    break;
                default:
                    break;
                }
                if(tmp != sn)    /* reinitialize the socket */
                {
                    #ifdef _LOOPBACK_DEBUG_
                        printf("%d : Fail to create socket.\r\n",sn);
                    #endif
                    return SOCKERR_SOCKNUM;
                }
            #ifdef _LOOPBACK_DEBUG_
                printf("%d:Socket opened[%d]\r\n",sn, getSn_SR(sn));
                //sock_state[sn] = 1;
            #endif
            break;
        default:
            break;
        }
    return 1;
}

uint8_t IperfClient_test(uint8_t sn, char *r_data)
{
    uint8_t temp_ip[4] ={0,};
    uint8_t temp_cnt = 0; 
    uint8_t temp_reg = 0;
    uint32_t n_time = 0, set_time = 0;
    uint16_t in_data[3]={0, }, i = 0;
    uint8_t tmp = 0;
    uint8_t *temp_send=NULL;
    int32_t ret;
    char *save_ip = NULL;
    char *ptr = strtok(r_data, ",");
    uint8_t *data= NULL;
    while(ptr != NULL)
    {
        if(temp_cnt > 3)
        {
            printf("parameter Error %d [%s]-%d", temp_cnt, __func__, __LINE__);
            break;
        }
        if(temp_cnt == 0)
            save_ip = ptr;
        else
        {
            in_data[temp_cnt-1] = atoi((const char *)ptr);
        }
        temp_cnt++;
        ptr = strtok(NULL, ",");
    }
    printf("port : %5d, size : %5d, count : %3d\r\n", in_data[0], in_data[1], in_data[2]);
    //printf("save ip : %s\r\n", save_ip);
    ptr = strtok(save_ip, ".");
    temp_cnt = 0;
    while(ptr != NULL)
    {
        temp_ip[temp_cnt++] = atoi((const char *)ptr);
        ptr = strtok(NULL, ".");
    }
    printf("set ip : %d.%d.%d.%d \r\n", temp_ip[0], temp_ip[1], temp_ip[2], temp_ip[3] );
    #if 1
    // 20230420 taylor

    data = (uint8_t *)calloc(in_data[1] + 1, sizeof(uint8_t));
    for(i=0; i<in_data[1]; i++)
    {
        data[i] = (i%10) + '0';
    }
    #if 0  //send data log display
    printf("send dsta[%d]=[\r\n", in_data[1]);
    for(i=0; i<in_data[2]; i++)
    {
        printf("%s", data);
    }
    printf("\r\n]\r\n");
    #endif
    //data = wiznet_logo;
    while(1)
    {

    	//ret = iperf_tcpc(sn, data, temp_ip, in_data[0], sizeof(wiznet_logo), 1, AS_IPV4);
        if(ret == 31925678)
        {
            break;
        }
        else if(ret == 56783192)
        {
            printf("error\r\n");
            break;
        }
    }
    free(data);
    #else
    tmp = socket(sn, Sn_MR_TCP4, in_data[0]+2, SOCK_IO_NONBLOCK|SF_TCP_NODELAY);
    if(tmp != sn)
    {
        printf("%d : Fail to create socket.\r\n",sn);
        return SOCKERR_SOCKNUM;
    }
    temp_send = (uint8_t *)calloc(in_data[1] + 1, sizeof(uint8_t));
    memset(temp_send, 0xAE, sizeof(uint8_t)*in_data[1] );
    ret = connect(sn, temp_ip, in_data[0], 4); /* Try to connect to TCP server(Socket, DestIP, DestPort) */
    #if 0
    if( ret != SOCK_OK) 
    {
        printf("%d : server connection error.\r\n",ret);
        free(temp_send);
        return ret;
    }
    #endif
    for(i=0; i<in_data[2]; i++)
    {
        ret = send(sn, temp_send, in_data[1]); // Data send process (User's buffer -> Destination through H/W Tx socket buffer)
        if(ret < 0) // Send Error occurred (sent data length < 0)
        {
            printf("%d : send error.\r\n",ret);
            close(sn); // socket close
            free(temp_send);
            return ret;
        }
    }
    if((ret=disconnect(sn)) != SOCK_OK)
    {
        printf("%d : disconnection error.\r\n",ret);
        free(temp_send);
        return ret;
    }
    free(temp_send);
    #endif
    printf("Iperf send complete.\r\n");
    return 0;
}
uint8_t TCP4_Client_CONNECT(uint8_t sn, char *r_data)
{
    uint8_t temp_ip[4] ={0,};
    uint8_t temp_cnt = 0; 
    uint8_t temp_reg = 0;
    uint32_t n_time = 0, set_time = 0;
    uint16_t in_data[2]={0, }, i = 0;
    uint8_t tmp = 0;
    uint8_t *temp_send=NULL;
    int32_t ret;
    char *save_ip = NULL;
    char *ptr = strtok(r_data, ",");
    uint8_t *data= NULL;
    while(ptr != NULL)
    {
        if(temp_cnt > 1)
        {
            printf("parameter Error %d [%s]-%d", temp_cnt, __func__, __LINE__);
            break;
        }
        if(temp_cnt == 0)
            save_ip = ptr;
        else
        {
            in_data[temp_cnt-1] = atoi((const char *)ptr);
        }
        temp_cnt++;
        ptr = strtok(NULL, ",");
    }
    printf("port : %5d,\r\n", in_data[0]);
    //printf("save ip : %s\r\n", save_ip);
    ptr = strtok(save_ip, ".");
    temp_cnt = 0;
    while(ptr != NULL)
    {
        temp_ip[temp_cnt++] = atoi(ptr);
        ptr = strtok(NULL, ".");
    }
    printf("set ip : %d.%d.%d.%d \r\n", temp_ip[0], temp_ip[1], temp_ip[2], temp_ip[3] );
    tmp = socket(sn, Sn_MR_TCP4, in_data[0]*10+2, SOCK_IO_NONBLOCK);
    if(tmp != sn)
    {
        printf("%d : Fail to create socket.[%02X]\r\n",sn, tmp);
        return SOCKERR_SOCKNUM;
    }
    while(getSn_SR(sn) != SOCK_INIT);
    //temp_send = (uint8_t *)calloc(in_data[1] + 1, sizeof(uint8_t));
    ///memset(temp_send, 0xAE, sizeof(uint8_t)*in_data[1] );
    ret = 0;
    while(ret == 0)
    {
        ret = connect(sn, temp_ip, in_data[0], 4); /* Try to connect to TCP server(Socket, DestIP, DestPort) */
        
        if( ret != SOCK_OK) 
        {
            printf("%d : server connection error. 0x%02x\r\n",ret, getSn_SR(sn));
            //free(temp_send);
            #if 0
            ret = close(sn);
            if( ret != SOCK_OK) 
            {
                printf("%d : socket close error.\r\n",ret);
            }
            #endif
            //return ret;
        }
    }
    return 0;
    
}



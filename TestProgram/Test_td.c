#include "W6300_TestProcess.h"
#include "Test_td.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "loopback.h"

#include "socket.h"

//cmd s, 1, 192.168.15.2, 5000, 100<LF
//Sn_RCR test
wiz_NetInfo lo_gWIZNETINFO;

uint8_t RCR_count_test(uint8_t *buff)
{
    uint8_t temp_ip[4] ={0,};
    uint8_t temp_cnt = 0; 
    uint8_t temp_reg = 0;
    uint16_t temp_sn = 0;
    uint32_t n_time = 0, set_time = 0;
    uint16_t in_data[3]={0, }, i = 0;
    uint8_t tmp = 0;
    uint8_t *temp_send=NULL;
    int32_t ret;
    int8_t ret_8 = 0;
    char *save_ip = NULL;
    char *ptr = strtok(buff, ",");
    uint8_t *data= NULL;
    int temp_status = 0;
    int temp_RCR = 0;
    while(ptr != NULL)
    {
        if(temp_cnt > 3)
        {
            printf("parameter Error %d [%s]-%d", temp_cnt, __func__, __LINE__);
            return 100;
            break;
        }
        else if(temp_cnt == 0) //socket number
        {
            temp_sn = atoi((const char *)ptr);
        }
        else if(temp_cnt == 1)
            save_ip = ptr;
        else
        {
            in_data[temp_cnt-2] = atoi((const char *)ptr);
        }
        temp_cnt++;
        ptr = strtok(NULL, ",");
    }
    printf("socket : %d, port : %5d, count : %3d\r\n", temp_sn, in_data[0], in_data[1]);
    //printf("save ip : %s\r\n", save_ip);
    ptr = strtok(save_ip, ".");
    temp_cnt = 0;
    while(ptr != NULL)
    {
        temp_ip[temp_cnt++] = atoi((const char *)ptr);
        ptr = strtok(NULL, ".");
    }
    printf("set ip : %d.%d.%d.%d \r\n", temp_ip[0], temp_ip[1], temp_ip[2], temp_ip[3] );
    
    temp_RCR = getSn_RCR(temp_sn);
    printf("Read RCR = %d\r\n", temp_RCR);
    setSn_RCR(temp_sn,in_data[1]);
    temp_RCR = getSn_RCR(temp_sn);
    printf("Write : %d, Read RCR = %d\r\n",in_data[1], temp_RCR);
    temp_status = getSn_SR(temp_sn);
    printf("socket:%d status:%d\r\n", temp_sn, temp_status);
    if(temp_status != 0)
    {
        ret_8 = close(temp_sn);
        printf("socket close %d\r\n", ret_8);
    }
    //socket open
    if ((ret_8 = socket(temp_sn, Sn_MR_TCP4, 50001, 0x00)) != temp_sn)
    {
        printf("socket open error %d\r\n", ret_8);
        return 101;
    }
    temp_status = getSn_SR(temp_sn);
    printf("2 socket:%d status:%d\r\n", temp_sn, temp_status);

    setSn_RCR(temp_sn,in_data[1]);
    temp_RCR = getSn_RCR(temp_sn);
    printf("2nd Write : %d, Read RCR = %d\r\n",in_data[1], temp_RCR);
    set_time = get_us_time();
    if ((ret_8 = connect(temp_sn, temp_ip, in_data[0], sizeof(temp_ip))) != SOCK_OK)
    {
        n_time = get_us_time();
        printf("connect error %d\r\n time : %ld\r\n", ret_8, n_time - set_time);
    }
    else
    {
        n_time = get_us_time();
        printf("connect success %d \r\n time : %ld\r\n", ret_8, n_time - set_time);
    }
    printf("test finish\r\n");
    return 0;
}

uint8_t load_wiz_net_data(wiz_NetInfo in_gWIZNETINFO)
{

    memcpy(&lo_gWIZNETINFO, &in_gWIZNETINFO, sizeof(wiz_NetInfo));
    printf("load net data \r\n");
    print_in_net_data(lo_gWIZNETINFO);
    return 0;
}

uint8_t print_in_net_data(wiz_NetInfo in_WIZNETINFO)
{
    //wizchip_getnetinfo(&in_WIZNETINFO);
	printf("Mac address: %02x:%02x:%02x:%02x:%02x:%02x\r\n",in_WIZNETINFO.mac[0],in_WIZNETINFO.mac[1],in_WIZNETINFO.mac[2],in_WIZNETINFO.mac[3],in_WIZNETINFO.mac[4],in_WIZNETINFO.mac[5]);
	printf("IP address : %d.%d.%d.%d\r\n",in_WIZNETINFO.ip[0],in_WIZNETINFO.ip[1],in_WIZNETINFO.ip[2],in_WIZNETINFO.ip[3]);
	printf("SN Mask	   : %d.%d.%d.%d\r\n",in_WIZNETINFO.sn[0],in_WIZNETINFO.sn[1],in_WIZNETINFO.sn[2],in_WIZNETINFO.sn[3]);
	printf("Gate way   : %d.%d.%d.%d\r\n",in_WIZNETINFO.gw[0],in_WIZNETINFO.gw[1],in_WIZNETINFO.gw[2],in_WIZNETINFO.gw[3]);
	printf("DNS Server : %d.%d.%d.%d\r\n",in_WIZNETINFO.dns[0],in_WIZNETINFO.dns[1],in_WIZNETINFO.dns[2],in_WIZNETINFO.dns[3]);
	printf("LLA  : %.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X\r\n",in_WIZNETINFO.lla[0],in_WIZNETINFO.lla[1],in_WIZNETINFO.lla[2],in_WIZNETINFO.lla[3],\
									in_WIZNETINFO.lla[4],in_WIZNETINFO.lla[5],in_WIZNETINFO.lla[6],in_WIZNETINFO.lla[7],\
									in_WIZNETINFO.lla[8],in_WIZNETINFO.lla[9],in_WIZNETINFO.lla[10],in_WIZNETINFO.lla[11],\
									in_WIZNETINFO.lla[12],in_WIZNETINFO.lla[13],in_WIZNETINFO.lla[14],in_WIZNETINFO.lla[15]);
	printf("GUA  : %.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X\r\n",in_WIZNETINFO.gua[0],in_WIZNETINFO.gua[1],in_WIZNETINFO.gua[2],in_WIZNETINFO.gua[3],\
									in_WIZNETINFO.gua[4],in_WIZNETINFO.gua[5],in_WIZNETINFO.gua[6],in_WIZNETINFO.gua[7],\
									in_WIZNETINFO.gua[8],in_WIZNETINFO.gua[9],in_WIZNETINFO.gua[10],in_WIZNETINFO.gua[11],\
									in_WIZNETINFO.gua[12],in_WIZNETINFO.gua[13],in_WIZNETINFO.gua[14],in_WIZNETINFO.gua[15]);
	printf("SN6  : %.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X\r\n",in_WIZNETINFO.sn6[0],in_WIZNETINFO.sn6[1],in_WIZNETINFO.sn6[2],in_WIZNETINFO.sn6[3],\
									in_WIZNETINFO.sn6[4],in_WIZNETINFO.sn6[5],in_WIZNETINFO.sn6[6],in_WIZNETINFO.sn6[7],\
									in_WIZNETINFO.sn6[8],in_WIZNETINFO.sn6[9],in_WIZNETINFO.sn6[10],in_WIZNETINFO.sn6[11],\
									in_WIZNETINFO.sn6[12],in_WIZNETINFO.sn6[13],in_WIZNETINFO.sn6[14],in_WIZNETINFO.sn6[15]);
	printf("GW6  : %.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X\r\n",in_WIZNETINFO.gw6[0],in_WIZNETINFO.gw6[1],in_WIZNETINFO.gw6[2],in_WIZNETINFO.gw6[3],\
									in_WIZNETINFO.gw6[4],in_WIZNETINFO.gw6[5],in_WIZNETINFO.gw6[6],in_WIZNETINFO.gw6[7],\
									in_WIZNETINFO.gw6[8],in_WIZNETINFO.gw6[9],in_WIZNETINFO.gw6[10],in_WIZNETINFO.gw6[11],\
									in_WIZNETINFO.gw6[12],in_WIZNETINFO.gw6[13],in_WIZNETINFO.gw6[14],in_WIZNETINFO.gw6[15]);
    return 0;
}
#if 0
uint8_t socket_read_para_data(uint8_t sn, wiz_NetTimeout *temp_nettime)
{
    //wiz_NetTimeout temp_nettime;
    wizchip_gettimeout(temp_nettime);
    printf("RCR : %d\r\n", temp_nettime->s_retry_cnt);
    printf("RTR : %d\r\n", temp_nettime->s_time_100us);
    printf("SLRCR : %d\r\n", temp_nettime->sl_retry_cnt);
    printf("SLRTR : %d\r\n", temp_nettime->sl_time_100us);

    return 0;
}
#endif
uint8_t read_sn_register(uint8_t fn_sn, uint16_t *fn_MSS, uint16_t *fn_RTR, uint8_t *fn_RCR, uint16_t *fn_s_port, uint16_t *fn_d_port, uint8_t *fn_d_IP)
{
    *fn_MSS = getSn_MSSR(fn_sn);
    *fn_RCR = getSn_RCR(fn_sn);
    *fn_RTR = getSn_RTR(fn_sn);
    *fn_s_port = getSn_PORTR(fn_sn);
    *fn_d_port = getSn_DPORTR(fn_sn);
    getSn_DIPR(fn_sn, fn_d_IP);
    printf("========== read socket register =========\r\n");
    printf("read MSSR : 0x%04X RCR : 0x%02x RTR : 0x%04x\r\n", *fn_MSS, *fn_RCR, *fn_RTR);
    printf("read Dest IP %d.%d.%d.%d \r\n", fn_d_IP[0], fn_d_IP[1], fn_d_IP[2], fn_d_IP[3]);
    printf("read Sorce Port : %d, Dest Port : %4d\r\n", *fn_s_port, *fn_d_port);
    return 0;
}
uint8_t comp_sn_register(uint8_t fn_sn, uint16_t fn_MSS_t, uint16_t fn_RTR_t, uint8_t fn_RCR_t, uint16_t fn_s_port_t, uint16_t fn_d_port_t, uint8_t *fn_d_IP_t)
{
    //uint8_t fn_sn;
    uint16_t fn_MSS=0, fn_RTR=0;
    uint8_t fn_RCR=0, ret = 0;
    uint16_t fn_s_port=0, fn_d_port=0;
    uint8_t fn_d_IP[4]={0,};
    fn_MSS = getSn_MSSR(fn_sn);
    fn_RCR = getSn_RCR(fn_sn);
    fn_RTR = getSn_RTR(fn_sn);
    fn_s_port = getSn_PORTR(fn_sn);
    fn_d_port = getSn_DPORTR(fn_sn);
    getSn_DIPR(fn_sn, fn_d_IP);
    printf("========== read socket register =========\r\n");
    printf("read MSSR : 0x%04X RCR : 0x%02x RTR : 0x%04x\r\n", fn_MSS, fn_RCR, fn_RTR);
    printf("read Dest IP %d.%d.%d.%d \r\n", fn_d_IP[0], fn_d_IP[1], fn_d_IP[2], fn_d_IP[3]);
    printf("read Sorce Port : %d, Dest Port : %4d\r\n", fn_s_port, fn_d_port);

    if(fn_MSS_t != fn_MSS)
    {
        printf("MSS In:0x%04X Read:0x%04X ", fn_MSS_t, fn_MSS);
        ret = 1;
    }
    if(fn_RTR_t != fn_RTR)
    {
        printf("RTR In:0x%04X Read:0x%04X ", fn_RTR_t, fn_RTR);
        ret = 1;
    }
    if(fn_RCR_t != fn_RCR)
    {
        printf("RCR In:0x%02X Read:0x%02X ", fn_RCR_t, fn_RCR);
        ret = 1;
    }
    if(fn_s_port_t != fn_s_port)
    {
        printf("S Port In:%4d Read:%4d ", fn_s_port_t, fn_s_port);
        ret = 1;
    }
    if(fn_d_port_t != fn_d_port)
    {
        printf("d Port In:%4d Read:%4d ", fn_d_port_t, fn_d_port);
        ret = 1;
    }
    if(memcmp(fn_d_IP_t, fn_d_IP, sizeof(uint8_t)*4)!=0)
    {
        printf("D IP In:%d.%d.%d.%d Read:%d.%d.%d.%d ", fn_d_IP_t[0], fn_d_IP_t[1], fn_d_IP_t[2], fn_d_IP_t[3], fn_d_IP[0], fn_d_IP[1], fn_d_IP[2], fn_d_IP[3]);
        ret = 1;
    }
    if(ret ==1) //not match data
    {
        printf("ERROR NOT MATCHED DATA!!\r\n");
    }
    else
    {
        printf("Data match success!!\r\n");
    }
    return ret;
}

uint8_t write_sn_register(uint8_t fn_sn, uint16_t fn_MSS, uint16_t fn_RTR, uint8_t fn_RCR, uint16_t fn_s_port, uint16_t fn_d_port, uint8_t *fn_d_IP)
{
    setSn_RCR(fn_sn, fn_RCR);
    setSn_RTR(fn_sn, fn_RTR);
    setSn_MSSR(fn_sn, fn_MSS);
    setSn_PORTR(fn_sn, fn_s_port);
    setSn_DPORTR(fn_sn, fn_d_port);
    setSn_DIPR(fn_sn, fn_d_IP);
    return 0;
}

uint8_t chip_sw_reset_data_test(uint8_t *buff)
{
    wiz_NetInfo temp_gWIZNETINFO;
    wiz_NetInfo temp_gWIZNETINFO1;

    uint8_t temp_send_data[11] ="0123456789";
    uint16_t send_data_len = 10;
    uint8_t temp_cnt=0;
    uint8_t temp_sn = 0;
    uint16_t temp_port = 0;
    uint8_t temp_ip[4] = {0,}, temp_r_ip[4] = {0,};
//    uint8_t *data= NULL;
    uint8_t temp_socket_status = 0; 
    uint16_t R_MSSR = 0, W_MSSR = 0, W_MSSR1 = 0;
    uint8_t R_RCR = 0, W_RCR = 0, W_RCR1 = 0;
    uint16_t R_RTR = 0, W_RTR = 0, W_RTR1 = 0;
    uint8_t temp_dest_IP[4] = {0,};
    uint16_t temp_dest_port = 0;
    uint16_t temp_sourc_port = 0;
    uint16_t freesize = 0;
    uint8_t total_ret = 0;
    int32_t ret=0;
    int8_t tmp = 0;
    temp_send_data[10]=0;

    char *save_ip = NULL;
    
    //parsing socket number, ip, port
    const char *ptr = strtok(buff, ",");
    while(ptr != NULL)
    {
        if(temp_cnt > 2)
        {
            printf("parameter Error %d [%s]-%d", temp_cnt, __func__, __LINE__);
            break;
        }
        else if(temp_cnt == 0) //
            temp_sn = atoi((const char *)ptr);
        else if(temp_cnt == 1) //
            save_ip = ptr;
        else
        {
            temp_port = atoi((const char *)ptr);
        }
        temp_cnt++;
        ptr = strtok(NULL, ",");
    }
    printf("port : %5d,\r\n", temp_port);
    ptr = strtok(save_ip, ".");
    temp_cnt = 0;
    while(ptr != NULL)
    {
        temp_ip[temp_cnt++] = atoi(ptr);
        ptr = strtok(NULL, ".");
    }
    printf("set ip : %d.%d.%d.%d \r\n", temp_ip[0], temp_ip[1], temp_ip[2], temp_ip[3] );
    printf("sn : %d, port : %d\r\n", temp_sn, temp_port);

    printf("========== SW Reset ==========\r\n");
    //chip sw reset
    CHIPUNLOCK();
    setSYCR0(SYCR0_RST);
    HAL_Delay(100);
    //delay
    NETUNLOCK();

    //printf("========== Load Net Info ==========\r\n");
    wizchip_setnetinfo(&lo_gWIZNETINFO);
    HAL_Delay(10);
    wizchip_getnetinfo(&temp_gWIZNETINFO);
    printf("========== Read Net Info ==========\r\n");
    print_in_net_data(temp_gWIZNETINFO);
    if(memcmp(&lo_gWIZNETINFO, &temp_gWIZNETINFO, sizeof(wiz_NetInfo)) != 0)
    {
        printf("ERROR net info not match \r\n");
        total_ret = 1;
    }
    else
    {
        printf("net info match data success!!\r\n");
    }
    //socket data read -> tcp mss, rcr, rtr, dst ip, dst port  TX/RX POINTER
    
    //read_sn_register(temp_sn, &R_MSSR, &R_RTR, &R_RCR, &temp_sourc_port, &temp_dest_port, temp_dest_IP);
    total_ret |= comp_sn_register(temp_sn, 0x0000, 0x0000, 0x00, 0x0000, 0x0000, temp_dest_IP);
    
    W_MSSR = 0x03eb; //1000
    W_RCR = 0x05;
    W_RTR = 0x1389;
    printf("========== write socket register ==========\r\n");
    printf("write MSSR 0x%04X, RCR 0x%02X, RTR 0x%04X \r\n", W_MSSR, W_RCR, W_RTR);
    temp_sourc_port = temp_port*10+2;
    printf("write sorce port : %d, dest port : %d\r\n", temp_sourc_port, temp_port);
    printf("write dest IP : %d.%d.%d.%d \r\n", temp_ip[0], temp_ip[1], temp_ip[2], temp_ip[3]);
    write_sn_register(temp_sn, W_MSSR, W_RTR, W_RCR, temp_sourc_port, temp_port, temp_ip);
    
    //read_sn_register(temp_sn, &R_MSSR, &R_RTR, &R_RCR, &temp_sourc_port, &temp_dest_port, temp_dest_IP);
    total_ret |= comp_sn_register(temp_sn, 0x0000, 0x0000, 0x00, temp_sourc_port, 0x0000, temp_dest_IP);
    
    
    temp_socket_status = getSn_SR(temp_sn);
    printf("socket status 0x%02x\r\n", temp_socket_status);
    
    //socket open
    
    printf("========== UDP socket open =========\r\n");
    setSn_MR(temp_sn, Sn_MR_UDP4);
    setSn_PORTR(temp_sn,temp_port*10+2);
    setSn_CR(temp_sn,Sn_CR_OPEN);    
    while(getSn_CR(temp_sn));
    temp_socket_status = getSn_SR(temp_sn);
    printf("socket status 0x%02x\r\n", temp_socket_status);
    
    total_ret |= comp_sn_register(temp_sn, W_MSSR, W_RTR, W_RCR, temp_sourc_port, 0x0000, temp_dest_IP);
    
    temp_sourc_port = temp_port*10+3;
    W_MSSR1 = 0x03ec;
    W_RTR1 = 0x138a;
    W_RCR1 = 0x06;
    printf("========== write socket register ==========\r\n");
    printf("write MSSR 0x%04X, RCR 0x%02X, RTR 0x%04X \r\n", W_MSSR1, W_RCR1, W_RTR1);
    printf("write sorce port : %d, dest port : %d\r\n", temp_sourc_port, temp_port+1);
    printf("write dest IP : %d.%d.%d.%d \r\n", temp_ip[0], temp_ip[1], temp_ip[2], temp_ip[3]);
    write_sn_register(temp_sn, W_MSSR1, W_RTR1, W_RCR1, temp_sourc_port, temp_port+1, temp_ip);
    printf("!! source port data is changed data after the socket open\r\n");
    total_ret |= comp_sn_register(temp_sn, W_MSSR, W_RTR, W_RCR, temp_sourc_port, 0x0000, temp_dest_IP);
    
    printf("========== UDP DATA SEND 1 ==========\r\n");
    freesize = getSn_TX_FSR(temp_sn);
    printf("tx free buffer size = %d\r\n", freesize);
    temp_cnt = 0;
    while((freesize = getSn_TX_FSR(temp_sn))< send_data_len)
    {
        temp_cnt++;
        if(temp_cnt > 50)
        {
            printf("freesize check count over %d - %d\r\n", freesize, temp_cnt);
            return -1;
        }
    }
    printf("send data[%d]=%s\r\n",send_data_len, temp_send_data);
    wiz_send_data(temp_sn, temp_send_data, send_data_len);
    setSn_CR(temp_sn,Sn_CR_SEND);
    while(getSn_CR(temp_sn));
  
    while(1)
    {
        tmp = getSn_IR(temp_sn);
        if(tmp & Sn_IR_SENDOK)
        {
         setSn_IRCLR(temp_sn, Sn_IR_SENDOK);
         break;
        }  
        else if(tmp & Sn_IR_TIMEOUT)
        {
         setSn_IRCLR(temp_sn, Sn_IR_TIMEOUT);
         printf("send time out error\r\n");
         total_ret = 1;
         break;
         //return SOCKERR_TIMEOUT;
        }
    }  
    //read_sn_register(temp_sn, &R_MSSR, &R_RTR, &R_RCR, &temp_sourc_port, &temp_dest_port, temp_dest_IP);
    total_ret |= comp_sn_register(temp_sn, W_MSSR1, W_RTR, W_RCR, temp_sourc_port, temp_port+1, temp_ip);

    W_MSSR = 0x03ed; //1000
    W_RCR1 = 0x09;
    W_RTR1 = 0x1311;
    printf("========== write socket register ==========\r\n");
    printf("write MSSR 0x%04X, RCR 0x%02X, RTR 0x%04X \r\n", W_MSSR, W_RCR, W_RTR);
    printf("write sorce port : %d, dest port : %d\r\n", temp_sourc_port, temp_port+1);
    printf("write dest IP : %d.%d.%d.%d \r\n", temp_ip[0], temp_ip[1], temp_ip[2], temp_ip[3]);
    write_sn_register(temp_sn, W_MSSR, W_RTR1, W_RCR1, temp_sourc_port, temp_port+1, temp_ip);
    printf("!! source port data is changed data after the socket open\r\n");
    total_ret |= comp_sn_register(temp_sn, W_MSSR1, W_RTR, W_RCR, temp_sourc_port, temp_port+1, temp_ip);

    printf("========== UDP DATA SEND 2 ==========\r\n");
    freesize = getSn_TX_FSR(temp_sn);
    printf("tx free buffer size = %d\r\n", freesize);
    temp_cnt = 0;
    while((freesize = getSn_TX_FSR(temp_sn))< send_data_len)
    {
        temp_cnt++;
        if(temp_cnt > 50)
        {
            printf("freesize check count over %d - %d\r\n", freesize, temp_cnt);
            return -1;
        }
    }
    printf("send data[%d]=%s\r\n",send_data_len, temp_send_data);
    wiz_send_data(temp_sn, temp_send_data, send_data_len);
    setSn_CR(temp_sn,Sn_CR_SEND);
    while(getSn_CR(temp_sn));
  
    while(1)
    {
        tmp = getSn_IR(temp_sn);
        if(tmp & Sn_IR_SENDOK)
        {
         setSn_IRCLR(temp_sn, Sn_IR_SENDOK);
         break;
        }  
        else if(tmp & Sn_IR_TIMEOUT)
        {
         setSn_IRCLR(temp_sn, Sn_IR_TIMEOUT);
         printf("send time out error\r\n");
         total_ret = 1;
         break;
         //return SOCKERR_TIMEOUT;
        }
    }  
    //read_sn_register(temp_sn, &R_MSSR, &R_RTR, &R_RCR, &temp_sourc_port, &temp_dest_port, temp_dest_IP);
    total_ret |= comp_sn_register(temp_sn, W_MSSR, W_RTR, W_RCR, temp_sourc_port, temp_port+1, temp_ip);
    printf("========== Socket Close ==========\r\n");
    close(temp_sn);
    printf("========== SW Reset ==========\r\n");
    //chip sw reset
    CHIPUNLOCK();
    setSYCR0(SYCR0_RST);
    HAL_Delay(100);
    //delay
    NETUNLOCK();

    //get net data
    memset(&temp_gWIZNETINFO, 0, sizeof(wiz_NetInfo));
    memset(&temp_gWIZNETINFO1, 0, sizeof(wiz_NetInfo));
    //wizchip_getnetinfo(&temp_gWIZNETINFO);
    getSHAR(temp_gWIZNETINFO.mac);

    getGAR(temp_gWIZNETINFO.gw);
    getSUBR(temp_gWIZNETINFO.sn);
    getSIPR(temp_gWIZNETINFO.ip);
   
    getGA6R(temp_gWIZNETINFO.gw6);
    getSUB6R(temp_gWIZNETINFO.sn6);
    getLLAR(temp_gWIZNETINFO.lla);
    getGUAR(temp_gWIZNETINFO.gua);
    printf("========== Read Net Info ==========\r\n");
    print_in_net_data(temp_gWIZNETINFO);
    if(memcmp(&temp_gWIZNETINFO1, &temp_gWIZNETINFO, sizeof(wiz_NetInfo)) != 0)
    {
        printf("ERROR net info not match \r\n");
        total_ret = 1;
    }
    else
    {
        printf("net info match data success!!\r\n");
    }
    //socket data read -> tcp mss, rcr, rtr, dst ip, dst port  TX/RX POINTER
    
    //read_sn_register(temp_sn, &R_MSSR, &R_RTR, &R_RCR, &temp_sourc_port, &temp_dest_port, temp_dest_IP);
    total_ret |= comp_sn_register(temp_sn, 0x0000, 0x0000, 0x00, 0x0000, 0x0000, temp_dest_IP);
    
    //read socket register
    //read_sn_register(temp_sn, &R_MSSR, &R_RTR, &R_RCR, &temp_sourc_port, &temp_dest_port, temp_dest_IP);

    temp_socket_status = getSn_SR(temp_sn);
    printf("socket status 0x%02x\r\n", temp_socket_status);
    
    //socket open
    //printf("UDP socket open \r\n");
    printf("========== UDP Socket Open ==========\r\n");
    setSn_MR(temp_sn, Sn_MR_UDP4);
    //setSn_PORTR(temp_sn,temp_port*10+2);
    setSn_CR(temp_sn,Sn_CR_OPEN);
    while(getSn_CR(temp_sn));
    temp_socket_status = getSn_SR(temp_sn);
    printf("socket status 0x%02x\r\n", temp_socket_status);
    //socket data read
    //read_sn_register(temp_sn, &R_MSSR, &R_RTR, &R_RCR, &temp_sourc_port, &temp_dest_port, temp_dest_IP);
    total_ret |= comp_sn_register(temp_sn, 0x05C0, 0x07d0, 0x07, 0x0000, 0x0000, temp_dest_IP);
    printf("========== Socket Close ==========\r\n");
    close(temp_sn);

    wizchip_setnetinfo(&lo_gWIZNETINFO);
    HAL_Delay(10);
    wizchip_getnetinfo(&temp_gWIZNETINFO);
    printf("========== Read Net Info ==========\r\n");
    print_in_net_data(temp_gWIZNETINFO);

    setSn_DPORTR(temp_sn, temp_port);
    setSn_DIPR(temp_sn, temp_ip);
    printf("set dest IP:%d.%d.%d.%d, port : %d\r\n", temp_ip[0], temp_ip[1], temp_ip[2], temp_ip[3], temp_port);

    printf("==========  TCP socket open =========\r\n");
    setSn_MR(temp_sn, Sn_MR_TCP4);
    //setSn_PORTR(temp_sn,temp_port*10+2);
    setSn_CR(temp_sn,Sn_CR_OPEN);
    while(getSn_CR(temp_sn));
    
    temp_socket_status = getSn_SR(temp_sn);
    printf("socket status 0x%02x\r\n", temp_socket_status);

    total_ret |= comp_sn_register(temp_sn, 0x05b4, 0x07d0, 0x07, 0x0000, 0x0000, temp_dest_IP);
    W_MSSR = 0x03eb; //1000
    W_RCR = 0x05;
    W_RTR = 0x1389;
    printf("========== write socket register ==========\r\n");
    printf("write MSSR 0x%04X, RCR 0x%02X, RTR 0x%04X \r\n", W_MSSR, W_RCR, W_RTR);
    temp_sourc_port = temp_port*10+2;
    printf("write sorce port : %d, dest port : %d\r\n", temp_sourc_port, temp_port);
    printf("write dest IP : %d.%d.%d.%d \r\n", temp_ip[0], temp_ip[1], temp_ip[2], temp_ip[3]);
    write_sn_register(temp_sn, W_MSSR, W_RTR, W_RCR, temp_sourc_port, temp_port, temp_ip);
    
    //read_sn_register(temp_sn, &R_MSSR, &R_RTR, &R_RCR, &temp_sourc_port, &temp_dest_port, temp_dest_IP);
    total_ret |= comp_sn_register(temp_sn, 0x05b4, 0x07d0, 0x07, temp_sourc_port, 0x0000, temp_dest_IP);

    printf("==========  TCP connect =========\r\n");
    setSn_CR(temp_sn,Sn_CR_CONNECT);
    while(getSn_CR(temp_sn));
    temp_cnt = 0;
    temp_socket_status = 0;
    while(temp_socket_status != SOCK_ESTABLISHED)
    {
        temp_socket_status = getSn_SR(temp_sn);
        printf("%d socket status 0x%02x\r\n", temp_cnt, temp_socket_status);
        temp_cnt++;
        if(temp_cnt > 10)
        {
            printf("TCP connect status count over \r\n");
            total_ret = 1;
            break;
        }
    }
    if(temp_cnt <= 10)
        printf("TCP connect success\r\n");
    total_ret |= comp_sn_register(temp_sn, W_MSSR, 0x07d0, 0x07, temp_sourc_port, temp_port, temp_ip);

    printf("TCP disconnect\r\n");
    setSn_CR(temp_sn,Sn_CR_DISCON);
    /* wait to process the command... */
    while(getSn_CR(temp_sn));
    while(getSn_SR(temp_sn) != SOCK_CLOSED)
    {
       if(getSn_IR(temp_sn) & Sn_IR_TIMEOUT)
       {
          close(temp_sn);
          //return SOCKERR_TIMEOUT;
          printf("TCP diconnect Time out Error!!\r\n");
          total_ret = 1;
          break;
       }
    }
    
    printf("total result [%s]\r\n",total_ret==0?"Pass":"Fail");
    
    
    return total_ret;
}
uint8_t chip_clk_switch_data_test(uint8_t *buff)
{
    wiz_NetInfo temp_gWIZNETINFO;
    wiz_NetInfo temp_gWIZNETINFO1;

    uint8_t temp_send_data[11] ="0123456789";
    uint16_t send_data_len = 10;
    uint8_t temp_cnt=0;
    uint8_t temp_sn = 0;
    uint16_t temp_port = 0;
    uint8_t temp_ip[4] = {0,}, temp_r_ip[4] = {0,};
//    uint8_t *data= NULL;
    uint8_t temp_socket_status = 0; 
    uint16_t R_MSSR = 0, W_MSSR = 0, W_MSSR1 = 0;
    uint8_t R_RCR = 0, W_RCR = 0, W_RCR1 = 0;
    uint16_t R_RTR = 0, W_RTR = 0, W_RTR1 = 0;
    uint8_t temp_dest_IP[4] = {0,};
    uint16_t temp_dest_port = 0;
    uint16_t temp_sourc_port = 0;
    uint16_t freesize = 0, maxSize = 0;
    uint8_t total_ret = 0;
    int32_t ret=0;
    int8_t tmp = 0;
    uint8_t temp_data =0;
    temp_send_data[10]=0;

    uint8_t temp_opmode = 0;
    char *save_ip = NULL;

    uint8_t temp_spi_mode =0;
    
    //parsing socket number, ip, port
    const char *ptr = strtok(buff, ",");
    while(ptr != NULL)
    {
        if(temp_cnt > 2)
        {
            printf("parameter Error %d [%s]-%d", temp_cnt, __func__, __LINE__);
            break;
        }
        else if(temp_cnt == 0) //
            temp_sn = atoi((const char *)ptr);
        else if(temp_cnt == 1) //
            save_ip = ptr;
        else
        {
            temp_port = atoi((const char *)ptr);
        }
        temp_cnt++;
        ptr = strtok(NULL, ",");
    }
    printf("port : %5d,\r\n", temp_port);
    ptr = strtok(save_ip, ".");
    temp_cnt = 0;
    while(ptr != NULL)
    {
        temp_ip[temp_cnt++] = atoi(ptr);
        ptr = strtok(NULL, ".");
    }
    printf("set ip : %d.%d.%d.%d \r\n", temp_ip[0], temp_ip[1], temp_ip[2], temp_ip[3] );
    printf("sn : %d, port : %d\r\n", temp_sn, temp_port);

    temp_spi_mode = get_g_spi_mode();
    printf("========== SW Reset ==========\r\n");
    //chip sw reset
    CHIPUNLOCK();
    setSYCR0(SYCR0_RST);
    HAL_Delay(100);
    //delay
    NETUNLOCK();

    //set 25Mhz
    printf("========== Set clk 25Mhz  ==========\r\n");
    temp_data = getSYCR1();
    setSYCR1(temp_data |  SYCR1_CLKSEL);
    temp_data = getSYCR1();
    printf("SYCR1 = 0x%02X\r\n", temp_data);
    HAL_Delay(10);
    
    //printf("========== Load Net Info ==========\r\n");
    wizchip_setnetinfo(&lo_gWIZNETINFO);
    HAL_Delay(10);
    wizchip_getnetinfo(&temp_gWIZNETINFO);

    printf("========== Read Net Info ==========\r\n");
    print_in_net_data(temp_gWIZNETINFO);
    if(memcmp(&lo_gWIZNETINFO, &temp_gWIZNETINFO, sizeof(wiz_NetInfo)) != 0)
    {
        printf("ERROR net info not match \r\n");
        total_ret = 1;
    }
    else
    {
        printf("net info match data success!!\r\n");
    }
    //socket data read -> tcp mss, rcr, rtr, dst ip, dst port  TX/RX POINTER

    temp_sourc_port = temp_port*10+3;
    W_MSSR = 0x03eb; //1000
    W_RCR = 0x05;
    W_RTR = 0x1389;
    printf("========== write socket register ==========\r\n");
    printf("write MSSR 0x%04X, RCR 0x%02X, RTR 0x%04X \r\n", W_MSSR, W_RCR, W_RTR);
    temp_sourc_port = temp_port*10+2;
    printf("write sorce port : %d, dest port : %d\r\n", temp_sourc_port, temp_port);
    printf("write dest IP : %d.%d.%d.%d \r\n", temp_ip[0], temp_ip[1], temp_ip[2], temp_ip[3]);
    write_sn_register(temp_sn, W_MSSR, W_RTR, W_RCR, temp_sourc_port, temp_port, temp_ip);
    
    //read_sn_register(temp_sn, &R_MSSR, &R_RTR, &R_RCR, &temp_sourc_port, &temp_dest_port, temp_dest_IP);
    total_ret |= comp_sn_register(temp_sn, 0x0000, 0x0000, 0x00, temp_sourc_port, 0x0000, temp_dest_IP);
    
    printf("==========  TCP socket open =========\r\n");
    setSn_MR(temp_sn, Sn_MR_TCP4);
    setSn_CR(temp_sn,Sn_CR_OPEN);
    while(getSn_CR(temp_sn));

    temp_socket_status = getSn_SR(temp_sn);
    printf("%d socket status 0x%02x\r\n", temp_cnt, temp_socket_status);

    //printf("========== Read socket reg ==========\r\n");
    total_ret |= comp_sn_register(temp_sn, W_MSSR, W_RTR, W_RCR, temp_sourc_port, 0x0000, temp_dest_IP);
    

    printf("========== socket %d tx mem W/R Test ==========\r\n",temp_sn);
    maxSize = getSn_TxMAX(temp_sn);
    printf("sn:%d, max tx size : %d\r\n", temp_sn, maxSize);
    temp_opmode = ((temp_spi_mode & 0x03) << 6) | 0x02 | ((temp_sn & 0x07) << 2);
    //total_ret |= mem_WR_sock_test(temp_spi_mode, temp_sn, maxSize, 1, 0);
    total_ret |= reg_WR_buf_S_Test(temp_opmode, 0, maxSize);

    printf("==========  TCP socket close =========\r\n");
     setSn_CR(temp_sn,Sn_CR_CLOSE);
    /* wait to process the command... */
    while( getSn_CR(temp_sn) );
    /* clear all interrupt of SOCKETn. */
    setSn_IRCLR(temp_sn, 0xFF);
    /* Release the sock_io_mode of SOCKETn. */
    while(getSn_SR(temp_sn) != SOCK_CLOSED);
    
    temp_socket_status = getSn_SR(temp_sn);
    printf("socket status 0x%02x\r\n", temp_socket_status);


    printf("========== Set clk 100Mhz  ==========\r\n");
    temp_data = getSYCR1();
    setSYCR1(temp_data & ~SYCR1_CLKSEL);
    temp_data = getSYCR1();
    printf("SYCR1 = 0x%02X\r\n", temp_data);
    //socket open

    //printf("========== Load Net Info ==========\r\n");
    wizchip_setnetinfo(&lo_gWIZNETINFO);
    HAL_Delay(10);
    wizchip_getnetinfo(&temp_gWIZNETINFO);

    printf("========== Read Net Info ==========\r\n");
    print_in_net_data(temp_gWIZNETINFO);
    if(memcmp(&lo_gWIZNETINFO, &temp_gWIZNETINFO, sizeof(wiz_NetInfo)) != 0)
    {
        printf("ERROR net info not match \r\n");
        total_ret = 1;
    }
    else
    {
        printf("net info match data success!!\r\n");
    }

    temp_sourc_port = temp_port*10+2;
    W_MSSR = 0x05b4; //1000
    W_RCR = 0x06;
    W_RTR = 0x1388;
    printf("========== write socket register ==========\r\n");
    printf("write MSSR 0x%04X, RCR 0x%02X, RTR 0x%04X \r\n", W_MSSR, W_RCR, W_RTR);
    temp_sourc_port = temp_port*10+2;
    printf("write sorce port : %d, dest port : %d\r\n", temp_sourc_port, temp_port);
    printf("write dest IP : %d.%d.%d.%d \r\n", temp_ip[0], temp_ip[1], temp_ip[2], temp_ip[3]);
    write_sn_register(temp_sn, W_MSSR, W_RTR, W_RCR, temp_sourc_port, temp_port, temp_ip);
    
    //read_sn_register(temp_sn, &R_MSSR, &R_RTR, &R_RCR, &temp_sourc_port, &temp_dest_port, temp_dest_IP);
    //total_ret |= comp_sn_register(temp_sn, 0x0000, 0x0000, 0x00, temp_sourc_port, 0x0000, temp_dest_IP);

    printf("========== TCP socket open =========\r\n");
    setSn_MR(temp_sn, Sn_MR_TCP4);
    setSn_CR(temp_sn,Sn_CR_OPEN);
    while(getSn_CR(temp_sn));
    temp_socket_status = getSn_SR(temp_sn);
    printf("%d socket status 0x%02x\r\n", temp_cnt, temp_socket_status);

    //printf("========== Read socket reg ==========\r\n");
    total_ret |= comp_sn_register(temp_sn, W_MSSR, W_RTR, W_RCR, temp_sourc_port, 0x0000, temp_dest_IP);
    

    printf("========== socket %d tx mem W/R Test ==========\r\n",temp_sn);
    //total_ret |= mem_WR_sock_test(temp_spi_mode, temp_sn, maxSize, 1, 0);
    total_ret |= reg_WR_buf_S_Test(temp_opmode, 0, maxSize);


    printf("==========  TCP connect =========\r\n");
    setSn_CR(temp_sn,Sn_CR_CONNECT);
    while(getSn_CR(temp_sn));
    temp_cnt = 0;
    temp_socket_status = 0;
    while(temp_socket_status != SOCK_ESTABLISHED)
    {
        temp_socket_status = getSn_SR(temp_sn);
        printf("%d socket status 0x%02x\r\n", temp_cnt, temp_socket_status);
        temp_cnt++;
        if(temp_cnt > 10)
        {
            printf("TCP connect status count over \r\n");
            total_ret = 1;
            break;
        }
    }
    if(temp_cnt <= 10)
        printf("TCP connect success\r\n");
    total_ret |= comp_sn_register(temp_sn, W_MSSR, W_RTR, W_RCR, temp_sourc_port, temp_port, temp_ip);

    printf("========== TCP DATA SEND ==========\r\n");
    freesize = getSn_TX_FSR(temp_sn);
    printf("tx free buffer size = %d\r\n", freesize);
    temp_cnt = 0;
    while((freesize = getSn_TX_FSR(temp_sn))< send_data_len)
    {
        temp_cnt++;
        if(temp_cnt > 50)
        {
            printf("freesize check count over %d - %d\r\n", freesize, temp_cnt);
            return -1;
        }
    }
    printf("send data[%d]=%s\r\n",send_data_len, temp_send_data);
    wiz_send_data(temp_sn, temp_send_data, send_data_len);
    setSn_CR(temp_sn,Sn_CR_SEND);
    while(getSn_CR(temp_sn));

    while(1)
    {
        tmp = getSn_IR(temp_sn);
        if(tmp & Sn_IR_SENDOK)
        {
         setSn_IRCLR(temp_sn, Sn_IR_SENDOK);
         printf("send complete!!\r\n");
         break;
        }  
        else if(tmp & Sn_IR_TIMEOUT)
        {
         setSn_IRCLR(temp_sn, Sn_IR_TIMEOUT);
         printf("send time out error\r\n");
         total_ret = 1;
         break;
         //return SOCKERR_TIMEOUT;
        }
    }
    

    printf("==========  TCP disconnect =========\r\n");
    setSn_CR(temp_sn,Sn_CR_DISCON);
    /* wait to process the command... */
    while(getSn_CR(temp_sn));
    while(getSn_SR(temp_sn) != SOCK_CLOSED)
    {
       if(getSn_IR(temp_sn) & Sn_IR_TIMEOUT)
       {
          close(temp_sn);
          //return SOCKERR_TIMEOUT;
          printf("TCP diconnect Time out Error!!\r\n");
          total_ret = 1;
          break;
       }
    }
    
    printf("total result [%s]\r\n",total_ret==0?"Pass":"Fail");
    
    return total_ret;
}

uint16_t tcp_send_test(uint8_t sn, uint16_t len)
{
    uint8_t *buf = NULL, *temp_buf = NULL;
    int32_t ret; // return value for SOCK_ERRORs
    datasize_t sentsize=0, data_size = len;
    uint16_t i = 0;

    buf = (uint8_t *)calloc(len + 1, sizeof(uint8_t));
    temp_buf = buf;
    for(i= 0; i<len; i++)
    {
        *temp_buf++ = (i % 10)+ '0';
    }

    while(data_size != sentsize)
    {
        ret = send(sn, buf+sentsize, data_size-sentsize); // Data send process (User's buffer -> Destination through H/W Tx socket buffer)
        if(ret < 0) // Send Error occurred (sent data length < 0)
        {
            close(sn); // socket close
            free(buf);
            return ret;
        }
        sentsize += ret; // Don't care SOCKERR_BUSY, because it is zero.
    }
    free(buf);
    return 0;
}

#ifndef __W6300_TESTPROCESS_H
#define __W6300_TESTPROCESS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"


#define Common_reg_MAX  65
#define Socket_reg_MAX  30


typedef struct QSPI_Set_Data_t
{
    char mode;
    int dummy;
    int instruction;
    int addr;
}QSPI_Set_Data;

typedef union TransUInt_t
{
	uint16_t Data16;
	uint8_t Data8[2];
}TransUInt;

uint32_t get_us_time();

void print_help_menu(void);
char Hex2Char(char const* szHex, unsigned char *rch);
char qspi_set_parse(char *r_data, QSPI_Set_Data *init_data);
uint8_t w6300_qspi_read_buf(uint8_t op_code, uint32_t AddrSel, uint8_t *pbuf, uint16_t len);
uint8_t w6300_qspi_write_buf(uint8_t op_code, uint32_t AddrSel, uint8_t *pbuf, uint16_t len);
uint8_t reg_WR_buf_Test(uint8_t op_code, uint16_t reg_addr, uint16_t len, uint8_t mode);
uint8_t reg_WR_buf_S_Test(uint8_t op_code, uint16_t reg_addr, uint16_t len);
uint8_t reg_WR_return_data(uint8_t op_code, uint16_t reg_addr, uint16_t len, uint8_t *tx, uint8_t *rx);
uint8_t reg_WR_return_data(uint8_t op_code, uint16_t reg_addr, uint16_t len, uint8_t *tx, uint8_t *rx);
uint8_t reg_WR_buf_T_Test(uint8_t op_code, uint16_t reg_addr, uint16_t len);
uint8_t reg_WR_byte_Test(uint8_t op_code, uint16_t reg_addr, uint16_t len, uint8_t mode);
uint8_t reg_WR_byte_T_Test(uint8_t op_code, uint16_t reg_addr, uint16_t len);
uint8_t reg_RO_buf_Test(uint8_t op_code, uint16_t reg_addr, uint8_t len, uint8_t mode);
uint8_t reg_WR_interval_comm(uint8_t op_code, uint8_t *save_num_data, uint8_t index_data, uint8_t burst, uint8_t *result_data, uint8_t mode);
uint8_t reg_R0_interval_comm(uint8_t op_code, uint8_t *save_num_data, uint8_t index_data, uint8_t mode);
uint8_t reg_R0_interval_sock(uint8_t op_code, uint8_t *save_num_data, uint8_t index_data);
uint8_t reg_WR_interval_sock(uint8_t op_code, uint8_t *save_num_data, uint8_t index_data, uint8_t *result_data);
uint8_t reg_WR_test_comm(uint8_t mode, uint8_t burst);
uint8_t reg_WR_test_socket(uint8_t mode, uint8_t socketNUM);
//uint8_t get_spi_mode(uint8_t mode);
uint8_t get_str_if_mode(uint8_t mode);
uint8_t mem_WR_sock_test(uint8_t mode, uint8_t socket_NUM, uint16_t sock_size, uint8_t burst, uint8_t time_mode);
uint8_t mem_WR_sock_dump_test(uint8_t mode, uint8_t socket_NUM, uint16_t sock_size, uint8_t burst);
uint8_t ping4_test(char *r_data);
uint8_t Socket_mem_setting_test(char *r_data);

void bus_Read_buf(uint8_t op_code, uint16_t reg_addr, uint8_t *data, uint16_t len);
void bus_Write_buf(uint8_t op_code, uint16_t reg_addr, uint8_t *data, uint16_t len);
uint8_t reg_WR_buf_Bus_Test(uint8_t op_code, uint16_t reg_addr, uint16_t len);


int8_t tcps_status(uint8_t sn, uint16_t port, uint8_t TCP_mode);
uint8_t IperfClient_test(uint8_t sn, char *r_data);
uint8_t TCP4_Client_CONNECT(uint8_t sn, char *r_data);






#ifdef __cplusplus
}
#endif

#endif /* __W6300_TESTPROCESS_H */



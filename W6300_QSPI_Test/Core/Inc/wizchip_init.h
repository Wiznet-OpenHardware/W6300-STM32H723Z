#ifndef WIZCHIP_INIT_H
#define WIZCHIP_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#include "wizchip_conf.h"
//#include "PHY_IP101G.h"

// #include "W6300_TestProcess.h"
// #include "W6300_TestProcess.h"

OSPI_HandleTypeDef hospi1;


uint8_t W6300_mode;//0; //W6100 >> 0xFF


void W6300Initialze(void);

uint8_t Get_W6300_main_IF_MODE(void);
void FPGA_Reset(void);

//data write/read function list
void W6300BusWriteByte(uint32_t addr, iodata_t data);
iodata_t W6300BusReadByte(uint32_t addr);

void W6300SpiWriteByte(uint8_t tx);
uint8_t W6300SpiReadByte(void);

uint8_t qspi_write_buf(uint8_t op_code, uint32_t AddrSel, uint8_t *pbuf, uint16_t len);
uint8_t qspi_read_buf(uint8_t op_code, uint32_t AddrSel, uint8_t *pbuf, uint16_t len);

// flow Control
void TRACE_ON(void);
void TRACE_OFF(void);
void chip_sw_reset(void);
void chip_hw_reset(void);
void W6300CsEnable(void);
void W6300CsDisable(void);

//char qspi_set_parameter(QSPI_Set_Data *init_data);
char send_spi_data(int len, unsigned char *data);
char recv_spi_data(int len, unsigned char *data);

uint32_t get_us_time();
uint32_t get_time(void);
void wiz_delay(uint32_t delay_time);

void TX_ON(void);
void TX_OFF(void);

//void print_help_menu(void);
//char Hex2Char(char const* szHex, unsigned char *rch);
//char qspi_set_parse(char *r_data, QSPI_Set_Data *init_data);

#ifdef __cplusplus
}
#endif
#endif

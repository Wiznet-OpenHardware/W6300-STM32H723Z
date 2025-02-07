#ifndef __TEST_TD_H
#define __TEST_TD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "wizchip_conf.h"

uint8_t RCR_count_test(uint8_t *buff);
uint8_t chip_sw_reset_data_test(uint8_t *buff);
uint8_t chip_clk_switch_data_test(uint8_t *buff);
uint8_t load_wiz_net_data(wiz_NetInfo in_gWIZNETINFO);
uint8_t print_in_net_data(wiz_NetInfo in_WIZNETINFO);
uint8_t socket_read_para_data(uint8_t sn);
uint16_t tcp_send_test(uint8_t sn, uint16_t len);
uint8_t get_g_spi_mode(void);
#ifdef __cplusplus
}
#endif

#endif /* __W6300_TESTPROCESS_H */

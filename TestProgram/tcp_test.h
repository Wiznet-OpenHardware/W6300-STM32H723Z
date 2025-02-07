/**
 * Copyright (c) 2023 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _TCP_TEST_H_
#define _TCP_TEST_H_

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * ----------------------------------------------------------------------------------------------------
 * Includes
 * ----------------------------------------------------------------------------------------------------
 */

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
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
int8_t tcp_client_nbyte_send_test(device_config_t *device_config, uint8_t *rx_data);
int8_t tcp_client_maximum_segment_size_test(device_config_t *device_config, uint8_t *rx_data);
int8_t tcp_connect_disconnect_test(device_config_t *device_config, uint8_t *rx_data);
int8_t tcp_psh_flag_test(device_config_t *device_config, uint8_t *rx_data);
int8_t tcp_ack_packet_test(device_config_t *device_config, uint8_t *rx_data);
int8_t tcp_rst_packet_test(device_config_t *device_config, uint8_t *rx_data);
int8_t tcp_extension_status_register_test(device_config_t *device_config, uint8_t *rx_data);

#ifdef __cplusplus
}
#endif

#endif /* _TCP_TEST_H_ */

/**
 * Copyright (c) 2023 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _CONFIG_DATA_H_
#define _CONFIG_DATA_H_

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
#define OK                          1
#define ERROR                       0

#define DEVICE_CONFIG_SUCCESS       (OK + 1)
#define DEVICE_CONFIG_FAIL          (ERROR - 10)
#define DEVICE_CONFIG_PARAM_INVALID (ERROR - 11)
#define TCP_TEST_SUCCESS            (OK + 2)
#define TCP_TEST_FAIL               (ERROR - 20)
#define TCP_TEST_PARAM_INVALID      (ERROR - 21)
#define UDP_TEST_SUCCESS            (OK + 3)
#define UDP_TEST_FAIL               (ERROR - 30)
#define UDP_TEST_PARAM_INVALID      (ERROR - 31)

//#define USE_TEST_DEBUG // uncomment to get additional information.
#ifdef USE_TEST_DEBUG
#define USE_TCP_DEBUG
#define USE_UDP_DEBUG
#endif /* USE_TEST_DEBUG */

#define SERIAL_BUF_MAX_SIZE_1K   (1024 * 1)
#define SERIAL_BUF_MAX_SIZE_2K   (1024 * 2)
#define SERIAL_BUF_MAX_SIZE_4K   (1024 * 4)
#define SERIAL_BUF_MAX_SIZE_8K   (1024 * 8)
#define SERIAL_BUF_MAX_SIZE_16K  (1024 * 16)
#define NETWORK_BUF_MAX_SIZE_1K  (1024 * 1)
#define NETWORK_BUF_MAX_SIZE_2K  (1024 * 2)
#define NETWORK_BUF_MAX_SIZE_4K  (1024 * 4)
#define NETWORK_BUF_MAX_SIZE_8K  (1024 * 8)
#define NETWORK_BUF_MAX_SIZE_16K (1024 * 16)

#define ANY_PORT 49152

/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */
struct _mac_info
{
  uint8_t mac[6];
};

struct _network_info
{
  uint8_t local_ipv4[4];
  uint8_t local_ipv6[16];
  uint16_t local_port;

  uint8_t remote_ipv4[4];
  uint8_t remote_ipv6[16];
  uint16_t remote_port;

  uint8_t subnet[4];
  uint8_t gateway[4];
  uint8_t dns[4];
};

typedef struct _device_config
{
  struct _mac_info mac_info;
  struct _network_info network_info;
} device_config_t;

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
int8_t set_device_config_to_default_value(device_config_t *device_config);
int8_t set_mac_to_default_value(device_config_t *device_config);
int8_t set_network_config_to_default_value(device_config_t *device_config);
int8_t set_mac(device_config_t *device_config, uint8_t *rx_data);
int8_t set_network_config(device_config_t *device_config, uint8_t *rx_data);
int8_t set_local_ip(device_config_t *device_config, uint8_t *rx_data);
int8_t set_local_port(device_config_t *device_config, uint8_t *rx_data);
int8_t set_remote_ip(device_config_t *device_config, uint8_t *rx_data);
int8_t set_remote_port(device_config_t *device_config, uint8_t *rx_data);
int8_t set_subnet(device_config_t *device_config, uint8_t *rx_data);
int8_t set_gateway(device_config_t *device_config, uint8_t *rx_data);
int8_t set_dns(device_config_t *device_config, uint8_t *rx_data);

void get_mac(device_config_t *device_config);
void get_network_config(device_config_t *device_config);
void get_local_ip(device_config_t *device_config);
void get_local_port(device_config_t *device_config);
void get_remote_ip(device_config_t *device_config);
void get_remote_port(device_config_t *device_config);
void get_subnet(device_config_t *device_config);
void get_gateway(device_config_t *device_config);
void get_dns(device_config_t *device_config);

#ifdef __cplusplus
}
#endif

#endif /* _CONFIG_DATA_H_ */

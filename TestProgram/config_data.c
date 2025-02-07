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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wizchip_conf.h"

#include "config_data.h"
#include "util.h"
#include "debug.h"

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
int8_t set_device_config_to_default_value(device_config_t *device_config)
{
  /* Restore MAC address */
  if (set_mac_to_default_value(device_config) <= 0)
  {
    return DEVICE_CONFIG_FAIL;
  }

  /* Restore network configuration*/
  if (set_network_config_to_default_value(device_config) <= 0)
  {
    return DEVICE_CONFIG_FAIL;
  }

  return DEVICE_CONFIG_SUCCESS;
}

int8_t set_mac_to_default_value(device_config_t *device_config)
{
  wiz_NetInfo net_info;

  /* Set MAC address to default value */
  device_config->mac_info.mac[0] = 0x00;
  device_config->mac_info.mac[1] = 0x08;
  device_config->mac_info.mac[2] = 0xdc;
  device_config->mac_info.mac[3] = 0x12;
  device_config->mac_info.mac[4] = 0x34;
  device_config->mac_info.mac[5] = 0x56;

  /* Load network configuration */
  if ((ctlnetwork(CN_GET_NETINFO, &net_info)) != 0)
  {
    return DEVICE_CONFIG_FAIL;
  }

  /* Store MAC address to default value */
  net_info.mac[0] = device_config->mac_info.mac[0];
  net_info.mac[1] = device_config->mac_info.mac[1];
  net_info.mac[2] = device_config->mac_info.mac[2];
  net_info.mac[3] = device_config->mac_info.mac[3];
  net_info.mac[4] = device_config->mac_info.mac[4];
  net_info.mac[5] = device_config->mac_info.mac[5];

  if ((ctlnetwork(CN_SET_NETINFO, &net_info)) != 0)
  {
    return DEVICE_CONFIG_FAIL;
  }

  return DEVICE_CONFIG_SUCCESS;
}

int8_t set_network_config_to_default_value(device_config_t *device_config)
{
  wiz_NetInfo net_info;

  /* Set network configuration to default value */
  device_config->network_info.local_ipv4[0] = 192;
  device_config->network_info.local_ipv4[1] = 168;
  device_config->network_info.local_ipv4[2] = 11;
  device_config->network_info.local_ipv4[3] = 2;
  device_config->network_info.local_port = 5000;

  device_config->network_info.remote_ipv4[0] = 192;
  device_config->network_info.remote_ipv4[1] = 168;
  device_config->network_info.remote_ipv4[2] = 11;
  device_config->network_info.remote_ipv4[3] = 3;
  device_config->network_info.remote_port = 5000;

  device_config->network_info.subnet[0] = 255;
  device_config->network_info.subnet[1] = 255;
  device_config->network_info.subnet[2] = 255;
  device_config->network_info.subnet[3] = 0;
  device_config->network_info.gateway[0] = 192;
  device_config->network_info.gateway[1] = 168;
  device_config->network_info.gateway[2] = 11;
  device_config->network_info.gateway[3] = 1;
  device_config->network_info.dns[0] = 8;
  device_config->network_info.dns[1] = 8;
  device_config->network_info.dns[2] = 8;
  device_config->network_info.dns[3] = 8;

  /* Load network configuration */
  if ((ctlnetwork(CN_GET_NETINFO, &net_info)) != 0)
  {
    return DEVICE_CONFIG_FAIL;
  }

  /* Store network configuration to default value */
  net_info.ip[0] = device_config->network_info.local_ipv4[0];
  net_info.ip[1] = device_config->network_info.local_ipv4[1];
  net_info.ip[2] = device_config->network_info.local_ipv4[2];
  net_info.ip[3] = device_config->network_info.local_ipv4[3];

  net_info.sn[0] = device_config->network_info.subnet[0];
  net_info.sn[1] = device_config->network_info.subnet[1];
  net_info.sn[2] = device_config->network_info.subnet[2];
  net_info.sn[3] = device_config->network_info.subnet[3];
  net_info.gw[0] = device_config->network_info.gateway[0];
  net_info.gw[1] = device_config->network_info.gateway[1];
  net_info.gw[2] = device_config->network_info.gateway[2];
  net_info.gw[3] = device_config->network_info.gateway[3];
  net_info.dns[0] = device_config->network_info.dns[0];
  net_info.dns[1] = device_config->network_info.dns[1];
  net_info.dns[2] = device_config->network_info.dns[2];
  net_info.dns[3] = device_config->network_info.dns[3];

  if ((ctlnetwork(CN_SET_NETINFO, &net_info)) != 0)
  {
    return DEVICE_CONFIG_FAIL;
  }

  return DEVICE_CONFIG_SUCCESS;
}

int8_t set_mac(device_config_t *device_config, uint8_t *rx_data)
{
  uint8_t mac[6] = {0, };
  wiz_NetInfo net_info;
  uint8_t temp_MAC[6] = {0, };
  int ret = 0;

  /* Parse & Check MAC address */
  if (!(is_macaddr(rx_data, (uint8_t *)".:-", mac)))
  {
    return DEVICE_CONFIG_PARAM_INVALID;
  }

  PRINT_DBG("MAC set address : %02X:%02X:%02X:%02X:%02X:%02X\r\n",
            mac[0],
            mac[1],
            mac[2],
            mac[3],
            mac[4],
            mac[5]);
  setSHAR(mac);
  getSHAR(temp_MAC);
  ret = memcmp(mac, temp_MAC, sizeof(uint8_t)*6);
  PRINT_DBG("MAC get address : %02X:%02X:%02X:%02X:%02X:%02X ret = %d\r\n",
	            temp_MAC[0],
	            temp_MAC[1],
	            temp_MAC[2],
	            temp_MAC[3],
	            temp_MAC[4],
	            temp_MAC[5],
              ret);
  
  #if 0
  /* Set MAC address */
  device_config->mac_info.mac[0] = mac[0];
  device_config->mac_info.mac[1] = mac[1];
  device_config->mac_info.mac[2] = mac[2];
  device_config->mac_info.mac[3] = mac[3];
  device_config->mac_info.mac[4] = mac[4];
  device_config->mac_info.mac[5] = mac[5];

  /* Load network configuration */
  if ((ctlnetwork(CN_GET_NETINFO, &net_info)) != 0)
  {
    return DEVICE_CONFIG_FAIL;
  }

  /* Store MAC address */
  net_info.mac[0] = device_config->mac_info.mac[0];
  net_info.mac[1] = device_config->mac_info.mac[1];
  net_info.mac[2] = device_config->mac_info.mac[2];
  net_info.mac[3] = device_config->mac_info.mac[3];
  net_info.mac[3] = device_config->mac_info.mac[4];
  net_info.mac[3] = device_config->mac_info.mac[5];

  if ((ctlnetwork(CN_SET_NETINFO, &net_info)) != 0)
  {
    return DEVICE_CONFIG_FAIL;
  }
  #endif

  return DEVICE_CONFIG_SUCCESS;
}

int8_t set_network_config(device_config_t *device_config, uint8_t *rx_data)
{
  char *ptr_rx_data = NULL;
  int local_port = 0;
  int remote_port = 0;
  uint8_t local_ipv4[4] = {0, };
  uint8_t remote_ipv4[4] = {0, };
  uint8_t subnet[4] = {0, };
  uint8_t gateway[4] = {0, };
  uint8_t dns[4] = {0, };
  wiz_NetInfo net_info;

  /* Parse & Check local IP */
  ptr_rx_data = strtok((char *)rx_data, (const char *)",");
  if (!(is_ipaddr((uint8_t *)ptr_rx_data, local_ipv4)))
  {
    return DEVICE_CONFIG_PARAM_INVALID;
  }

  PRINT_DBG("Local IP : %d.%d.%d.%d\r\n", local_ipv4[0], local_ipv4[1], local_ipv4[2], local_ipv4[3]);

  /* Set local IP */
  device_config->network_info.local_ipv4[0] = local_ipv4[0];
  device_config->network_info.local_ipv4[1] = local_ipv4[1];
  device_config->network_info.local_ipv4[2] = local_ipv4[2];
  device_config->network_info.local_ipv4[3] = local_ipv4[3];

  /* Parse local port */
  rx_data += (strlen(ptr_rx_data) + 2);
  ptr_rx_data = strtok((char *)rx_data, (const char *)",");
  local_port = atoi((const char *)ptr_rx_data);

  /* Check local port */
  if ((local_port < 0) || (local_port > 65535)) // 0 <= local port <= 65535
  {
    PRINT_DBG("Local port error : %d\r\n", local_port);

    return DEVICE_CONFIG_PARAM_INVALID;
  }

  PRINT_DBG("Local port : %d\r\n", local_port);

  /* Set local port */
  device_config->network_info.local_port = local_port;

  /* Parse & Check remote IP */
  rx_data += (strlen(ptr_rx_data) + 2);
  ptr_rx_data = strtok((char *)rx_data, (const char *)",");
  if (!(is_ipaddr((uint8_t *)ptr_rx_data, remote_ipv4)))
  {
    return DEVICE_CONFIG_PARAM_INVALID;
  }

  PRINT_DBG("Remote IP : %d.%d.%d.%d\r\n", remote_ipv4[0], remote_ipv4[1], remote_ipv4[2], remote_ipv4[3]);

  /* Set remote IP */
  device_config->network_info.remote_ipv4[0] = remote_ipv4[0];
  device_config->network_info.remote_ipv4[1] = remote_ipv4[1];
  device_config->network_info.remote_ipv4[2] = remote_ipv4[2];
  device_config->network_info.remote_ipv4[3] = remote_ipv4[3];

  /* Parse remote port */
  rx_data += (strlen(ptr_rx_data) + 2);
  ptr_rx_data = strtok((char *)rx_data, (const char *)",");
  remote_port = atoi((const char *)ptr_rx_data);

  /* Check remote port */
  if ((remote_port < 0) || (remote_port > 65535)) // 0 <= remote port <= 65535
  {
    PRINT_DBG("Remote port error : %d\r\n", remote_port);

    return DEVICE_CONFIG_PARAM_INVALID;
  }

  PRINT_DBG("Remote port : %d\r\n", remote_port);

  /* Set remote port */
  device_config->network_info.remote_port = remote_port;

  /* Parse & Check subnet mask */
  rx_data += (strlen(ptr_rx_data) + 2);
  ptr_rx_data = strtok((char *)rx_data, (const char *)",");
  if (!(is_ipaddr((uint8_t *)ptr_rx_data, subnet)))
  {
    return DEVICE_CONFIG_PARAM_INVALID;
  }

  PRINT_DBG("Subnet mask : %d.%d.%d.%d\r\n", subnet[0], subnet[1], subnet[2], subnet[3]);

  /* Set subnet mask */
  device_config->network_info.subnet[0] = subnet[0];
  device_config->network_info.subnet[1] = subnet[1];
  device_config->network_info.subnet[2] = subnet[2];
  device_config->network_info.subnet[3] = subnet[3];

  /* Parse & Check gateway */
  rx_data += (strlen(ptr_rx_data) + 2);
  ptr_rx_data = strtok((char *)rx_data, (const char *)",");
  if (!(is_ipaddr((uint8_t *)ptr_rx_data, gateway)))
  {
    return DEVICE_CONFIG_PARAM_INVALID;
  }

  PRINT_DBG("Gateway : %d.%d.%d.%d\r\n", gateway[0], gateway[1], gateway[2], gateway[3]);

  /* Set gateway */
  device_config->network_info.gateway[0] = gateway[0];
  device_config->network_info.gateway[1] = gateway[1];
  device_config->network_info.gateway[2] = gateway[2];
  device_config->network_info.gateway[3] = gateway[3];

  /* Parse & Check DNS */
  rx_data += (strlen(ptr_rx_data) + 2);
  if (!(is_ipaddr(rx_data, dns)))
  {
    return DEVICE_CONFIG_PARAM_INVALID;
  }

  PRINT_DBG("DNS : %d.%d.%d.%d\r\n", dns[0], dns[1], dns[2], dns[3]);

  /* Set DNS */
  device_config->network_info.dns[0] = dns[0];
  device_config->network_info.dns[1] = dns[1];
  device_config->network_info.dns[2] = dns[2];
  device_config->network_info.dns[3] = dns[3];

  /* Load network configuration */
  if ((ctlnetwork(CN_GET_NETINFO, &net_info)) != 0)
  {
    return DEVICE_CONFIG_FAIL;
  }

  /* Store network configuration*/
  net_info.ip[0] = device_config->network_info.local_ipv4[0];
  net_info.ip[1] = device_config->network_info.local_ipv4[1];
  net_info.ip[2] = device_config->network_info.local_ipv4[2];
  net_info.ip[3] = device_config->network_info.local_ipv4[3];

  net_info.sn[0] = device_config->network_info.subnet[0];
  net_info.sn[1] = device_config->network_info.subnet[1];
  net_info.sn[2] = device_config->network_info.subnet[2];
  net_info.sn[3] = device_config->network_info.subnet[3];
  net_info.gw[0] = device_config->network_info.gateway[0];
  net_info.gw[1] = device_config->network_info.gateway[1];
  net_info.gw[2] = device_config->network_info.gateway[2];
  net_info.gw[3] = device_config->network_info.gateway[3];
  net_info.dns[0] = device_config->network_info.dns[0];
  net_info.dns[1] = device_config->network_info.dns[1];
  net_info.dns[2] = device_config->network_info.dns[2];
  net_info.dns[3] = device_config->network_info.dns[3];

  if ((ctlnetwork(CN_SET_NETINFO, &net_info)) != 0)
  {
    return DEVICE_CONFIG_FAIL;
  }

  return DEVICE_CONFIG_SUCCESS;
}

int8_t set_local_ip(device_config_t *device_config, uint8_t *rx_data)
{
  uint8_t local_ipv4[4] = {0, };
  wiz_NetInfo net_info;

  /* Parse & Check local IP */
  if (!(is_ipaddr(rx_data, local_ipv4)))
  {
    return DEVICE_CONFIG_PARAM_INVALID;
  }

  PRINT_DBG("Local IP : %d.%d.%d.%d\r\n", local_ipv4[0], local_ipv4[1], local_ipv4[2], local_ipv4[3]);

  /* Set local IP */
  device_config->network_info.local_ipv4[0] = local_ipv4[0];
  device_config->network_info.local_ipv4[1] = local_ipv4[1];
  device_config->network_info.local_ipv4[2] = local_ipv4[2];
  device_config->network_info.local_ipv4[3] = local_ipv4[3];

  /* Load network configuration */
  if ((ctlnetwork(CN_GET_NETINFO, &net_info)) != 0)
  {
    return DEVICE_CONFIG_FAIL;
  }

  /* Store local IP */
  net_info.ip[0] = device_config->network_info.local_ipv4[0];
  net_info.ip[1] = device_config->network_info.local_ipv4[1];
  net_info.ip[2] = device_config->network_info.local_ipv4[2];
  net_info.ip[3] = device_config->network_info.local_ipv4[3];

  if ((ctlnetwork(CN_SET_NETINFO, &net_info)) != 0)
  {
    return DEVICE_CONFIG_FAIL;
  }

  return DEVICE_CONFIG_SUCCESS;
}

int8_t set_local_port(device_config_t *device_config, uint8_t *rx_data)
{
  int local_port = 0;

  /* Parse local port */
  local_port = atoi((const char *)rx_data);

  /* Check local port */
  if ((local_port < 0) || (local_port > 65535)) // 0 <= local port <= 65535
  {
    PRINT_DBG("Local port error : %d\r\n", local_port);

    return DEVICE_CONFIG_PARAM_INVALID;
  }

  PRINT_DBG("Local port : %d\r\n", local_port);

  /* Set local port */
  device_config->network_info.local_port = local_port;

  return DEVICE_CONFIG_SUCCESS;
}

int8_t set_remote_ip(device_config_t *device_config, uint8_t *rx_data)
{
  uint8_t remote_ipv4[4] = {0, };

  /* Parse & Check remote IP */
  if (!(is_ipaddr(rx_data, remote_ipv4)))
  {
    return DEVICE_CONFIG_PARAM_INVALID;
  }

  PRINT_DBG("Remote IP : %d.%d.%d.%d\r\n", remote_ipv4[0], remote_ipv4[1], remote_ipv4[2], remote_ipv4[3]);

  /* Set remote IP */
  device_config->network_info.remote_ipv4[0] = remote_ipv4[0];
  device_config->network_info.remote_ipv4[1] = remote_ipv4[1];
  device_config->network_info.remote_ipv4[2] = remote_ipv4[2];
  device_config->network_info.remote_ipv4[3] = remote_ipv4[3];

  return DEVICE_CONFIG_SUCCESS;
}

int8_t set_remote_port(device_config_t *device_config, uint8_t *rx_data)
{
  int remote_port = 0;

  /* Parse remote port */
  remote_port = atoi((const char *)rx_data);

  /* Check remote port */
  if ((remote_port < 0) || (remote_port > 65535)) // 0 <= remote port <= 65535
  {
    PRINT_DBG("Remote port error : %d\r\n", remote_port);

    return DEVICE_CONFIG_PARAM_INVALID;
  }

  PRINT_DBG("Remote port : %d\r\n", remote_port);

  /* Set remote port */
  device_config->network_info.remote_port = remote_port;

  return DEVICE_CONFIG_SUCCESS;
}

int8_t set_subnet(device_config_t *device_config, uint8_t *rx_data)
{
  uint8_t subnet[4] = {0, };
  wiz_NetInfo net_info;

  /* Parse & Check subnet mask */
  if (!(is_ipaddr(rx_data, subnet)))
  {
    return DEVICE_CONFIG_PARAM_INVALID;
  }

  PRINT_DBG("Subnet mask : %d.%d.%d.%d\r\n", subnet[0], subnet[1], subnet[2], subnet[3]);

  /* Set subnet mask */
  device_config->network_info.subnet[0] = subnet[0];
  device_config->network_info.subnet[1] = subnet[1];
  device_config->network_info.subnet[2] = subnet[2];
  device_config->network_info.subnet[3] = subnet[3];

  /* Load network configuration */
  if ((ctlnetwork(CN_GET_NETINFO, &net_info)) != 0)
  {
    return DEVICE_CONFIG_FAIL;
  }

  /* Store subnet mask */
  net_info.sn[0] = device_config->network_info.subnet[0];
  net_info.sn[1] = device_config->network_info.subnet[1];
  net_info.sn[2] = device_config->network_info.subnet[2];
  net_info.sn[3] = device_config->network_info.subnet[3];

  if ((ctlnetwork(CN_SET_NETINFO, &net_info)) != 0)
  {
    return DEVICE_CONFIG_FAIL;
  }

  return DEVICE_CONFIG_SUCCESS;
}

int8_t set_gateway(device_config_t *device_config, uint8_t *rx_data)
{
  uint8_t gateway[4] = {0, };
  wiz_NetInfo net_info;

  /* Parse & Check gateway */
  if (!(is_ipaddr(rx_data, gateway)))
  {
    return DEVICE_CONFIG_PARAM_INVALID;
  }

  PRINT_DBG("Gateway : %d.%d.%d.%d\r\n", gateway[0], gateway[1], gateway[2], gateway[3]);

  /* Set gateway */
  device_config->network_info.gateway[0] = gateway[0];
  device_config->network_info.gateway[1] = gateway[1];
  device_config->network_info.gateway[2] = gateway[2];
  device_config->network_info.gateway[3] = gateway[3];

  /* Load network configuration */
  if ((ctlnetwork(CN_GET_NETINFO, &net_info)) != 0)
  {
    return DEVICE_CONFIG_FAIL;
  }

  /* Store gateway */
  net_info.gw[0] = device_config->network_info.gateway[0];
  net_info.gw[1] = device_config->network_info.gateway[1];
  net_info.gw[2] = device_config->network_info.gateway[2];
  net_info.gw[3] = device_config->network_info.gateway[3];

  if ((ctlnetwork(CN_SET_NETINFO, &net_info)) != 0)
  {
    return DEVICE_CONFIG_FAIL;
  }

  return DEVICE_CONFIG_SUCCESS;
}

int8_t set_dns(device_config_t *device_config, uint8_t *rx_data)
{
  uint8_t dns[4] = {0, };
  wiz_NetInfo net_info;

  /* Parse & Check DNS */
  if (!(is_ipaddr(rx_data, dns)))
  {
    return DEVICE_CONFIG_PARAM_INVALID;
  }

  PRINT_DBG("DNS : %d.%d.%d.%d\r\n", dns[0], dns[1], dns[2], dns[3]);

  /* Set DNS */
  device_config->network_info.dns[0] = dns[0];
  device_config->network_info.dns[1] = dns[1];
  device_config->network_info.dns[2] = dns[2];
  device_config->network_info.dns[3] = dns[3];

  /* Load network configuration */
  if ((ctlnetwork(CN_GET_NETINFO, &net_info)) != 0)
  {
    return DEVICE_CONFIG_FAIL;
  }

  /* Store DNS */
  net_info.dns[0] = device_config->network_info.dns[0];
  net_info.dns[1] = device_config->network_info.dns[1];
  net_info.dns[2] = device_config->network_info.dns[2];
  net_info.dns[3] = device_config->network_info.dns[3];

  if ((ctlnetwork(CN_SET_NETINFO, &net_info)) != 0)
  {
    return DEVICE_CONFIG_FAIL;
  }

  return DEVICE_CONFIG_SUCCESS;
}

void get_mac(device_config_t *device_config)
{
	uint8_t temp_MAC[6]={0,};
	getSHAR(temp_MAC);
	PRINT_INF("%02X:%02X:%02X:%02X:%02X:%02X\r\n",
	            temp_MAC[0],
	            temp_MAC[1],
	            temp_MAC[2],
	            temp_MAC[3],
	            temp_MAC[4],
	            temp_MAC[5]);
#if 0
  wiz_NetInfo net_info;

  ctlnetwork(CN_GET_NETINFO, &net_info);

  PRINT_INF("%02X:%02X:%02X:%02X:%02X:%02X\r\n",
            net_info.mac[0],
            net_info.mac[1],
            net_info.mac[2],
            net_info.mac[3],
            net_info.mac[4],
            net_info.mac[5]);
#endif
}

void get_network_config(device_config_t *device_config)
{
  wiz_NetInfo net_info;

  ctlnetwork(CN_GET_NETINFO, &net_info);

  PRINT_INF("%d.%d.%d.%d, %d, %d.%d.%d.%d, %d, %d.%d.%d.%d, %d.%d.%d.%d, %d.%d.%d.%d\r\n",
            net_info.ip[0],
            net_info.ip[1],
            net_info.ip[2],
            net_info.ip[3],
            device_config->network_info.local_port,
            device_config->network_info.remote_ipv4[0],
            device_config->network_info.remote_ipv4[1],
            device_config->network_info.remote_ipv4[2],
            device_config->network_info.remote_ipv4[3],
            device_config->network_info.remote_port,
            net_info.sn[0],
            net_info.sn[1],
            net_info.sn[2],
            net_info.sn[3],
            net_info.gw[0],
            net_info.gw[1],
            net_info.gw[2],
            net_info.gw[3],
            net_info.dns[0],
            net_info.dns[1],
            net_info.dns[2],
            net_info.dns[3]);
}

void get_local_ip(device_config_t *device_config)
{
  wiz_NetInfo net_info;

  ctlnetwork(CN_GET_NETINFO, &net_info);

  PRINT_INF("%d.%d.%d.%d\r\n",
            net_info.ip[0],
            net_info.ip[1],
            net_info.ip[2],
            net_info.ip[3]);
}

void get_local_port(device_config_t *device_config)
{
  PRINT_INF("%d\r\n", device_config->network_info.local_port);
}

void get_remote_ip(device_config_t *device_config)
{
  PRINT_INF("%d.%d.%d.%d\r\n",
            device_config->network_info.remote_ipv4[0],
            device_config->network_info.remote_ipv4[1],
            device_config->network_info.remote_ipv4[2],
            device_config->network_info.remote_ipv4[3]);
}

void get_remote_port(device_config_t *device_config)
{
  PRINT_INF("%d\r\n", device_config->network_info.remote_port);
}

void get_subnet(device_config_t *device_config)
{
  wiz_NetInfo net_info;

  ctlnetwork(CN_GET_NETINFO, &net_info);

  PRINT_INF("%d.%d.%d.%d\r\n",
            net_info.sn[0],
            net_info.sn[1],
            net_info.sn[2],
            net_info.sn[3]);
}

void get_gateway(device_config_t *device_config)
{
  wiz_NetInfo net_info;

  ctlnetwork(CN_GET_NETINFO, &net_info);

  PRINT_INF("%d.%d.%d.%d\r\n",
            net_info.gw[0],
            net_info.gw[1],
            net_info.gw[2],
            net_info.gw[3]);
}

void get_dns(device_config_t *device_config)
{
  wiz_NetInfo net_info;

  ctlnetwork(CN_GET_NETINFO, &net_info);

  PRINT_INF("%d.%d.%d.%d\r\n",
            net_info.dns[0],
            net_info.dns[1],
            net_info.dns[2],
            net_info.dns[3]);
}

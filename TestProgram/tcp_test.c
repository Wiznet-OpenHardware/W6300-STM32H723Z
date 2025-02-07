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

#include "w6100.h"
#include "socket.h"
#include "wizchip_conf.h"

#include "config_data.h"
#include "tcp_test.h"
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
uint8_t g_tcp_buf[NETWORK_BUF_MAX_SIZE_2K] = {0, };
uint16_t g_tcp_buf_len = 0;

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
int8_t tcp_client_nbyte_send_test(device_config_t *device_config, uint8_t *rx_data)
{
  int retval = 0;
  int i = 0;
  uint8_t ipv = 0;
  uint8_t s = 0;
  uint8_t sn_ir_flag = 0;
  uint16_t sn_tx_rd_flag = 0;
  uint16_t sn_tx_wr_flag = 0;
  uint16_t data_size = 0;
  uint16_t send_num = 0;
  uint16_t any_port = ANY_PORT;

  memset(g_tcp_buf, 0, sizeof(g_tcp_buf));

  /* Parse IP version */
  ipv = atoi((const char *)rx_data);

  /* Check IP version */
  if (ipv == 0)      // IPv4
  {
    ipv = AS_IPV4;
  }
  else if (ipv == 1) // IPv6
  {
    ipv = AS_IPV6;
  }
  else if (ipv == 2) // Dual
  {
    ipv = AS_IPDUAL;
  }
  else
  {
    PRINT_DBG("IP version setting error : %d\r\n", ipv);

    return TCP_TEST_PARAM_INVALID;
  }

  PRINT_DBG("IP version : %d\r\n", ipv);

  /* Parse socket number */
  rx_data = (unsigned char *)strchr((const char *)rx_data, (int)',');
  rx_data += 2;
  s = atoi((const char *)rx_data);

  /* Check socket number */
  if ((s < 0) || (s > 7)) // 0 <= socket number <= 7
  {
    PRINT_DBG("Socket number setting error : %d\r\n", s);

    return TCP_TEST_PARAM_INVALID;
  }

  PRINT_DBG("Socket number : %d\r\n", s);

  /* Parse data size */
  rx_data = (unsigned char *)strchr((const char *)rx_data, (int)',');
  rx_data += 2;
  data_size = atoi((const char *)rx_data);

  /* Check data size */
  if ((data_size < 1) || (data_size > 2048)) // 1 <= data size <= 2048
  {
    PRINT_DBG("Data size error : %d\r\n", data_size);

    return TCP_TEST_PARAM_INVALID;
  }

  PRINT_DBG("Data size : %d\r\n", data_size);

  /* Parse number of times to send */
  rx_data = (unsigned char *)strchr((const char *)rx_data, (int)',');
  rx_data += 2;
  send_num = atoi((const char *)rx_data);

  /* Check number of times to send */
  if ((send_num < 1) || (send_num > 255)) // 1 <= number of times to send <= 255
  {
    PRINT_DBG("Nmber of times to send error : %d\r\n", send_num);

    return TCP_TEST_PARAM_INVALID;
  }

  PRINT_DBG("Number of times to send : %d\r\n", send_num);

  while (1)
  {
    switch (getSn_SR(s))
    {
      case SOCK_ESTABLISHED:
      {
        /* Make test data */
        g_tcp_buf_len = data_size;

        for (i = 0; i < g_tcp_buf_len; i++)
        {
          g_tcp_buf[i] = 0xff;
        }

#ifdef USE_TCP_DEBUG
        for (i = 0; i < g_tcp_buf_len; i++)
        {
          printf("0x%02x ", g_tcp_buf[i]);

          if (((i + 1) % 16) == 0)
          {
            printf("\r\n");
          }
        }
#endif /* USE_TCP_DEBUG */

        for (i = 0; i < send_num; i++)
        {
          if ((retval = send(s, g_tcp_buf, g_tcp_buf_len)) < 0)
          {
            PRINT_DBG("Send error : %d\r\n", retval);

            return retval;
          }

          PRINT_DBG("Send[%03d] OK : %d\r\n", i + 1, retval);

          /* Check socket n interrupt register & Tx read pointer register & Tx write pointer register */
          sn_ir_flag = getSn_IR(s);
          sn_tx_rd_flag = getSn_TX_RD(s);
          sn_tx_wr_flag = getSn_TX_WR(s);

          if (!(sn_ir_flag == SIK_SENT))
          {
            PRINT_DBG("Sn_IR[SIK_SENT] error : 0x%02x\r\n", sn_ir_flag);

            setSn_IR(s, SIK_ALL);

            return TCP_TEST_FAIL;
          }

          if (!(sn_tx_rd_flag == sn_tx_wr_flag))
          {
            PRINT_DBG("Sn_TX_RD error : 0x%02x\r\n", sn_tx_rd_flag);
            PRINT_DBG("Sn_TX_WR error : 0x%02x\r\n", sn_tx_wr_flag);

            return TCP_TEST_FAIL;
          }

          PRINT_DBG("Sn_IR[SIK_SENT] : 0x%02x\r\n", sn_ir_flag);
          PRINT_DBG("Sn_TX_RD : 0x%02x\r\n", sn_tx_rd_flag);
          PRINT_DBG("Sn_TX_WR : 0x%02x\r\n", sn_tx_wr_flag);

          /* Clear socket n interrupt register */
          //setSn_IR(s, SIK_SENT);
          ctlsocket(s, CS_CLR_INTERRUPT, (void *)SIK_SENT);
        }

        if ((retval = disconnect(s)) != SOCK_OK)
        {
          PRINT_DBG("Disconnect error : %d\r\n", retval);

          return retval;
        }

        /* Check socket n interrupt register */
        sn_ir_flag = getSn_IR(s);

        if (!(sn_ir_flag == SIK_DISCONNECTED))
        {
          PRINT_DBG("Sn_IR[SIK_DISCONNECTED] error : 0x%02x\r\n", sn_ir_flag);

          setSn_IR(s, SIK_ALL);

          return TCP_TEST_FAIL;
        }

        PRINT_DBG("Sn_IR[SIK_DISCONNECTED] : 0x%02x\r\n", sn_ir_flag);

        /* Clear socket n interrupt register */
        setSn_IR(s, SIK_DISCONNECTED);

        return TCP_TEST_SUCCESS;
      }
      break;

      case SOCK_CLOSE_WAIT:
      {
        if ((retval = disconnect(s)) != SOCK_OK)
        {
          PRINT_DBG("Disconnect error : %d\r\n", retval);

          return retval;
        }

        PRINT_DBG("Disconnect OK : %d\r\n", retval);
      }
      break;

      case SOCK_INIT:
      {
        if (ipv == AS_IPV4)
        {
          if ((retval = connect(s, device_config->network_info.remote_ipv4, device_config->network_info.remote_port, sizeof(device_config->network_info.remote_ipv4))) != SOCK_OK)
          {
            PRINT_DBG("Connect error : %d\r\n", retval);

            return retval;
          }

          /* Check socket n interrupt register */
          sn_ir_flag = getSn_IR(s);

          if (!(sn_ir_flag == SIK_CONNECTED))
          {
            PRINT_DBG("Sn_IR[SIK_CONNECTED] error : 0x%02x\r\n", sn_ir_flag);

            setSn_IR(s, SIK_ALL);

            return TCP_TEST_FAIL;
          }

          PRINT_DBG("Sn_IR[SIK_CONNECTED] : 0x%02x\r\n", sn_ir_flag);

          /* Clear socket n interrupt register */
          setSn_IR(s, SIK_CONNECTED);
        }
        else if (ipv == AS_IPV6)
        {
          if ((retval = connect(s, device_config->network_info.remote_ipv6, device_config->network_info.remote_port, sizeof(device_config->network_info.remote_ipv6))) != SOCK_OK)
          {
            PRINT_DBG("Connect error : %d\r\n", retval);

            return retval;
          }

          /* Check socket n interrupt register */
          sn_ir_flag = getSn_IR(s);

          if (!(sn_ir_flag == SIK_CONNECTED))
          {
            PRINT_DBG("Sn_IR[SIK_CONNECTED] error : 0x%02x\r\n", sn_ir_flag);

            setSn_IR(s, SIK_ALL);

            return TCP_TEST_FAIL;
          }

          PRINT_DBG("Sn_IR[SIK_CONNECTED] : 0x%02x\r\n", sn_ir_flag);

          /* Clear socket n interrupt register */
          setSn_IR(s, SIK_CONNECTED);
        }
        else if (ipv == AS_IPDUAL)
        {
#if 1
          if ((retval = connect(s, device_config->network_info.remote_ipv4, device_config->network_info.remote_port, sizeof(device_config->network_info.remote_ipv4))) != SOCK_OK)
          {
            PRINT_DBG("Connect error : %d\r\n", retval);

            return retval;
          }
#else
          if ((retval = connect(s, device_config->network_info.remote_ipv6, device_config->network_info.remote_port, sizeof(device_config->network_info.remote_ipv6))) != SOCK_OK)
          {
            PRINT_DBG("Connect error : %d\r\n", retval);

            return retval;
          }
#endif

          /* Check socket n interrupt register */
          sn_ir_flag = getSn_IR(s);

          if (!(sn_ir_flag == SIK_CONNECTED))
          {
            PRINT_DBG("Sn_IR[SIK_CONNECTED] error : 0x%02x\r\n", sn_ir_flag);

            setSn_IR(s, SIK_ALL);

            return TCP_TEST_FAIL;
          }

          PRINT_DBG("Sn_IR[SIK_CONNECTED] : 0x%02x\r\n", sn_ir_flag);

          /* Clear socket n interrupt register */
          setSn_IR(s, SIK_CONNECTED);
        }
        else
        {
          ;
        }

        PRINT_DBG("Connect OK : %d\r\n", retval);
      }
      break;

      case SOCK_CLOSED:
      {
        close(s);

        if (ipv == AS_IPV4)
        {
          if ((retval = socket(s, Sn_MR_TCP4, any_port++, 0x00)) != s)
          {
            PRINT_DBG("Socket error : %d\r\n", retval);

            return retval;
          }
        }
        else if (ipv == AS_IPV6)
        {
          if ((retval = socket(s, Sn_MR_TCP6, any_port++, 0x00)) != s)
          {
            PRINT_DBG("Socket error : %d\r\n", retval);

            return retval;
          }
        }
        else if (ipv == AS_IPDUAL)
        {
          if ((retval = socket(s, Sn_MR_TCPD, any_port++, 0x00)) != s)
          {
            PRINT_DBG("Socket error : %d\r\n", retval);

            if (any_port == 0xffff)
            {
              any_port = ANY_PORT;
            }

            return retval;
          }
        }
        else
        {
          ;
        }

        PRINT_DBG("Socket OK : %d\r\n", retval);
      }
      break;

      default:
        break;
    }
  }
}

int8_t tcp_client_maximum_segment_size_test(device_config_t *device_config, uint8_t *rx_data)
{
  int retval = 0;
  int i = 0;
  uint8_t ipv = 0;
  uint8_t s = 0;
  uint8_t send_receive_mode = 0; // 0 : send mode, 1 : receive mode
  uint8_t sn_ir_flag = 0;
  uint16_t sn_tx_rd_flag = 0;
  uint16_t sn_tx_wr_flag = 0;
  uint16_t sn_rx_rd_flag = 0;
  uint16_t sn_rx_wr_flag = 0;
  uint16_t data_size = 0;
  uint16_t send_receive_num = 0;
  uint16_t received_num = 0;
  uint16_t received_size = 0;
  uint16_t maximum_segment_size = 0;
  uint16_t any_port = ANY_PORT;

  memset(g_tcp_buf, 0, sizeof(g_tcp_buf));

  /* Parse IP version */
  ipv = atoi((const char *)rx_data);

  /* Check IP version */
  if (ipv == 0)      // IPv4
  {
    ipv = AS_IPV4;
  }
  else if (ipv == 1) // IPv6
  {
    ipv = AS_IPV6;
  }
  else if (ipv == 2) // Dual
  {
    ipv = AS_IPDUAL;
  }
  else
  {
    PRINT_DBG("IP version setting error : %d\r\n", ipv);

    return TCP_TEST_PARAM_INVALID;
  }

  PRINT_DBG("IP version : %d\r\n", ipv);

  /* Parse maximum segment size */
  rx_data = (unsigned char *)strchr((const char *)rx_data, (int)',');
  rx_data += 2;
  maximum_segment_size = atoi((const char *)rx_data);

  /* Check maximum segment size */
  if ((maximum_segment_size < 1) || (maximum_segment_size > 2048)) // 1 <= maximum segment size <= 2048
  {
    PRINT_DBG("Maximum segment size error : %d\r\n", maximum_segment_size);

    return TCP_TEST_PARAM_INVALID;
  }

  PRINT_DBG("Maximum segment size : %d\r\n", maximum_segment_size);

  /* Parse socket number */
  rx_data = (unsigned char *)strchr((const char *)rx_data, (int)',');
  rx_data += 2;
  s = atoi((const char *)rx_data);

  /* Check socket number */
  if ((s < 0) || (s > 7)) // 0 <= socket number <= 7
  {
    PRINT_DBG("Socket number setting error : %d\r\n", s);

    return TCP_TEST_PARAM_INVALID;
  }

  PRINT_DBG("Socket number : %d\r\n", s);

  /* Parse data size */
  rx_data = (unsigned char *)strchr((const char *)rx_data, (int)',');
  rx_data += 2;
  data_size = atoi((const char *)rx_data);

  /* Check data size */
  if ((data_size < 0) || (data_size > 2048)) // data size == 0 : receive mode, 1 <= data size <= 2048 : send mode
  {
    PRINT_DBG("Data size error : %d\r\n", data_size);

    return TCP_TEST_PARAM_INVALID;
  }

  PRINT_DBG("Data size : %d\r\n", data_size);

  /* Set send/receive mode */
  if (data_size == 0) // receive mode
  {
    send_receive_mode = 1;
  }
  else if ((data_size >= 1) && (data_size <= 2048)) // send mode
  {
    send_receive_mode = 0;
  }
  else
  {
    ;
  }

  PRINT_DBG("Send/Receive mode : %s\r\n", send_receive_mode ? "Receive" : "Send");

  /* Parse number of times to send/receive */
  rx_data = (unsigned char *)strchr((const char *)rx_data, (int)',');
  rx_data += 2;
  send_receive_num = atoi((const char *)rx_data);

  /* Check number of times to send/receive */
  if ((send_receive_num < 1) || (send_receive_num > 255)) // 1 <= number of times to send/receive <= 255
  {
    PRINT_DBG("Nmber of times to send/receive error : %d\r\n", send_receive_num);

    return TCP_TEST_PARAM_INVALID;
  }

  PRINT_DBG("Number of times to send/receive : %d\r\n", send_receive_num);

  while (1)
  {
    switch (getSn_SR(s))
    {
      case SOCK_ESTABLISHED:
      {
        if (send_receive_mode == 0) // send mode
        {
          /* Make test data */
          g_tcp_buf_len = data_size;

          for (i = 0; i < g_tcp_buf_len; i++)
          {
            g_tcp_buf[i] = 0xff;
          }

#ifdef USE_TCP_DEBUG
          for (i = 0; i < g_tcp_buf_len; i++)
          {
            printf("0x%02x ", g_tcp_buf[i]);

            if (((i + 1) % 16) == 0)
            {
              printf("\r\n");
            }
          }
#endif /* USE_TCP_DEBUG */

          for (i = 0; i < send_receive_num; i++)
          {
            if ((retval = send(s, g_tcp_buf, g_tcp_buf_len)) < 0)
            {
              PRINT_DBG("Send error : %d\r\n", retval);

              return retval;
            }

            PRINT_DBG("Send[%03d] OK : %d\r\n", i + 1, retval);

            /* Check socket n interrupt register & Tx read pointer register & Tx write pointer register */
            sn_ir_flag = getSn_IR(s);
            sn_tx_rd_flag = getSn_TX_RD(s);
            sn_tx_wr_flag = getSn_TX_WR(s);

            if (!(sn_ir_flag == SIK_SENT))
            {
              PRINT_DBG("Sn_IR[SIK_SENT] error : 0x%02x\r\n", sn_ir_flag);

              setSn_IR(s, SIK_ALL);

              return TCP_TEST_FAIL;
            }

            if (!(sn_tx_rd_flag == sn_tx_wr_flag))
            {
              PRINT_DBG("Sn_TX_RD error : 0x%02x\r\n", sn_tx_rd_flag);
              PRINT_DBG("Sn_TX_WR error : 0x%02x\r\n", sn_tx_wr_flag);

              return TCP_TEST_FAIL;
            }

            PRINT_DBG("Sn_IR[SIK_SENT] : 0x%02x\r\n", sn_ir_flag);
            PRINT_DBG("Sn_TX_RD : 0x%02x\r\n", sn_tx_rd_flag);
            PRINT_DBG("Sn_TX_WR : 0x%02x\r\n", sn_tx_wr_flag);

            /* Clear socket n interrupt register */
            //setSn_IR(s, SIK_SENT);
            ctlsocket(s, CS_CLR_INTERRUPT, (void *)SIK_SENT);
          }

          if ((retval = disconnect(s)) != SOCK_OK)
          {
            PRINT_DBG("Disconnect error : %d\r\n", retval);

            return retval;
          }

          /* Check socket n interrupt register */
          sn_ir_flag = getSn_IR(s);

          if (!(sn_ir_flag == SIK_DISCONNECTED))
          {
            PRINT_DBG("Sn_IR[SIK_DISCONNECTED] error : 0x%02x\r\n", sn_ir_flag);

            setSn_IR(s, SIK_ALL);

            return TCP_TEST_FAIL;
          }

          PRINT_DBG("Sn_IR[SIK_DISCONNECTED] : 0x%02x\r\n", sn_ir_flag);

          /* Clear socket n interrupt register */
          setSn_IR(s, SIK_DISCONNECTED);

          return TCP_TEST_SUCCESS;
        }
        else if (send_receive_mode == 1) // receive mode
        {
          do
          {
            if ((received_size = getSn_RX_RSR(s)) > 0)
            {
              if (received_size > NETWORK_BUF_MAX_SIZE_2K)
              {
                received_size = NETWORK_BUF_MAX_SIZE_2K;
              }

              if ((retval = recv(s, g_tcp_buf, received_size) <= 0))
              {
                PRINT_DBG("Receive error : %d\r\n", retval);

                return retval;
              }

              PRINT_DBG("Receive[%03d] OK : %d\r\n", received_num + 1, retval);

              /* Check socket n interrupt register & Tx read pointer register & Tx write pointer register */
              sn_ir_flag = getSn_IR(s);
              sn_rx_rd_flag = getSn_RX_RD(s);
              sn_rx_wr_flag = getSn_RX_WR(s);

              if (!(sn_ir_flag == SIK_RECEIVED))
              {
                PRINT_DBG("Sn_IR[SIK_RECEIVED] error : 0x%02x\r\n", sn_ir_flag);

                setSn_IR(s, SIK_ALL);

                return TCP_TEST_FAIL;
              }

              if (!(sn_rx_rd_flag == sn_rx_wr_flag))
              {
                PRINT_DBG("Sn_RX_RD error : 0x%02x\r\n", sn_rx_rd_flag);
                PRINT_DBG("Sn_RX_WR error : 0x%02x\r\n", sn_rx_wr_flag);

                return TCP_TEST_FAIL;
              }

              PRINT_DBG("Sn_IR[SIK_RECEIVED] : 0x%02x\r\n", sn_ir_flag);
              PRINT_DBG("Sn_RX_RD : 0x%02x\r\n", sn_rx_rd_flag);
              PRINT_DBG("Sn_RX_WR : 0x%02x\r\n", sn_rx_wr_flag);

              /* Check socket n interrupt register */
              //setSn_IR(s, SIK_RECEIVED);
              ctlsocket(s, CS_CLR_INTERRUPT, (void *)SIK_RECEIVED);

              received_num++;
            }
          } while (received_num < send_receive_num);

          if ((retval = disconnect(s)) != SOCK_OK)
          {
            PRINT_DBG("Disconnect error : %d\r\n", retval);

            return retval;
          }

          /* Check socket n interrupt register */
          sn_ir_flag = getSn_IR(s);

          if (!(sn_ir_flag == SIK_DISCONNECTED))
          {
            PRINT_DBG("Sn_IR[SIK_DISCONNECTED] error : 0x%02x\r\n", sn_ir_flag);

            setSn_IR(s, SIK_ALL);

            return TCP_TEST_FAIL;
          }

          PRINT_DBG("Sn_IR[SIK_DISCONNECTED] : 0x%02x\r\n", sn_ir_flag);

          /* Clear socket n interrupt register */
          setSn_IR(s, SIK_DISCONNECTED);

          return TCP_TEST_SUCCESS;
        }
        else
        {
          ;
        }
      }
      break;

      case SOCK_CLOSE_WAIT:
      {
        if ((retval = disconnect(s)) != SOCK_OK)
        {
          PRINT_DBG("Disconnect error : %d\r\n", retval);

          return retval;
        }

        PRINT_DBG("Disconnect OK : %d\r\n", retval);
      }
      break;

      case SOCK_INIT:
      {
        if (ipv == AS_IPV4)
        {
          if ((retval = connect(s, device_config->network_info.remote_ipv4, device_config->network_info.remote_port, sizeof(device_config->network_info.remote_ipv4))) != SOCK_OK)
          {
            PRINT_DBG("Connect error : %d\r\n", retval);

            return retval;
          }

          /* Check socket n interrupt register */
          sn_ir_flag = getSn_IR(s);

          if (!(sn_ir_flag == SIK_CONNECTED))
          {
            PRINT_DBG("Sn_IR[SIK_CONNECTED] error : 0x%02x\r\n", sn_ir_flag);

            setSn_IR(s, SIK_ALL);

            return TCP_TEST_FAIL;
          }

          PRINT_DBG("Sn_IR[SIK_CONNECTED] : 0x%02x\r\n", sn_ir_flag);

          /* Clear socket n interrupt register */
          setSn_IR(s, SIK_CONNECTED);
        }
        else if (ipv == AS_IPV6)
        {
          if ((retval = connect(s, device_config->network_info.remote_ipv6, device_config->network_info.remote_port, sizeof(device_config->network_info.remote_ipv6))) != SOCK_OK)
          {
            PRINT_DBG("Connect error : %d\r\n", retval);

            return retval;
          }

          /* Check socket n interrupt register */
          sn_ir_flag = getSn_IR(s);

          if (!(sn_ir_flag == SIK_CONNECTED))
          {
            PRINT_DBG("Sn_IR[SIK_CONNECTED] error : 0x%02x\r\n", sn_ir_flag);

            setSn_IR(s, SIK_ALL);

            return TCP_TEST_FAIL;
          }

          PRINT_DBG("Sn_IR[SIK_CONNECTED] : 0x%02x\r\n", sn_ir_flag);

          /* Clear socket n interrupt register */
          setSn_IR(s, SIK_CONNECTED);
        }
        else if (ipv == AS_IPDUAL)
        {
#if 1
          if ((retval = connect(s, device_config->network_info.remote_ipv4, device_config->network_info.remote_port, sizeof(device_config->network_info.remote_ipv4))) != SOCK_OK)
          {
            PRINT_DBG("Connect error : %d\r\n", retval);

            return retval;
          }
#else
          if ((retval = connect(s, device_config->network_info.remote_ipv6, device_config->network_info.remote_port, sizeof(device_config->network_info.remote_ipv6))) != SOCK_OK)
          {
            PRINT_DBG("Connect error : %d\r\n", retval);

            return retval;
          }
#endif

          /* Check socket n interrupt register */
          sn_ir_flag = getSn_IR(s);

          if (!(sn_ir_flag == SIK_CONNECTED))
          {
            PRINT_DBG("Sn_IR[SIK_CONNECTED] error : 0x%02x\r\n", sn_ir_flag);

            setSn_IR(s, SIK_ALL);

            return TCP_TEST_FAIL;
          }

          PRINT_DBG("Sn_IR[SIK_CONNECTED] : 0x%02x\r\n", sn_ir_flag);

          /* Clear socket n interrupt register */
          setSn_IR(s, SIK_CONNECTED);
        }
        else
        {
          ;
        }

        PRINT_DBG("Connect OK : %d\r\n", retval);
      }
      break;

      case SOCK_CLOSED:
      {
        close(s);

        if (ipv == AS_IPV4)
        {
          /* Set maximum segment size */
          setSn_MSSR(s, maximum_segment_size);
          PRINT_DBG("Socket %d maximum segment size : %d\r\n", s, getSn_MSSR(s));

          if ((retval = socket(s, Sn_MR_TCP4, any_port++, 0x00)) != s)
          {
            PRINT_DBG("Socket error : %d\r\n", retval);

            return retval;
          }
        }
        else if (ipv == AS_IPV6)
        {
          /* Set maximum segment size */
          setSn_MSSR(s, maximum_segment_size);
          PRINT_DBG("Socket %d maximum segment size : %d\r\n", s, getSn_MSSR(s));

          if ((retval = socket(s, Sn_MR_TCP6, any_port++, 0x00)) != s)
          {
            PRINT_DBG("Socket error : %d\r\n", retval);

            return retval;
          }
        }
        else if (ipv == AS_IPDUAL)
        {
          /* Set maximum segment size */
          setSn_MSSR(s, maximum_segment_size);
          PRINT_DBG("Socket %d maximum segment size : %d\r\n", s, getSn_MSSR(s));

          if ((retval = socket(s, Sn_MR_TCPD, any_port++, 0x00)) != s)
          {
            PRINT_DBG("Socket error : %d\r\n", retval);

            if (any_port == 0xffff)
            {
              any_port = ANY_PORT;
            }

            return retval;
          }
        }
        else
        {
          ;
        }

        PRINT_DBG("Socket OK : %d\r\n", retval);
      }
      break;

      default:
        break;
    }
  }
}

int8_t tcp_connect_disconnect_test(device_config_t *device_config, uint8_t *rx_data)
{
  int retval = 0;
  int i = 0;
  uint8_t ipv = 0;
  uint8_t s = 0;
  uint8_t sn_ir_flag = 0;
  uint16_t sn_tx_rd_flag = 0;
  uint16_t sn_tx_wr_flag = 0;
  uint16_t any_port = ANY_PORT;

  memset(g_tcp_buf, 0, sizeof(g_tcp_buf));

  /* Parse IP version */
  ipv = atoi((const char *)rx_data);

  /* Check IP version */
  if (ipv == 0) // IPv4
  {
    ipv = AS_IPV4;
  }
  else if (ipv == 1) // IPv6
  {
    ipv = AS_IPV6;
  }
  else if (ipv == 2) // Dual
  {
    ipv = AS_IPDUAL;
  }
  else
  {
    PRINT_DBG("IP version setting error : %d\r\n", ipv);

    return TCP_TEST_PARAM_INVALID;
  }

  PRINT_DBG("IP version : %d\r\n", ipv);

  /* Parse socket number */
  rx_data = (unsigned char *)strchr((const char *)rx_data, (int)',');
  rx_data += 2;
  s = atoi((const char *)rx_data);

  /* Check socket number */
  if ((s < 0) || (s > 7)) // 0 <= socket number <= 7
  {
    PRINT_DBG("Socket number setting error : %d\r\n", s);

    return TCP_TEST_PARAM_INVALID;
  }

  PRINT_DBG("Socket number : %d\r\n", s);

  while (1)
  {
    switch (getSn_SR(s))
    {
      case SOCK_ESTABLISHED:
      {
        /* Make test data */
        g_tcp_buf_len = NETWORK_BUF_MAX_SIZE_2K;

        for (i = 0; i < g_tcp_buf_len; i++)
        {
          g_tcp_buf[i] = 0xff;
        }

#ifdef USE_TCP_DEBUG
        for (i = 0; i < g_tcp_buf_len; i++)
        {
          printf("0x%02x ", g_tcp_buf[i]);

          if (((i + 1) % 16) == 0)
          {
            printf("\r\n");
          }
        }
#endif /* USE_TCP_DEBUG */

        if ((retval = send(s, g_tcp_buf, g_tcp_buf_len)) < 0)
        {
          PRINT_DBG("Send error : %d\r\n", retval);

          return retval;
        }

        PRINT_DBG("Send OK : %d\r\n", retval);

        /* Check socket n interrupt register & Tx read pointer register & Tx write pointer register */
        sn_ir_flag = getSn_IR(s);
        sn_tx_rd_flag = getSn_TX_RD(s);
        sn_tx_wr_flag = getSn_TX_WR(s);

        if (!(sn_ir_flag == SIK_SENT))
        {
          PRINT_DBG("Sn_IR[SIK_SENT] error : 0x%02x\r\n", sn_ir_flag);

          setSn_IR(s, SIK_ALL);

          return TCP_TEST_FAIL;
        }

        if (!(sn_tx_rd_flag == sn_tx_wr_flag))
        {
          PRINT_DBG("Sn_TX_RD error : 0x%02x\r\n", sn_tx_rd_flag);
          PRINT_DBG("Sn_TX_WR error : 0x%02x\r\n", sn_tx_wr_flag);

          return TCP_TEST_FAIL;
        }

        PRINT_DBG("Sn_IR[SIK_SENT] : 0x%02x\r\n", sn_ir_flag);
        PRINT_DBG("Sn_TX_RD : 0x%02x\r\n", sn_tx_rd_flag);
        PRINT_DBG("Sn_TX_WR : 0x%02x\r\n", sn_tx_wr_flag);

        /* Clear socket n interrupt register */
        setSn_IR(s, SIK_SENT);

        if ((retval = disconnect(s)) != SOCK_OK)
        {
          PRINT_DBG("Disconnect error : %d\r\n", retval);

          return retval;
        }

        /* Check socket n interrupt register */
        sn_ir_flag = getSn_IR(s);

        if (!(sn_ir_flag == SIK_DISCONNECTED))
        {
          PRINT_DBG("Sn_IR[SIK_DISCONNECTED] error : 0x%02x\r\n", sn_ir_flag);

          setSn_IR(s, SIK_ALL);

          return TCP_TEST_FAIL;
        }

        PRINT_DBG("Sn_IR[SIK_DISCONNECTED] : 0x%02x\r\n", sn_ir_flag);

        /* Clear socket n interrupt register */
        setSn_IR(s, SIK_DISCONNECTED);

        return TCP_TEST_SUCCESS;
      }
      break;

      case SOCK_CLOSE_WAIT:
      {
        if ((retval = disconnect(s)) != SOCK_OK)
        {
          PRINT_DBG("Disconnect error : %d\r\n", retval);

          return retval;
        }

        PRINT_DBG("Disconnect OK : %d\r\n", retval);
      }
      break;

      case SOCK_INIT:
      {
        if (ipv == AS_IPV4)
        {
          if ((retval = connect(s, device_config->network_info.remote_ipv4, device_config->network_info.remote_port, sizeof(device_config->network_info.remote_ipv4))) != SOCK_OK)
          {
            PRINT_DBG("Connect error : %d\r\n", retval);

            return retval;
          }

          /* Check socket n interrupt register */
          sn_ir_flag = getSn_IR(s);

          if (!(sn_ir_flag == SIK_CONNECTED))
          {
            PRINT_DBG("Sn_IR[SIK_CONNECTED] error : 0x%02x\r\n", sn_ir_flag);

            setSn_IR(s, SIK_ALL);

            return TCP_TEST_FAIL;
          }

          PRINT_DBG("Sn_IR[SIK_CONNECTED] : 0x%02x\r\n", sn_ir_flag);

          /* Clear socket n interrupt register */
          setSn_IR(s, SIK_CONNECTED);
        }
        else if (ipv == AS_IPV6)
        {
          if ((retval = connect(s, device_config->network_info.remote_ipv6, device_config->network_info.remote_port, sizeof(device_config->network_info.remote_ipv6))) != SOCK_OK)
          {
            PRINT_DBG("Connect error : %d\r\n", retval);

            return retval;
          }

          /* Check socket n interrupt register */
          sn_ir_flag = getSn_IR(s);

          if (!(sn_ir_flag == SIK_CONNECTED))
          {
            PRINT_DBG("Sn_IR[SIK_CONNECTED] error : 0x%02x\r\n", sn_ir_flag);

            setSn_IR(s, SIK_ALL);

            return TCP_TEST_FAIL;
          }

          PRINT_DBG("Sn_IR[SIK_CONNECTED] : 0x%02x\r\n", sn_ir_flag);

          /* Clear socket n interrupt register */
          setSn_IR(s, SIK_CONNECTED);
        }
        else if (ipv == AS_IPDUAL)
        {
#if 1
          if ((retval = connect(s, device_config->network_info.remote_ipv4, device_config->network_info.remote_port, sizeof(device_config->network_info.remote_ipv4))) != SOCK_OK)
          {
            PRINT_DBG("Connect error : %d\r\n", retval);

            return retval;
          }
#else
          if ((retval = connect(s, device_config->network_info.remote_ipv6, device_config->network_info.remote_port, sizeof(device_config->network_info.remote_ipv6))) != SOCK_OK)
          {
            PRINT_DBG("Connect error : %d\r\n", retval);

            return retval;
          }
#endif

          /* Check socket n interrupt register */
          sn_ir_flag = getSn_IR(s);

          if (!(sn_ir_flag == SIK_CONNECTED))
          {
            PRINT_DBG("Sn_IR[SIK_CONNECTED] error : 0x%02x\r\n", sn_ir_flag);

            setSn_IR(s, SIK_ALL);

            return TCP_TEST_FAIL;
          }

          PRINT_DBG("Sn_IR[SIK_CONNECTED] : 0x%02x\r\n", sn_ir_flag);

          /* Clear socket n interrupt register */
          setSn_IR(s, SIK_CONNECTED);
        }
        else
        {
          ;
        }

        PRINT_DBG("Connect OK : %d\r\n", retval);
      }
      break;

      case SOCK_CLOSED:
      {
        close(s);

        if (ipv == AS_IPV4)
        {
          if ((retval = socket(s, Sn_MR_TCP4, any_port++, 0x00)) != s)
          {
            PRINT_DBG("Socket error : %d\r\n", retval);

            return retval;
          }
        }
        else if (ipv == AS_IPV6)
        {
          if ((retval = socket(s, Sn_MR_TCP6, any_port++, 0x00)) != s)
          {
            PRINT_DBG("Socket error : %d\r\n", retval);

            return retval;
          }
        }
        else if (ipv == AS_IPDUAL)
        {
          if ((retval = socket(s, Sn_MR_TCPD, any_port++, 0x00)) != s)
          {
            PRINT_DBG("Socket error : %d\r\n", retval);

            if (any_port == 0xffff)
            {
              any_port = ANY_PORT;
            }

            return retval;
          }
        }
        else
        {
          ;
        }

        PRINT_DBG("Socket OK : %d\r\n", retval);
      }
      break;

      default:
        break;
    }
  }
}

int8_t tcp_psh_flag_test(device_config_t *device_config, uint8_t *rx_data)
{
  return TCP_TEST_SUCCESS;
}

int8_t tcp_ack_packet_test(device_config_t *device_config, uint8_t *rx_data)
{
  return TCP_TEST_SUCCESS;
}

int8_t tcp_rst_packet_test(device_config_t *device_config, uint8_t *rx_data)
{
  return TCP_TEST_SUCCESS;
}

int8_t tcp_extension_status_register_test(device_config_t *device_config, uint8_t *rx_data)
{
  return TCP_TEST_SUCCESS;
}

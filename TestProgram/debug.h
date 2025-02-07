/**
 * Copyright (c) 2023 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _DEBUG_H_
#define _DEBUG_H_

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
#define USE_PRINT_INF   // uncomment to enable infomation messages for query command.
#define USE_PRINT_RES   // uncomment to enable result messages.
#define USE_PRINT_DBG   // uncomment to enable debug messages.
//#define USE_DETAIL_INFO // uncomment to enable detail messages.

#ifdef USE_PRINT_INF
#define _PRINT_INF(fmt, ...) \
  printf("[INF] " fmt, ##__VA_ARGS__)
#endif /* USE_PRINT_INF */

#ifdef USE_PRINT_RES
#define _PRINT_RES(fmt, ...) \
  printf("[RES] " fmt, ##__VA_ARGS__)
#endif /* USE_PRINT_RES */

#ifdef USE_PRINT_DBG
#ifdef USE_DETAIL_INFO
#define _PRINT_DBG(fmt, ...) \
  printf("[DBG] %s, %s(#%d), " fmt, __func__, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define _PRINT_DBG(fmt, ...) \
  printf("[DBG] " fmt, ##__VA_ARGS__)
#endif /* USE_DETAIL_INFO */
#endif /* USE_PRINT_DBG */

#define PRINT_INF(f, a...) _PRINT_INF(f, ##a)
#define PRINT_RES(f, a...) _PRINT_RES(f, ##a)
#define PRINT_DBG(f, a...) _PRINT_DBG(f, ##a)

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

#ifdef __cplusplus
}
#endif

#endif /* _DEBUG_H_ */

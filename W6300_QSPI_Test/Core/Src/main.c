/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : W6300 BUS(FMC 8-bit indirect) + MACRAW 테스트
  *                   NUCLEO-H723ZG + WIZ630MJ, 디버그 UART = USART3(ST-LINK VCP) 115200
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "wizchip_conf.h"
#include "socket.h"
#include "wizchip_init.h"
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MACRAW_ARP_TARGET_IP   {192, 168, 11, 42}   /* MACRAW 테스트에서 ARP 요청을 보낼 상대(PC) IP */
/* USER CODE END PD */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart3;    /* 디버그 UART (PD8/PD9 → ST-LINK VCP) */
SRAM_HandleTypeDef hsram1;    /* FMC Bank3 (0x68000000) ← W6300 BUS */

/* USER CODE BEGIN PV */
/* 장비 네트워크 설정. MACRAW 는 MAC(SHAR)만 쓰고, ARP 테스트 프레임의 송신 IP 로 SIPR 을 사용 */
wiz_NetInfo gWIZNETINFO = {.mac = {0x00, 0x08, 0xdc, 0xa3, 0xb4, 0xc5},
                           .ip = {192, 168, 11, 99},
                           .sn = {255, 255, 255, 0},
                           .gw = {192, 168, 11, 1},
                           .dns = {8, 8, 8, 8},
                           .lla = {0xfe, 0x80, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00,
                                   0x02, 0x08, 0xdc, 0xff,
                                   0xfe, 0xff, 0xff, 0xff},
                           .gua = {0x20, 0x01, 0x02, 0xb8,
                                   0x00, 0x10, 0x00, 0x01,
                                   0x02, 0x08, 0xdc, 0xff,
                                   0xfe, 0xff, 0xff, 0xff},
                           .sn6 = {0xff, 0xff, 0xff, 0xff,
                                   0xff, 0xff, 0xff, 0xff,
                                   0x00, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00},
                           .gw6 = {0xfe, 0x80, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00,
                                   0x02, 0x00, 0x87, 0xff,
                                   0xfe, 0x08, 0x4c, 0x81}};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_FMC_Init(void);
static void MX_USART3_UART_Init(void);
static void MPU_Config_FMC_Region(void);

/* USER CODE BEGIN 0 */

/* ===== MACRAW 테스트 =====
   소켓0을 MACRAW로 열고
     (1) Sn_SR == SOCK_MACRAW(0x42) 인지 확인
     (2) 1초마다 MACRAW_ARP_TARGET_IP 로 ARP 요청 프레임 송신 (브로드캐스트)
     (3) 수신 프레임을 dst/src MAC, EtherType, 길이만 출력. ARP 응답이 오면 TX/RX 모두 통과.
   데이터 경로는 wiz_send_data / wiz_recv_data + Sn_CR 명령을 직접 써서
   "2바이트 길이헤더 읽기 → RECV 명령" 순서를 그대로 보여준다. */
static uint16_t macraw_build_arp_req(uint8_t *f, const uint8_t *my_mac, const uint8_t *my_ip, const uint8_t *tgt_ip)
{
  uint16_t i = 0;
  memset(f + i, 0xFF, 6);        i += 6;   /* dst MAC = broadcast */
  memcpy(f + i, my_mac, 6);      i += 6;   /* src MAC */
  f[i++] = 0x08; f[i++] = 0x06;            /* EtherType = ARP */
  f[i++] = 0x00; f[i++] = 0x01;            /* HTYPE = Ethernet */
  f[i++] = 0x08; f[i++] = 0x00;            /* PTYPE = IPv4 */
  f[i++] = 6;    f[i++] = 4;               /* HLEN, PLEN */
  f[i++] = 0x00; f[i++] = 0x01;            /* OPER = request */
  memcpy(f + i, my_mac, 6);      i += 6;   /* SHA */
  memcpy(f + i, my_ip, 4);       i += 4;   /* SPA */
  memset(f + i, 0x00, 6);        i += 6;   /* THA */
  memcpy(f + i, tgt_ip, 4);      i += 4;   /* TPA */
  while (i < 60) f[i++] = 0;               /* 최소 프레임 길이(60, CRC 제외)까지 패딩. CRC는 칩이 붙임 */
  return i;
}

static void macraw_test(uint8_t sn)
{
  static uint8_t frame[1536];
  uint8_t  my_mac[6], my_ip[4], hdr[2], st;
  const uint8_t tgt_ip[4] = MACRAW_ARP_TARGET_IP;
  uint16_t rsr, flen, len, etype;
  uint32_t rx_cnt = 0, tx_cnt = 0, last_tx = 0, t;

  getSHAR(my_mac);
  getSIPR(my_ip);
  printf("\r\n===== MACRAW TEST (IF=BUS) =====\r\n");
  printf("  CIDR=0x%04x VER=0x%04x  MAC=%02x:%02x:%02x:%02x:%02x:%02x  IP=%d.%d.%d.%d\r\n",
         getCIDR(), getVER(),
         my_mac[0], my_mac[1], my_mac[2], my_mac[3], my_mac[4], my_mac[5],
         my_ip[0], my_ip[1], my_ip[2], my_ip[3]);

  /* (1) 소켓0 MACRAW 오픈. flag=Sn_MR_MF(=SF_ETHER_OWN): 자기/브로드캐스트/멀티캐스트 프레임만 수신.
         모든 프레임(promiscuous)을 보려면 flag 0. */
  close(sn);
  if (socket(sn, Sn_MR_MACRAW, 0, Sn_MR_MF) != sn) { printf("  socket(MACRAW) FAIL\r\n"); return; }
  st = getSn_SR(sn);
  printf("  Sn_MR=0x%02x Sn_SR=0x%02x -> %s\r\n", getSn_MR(sn), st,
         (st == SOCK_MACRAW) ? "SOCK_MACRAW OK" : "NOT MACRAW (FAIL)");
  if (st != SOCK_MACRAW) return;

  while (1)
  {
    /* (2) 1초마다 ARP 요청 송신 */
    if (HAL_GetTick() - last_tx >= 1000)
    {
      last_tx = HAL_GetTick();
      len = macraw_build_arp_req(frame, my_mac, my_ip, tgt_ip);
      if (getSn_TX_FSR(sn) >= len)
      {
        wiz_send_data(sn, frame, len);
        setSn_CR(sn, Sn_CR_SEND);
        while (getSn_CR(sn));
        t = HAL_GetTick();
        while (!(getSn_IR(sn) & (Sn_IR_SENDOK | Sn_IR_TIMEOUT)))
          if (HAL_GetTick() - t > 100) break;                     /* 100ms 안에 SENDOK 없으면 실패 */
        if (getSn_IR(sn) & Sn_IR_SENDOK)
        {
          setSn_IR(sn, Sn_IR_SENDOK);
          tx_cnt++;
          printf("TX#%lu ARP who-has %d.%d.%d.%d (%u bytes)\r\n",
                 (unsigned long)tx_cnt, tgt_ip[0], tgt_ip[1], tgt_ip[2], tgt_ip[3], (unsigned)len);
        }
        else
        {
          setSn_IR(sn, Sn_IR_TIMEOUT);
          printf("TX FAIL: no SENDOK (Sn_IR=0x%02x)\r\n", getSn_IR(sn));
        }
      }
    }

    /* (3) 수신: [2바이트 길이(헤더 자신 포함)] + 프레임. 각 읽기 뒤 RECV 명령. */
    rsr = getSn_RX_RSR(sn);
    if (rsr < 2) continue;

    wiz_recv_data(sn, hdr, 2);
    setSn_CR(sn, Sn_CR_RECV);
    while (getSn_CR(sn));
    flen = (uint16_t)(((hdr[0] & 0x07) << 8) | hdr[1]);   /* 라이브러리와 동일하게 상위 5비트는 정보 비트로 취급 */
    if (flen < 16 || (flen - 2) > sizeof(frame))
    {
      /* 길이 헤더가 깨짐 = 인터페이스 읽기 경로(주소 시퀀스/auto-increment) 의심. 테스트 중단 */
      printf("RX FAIL: bad length header 0x%02x%02x (rsr=%u) -> check interface read path\r\n", hdr[0], hdr[1], (unsigned)rsr);
      return;
    }
    flen -= 2;
    wiz_recv_data(sn, frame, flen);
    setSn_CR(sn, Sn_CR_RECV);
    while (getSn_CR(sn));
    rx_cnt++;

    etype = (uint16_t)((frame[12] << 8) | frame[13]);
    printf("RX#%lu len=%u dst=%02x:%02x:%02x:%02x:%02x:%02x src=%02x:%02x:%02x:%02x:%02x:%02x type=0x%04x",
           (unsigned long)rx_cnt, (unsigned)flen,
           frame[0], frame[1], frame[2], frame[3], frame[4], frame[5],
           frame[6], frame[7], frame[8], frame[9], frame[10], frame[11], etype);
    /* ARP 응답(OPER=2)이고 TPA가 내 IP면 우리가 보낸 요청에 대한 답 → TX/RX 왕복 확인 */
    if (etype == 0x0806 && flen >= 42 && frame[20] == 0 && frame[21] == 2 && memcmp(frame + 38, my_ip, 4) == 0)
      printf("  >> ARP REPLY %d.%d.%d.%d is-at %02x:%02x:%02x:%02x:%02x:%02x -> MACRAW TX/RX OK",
             frame[28], frame[29], frame[30], frame[31],
             frame[22], frame[23], frame[24], frame[25], frame[26], frame[27]);
    printf("\r\n");
  }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* MCU Configuration--------------------------------------------------------*/
  HAL_Init();
  SystemClock_Config();
  PeriphCommonClock_Config();

  MX_GPIO_Init();
  MX_USART3_UART_Init();

  /* USER CODE BEGIN 2 */
  printf("\r\n===== W6300 BUS + MACRAW TEST (NUCLEO-H723ZG + WIZ630MJ) =====\r\n");
  printf("Compile %s %s\r\n", __DATE__, __TIME__);
  printf("SYSCLK=%lu  PCLK1(USART3)=%lu\r\n", HAL_RCC_GetSysClockFreq(), HAL_RCC_GetPCLK1Freq());

  /* W6300 리셋 → FMC(BUS) 구성 → 칩 초기화. 리셋 중엔 FMC 핀을 물리지 않도록 리셋을 먼저 한다.
     ※ W6300 MODE 핀은 WIZ630MJ 점퍼로 BUS(MODE0=HIGH)에 맞춰 둘 것. 펌웨어는 MODE 핀을 읽지 않음. */
  chip_hw_reset();
  MPU_Config_FMC_Region();
  MX_FMC_Init();

  W6300Initialze();                            /* BUS 콜백 등록 → CIDR 확인 → PHY 링크 대기 → 버퍼/인터럽트 설정 */
  ctlnetwork(CN_SET_NETINFO, &gWIZNETINFO);    /* MAC/IP 설정 */

  macraw_test(0);                              /* 정상이면 리턴하지 않음 */
  printf("MACRAW test stopped.\r\n");
  /* USER CODE END 2 */

  while (1)
  {
  }
}

/* printf → USART3 */
int _write(int fd, char *str, int len)
{
  (void)fd;
  for (int i = 0; i < len; i++)
    HAL_UART_Transmit(&huart3, (uint8_t *)&str[i], 1, 0xFFFF);
  return len;
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  * NUCLEO-H723ZG: HSE = ST-LINK MCO 8MHz (BYPASS). HSE_VALUE(stm32h7xx_hal_conf.h) 도 8MHz 여야 함.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;                // 8/1 = 8MHz (PLL 입력)
  RCC_OscInitStruct.PLL.PLLN = 60;               // 8*60 = 480MHz (VCO)
  RCC_OscInitStruct.PLL.PLLP = 1;                // 480MHz SYSCLK
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;  // 입력 8MHz → RANGE_3 (8~16MHz)
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;  // 480 ∈ [192,836]
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration (FMC 커널 클럭 = PLL2R)
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /* PLL2: 8MHz / M5 * N120 = 192MHz VCO, /R10 = 19.2MHz → FMC 커널 클럭 */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_FMC;
  PeriphClkInitStruct.PLL2.PLL2M = 5;
  PeriphClkInitStruct.PLL2.PLL2N = 120;
  PeriphClkInitStruct.PLL2.PLL2P = 5;
  PeriphClkInitStruct.PLL2.PLL2Q = 4;
  PeriphClkInitStruct.PLL2.PLL2R = 10;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_2;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
  PeriphClkInitStruct.FmcClockSelection = RCC_FMCCLKSOURCE_PLL2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief FMC Bank3(0x68000000) 영역을 MPU 로 Strongly-Ordered(비캐시/비버퍼/공유) 설정.
  *        W6300 인다이렉트 접근(주소 레지스터 3바이트 → 데이터 레지스터)은 쓰기 순서가 지켜져야 하므로 필수.
  */
static void MPU_Config_FMC_Region(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct;

  HAL_MPU_Disable();

  MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
  MPU_InitStruct.Number           = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress      = 0x68000000;
  MPU_InitStruct.Size             = MPU_REGION_SIZE_64KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/* FMC initialization function — W6300 BUS: Bank3(NE3=PG6), 8-bit, 커널 19.2MHz */
static void MX_FMC_Init(void)
{
  FMC_NORSRAM_TimingTypeDef Timing = {0};

  hsram1.Instance = FMC_NORSRAM_DEVICE;
  hsram1.Extended = FMC_NORSRAM_EXTENDED_DEVICE;
  /* hsram1.Init */
  hsram1.Init.NSBank = FMC_NORSRAM_BANK3;
  hsram1.Init.DataAddressMux = FMC_DATA_ADDRESS_MUX_DISABLE;
  hsram1.Init.MemoryType = FMC_MEMORY_TYPE_SRAM;
  hsram1.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_8;
  hsram1.Init.BurstAccessMode = FMC_BURST_ACCESS_MODE_ENABLE;
  hsram1.Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW;
  hsram1.Init.WaitSignalActive = FMC_WAIT_TIMING_BEFORE_WS;
  hsram1.Init.WriteOperation = FMC_WRITE_OPERATION_ENABLE;
  hsram1.Init.WaitSignal = FMC_WAIT_SIGNAL_ENABLE;
  hsram1.Init.ExtendedMode = FMC_EXTENDED_MODE_DISABLE;
  hsram1.Init.AsynchronousWait = FMC_ASYNCHRONOUS_WAIT_DISABLE;
  hsram1.Init.WriteBurst = FMC_WRITE_BURST_ENABLE;
  hsram1.Init.ContinuousClock = FMC_CONTINUOUS_CLOCK_SYNC_ONLY;
  hsram1.Init.WriteFifo = FMC_WRITE_FIFO_DISABLE;
  hsram1.Init.PageSize = FMC_PAGE_SIZE_NONE;
  /* Timing (FMC 커널 클럭 사이클 단위) */
  Timing.AddressSetupTime = 4;
  Timing.AddressHoldTime = 15;
  Timing.DataSetupTime = 2;
  Timing.BusTurnAroundDuration = 1;
  Timing.CLKDivision = 16;
  Timing.DataLatency = 0;
  Timing.AccessMode = FMC_ACCESS_MODE_A;

  if (HAL_SRAM_Init(&hsram1, &Timing, NULL) != HAL_OK)
  {
    Error_Handler( );
  }
}

/* USART3 (PD8=TX, PD9=RX → ST-LINK VCP) 115200 8N1 */
static void MX_USART3_UART_Init(void)
{
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK) { Error_Handler(); }
}

/**
  * @brief GPIO Initialization Function
  *        FMC 핀(D0~D7, A0/A1, NOE/NWE, NE3)은 HAL_SRAM_MspInit(stm32h7xx_hal_msp.c)에서 설정.
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /* 초기 출력 레벨. RSTn=0 → W6300 리셋 유지 (chip_hw_reset()에서 해제) */
  HAL_GPIO_WritePin(GPIOE, MOD4_Pin|MOD5_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOF, RSTn_Pin|Trace_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(SPI_EN_GPIO_Port, SPI_EN_Pin, GPIO_PIN_RESET);

  /* MOD4/MOD5 (PE2/PE4) 출력 LOW */
  GPIO_InitStruct.Pin = MOD4_Pin|MOD5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /* IRQ (PF3) — W6300 INTn, EXTI falling (본 테스트는 폴링만 사용) */
  GPIO_InitStruct.Pin = IRQ_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(IRQ_GPIO_Port, &GPIO_InitStruct);

  /* RSTn (PF4) — W6300 리셋 */
  GPIO_InitStruct.Pin = RSTn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(RSTn_GPIO_Port, &GPIO_InitStruct);

  /* SPI_EN (PC4), Trace (PF15) 출력 LOW */
  GPIO_InitStruct.Pin = SPI_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(SPI_EN_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = Trace_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(Trace_GPIO_Port, &GPIO_InitStruct);

  /* MOD0~MOD3 (PC0, PD3, PG2, PG3) — W6300 MODE 핀 모니터용 입력.
     실제 모드는 WIZ630MJ 점퍼가 정함 (BUS = MODE0 HIGH). 펌웨어는 읽지 않음. */
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Pin = MOD0_Pin;
  HAL_GPIO_Init(MOD0_GPIO_Port, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = MOD1_Pin;
  HAL_GPIO_Init(MOD1_GPIO_Port, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = MOD2_Pin|MOD3_Pin;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  /* USART3: PD8=TX, PD9=RX (AF7) */
  __HAL_RCC_USART3_CLK_ENABLE();
  GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  __disable_irq();
  printf("Error_Handler\r\n");
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

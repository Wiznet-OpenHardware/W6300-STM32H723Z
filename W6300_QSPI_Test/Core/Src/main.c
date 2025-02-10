/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "loopback.h"
#include "wizchip_conf.h"
#include "socket.h"
#include "wizchip_init.h"
#include "loopback.h"

#include "iperf/cJSON.h" // JSON handling library
#include "iperf/iperf.h" // iperf test library


#define TCPS_EN    1
#define NDA        1
#define RTLVERSiON 227
#define MAIN_CLK_100MHZ 0
#define ETHERNET_BUF_MAX_SIZE (1024 * 16)

//#define soketFLagLihan_TEST 0x00
#define SOCKET_NO_DELAY_ACK 0x20
static uint8_t g_udp_buf_main[ETHERNET_BUF_MAX_SIZE] = {
    0,
};

PLL2_ClocksTypeDef temp_PLL2_Clk_data;

#if 1
//#define FPGA_USED
#endif

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

MDMA_HandleTypeDef hmdma_octospi1_fifo_th;

SPI_HandleTypeDef hspi2;

UART_HandleTypeDef huart2;

NOR_HandleTypeDef hnor1;

/* USER CODE BEGIN PV */
//uint8_t W6300_mode = QSPI_MODE;//0; //W6100 >> 0xFF
wiz_NetInfo gWIZNETINFO = {.mac = {0x00, 0x08, 0xdc, 0xa3, 0xb4, 0xc5},
                           .ip = {192, 168, 11, 44},
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

uint8_t WIZ_Dest_IP[4] = {192, 168, 11, 2};                  //DST_IP Address

uint8_t DestIP6_L[16] = {	0xfe,0x80,0x00,0x00,
							0x00,0x00,0x00,0x00,
							0x46,0x6e,0xa9,0x69,
							0x96,0x8a,0x68,0xbd
						};

uint8_t DestIP6_G[16] = {0x20,0x01,0x02,0xb8,
                          0x00,0x10,0x00,0x01,
                          0x31,0x71,0x98,0x05,
                          0x70,0x24,0x4b,0xb1
                         };

uint8_t Router_IP[16]= {0xff,0x02,0x00,0x00,
                          0x00,0x00,0x00,0x00,
                          0x00,0x00,0x00,0x00,
                          0x00,0x00,0x00,0x02
                         };

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_MDMA_Init(void);
static void MX_OCTOSPI1_Init(void);
static void MX_FMC_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_SPI2_Init(void);
/* USER CODE BEGIN PFP */
void print_network_information(void);
/* USER CODE END PFP */
char SPI_CLK_SET(uint16_t set_clk_data);

/* Private user code ---------------------------------------------------------*/

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

volatile uint8_t rxData;

#define SOCKET_IPERF 0

/* Port */
#define PORT_IPERF 5007
#define MAX_RESULT_LEN 1024   
#define SOCKET_CTRL 0
#define SOCKET_DATA 1
/* Cookie size */
#define COOKIE_SIZE 37

/* iperf3 Commands */
#define PARAM_EXCHANGE 9
#define CREATE_STREAMS 10
#define TEST_START 1
#define TEST_RUNNING 2
#define TEST_END 4
#define EXCHANGE_RESULTS 13
#define DISPLAY_RESULTS 14
#define IPERF_DONE 16

static uint8_t g_iperf_buf[ETHERNET_BUF_MAX_SIZE * 2] = {
    0,
};

static uint8_t cookie[COOKIE_SIZE] = {0};

//uint8_t reg_WR_buf_Test(uint8_t op_code, uint16_t reg_addr, uint16_t len);
//uint8_t reg_WR_return_data(uint8_t op_code, uint16_t reg_addr, uint16_t len, uint8_t *tx, uint8_t *rx);

/* USER CODE END PFP */
/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */

void handle_param_exchange(uint8_t socket_ctrl, bool *reverse, bool *udp);
void handle_create_streams(uint8_t socket_ctrl, bool udp);
void start_iperf_test(uint8_t socket_ctrl, uint8_t socket_data, Stats *stats, bool reverse, bool udp);
void exchange_results(uint8_t socket_ctrl, Stats *stats);


int main(void)
{
 W6300_mode = QSPI_MODE;//0; //W6100 >> 0xFF
  /* USER CODE BEGIN 1 */
  int i = 0;
  int ret;
  uint8_t syslock = SYS_NET_LOCK;
  uint8_t mode_set[2] = {0, };
  uint8_t sn = 0, status = 0;
  uint8_t tmp = 0;
  uint16_t destport = 22000;
  uint16_t test_mode = 0;
  PLL2_ClocksTypeDef PLL2_Clk_data;
  char mode_char[4][5]={"Sing","Dual","Quad"};

  int retval = 0;

  /* USER CODE END 1 */
  /* MCU Configuration--------------------------------------------------------*/
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  MX_TIM2_Init(); 
  /* USER CODE BEGIN Init */
  /* USER CODE END Init */
  /* Configure the system clock */
  SystemClock_Config();

/* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_MDMA_Init();
  MX_OCTOSPI1_Init();
  //MX_FMC_Init();
  MX_USART2_UART_Init();
  MX_SPI2_Init();
  /* USER CODE BEGIN 2 */
  
  // FMC ?��?�� 계산
  //uint32_t fmc_clock = Get_FMC_Clock();
  // FMC ?��?�� 출력
  //printf("FMC Clock Frequency: %lu Hz\n", fmc_clock);

  printf("W6300 test Program V%04d \r\n", RTLVERSiON);
  printf("Compile %s - %s \r\n", __DATE__, __TIME__);
  HAL_UART_Receive_IT(&huart2, &rxData, 1);
  //GPIO_PIN_SET,GPIO_PIN_RESET
  HAL_RCCEx_GetPLL2ClockFreq(&PLL2_Clk_data);
  printf("SET PLL2 P:%ld, Q:%ld, R:%ld \r\n", PLL2_Clk_data.PLL2_P_Frequency, PLL2_Clk_data.PLL2_Q_Frequency, PLL2_Clk_data.PLL2_R_Frequency);
  printf("QSPI CLK %d Mhz \r\n", (uint16_t)(PLL2_Clk_data.PLL2_R_Frequency / hospi1.Init.ClockPrescaler / 1000000));


  if(QSPI_MODE < 0x03)
  {
    printf("Software Mode set : QSPI %s\r\n",mode_char[QSPI_MODE]);
  }
  else
  {
    printf("Software Mode set : BUS %02x \r\n", QSPI_MODE);
  }

	chip_hw_reset();
#ifndef FPGA_USED
  uint8_t mode = QSPI_MODE;

  mode |= HAL_GPIO_ReadPin(MOD0_GPIO_Port, MOD0_Pin) << 0; // MOD0 �???????? (?��?�� 비트)
  mode |= HAL_GPIO_ReadPin(MOD1_GPIO_Port, MOD1_Pin) << 1; // MOD1 �????????
  mode |= HAL_GPIO_ReadPin(MOD2_GPIO_Port, MOD2_Pin) << 2; // MOD2 �????????
  mode |= HAL_GPIO_ReadPin(MOD3_GPIO_Port, MOD3_Pin) << 3; // MOD3 �???????? (?��?�� 비트)

  printf("Hardware Mode Pin set : 0x%02X\r\n", mode);
#endif

#if 1 //Add 2024-09-06
  // delay for w6300 system ready
  HAL_Delay(1000*3);
#endif

  W6300Initialze();
  ctlwizchip(CW_SYS_UNLOCK, &syslock);
  printf("W6300Initialze_ok \r\n"); 
  ctlnetwork(CN_SET_NETINFO, &gWIZNETINFO);

  //printf("VERSION(%04x) = %04x \r\n", _VER_, getVER());
  //SET_W6300_IF_MODE
  printf("CHIP ID(%04x) = 0x%04x \r\n", _CIDR_, getCIDR());
  printf("VERSION(%04x) = 0x%04x \r\n", _VER_, getVER());
  for (i = 0; i < 8; i++)
  {
    printf("%d : max size = %d k \r\n", i, getSn_TxMAX(i));
  }

  print_network_information();

  printf("RTL : %x\r\n",WIZCHIP_READ((_W6300_IO_BASE_ + (0x0004 << 8) + WIZCHIP_CREG_BLOCK)));
  printf("\r\n>");
  fflush(stdout);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  //TEST code - Set up clk _by_lihan
  HAL_RCCEx_GetPLL2ClockFreq(&temp_PLL2_Clk_data);
  printf("QSPI CLK %dMhz \r\n", temp_PLL2_Clk_data.PLL2_R_Frequency / 2 / 1000000);
  SPI_CLK_SET(75);
  HAL_RCCEx_GetPLL2ClockFreq(&temp_PLL2_Clk_data);
  printf("QSPI CLK %dMhz \r\n", temp_PLL2_Clk_data.PLL2_R_Frequency / 2 / 1000000);

  set_loopback_mode_W6x00(AS_IPDUAL);

  uint8_t *data= NULL;
  uint8_t iperf_mode = 0, iperf_sn = 0;

  int rsrlen2 = getSn_TXBUF_SIZE(sn) ;
  printf ( "getSn_TXBUF_SIZE = %d KB \r\n " ,  rsrlen2); 

  bool reverse = false;
  //bool reverse = true;

  bool udp = false;
  uint8_t socket_status;
  uint16_t received_len;
  uint32_t pack_len = 0;
  Stats stats;

  SystemCoreClockUpdate();
  printf("System Clock: %lu Hz\n", SystemCoreClock /2 );
  // while (1){
  //   wiz_delay(100);
  //   uint32_t ms100 =  get_time_us() / 1000 / 100 ;
  //   printf("time = %lu . %lu sec \r\n ",ms100 /10, ms100 % 10); ;
      
  // }



  socket(SOCKET_CTRL, Sn_MR_TCP, PORT_IPERF, SOCKET_NO_DELAY_ACK);
  listen(SOCKET_CTRL);

  while (1)
  {
    stats_init(&stats, 1000);
    socket_status = getSn_SR(SOCKET_CTRL);
    // printf("socket_status = %d \r\n", socket_status);
    if (socket_status == SOCK_ESTABLISHED) {

      handle_param_exchange(SOCKET_CTRL, &reverse, &udp);
      handle_create_streams(SOCKET_CTRL, udp);
      if (reverse){
        memset(g_iperf_buf, 0xAA, ETHERNET_BUF_MAX_SIZE /2  );
      }
      start_iperf_test(SOCKET_CTRL, SOCKET_DATA, &stats, reverse, udp);

      disconnect(SOCKET_DATA);
      disconnect(SOCKET_CTRL);
    } else if (socket_status == SOCK_CLOSE_WAIT) {
      disconnect(SOCKET_CTRL);
    } else if (socket_status == SOCK_CLOSED) {
      socket(SOCKET_CTRL, Sn_MR_TCP, PORT_IPERF, 0);
      listen(SOCKET_CTRL);
    }
  }
}

  /* USER CODE END 3 */

// week_Function Redefined
int _write(int fd, char *str, int len) 
{

  for (int i = 0; i < len; i++)
  {
    HAL_UART_Transmit(&huart2, (uint8_t *)&str[i], 1, 0xFFFF);
  }
  return len; //
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
//
  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 2;
  RCC_OscInitStruct.PLL.PLLN = 44;
  RCC_OscInitStruct.PLL.PLLP = 1;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
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
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_FMC|RCC_PERIPHCLK_OSPI;
  PeriphClkInitStruct.PLL2.PLL2M = 5;
  PeriphClkInitStruct.PLL2.PLL2N = 120;
  PeriphClkInitStruct.PLL2.PLL2P = 5;
  PeriphClkInitStruct.PLL2.PLL2Q = 4;
  PeriphClkInitStruct.PLL2.PLL2R = 10;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_2;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
  PeriphClkInitStruct.FmcClockSelection = RCC_FMCCLKSOURCE_PLL2;
  PeriphClkInitStruct.OspiClockSelection = RCC_OSPICLKSOURCE_PLL2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}
/**
  * @brief OCTOSPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_OCTOSPI1_Init(void)
{

  /* USER CODE BEGIN OCTOSPI1_Init 0 */

  /* USER CODE END OCTOSPI1_Init 0 */

  OSPIM_CfgTypeDef sOspiManagerCfg = {0};

  /* USER CODE BEGIN OCTOSPI1_Init 1 */

  /* USER CODE END OCTOSPI1_Init 1 */
  /* OCTOSPI1 parameter configuration*/
  hospi1.Instance = OCTOSPI1;
  hospi1.Init.FifoThreshold = 1;
  hospi1.Init.DualQuad = HAL_OSPI_DUALQUAD_DISABLE;
  hospi1.Init.MemoryType = HAL_OSPI_MEMTYPE_MICRON;
  hospi1.Init.DeviceSize = 17;
  hospi1.Init.ChipSelectHighTime = 1;
  hospi1.Init.FreeRunningClock = HAL_OSPI_FREERUNCLK_DISABLE;
  hospi1.Init.ClockMode = HAL_OSPI_CLOCK_MODE_3;
  hospi1.Init.WrapSize = HAL_OSPI_WRAP_NOT_SUPPORTED;
  hospi1.Init.ClockPrescaler = 2;
  
  #if 1
  hospi1.Init.SampleShifting = HAL_OSPI_SAMPLE_SHIFTING_NONE;
  hospi1.Init.DelayHoldQuarterCycle = HAL_OSPI_DHQC_DISABLE;
  #else //by_lihan for TEST 
  hospi1.Init.SampleShifting = HAL_OSPI_SAMPLE_SHIFTING_HALFCYCLE ; 
  //hospi1.Init.SampleShifting = HAL_OSPI_SAMPLE_SHIFTING_NONE;
  hospi1.Init.DelayHoldQuarterCycle = HAL_OSPI_DHQC_DISABLE;
  //hospi1.Init.DelayHoldQuarterCycle = HAL_OSPI_DHQC_ENABLE;         
  #endif 

  hospi1.Init.ChipSelectBoundary = 0;
  hospi1.Init.DelayBlockBypass = HAL_OSPI_DELAY_BLOCK_BYPASSED;
  hospi1.Init.MaxTran = 0;
  hospi1.Init.Refresh = 0;
  if (HAL_OSPI_Init(&hospi1) != HAL_OK)
  {
    Error_Handler();
  }
  sOspiManagerCfg.ClkPort = 1;
  sOspiManagerCfg.NCSPort = 1;
  sOspiManagerCfg.IOLowPort = HAL_OSPIM_IOPORT_1_LOW;
  if (HAL_OSPIM_Config(&hospi1, &sOspiManagerCfg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN OCTOSPI1_Init 2 */

  /* USER CODE END OCTOSPI1_Init 2 */

}
/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_4BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_HARD_OUTPUT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 0x0;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  hspi2.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi2.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi2.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi2.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi2.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi2.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi2.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi2.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi2.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}
/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}
/**
  * Enable MDMA controller clock
  */
static void MX_MDMA_Init(void)
{

  /* MDMA controller clock enable */
  __HAL_RCC_MDMA_CLK_ENABLE();
  /* Local variables */

  /* MDMA interrupt initialization */
  /* MDMA_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(MDMA_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(MDMA_IRQn);

}
/* FMC initialization function */
static void MX_FMC_Init(void)
{

  /* USER CODE BEGIN FMC_Init 0 */

  /* USER CODE END FMC_Init 0 */

  FMC_NORSRAM_TimingTypeDef Timing = {0};

  /* USER CODE BEGIN FMC_Init 1 */
  hnor1.CommandSet = 0x0013;
  /* USER CODE END FMC_Init 1 */

  /** Perform the NOR1 memory initialization sequence
  */
  hnor1.Instance = FMC_NORSRAM_DEVICE;
  hnor1.Extended = FMC_NORSRAM_EXTENDED_DEVICE;
  /* hnor1.Init */
  hnor1.Init.NSBank = FMC_NORSRAM_BANK1;
  hnor1.Init.DataAddressMux = FMC_DATA_ADDRESS_MUX_DISABLE;
  hnor1.Init.MemoryType = FMC_MEMORY_TYPE_NOR;
  hnor1.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_8;
  hnor1.Init.BurstAccessMode = FMC_BURST_ACCESS_MODE_DISABLE;
  hnor1.Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW;
  hnor1.Init.WaitSignalActive = FMC_WAIT_TIMING_BEFORE_WS;
  hnor1.Init.WriteOperation = FMC_WRITE_OPERATION_ENABLE;
  hnor1.Init.WaitSignal = FMC_WAIT_SIGNAL_DISABLE;
  hnor1.Init.ExtendedMode = FMC_EXTENDED_MODE_DISABLE;
  hnor1.Init.AsynchronousWait = FMC_ASYNCHRONOUS_WAIT_DISABLE;
  hnor1.Init.WriteBurst = FMC_WRITE_BURST_DISABLE;
  hnor1.Init.ContinuousClock = FMC_CONTINUOUS_CLOCK_SYNC_ONLY;
  hnor1.Init.WriteFifo = FMC_WRITE_FIFO_ENABLE;
  hnor1.Init.PageSize = FMC_PAGE_SIZE_NONE;
  /* Timing */
  Timing.AddressSetupTime = 5;
  Timing.AddressHoldTime = 15;
  Timing.DataSetupTime = 5;
  Timing.BusTurnAroundDuration = 5;
  Timing.CLKDivision = 16;
  Timing.DataLatency = 17;
  Timing.AccessMode = FMC_ACCESS_MODE_A;
  /* ExtTiming */

  if (HAL_NOR_Init(&hnor1, &Timing, NULL) != HAL_OK)
  {
    Error_Handler( );
  }

  /* USER CODE BEGIN FMC_Init 2 */

  /* USER CODE END FMC_Init 2 */
}
/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, MOD4_Pin|MOD5_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, RSTn_Pin|Trace_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI_EN_GPIO_Port, SPI_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : MOD4_Pin MOD5_Pin */
  GPIO_InitStruct.Pin = MOD4_Pin|MOD5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : IRQ_Pin */
  GPIO_InitStruct.Pin = IRQ_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(IRQ_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : RSTn_Pin */
  GPIO_InitStruct.Pin = RSTn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(RSTn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : MOD0_Pin */
  GPIO_InitStruct.Pin = MOD0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(MOD0_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI_EN_Pin */
  GPIO_InitStruct.Pin = SPI_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(SPI_EN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Trace_Pin */
  GPIO_InitStruct.Pin = Trace_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(Trace_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : MOD2_Pin MOD3_Pin */
  GPIO_InitStruct.Pin = MOD2_Pin|MOD3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pin : MOD1_Pin */
  GPIO_InitStruct.Pin = MOD1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(MOD1_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}
/* USER CODE BEGIN 4 */
void print_network_information(void)
{
	wiz_NetInfo gWIZNETINFO1;
#if 0

#else
	wizchip_getnetinfo(&gWIZNETINFO1);
		printf("Mac address: %02x:%02x:%02x:%02x:%02x:%02x\r\n",gWIZNETINFO1.mac[0],gWIZNETINFO1.mac[1],gWIZNETINFO1.mac[2],gWIZNETINFO1.mac[3],gWIZNETINFO1.mac[4],gWIZNETINFO1.mac[5]);
		printf("IP address : %d.%d.%d.%d\r\n",gWIZNETINFO1.ip[0],gWIZNETINFO1.ip[1],gWIZNETINFO1.ip[2],gWIZNETINFO1.ip[3]);
		printf("SN Mask	   : %d.%d.%d.%d\r\n",gWIZNETINFO1.sn[0],gWIZNETINFO1.sn[1],gWIZNETINFO1.sn[2],gWIZNETINFO1.sn[3]);
		printf("Gate way   : %d.%d.%d.%d\r\n",gWIZNETINFO1.gw[0],gWIZNETINFO1.gw[1],gWIZNETINFO1.gw[2],gWIZNETINFO1.gw[3]);
		//printf("DNS Server : %d.%d.%d.%d\r\n",gWIZNETINFO1.dns[0],gWIZNETINFO1.dns[1],gWIZNETINFO1.dns[2],gWIZNETINFO1.dns[3]);
		printf("LLA  : %.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X\r\n",gWIZNETINFO1.lla[0],gWIZNETINFO1.lla[1],gWIZNETINFO1.lla[2],gWIZNETINFO1.lla[3],\
										gWIZNETINFO1.lla[4],gWIZNETINFO1.lla[5],gWIZNETINFO1.lla[6],gWIZNETINFO1.lla[7],\
										gWIZNETINFO1.lla[8],gWIZNETINFO1.lla[9],gWIZNETINFO1.lla[10],gWIZNETINFO1.lla[11],\
										gWIZNETINFO1.lla[12],gWIZNETINFO1.lla[13],gWIZNETINFO1.lla[14],gWIZNETINFO1.lla[15]);
		printf("GUA  : %.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X\r\n",gWIZNETINFO1.gua[0],gWIZNETINFO1.gua[1],gWIZNETINFO1.gua[2],gWIZNETINFO1.gua[3],\
										gWIZNETINFO1.gua[4],gWIZNETINFO1.gua[5],gWIZNETINFO1.gua[6],gWIZNETINFO1.gua[7],\
										gWIZNETINFO1.gua[8],gWIZNETINFO1.gua[9],gWIZNETINFO1.gua[10],gWIZNETINFO1.gua[11],\
										gWIZNETINFO1.gua[12],gWIZNETINFO1.gua[13],gWIZNETINFO1.gua[14],gWIZNETINFO1.gua[15]);
		printf("SN6  : %.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X\r\n",gWIZNETINFO1.sn6[0],gWIZNETINFO1.sn6[1],gWIZNETINFO1.sn6[2],gWIZNETINFO1.sn6[3],\
										gWIZNETINFO1.sn6[4],gWIZNETINFO1.sn6[5],gWIZNETINFO1.sn6[6],gWIZNETINFO1.sn6[7],\
										gWIZNETINFO1.sn6[8],gWIZNETINFO1.sn6[9],gWIZNETINFO1.sn6[10],gWIZNETINFO1.sn6[11],\
										gWIZNETINFO1.sn6[12],gWIZNETINFO1.sn6[13],gWIZNETINFO1.sn6[14],gWIZNETINFO1.sn6[15]);
		printf("GW6  : %.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X\r\n",gWIZNETINFO1.gw6[0],gWIZNETINFO1.gw6[1],gWIZNETINFO1.gw6[2],gWIZNETINFO1.gw6[3],\
										gWIZNETINFO1.gw6[4],gWIZNETINFO1.gw6[5],gWIZNETINFO1.gw6[6],gWIZNETINFO1.gw6[7],\
										gWIZNETINFO1.gw6[8],gWIZNETINFO1.gw6[9],gWIZNETINFO1.gw6[10],gWIZNETINFO1.gw6[11],\
										gWIZNETINFO1.gw6[12],gWIZNETINFO1.gw6[13],gWIZNETINFO1.gw6[14],gWIZNETINFO1.gw6[15]);
#endif
}
int print_network_information_1(void)
{
	int ret = 0;
	wiz_NetInfo gWIZNETINFO1;
	wizchip_getnetinfo(&gWIZNETINFO1);
		printf("Mac address: %02x:%02x:%02x:%02x:%02x:%02x\r\n",gWIZNETINFO1.mac[0],gWIZNETINFO1.mac[1],gWIZNETINFO1.mac[2],gWIZNETINFO1.mac[3],gWIZNETINFO1.mac[4],gWIZNETINFO1.mac[5]);
		printf("IP address : %d.%d.%d.%d\r\n",gWIZNETINFO1.ip[0],gWIZNETINFO1.ip[1],gWIZNETINFO1.ip[2],gWIZNETINFO1.ip[3]);
		printf("SN Mask	   : %d.%d.%d.%d\r\n",gWIZNETINFO1.sn[0],gWIZNETINFO1.sn[1],gWIZNETINFO1.sn[2],gWIZNETINFO1.sn[3]);
		printf("Gate way   : %d.%d.%d.%d\r\n",gWIZNETINFO1.gw[0],gWIZNETINFO1.gw[1],gWIZNETINFO1.gw[2],gWIZNETINFO1.gw[3]);
#if 0
		//printf("DNS Server : %d.%d.%d.%d\r\n",gWIZNETINFO1.dns[0],gWIZNETINFO1.dns[1],gWIZNETINFO1.dns[2],gWIZNETINFO1.dns[3]);
		printf("LLA  : %.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X\r\n",gWIZNETINFO1.lla[0],gWIZNETINFO1.lla[1],gWIZNETINFO1.lla[2],gWIZNETINFO1.lla[3],\
										gWIZNETINFO1.lla[4],gWIZNETINFO1.lla[5],gWIZNETINFO1.lla[6],gWIZNETINFO1.lla[7],\
										gWIZNETINFO1.lla[8],gWIZNETINFO1.lla[9],gWIZNETINFO1.lla[10],gWIZNETINFO1.lla[11],\
										gWIZNETINFO1.lla[12],gWIZNETINFO1.lla[13],gWIZNETINFO1.lla[14],gWIZNETINFO1.lla[15]);
		printf("GUA  : %.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X\r\n",gWIZNETINFO1.gua[0],gWIZNETINFO1.gua[1],gWIZNETINFO1.gua[2],gWIZNETINFO1.gua[3],\
										gWIZNETINFO1.gua[4],gWIZNETINFO1.gua[5],gWIZNETINFO1.gua[6],gWIZNETINFO1.gua[7],\
										gWIZNETINFO1.gua[8],gWIZNETINFO1.gua[9],gWIZNETINFO1.gua[10],gWIZNETINFO1.gua[11],\
										gWIZNETINFO1.gua[12],gWIZNETINFO1.gua[13],gWIZNETINFO1.gua[14],gWIZNETINFO1.gua[15]);
		printf("SN6  : %.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X\r\n",gWIZNETINFO1.sn6[0],gWIZNETINFO1.sn6[1],gWIZNETINFO1.sn6[2],gWIZNETINFO1.sn6[3],\
										gWIZNETINFO1.sn6[4],gWIZNETINFO1.sn6[5],gWIZNETINFO1.sn6[6],gWIZNETINFO1.sn6[7],\
										gWIZNETINFO1.sn6[8],gWIZNETINFO1.sn6[9],gWIZNETINFO1.sn6[10],gWIZNETINFO1.sn6[11],\
										gWIZNETINFO1.sn6[12],gWIZNETINFO1.sn6[13],gWIZNETINFO1.sn6[14],gWIZNETINFO1.sn6[15]);
		printf("GW6  : %.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X:%.2X%.2X\r\n",gWIZNETINFO1.gw6[0],gWIZNETINFO1.gw6[1],gWIZNETINFO1.gw6[2],gWIZNETINFO1.gw6[3],\
										gWIZNETINFO1.gw6[4],gWIZNETINFO1.gw6[5],gWIZNETINFO1.gw6[6],gWIZNETINFO1.gw6[7],\
										gWIZNETINFO1.gw6[8],gWIZNETINFO1.gw6[9],gWIZNETINFO1.gw6[10],gWIZNETINFO1.gw6[11],\
										gWIZNETINFO1.gw6[12],gWIZNETINFO1.gw6[13],gWIZNETINFO1.gw6[14],gWIZNETINFO1.gw6[15]);
#endif
	ret = memcmp(gWIZNETINFO1.mac, gWIZNETINFO.mac, 6);
	if(ret != 0)
	{
		printf("not match mac \r\n");
		return ret;
	}
	ret = memcmp(gWIZNETINFO1.ip, gWIZNETINFO.ip, 4);
	if(ret != 0)
	{
		printf("not match ip \r\n");
		return ret;
	}
	ret = memcmp(gWIZNETINFO1.sn, gWIZNETINFO.sn, 4);
	if(ret != 0)
	{
		printf("not match subnet mask \r\n");
		return ret;
	}
	ret = memcmp(gWIZNETINFO1.gw, gWIZNETINFO.gw, 4);
	if(ret != 0)
	{
		printf("not match gateway \r\n");
		return ret;
	}
  return 0;
}
char SPI_CLK_SET(uint16_t set_clk_data)
{
    //    0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 
    //clk 5, 10, 15, 20, 25, 30, 35, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    PLL2_ClocksTypeDef PLL2_Clk_data;
    uint16_t temp_PLL2N = 40, temp_PLL2R = 8;
    #if V1
    temp_PLL2N = 40;
    temp_PLL2R = 8;
    HAL_RCCEx_GetPLL2ClockFreq(&PLL2_Clk_data);
    printf("before PLL2 P:%d, Q:%d, R:%d \r\n", PLL2_Clk_data.PLL2_P_Frequency, PLL2_Clk_data.PLL2_Q_Frequency, PLL2_Clk_data.PLL2_R_Frequency);

    if(set_clk_data <15)
    {
        temp_PLL2N = 40;
        temp_PLL2R = (uint16_t)(160/set_clk_data);
    }
    else
    {
        temp_PLL2N = (uint16_t)(set_clk_data * 2);
    }
    PeriphClkInitStruct.PLL2.PLL2M = 8;
    PeriphClkInitStruct.PLL2.PLL2P = 2;
    PeriphClkInitStruct.PLL2.PLL2Q = 2;
    #else
    temp_PLL2N = 40;
    temp_PLL2R = 8;
    HAL_RCCEx_GetPLL2ClockFreq(&PLL2_Clk_data);
    printf("before PLL2 P:%d, Q:%d, R:%d \r\n", PLL2_Clk_data.PLL2_P_Frequency, PLL2_Clk_data.PLL2_Q_Frequency, PLL2_Clk_data.PLL2_R_Frequency);

    if(set_clk_data <2)
    {
        temp_PLL2N = 48;
        temp_PLL2R = 120;
    }
    else if(set_clk_data <5)
    {
        temp_PLL2N = (uint16_t)(set_clk_data * 32);
        temp_PLL2R = 80;
    }
    else if(set_clk_data <21)
    {
        temp_PLL2N = (uint16_t)(set_clk_data * 8);
        temp_PLL2R = 20;
    }
    else
    {
        temp_PLL2N = (uint16_t)(set_clk_data * 2);
        temp_PLL2R = 5;
    }
    PeriphClkInitStruct.PLL2.PLL2M = 5;
    PeriphClkInitStruct.PLL2.PLL2P = 2;
    PeriphClkInitStruct.PLL2.PLL2Q = 5;
    #endif
    printf("PLL2 N:%d, R:%d\r\n",temp_PLL2N, temp_PLL2R);
    //__HAL_RCC_OCTOSPIM_CLK_DISABLE();
    //__HAL_RCC_OSPI1_CLK_DISABLE();
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_OSPI;
    
    //PeriphClkInitStruct.PLL2.PLL2N = 40;
    PeriphClkInitStruct.PLL2.PLL2N = temp_PLL2N;
    ///PeriphClkInitStruct.PLL2.PLL2R = 8;
    PeriphClkInitStruct.PLL2.PLL2R = temp_PLL2R;
    PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
    PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
    PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
    PeriphClkInitStruct.OspiClockSelection = RCC_OSPICLKSOURCE_PLL2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    //__HAL_RCC_OCTOSPIM_CLK_ENABLE();
    //__HAL_RCC_OSPI1_CLK_ENABLE();
    //MX_OCTOSPI1_Init();
    HAL_RCCEx_GetPLL2ClockFreq(&PLL2_Clk_data);
    printf("after PLL2 P:%d, Q:%d, R:%d \r\n", PLL2_Clk_data.PLL2_P_Frequency, PLL2_Clk_data.PLL2_Q_Frequency, PLL2_Clk_data.PLL2_R_Frequency);
    printf("QSPI CLK %dMhz \r\n", PLL2_Clk_data.PLL2_R_Frequency/2/1000000);
}
#define V1 0

/* USER CODE END 4 */
/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
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
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

void handle_param_exchange(uint8_t socket_ctrl, bool *reverse, bool *udp) 
{
    char buffer[512] = {0};
    uint8_t cmd;
    uint16_t len = 0;
    uint8_t raw_len[4] = {0};
    int cookie_len;
    cJSON *json;
    cJSON *reverseItem;
    cJSON *udpItem;

    cookie_len = recv(socket_ctrl, cookie, COOKIE_SIZE);
    if (cookie_len != COOKIE_SIZE) {
        printf("[iperf] Failed to receive cookie. Received: %d bytes\n", cookie_len);
        return;
    }
    printf("[iperf] Received cookie: %s\n", cookie);

    cmd = PARAM_EXCHANGE;
    send(socket_ctrl, &cmd, 1);
    recv(socket_ctrl, raw_len, 4);

    len = (raw_len[0] << 24) | (raw_len[1] << 16) | (raw_len[2] << 8) | raw_len[3];
    printf("[iperf] Raw length bytes: 0x%02X 0x%02X 0x%02X 0x%02X, Parsed length: %d\n",
           raw_len[0], raw_len[1], raw_len[2], raw_len[3], len);

    recv(socket_ctrl, (uint8_t *)buffer, len);
    buffer[len] = '\0'; // Null-terminate

    printf("[iperf] Received parameters: %s\n", buffer);

    json = cJSON_Parse(buffer);
    if (json == NULL) {
        printf("[iperf] Failed to parse JSON: %s\n", cJSON_GetErrorPtr());
    } else {
        printf("[iperf] Parsed JSON: %s\n", cJSON_Print(json));
        reverseItem = cJSON_GetObjectItem(json, "reverse");
        udpItem = cJSON_GetObjectItem(json, "udp");

        *reverse = (reverseItem && cJSON_IsBool(reverseItem)) ? reverseItem->valueint : 0;
        *udp = (udpItem && cJSON_IsBool(udpItem)) ? udpItem->valueint : 0;
        cJSON_Delete(json);
        printf("[iperf] Parsed JSON: reverse=%d, udp=%d\n", *reverse, *udp);
    }
}

void handle_create_streams(uint8_t socket_ctrl, bool udp) 
{
    uint8_t cmd = CREATE_STREAMS;
    uint8_t received;

    send(socket_ctrl, &cmd, 1);
    printf("[iperf] Sent CREATE_STREAMS command.\n");

    socket(SOCKET_DATA, Sn_MR_TCP, PORT_IPERF, SOCKET_NO_DELAY_ACK);
    listen(SOCKET_DATA);

    // Wait for client to connect to data socket
    while (getSn_SR(SOCKET_DATA) != SOCK_ESTABLISHED) {
        if (getSn_SR(SOCKET_DATA) == SOCK_CLOSED) {
            printf("[iperf] Data socket closed unexpectedly.\n");
            return;
        }
    }
    printf("[iperf] Data connection established.\n");

    // Receive cookie on data socket
    received = recv(SOCKET_DATA, cookie, COOKIE_SIZE);
    if (received > 0) {
        printf("[iperf] Received data cookie: %s\n", cookie);
    }
}

void start_iperf_test(uint8_t socket_ctrl, uint8_t socket_data, Stats *stats, bool reverse, bool udp)
{
    bool running = true;
    uint8_t cmd = 0;
    uint32_t total_bytes = 0;
    uint32_t pack_len = 0;

    printf("[iperf] Starting data stream test...\n");

    // Start test
    cmd = TEST_START;
    send(socket_ctrl, &cmd, 1);

    // Running test
    cmd = TEST_RUNNING;
    send(socket_ctrl, &cmd, 1);

    stats_start(stats);

    while (running) {
        if (getSn_RX_RSR(SOCKET_CTRL) > 0) {
            recv(SOCKET_CTRL, &cmd, 1);
            if (cmd == TEST_END) {
                printf("[iperf] TEST_END command received. Stopping test...\n");
                running = false;
                break;
            }
        }

        if (reverse) {
            // memset(g_iperf_buf, 0xAA, ETHERNET_BUF_MAX_SIZE /2  ); // 
            uint16_t sent_sizse =  send(socket_data, g_iperf_buf, ETHERNET_BUF_MAX_SIZE / 4);
            stats_add_bytes(stats,sent_sizse );
        } else {
            // getsockopt(socket_data, SO_RECVBUF, &pack_len);
            getsockopt(socket_data, SO_RECVBUF, &pack_len);
            if (pack_len > 0)
            {
                //uint16_t recvSize  =  recv(socket_data, (uint8_t *)g_iperf_buf, ETHERNET_BUF_MAX_SIZE / 2 ); 
                uint16_t recvSize  =  recv(socket_data, (uint8_t *)g_iperf_buf, ETHERNET_BUF_MAX_SIZE - 1  ); // more fast
                stats_add_bytes(stats, recvSize);
            }
            else if (pack_len == 0) {
                stats_update(stats, false);
            } else {
                printf("[iperf] Error during data reception\n");
                break;
            }
        }
        stats_update(stats, false);
    }
    stats_stop(stats);

    exchange_results(SOCKET_CTRL, stats);
}

void exchange_results(uint8_t socket_ctrl, Stats *stats) 
{
    uint8_t cmd = EXCHANGE_RESULTS;
    uint32_t result_len = 0;
    uint8_t length_bytes[4];
    char buffer[1024];
    char *results_str;
    uint32_t results_len;
    cJSON *results;
    cJSON *streams;
    cJSON *stream;

    // Ask to exchange results
    send(socket_ctrl, &cmd, 1);
    printf("[iperf] Sent EXCHANGE_RESULTS command.\n");

    // Receive client results
    recv(socket_ctrl, (uint8_t *)&result_len, 4);
    result_len = (result_len << 24) | ((result_len << 8) & 0x00FF0000) | ((result_len >> 8) & 0x0000FF00) | (result_len >> 24); // Convert to host-endian

    if (result_len > sizeof(buffer)) {
        printf("[iperf] Received result length exceeds buffer size.\n");
        return;
    }

    recv(socket_ctrl, (uint8_t *)buffer, result_len);
    buffer[result_len] = '\0'; // Null-terminate the received JSON data
    printf("[iperf] Client results received: %s\n", buffer);

    // Prepare server results
    results = cJSON_CreateObject();
    cJSON_AddNumberToObject(results, "cpu_util_total", 1);
    cJSON_AddNumberToObject(results, "cpu_util_user", 0.5);
    cJSON_AddNumberToObject(results, "cpu_util_system", 0.5);
    cJSON_AddNumberToObject(results, "sender_has_retransmits", 1);
    cJSON_AddStringToObject(results, "congestion_used", "cubic");

    // Streams object
    streams = cJSON_CreateArray();
    stream = cJSON_CreateObject();
    cJSON_AddNumberToObject(stream, "id", 1);
    cJSON_AddNumberToObject(stream, "bytes", stats->nb0);
    cJSON_AddNumberToObject(stream, "retransmits", 0);
    cJSON_AddNumberToObject(stream, "jitter", 0);
    cJSON_AddNumberToObject(stream, "errors", 0);
    cJSON_AddNumberToObject(stream, "packets", stats->np0);  // 총 패킷 수 추가
    cJSON_AddNumberToObject(stream, "start_time", 0);
    cJSON_AddNumberToObject(stream, "end_time", (double)(stats->t3 - stats->t0) / 1000000.0);  // 종료 시간 계산
    cJSON_AddItemToArray(streams, stream);
    cJSON_AddItemToObject(results, "streams", streams);

    // Serialize JSON to string
    results_str = cJSON_PrintUnformatted(results);
    results_len = strlen(results_str);

    // Send server results
    length_bytes[0] = (results_len >> 24) & 0xFF;
    length_bytes[1] = (results_len >> 16) & 0xFF;
    length_bytes[2] = (results_len >> 8) & 0xFF;
    length_bytes[3] = results_len & 0xFF;

    send(socket_ctrl, length_bytes, 4);
    send(socket_ctrl, (uint8_t *)results_str, results_len);

    printf("[iperf] Server results sent.\n");

    cJSON_Delete(results);

    // Ask to display results
    cmd = DISPLAY_RESULTS;
    send(socket_ctrl, &cmd, 1);

    // Wait for IPERF_DONE command
    recv(socket_ctrl, &cmd, 1);
    if (cmd == IPERF_DONE) {
        printf("[iperf] Test completed successfully.\n");
    } else {
        printf("[iperf] Unexpected command received: %d\n", cmd);
    }
}

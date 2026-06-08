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

#define _LOOPBACK_MODE_  0 
#define _IPERF_SEND_MODE_  1
#define _IPERF_RECV_MODE_  2 

#define _TESTMODE_ _LOOPBACK_MODE_ 

#define TCPS_EN    1
#define NDA        1
#define RTLVERSiON 227
#define MAIN_CLK_100MHZ 0
#define ETHERNET_BUF_MAX_SIZE (1024 * 32)

#define SOCKET 0
#define PORT_IPERF 5010

static uint8_t g_udp_buf_main[ETHERNET_BUF_MAX_SIZE * 2 ] = {
    0,
};

PLL2_ClocksTypeDef temp_PLL2_Clk_data;

#if 1
//#define FPGA_USED
#endif

UART_HandleTypeDef huart3;   // ← 추가 (huart2는 아래 Private variables 섹션에 선언됨)


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

SRAM_HandleTypeDef hsram1;

/* USER CODE BEGIN PV */
//int __io_putchar(int ch)
//{
//    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
//    return ch;
//}
//uint8_t W6300_mode = QSPI_MODE;//0; //W6100 >> 0xFF
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
void MPU_Config_FMC_Region(void);
/* Private user code ---------------------------------------------------------*/

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

volatile uint8_t rxData;

static uint8_t g_iperf_buf[ETHERNET_BUF_MAX_SIZE  *2] = {
    0,
};

#if 1 // added
uint8_t is_testing = 0; // 0 : not testing, 1 : testing
#endif

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



#define EXT_MEM_BASE 0x68000000

/* ===== 런타임 인터페이스 모드 선택용 strap 핀 =====
   부팅 때 이 핀을 읽어 BUS/QSPI를 자동 선택. HIGH=BUS, LOW=QSPI(Quad). 기본(풀다운)=QSPI.
   ※ WIZ630MJ 점퍼와 같은 모드가 되도록 strap을 맞출 것.
   ※ 다른 핀으로 바꾸려면 아래 3줄만 수정. */
#define IFMODE_STRAP_PORT      GPIOC
#define IFMODE_STRAP_PIN       GPIO_PIN_0
#define IFMODE_STRAP_CLK_EN()  __HAL_RCC_GPIOC_CLK_ENABLE()

void W6100BusWriteByte_2(uint32_t addr, iodata_t data)
{
volatile uint8_t* pExt = (volatile uint8_t*)EXT_MEM_BASE;
    pExt[0] = 0;  // A[1:0] = 0
    pExt[1] = 0;  // A[1:0] = 1
    pExt[2] = 0;     // A[1:0] = 2

	(*(volatile uint8_t *)((uint32_t)(addr)) = (data)); 
}

volatile iodata_t W6100BusReadByte_test(uint32_t addr)
{
	iodata_t ret;
volatile uint8_t* pExt = (volatile uint8_t*)EXT_MEM_BASE;
 pExt[0] = (addr >> 16) & 0xff;  // A[1:0] = 0
 pExt[1] = (addr >>  8) & 0xff;  // A[1:0] = 1
 pExt[2] = (addr >>  0) & 0xff;     // A[1:0] = 2
 ret = pExt[3];
 return ret ;
}

iodata_t W6100BusReadburst_test(uint32_t addr, uint32_t len)
{
	iodata_t ret;
volatile uint8_t* pExt = (volatile uint8_t*)EXT_MEM_BASE;
 pExt[0] = (addr >> 16) && 0xff;  // A[1:0] = 0
 pExt[1] = (addr >>  8) && 0xff;  // A[1:0] = 1
 pExt[2] = (addr >>  0) && 0xff;     // A[1:0] = 2
 // for()1
   ret = pExt[3];
 return ret ;
}




int main(void)
{

  W6300_mode = QSPI_MODE;//0; //W6100 >> 0xFF
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

  /* MCU Configuration--------------------------------------------------------*/
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* === 런타임 인터페이스 모드 선택 (strap 핀) ===
     strap 읽어 W6300_IF_MODE 결정. HIGH=BUS, LOW=QSPI(Quad). 기본(풀다운)=QSPI.
     MX_GPIO_Init / 페리페럴 init 보다 먼저 정해야 그쪽이 모드대로 갈림. */
  IFMODE_STRAP_CLK_EN();
  {
    GPIO_InitTypeDef sgp = {0};
    sgp.Pin  = IFMODE_STRAP_PIN;
    sgp.Mode = GPIO_MODE_INPUT;
    sgp.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(IFMODE_STRAP_PORT, &sgp);
  }
  W6300_IF_MODE = (HAL_GPIO_ReadPin(IFMODE_STRAP_PORT, IFMODE_STRAP_PIN) == GPIO_PIN_SET)
                  ? BUS_MODE : QSPI_MODE_QUAD;
  W6300_mode = W6300_IF_MODE;

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_MDMA_Init();
  if (W6300_IF_MODE == BUS_MODE) {
    MPU_Config_FMC_Region();   // BUS: FMC 사용 (OCTOSPI OFF)
    MX_FMC_Init();
  } else {
    MX_OCTOSPI1_Init();        // QSPI: OCTOSPI 사용 (FMC OFF)
  }
  MX_USART2_UART_Init();
  // /MX_SPI2_Init();
  HAL_Delay(1000);

//  MX_USART2_UART_Init();
  MX_USART3_UART_Init();   // ← 추가

  // 클럭 검증용: HSE_VALUE를 8MHz로 맞춘 뒤 아래가 480000000 / 120000000 으로 떠야 정상.
  // 깨져 나오면 HSE_VALUE(stm32h7xx_hal_conf.h) 와 실제 HSE가 안 맞는 것.
  printf("SYSCLK=%lu  PCLK1(USART3)=%lu\r\n",
         HAL_RCC_GetSysClockFreq(), HAL_RCC_GetPCLK1Freq());

  /* 현재 인터페이스 모드 — strap 핀으로 결정된 런타임 값 */
  printf("==== Interface Mode = %s  (IF_MODE=0x%02X) ====\r\n",
         (W6300_IF_MODE == BUS_MODE) ? "BUS (8-bit FMC)" : "QSPI (Quad)",
         W6300_IF_MODE);

  // while (1)
  // {
  //   printf("alive %d\r\n", i++);
  //   HAL_Delay(500);
  // }
  printf("W6300 test Program V%04d \r\n", RTLVERSiON);
  printf("Compile %s - %s \r\n", __DATE__, __TIME__);
  HAL_UART_Receive_IT(&huart2, &rxData, 1);
  //GPIO_PIN_SET,GPIO_PIN_RESET
  HAL_RCCEx_GetPLL2ClockFreq(&PLL2_Clk_data);
  printf("SET PLL2 P:%ld, Q:%ld, R:%ld \r\n", PLL2_Clk_data.PLL2_P_Frequency, PLL2_Clk_data.PLL2_Q_Frequency, PLL2_Clk_data.PLL2_R_Frequency);
  printf("QSPI CLK %d Mhz \r\n", (uint16_t)(PLL2_Clk_data.PLL2_R_Frequency / hospi1.Init.ClockPrescaler / 1000000));

	chip_hw_reset();
  uint8_t mode = QSPI_MODE;

  mode |= HAL_GPIO_ReadPin(MOD0_GPIO_Port, MOD0_Pin) << 0; // MOD0 �???????? (?��?�� 비트)
  mode |= HAL_GPIO_ReadPin(MOD1_GPIO_Port, MOD1_Pin) << 1; // MOD1 �????????
  mode |= HAL_GPIO_ReadPin(MOD2_GPIO_Port, MOD2_Pin) << 2; // MOD2 �????????
  mode |= HAL_GPIO_ReadPin(MOD3_GPIO_Port, MOD3_Pin) << 3; // MOD3 �???????? (?��?�� 비트)

  printf("Hardware Mode Pin set : 0x%02X\r\n", mode);
  printf("W6300 test Program _lihan \r\n");
  HAL_Delay(1000);


  // W6100BusWriteBurst(0x68000000,tAD,3,1);
  // HAL_Delay(10);
  volatile uint8_t* add ;  
  volatile uint8_t* pExt = (volatile uint8_t*)EXT_MEM_BASE;
  volatile uint8_t* result2 = (volatile uint8_t*)(EXT_MEM_BASE+3);

#if 1 //Add 2024-09-06
  // delay for w6300 system ready
  HAL_Delay(1000*1);
#endif

	chip_hw_reset();
  printf("W6300Initialze_start\r\n"); 

  W6300Initialze();
 printf(" _WIZCHIP_IO_MODE_ = %04x // _WIZCHIP_IO_MODE_BUS == %04x",_WIZCHIP_IO_MODE_ ,_WIZCHIP_IO_MODE_BUS_);

  printf("CHIP ID(%04x) = 0x%04x \r\n", _CIDR_, WIZCHIP_READ(000000));
  printf("CHIP ID(%04x) = 0x%04x \r\n", _CIDR_, getCIDR());
  printf("VERSION(%04x) = 0x%04x \r\n", _VER_, getVER());


  //ctlwizchip(CW_SYS_UNLOCK, &syslock);
  printf("W6300Initialze_ok \r\n"); 
  ctlnetwork(CN_SET_NETINFO, &gWIZNETINFO);

  //SET_W6300_IF_MODE
  printf("CHIP ID(%04x) = 0x%04x \r\n", _CIDR_, getCIDR());
  printf("VERSION(%04x) = 0x%04x \r\n", _VER_, getVER());
  
  for (i = 0; i < 8; i++)
  {
    printf("%d : txmax size = %d k \r\n", i, getSn_TxMAX(i));
    printf("%d : rxmax size = %d k \r\n", i, getSn_RxMAX(i));
  }

  print_network_information();

  printf("RTL : %x\r\n",WIZCHIP_READ((_W6300_IO_BASE_ + (0x0004 << 8) + WIZCHIP_CREG_BLOCK)));
  printf("\r\n>");
  fflush(stdout);

#if 0
  HAL_RCCEx_GetPLL2ClockFreq(&temp_PLL2_Clk_data);
  printf("QSPI CLK %dMhz \r\n", temp_PLL2_Clk_data.PLL2_R_Frequency / 2 / 1000000);
  SPI_CLK_SET(10);
  HAL_RCCEx_GetPLL2ClockFreq(&temp_PLL2_Clk_data);
  printf("QSPI CLK %dMhz \r\n", temp_PLL2_Clk_data.PLL2_R_Frequency / 2 / 1000000);
#endif 




  printf ( "getSn_TXBUF_SIZE = %d KB \r\n " ,  getSn_TXBUF_SIZE(sn)); 
  if(W6300_IF_MODE < 0x03)
  {
    printf("Software Mode set : QSPI %s\r\n",mode_char[W6300_IF_MODE]);
  }
  else
  {
    printf("Software Mode set : BUS %02x \r\n", W6300_IF_MODE);
  }
  HAL_RCCEx_GetPLL2ClockFreq(&temp_PLL2_Clk_data);
  printf("QSPI CLK %dMhz \r\n", temp_PLL2_Clk_data.PLL2_R_Frequency / 2 / 1000000);
  set_loopback_mode_W6x00(AS_IPDUAL);
  printf("IP_mode = %d \r\n", check_loopback_mode_W6x00());


  while (1)
  {

#if _TESTMODE_ == _LOOPBACK_MODE_
   
    if ((retval = loopback_tcps(4, g_udp_buf_main, 5080)) < 0)
    {
      printf(" loopback_udps error : %d\n", retval);
      while (1)
          ;
    }
   
#elif _TESTMODE_ == _IPERF_RECV_MODE_

     uint32_t pack_len = 0;
    switch(getSn_SR(SOCKET))
    {
        case SOCK_ESTABLISHED :
            printf("test\r\n");

            while(1)
            {
                getsockopt(SOCKET, SO_RECVBUF, &pack_len);
                if (pack_len > 0)
                {
                    recv(SOCKET, (uint8_t *)g_iperf_buf,  ETHERNET_BUF_MAX_SIZE  - 1  );
                }
            }
            break;
        case SOCK_CLOSE_WAIT :
            disconnect(SOCKET);
            break;
        case SOCK_INIT :
            listen(SOCKET);
            break;
        case SOCK_CLOSED:
            socket(SOCKET, Sn_MR_TCP, PORT_IPERF, 0x20); // 0x20 = no delay ack
            break;
        default:
            break;
    }

#elif _TESTMODE_ == _IPERF_SEND_MODE_
    
    iperf_tcpc(SOCKET, g_udp_buf_main, WIZ_Dest_IP, PORT_IPERF, 1000000, 1);
   
#endif 
  }
}

// week_Function Redefined
#if 0
int _write(int fd, char *str, int len) 
{

  for (int i = 0; i < len; i++)
  {
    HAL_UART_Transmit(&huart2, (uint8_t *)&str[i], 1, 0xFFFF);
  }
  return len; //
}
#else
int _write(int fd, char *str, int len)
{
  for (int i = 0; i < len; i++)
    HAL_UART_Transmit(&huart3, (uint8_t *)&str[i], 1, 0xFFFF);  // huart2 → huart3
  return len;
}
#endif


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
#if 0
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
#else

  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 1;                // 8/1 = 8MHz (PLL 입력)
    RCC_OscInitStruct.PLL.PLLN = 60;               // 8*60 = 480MHz (VCO) ← 836 이하 ✓
    RCC_OscInitStruct.PLL.PLLP = 1;                // 480MHz SYSCLK
    RCC_OscInitStruct.PLL.PLLQ = 2;
    RCC_OscInitStruct.PLL.PLLR = 2;
    RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;  // 입력 8MHz → RANGE_3 (8~16MHz) ✓
    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;  // 480 ∈ [192,836] ✓
#endif

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
void MPU_Config_FMC_Region(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct;

  /* MPU 비활성화 */
  HAL_MPU_Disable();

  /* FMC 메모리 영역(예: Bank3: 0x68000000 ~ 0x6BFFFFFF)의 MPU 설정 */
  MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
  MPU_InitStruct.Number           = MPU_REGION_NUMBER0;  // 사용 가능한 MPU 영역 번호 선택 (0~7)
  MPU_InitStruct.BaseAddress      = 0x68000000;
  MPU_InitStruct.Size             = MPU_REGION_SIZE_64KB;  // 실제 FMC 영역 크기에 맞춰 조정 (예: 64MB)
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;        // TEX=0
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE; // 필요에 따라

  /* 이 설정은 Strongly Ordered 메모리 타입(비캐시, 비버퍼, 공유)로 설정됩니다 */
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* MPU 활성화: 기본적으로 Privileged Access Default 설정 사용 */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}
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

  /* USER CODE END FMC_Init 1 */

  /** Perform the SRAM1 memory initialization sequence
  */
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
  /* Timing */

    //   Timing.AddressSetupTime         = 30;  // 예) 3+1=4 사이클 => 20ns (200MHz 기준)
    // Timing.AddressHoldTime          = 20;  // 예) 2+1=3 사이클 => 15ns
    // Timing.DataSetupTime            = 90;  // 예) 9+1=10 사이클 => 50ns
    // Timing.BusTurnAroundDuration    = 10;  // Read→Write 전환 시 여유
    // Timing.CLKDivision              = 10;  // 비동기 모드면 1로 설정 (동기 모드가 아니라면)
    // Timing.DataLatency              = 0;  // 비동기 모드면 보통 0
    //   Timing.AccessMode               = FMC_ACCESS_MODE_A;
  Timing.AddressSetupTime = 4;
  Timing.AddressHoldTime = 15;
  Timing.DataSetupTime = 2;
  Timing.BusTurnAroundDuration = 1;
  Timing.CLKDivision = 16;
  Timing.DataLatency = 0;
  Timing.AccessMode = FMC_ACCESS_MODE_A;
  /* ExtTiming */

  if (HAL_SRAM_Init(&hsram1, &Timing, NULL) != HAL_OK)
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


  if (W6300_IF_MODE == BUS_MODE)   /* BUS 모드에서만 FMC 핀 설정 (런타임 분기) */
  {
  /* PF0/PF1=A0/A1, PD4/PD5=NOE/NWE.
     QSPI 모드에선 이 핀들이 W6300 QD0~3과 같은 칩 핀이라, 설정하면 OCTOSPI와 충돌.
     → QSPI 땐 High-Z 유지 위해 설정하지 않음. */
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;         // AF Push-Pull
  GPIO_InitStruct.Pull = GPIO_NOPULL;             // Pull-Up/Down 필요 시 수정
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;      // FMC는 보통 AF12 (시리즈마다 다를 수 있음)
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;         // AF Push-Pull
  GPIO_InitStruct.Pull = GPIO_NOPULL;             // Pull-Up/Down 필요 시 수정
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;      // FMC는 보통 AF12 (시리즈마다 다를 수 있음)
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  }

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);


  /* USART3: PD8=TX, PD9=RX */
  __HAL_RCC_USART3_CLK_ENABLE();
  GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

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

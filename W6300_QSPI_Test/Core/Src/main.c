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
#include "../TestProgram/ES_TEST/Chip_init_TEST.h"

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

#define socket_0 0
#define socket_0_port 5000
#define socket_1 1
#define socket_1_port 5001

#include "stm32h7xx_hal_flash.h"
#include "stm32h7xx_hal_flash_ex.h"
// #include "stm32_hal_legacy.h"
static void read_IP_info_to_flash(uint8_t *IPaddres,uint32_t offset);
static void Write_IP_info_to_flash(uint8_t *IPaddres ,uint32_t offset);


static void Write_IP_info_to_flash (uint8_t *IPaddres ,uint32_t offset)
{
    // Bank1의 시작 주소 0x08000000에서 200KB(0x32000)를 더한 주소
    uint32_t writeAddress = 0x08020000 + (offset * 0x00020000);

    // Flash Unlock (쓰기 전 잠금 해제)
    HAL_FLASH_Unlock();

    // 1) 섹터 Erase
    // 0x08032000는 Sector 1에 속한다고 가정 (섹터 크기가 128KB라면,
    // Sector 0: 0x08000000 ~ 0x0801FFFF, Sector 1: 0x08020000 ~ 0x0803FFFF)
    FLASH_EraseInitTypeDef eraseInit;
    uint32_t sectorError = 0;

    eraseInit.TypeErase    = FLASH_TYPEERASE_SECTORS;
    eraseInit.Banks        = FLASH_BANK_1;         // Bank1 사용
    eraseInit.Sector       = 1 + offset;                    // Sector 1 (주소 범위 0x08020000 ~ 0x0803FFFF)
    eraseInit.NbSectors    = 1 + offset;
    eraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_4;  // STM32H7는 RANGE_4 사용

    if (HAL_FLASHEx_Erase(&eraseInit, &sectorError) != HAL_OK)
    {
        printf("HAL_FLASHEx_Erase Error, SectorError = 0x%08lx\r\n", (unsigned long)sectorError);
        HAL_FLASH_Lock();
        return;
    }

    // 2) 32바이트 데이터 버퍼 준비 (FLASH_TYPEPROGRAM_FLASHWORD는 256비트, 즉 32바이트 단위)
    uint8_t myData[32];
    memset(myData, 0xFF, sizeof(myData));  // Erase된 상태는 모두 0xFF
   
    myData[0] = *(uint8_t*)(IPaddres);
    myData[1] = *(uint8_t*)(IPaddres+1);
    myData[2] = *(uint8_t*)(IPaddres+2);
    myData[3] = *(uint8_t*)(IPaddres+3);
 
    // 3) FLASH 프로그래밍: 32바이트 단위로 기록
    // 세 번째 인자는 데이터 버퍼의 시작 주소여야 합니다.
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, writeAddress, (uint32_t)myData) != HAL_OK)
    {
        uint32_t flashError = HAL_FLASH_GetError();
        printf("HAL_FLASH_Program Error, error code: 0x%08lx\r\n", (unsigned long)flashError);
        HAL_FLASH_Lock();
        return;
    }

    // 4) Flash Lock (쓰기 완료 후 잠금)

}



static void read_IP_info_to_flash(uint8_t *IPaddres,uint32_t offset)
{
    // Bank1의 시작 주소 0x08000000에서 200KB(0x32000)를 더한 주소
    // 0x08000000 + 0x32000 = 0x08032000

    uint32_t ReadAddress = 0x08020000 + (offset * 0x00020000);

    HAL_FLASH_Unlock();
    uint32_t readValue = *(uint32_t*)(ReadAddress);
    HAL_FLASH_Lock();

    uint8_t myData[4];
    myData[0] = (uint8_t)(readValue & 0xFF);
    myData[1] = (uint8_t)((readValue >> 8) & 0xFF);
    myData[2] = (uint8_t)((readValue >> 16) & 0xFF);
    myData[3] = (uint8_t)((readValue >> 24) & 0xFF);

    *IPaddres     = (uint8_t)(readValue & 0xFF);
    *(IPaddres+1) = (uint8_t)((readValue >> 8) & 0xFF);
    *(IPaddres+2) = (uint8_t)((readValue >> 16) & 0xFF);
    *(IPaddres+3) = (uint8_t)((readValue >> 24) & 0xFF);
}



void ES_loopback(void){
  PRINT_TEST_NAME();
    uint8_t result ; 
    int32_t ret;
    uint16_t sentsize=0;
    int8_t status0,inter;
    int8_t status1;
    uint8_t tmp = 0;
    uint16_t received_size;
    uint8_t arg_tmp8;
    uint8_t* mode_msg;


    uint8_t socket0_temp = 0;
    uint8_t socket1_temp = 0;   
    uint8_t pingSourceIP[4] ;
    getSIPR(pingSourceIP);

    set_phy_loopback_mode_MDIO();
 


    getsockopt(socket_0, SO_STATUS, &status0);
    getsockopt(socket_1, SO_STATUS, &status1);
  
    while(status0 != SOCK_CLOSED) { HAL_Delay(100);}
    while(status1 != SOCK_CLOSED) { HAL_Delay(100);}

    socket0_temp = socket(socket_0, Sn_MR_TCP4, socket_0_port, 0);
    socket1_temp = socket(socket_1, Sn_MR_TCP4, socket_1_port, 0);

  
    while(status0 != SOCK_INIT) { HAL_Delay(100);getsockopt(socket_0, SO_STATUS, &status0);}
    while(status1 != SOCK_INIT) { HAL_Delay(100);getsockopt(socket_1, SO_STATUS, &status1);}

    printf("success = socket\r\n");

    getsockopt(socket_0, SO_STATUS, &status0);
    getsockopt(socket_1, SO_STATUS, &status1);
    printf("sock_stat = %d / %d \r\n" , status0,status1); 

    if( (ret = listen(socket_0)) != SOCK_OK) return ret;

    printf("\t\t%d:Listen, TCP server loopback, port [%d] \r\n", socket_0, socket_0_port);
    
   
    while(status0 != SOCK_LISTEN) { 
      HAL_Delay(100);
      getsockopt(socket_0, SO_STATUS, &status0);
      
    }
        
    // HAL_Delay(1000);s
    //   /* for debug*/
    //     getsockopt(socket_0, SO_STATUS, &status0);
    //     if (status0 == SOCK_LISTEN){
    //       printf("connect_sock_stat = SOCK_LISTEN \r\n"); 
    //     }else{
    //       printf("connect_sock_stat = %d \r\n" ,status0); 
    //     }

    uint8_t destip[4] ={ 192,168,11,44} ;
    while( status1 != SOCK_ESTABLISHED ){
      getsockopt(socket_0, SO_STATUS, &status0);
      getsockopt(socket_1, SO_STATUS, &status1);
      printf("sock_stat = %d / %d \r\n" , status0,status1); 

      ES_PHY_MDIO_READ_TEST(0x0000) ; 
      if (status1 == SOCK_CLOSED ){
         socket(socket_1, Sn_MR_TCP4, socket_1_port, 0);
      }else if (status1 == SOCK_INIT ) {
        printf("%d:Try to connect to the %d.%d.%d.%d, %d\r\n", socket_1, destip[0], destip[1], destip[2], destip[3], socket_0_port);
        ret = connect(socket_1, destip, socket_0_port, 4);
        printf("connect RESULT =  %d \r\n" , ret);
      }
    
      HAL_Delay(1000) ; 
    }

    HAL_Delay(100);
    getsockopt(socket_0, SO_STATUS, &status0);
    getsockopt(socket_1, SO_STATUS, &status1);
    printf("sock_stat = %d / %d \r\n" , status0,status1); 
    HAL_Delay(100);

    while(status1 != SOCK_ESTABLISHED){
       HAL_Delay(100);
    }
      printf("success = SOCK_ESTABLISHED \r\n");
    PRINT_RESULT(SUCCESS) ; 

}

void ES_loopback_udp(void){
  PRINT_TEST_NAME();
    uint8_t result ; 
    int32_t ret;
    uint16_t sentsize=0;
    int8_t status0,inter;
    int8_t status1;
    uint8_t tmp = 0;
    uint16_t received_size;
    uint8_t arg_tmp8;
    uint8_t* mode_msg;


    uint8_t socket0_temp = 0;
    uint8_t socket1_temp = 0;   
    uint8_t pingSourceIP[4] = {255,255,255,255} ;
    getSIPR(pingSourceIP);

    // set_phy_loopback_mode_MDIO();
 
    getsockopt(socket_0, SO_STATUS, &status0);
    getsockopt(socket_1, SO_STATUS, &status1);
  
    while(status0 != SOCK_CLOSED) { HAL_Delay(100);}
    while(status1 != SOCK_CLOSED) { HAL_Delay(100);}

    if((socket0_temp = socket(socket_0, Sn_MR_UDP, socket_0_port, 0x00)) == socket_0){
      printf("[iOLB5]%d:_Opened, UDP loopback, port [%d]\r\n", socket_0, socket_0_port);
    }

    if((socket1_temp = socket(socket_1, Sn_MR_UDP, socket_1_port, 0x00)) == socket_1){
      printf("[iOLB5]%d:_Opened, UDP loopback, port [%d]\r\n", socket_1, socket_1_port);
    }

    HAL_Delay(100);
    getsockopt(socket_0, SO_STATUS, &status0);
    getsockopt(socket_1, SO_STATUS, &status1);
    printf("sock_stat = %d / %d \r\n" , status0,status1); 
    HAL_Delay(100);
    while(status0 != SOCK_UDP) { HAL_Delay(100);}
    while(status1 != SOCK_UDP) { HAL_Delay(100);}

    uint8_t* send_buf = "hello_world";
    uint8_t* recieve_buf = "hello_world";
 
    ret = sendto(socket_1, send_buf, 6, pingSourceIP, socket_0,4);
    if(ret < 0)
    {
      printf("sendto = %d \r\n", ret ) ;
    }
    uint8_t size = getSn_RX_RSR(socket_0);
    printf("size = %d \r\n", size ) ; 

    recvfrom(socket_0, recieve_buf, size, pingSourceIP, socket_1,4);

    for ( int i = 0; i < size ; i++ ){
      printf(" %c " , recieve_buf[i]);
    }
    printf("\r\n" );
    PRINT_RESULT(SUCCESS) ; 
}


int GetIPAddress(uint8_t *ip )
{
    char line[100];
    int value;
    int count = 0;
    
    for (int i = 0; i < 4; i++) {
        printf("Enter IP  %d (or press ENTER to stop): ", i + 1);
        
        // 한 줄 전체 입력 (엔터까지)
        if (fgets(line, sizeof(line), stdin) == NULL) {
            break;  // 입력 오류
        }
        // 개행문자 제거
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }
        // 빈 줄이면 입력 중단
        if (line[0] == '\0') {
            break;
        }
        // 문자열을 정수로 변환 (숫자 입력)
        if (sscanf(line, "%d", &value) != 1) {
            break;
        }
        ip[i] = (uint8_t)value;
        count++;
    }
    // 네 번 모두 성공적으로 입력받으면 성공 플래그 1, 아니면 0 반환
    return (count == 4) ? 1 : 0;
}

void ES_LOOPBACK_TEST(void){
  PRINT_TEST_NAME();
  int retval ; 
  while (1)
  {
    if ((retval = loopback_tcpc(SOCKET, g_udp_buf_main, WIZ_Dest_IP, 5010)) < 0)
    {
      if (retval == -999){
        PRINT_RESULT(SUCCESS);
        return SUCCESS;
      }
      printf(" loopback_udps error : %d\n", retval);
      while (1)
          ;
    }
  }
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

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_MDMA_Init();
  MX_OCTOSPI1_Init();
  //MX_FMC_Init();
  MX_USART2_UART_Init();
  MX_SPI2_Init();

  setvbuf(stdout, NULL, _IONBF, 0);
  uint32_t value = 0;

  if ( HAL_GPIO_ReadPin(MOD0_GPIO_Port, MOD0_Pin) &&
        HAL_GPIO_ReadPin(MOD1_GPIO_Port, MOD1_Pin) &&
        HAL_GPIO_ReadPin(MOD2_GPIO_Port, MOD2_Pin) &&
        HAL_GPIO_ReadPin(MOD3_GPIO_Port, MOD3_Pin))
    {
    printf( " mode pin = 0xff \r\n" ); 

    printf_RED("--> IP Info Change Mode\r\n");

    uint8_t inNum; 
    uint8_t inputIP[4] ; 
    printf_GREEN("W6300 IP input\r\n" );
    if(GetIPAddress(gWIZNETINFO.ip) == 1){
      printf("You entered: %d.%d.%d.%d \r\n", gWIZNETINFO.ip[0] ,gWIZNETINFO.ip[1] ,gWIZNETINFO.ip[2] ,gWIZNETINFO.ip[3] );
    }else{
      printf("get ip--- fail\r\n");
    }
    Write_IP_info_to_flash(gWIZNETINFO.ip, 0 );

    printf_GREEN("Dest IP input\r\n" );
    if(GetIPAddress(WIZ_Dest_IP) == 1){
      printf("You entered: %d.%d.%d.%d \r\n", WIZ_Dest_IP[0] ,WIZ_Dest_IP[1] ,WIZ_Dest_IP[2] ,WIZ_Dest_IP[3] );
    }else{
      printf("get ip--- fail\r\n");
    }
    Write_IP_info_to_flash(WIZ_Dest_IP, 1 );

    read_IP_info_to_flash(gWIZNETINFO.ip,0);
    printf("read [W6300] IP: %03d.%03d.%03d.%03d \r\n", gWIZNETINFO.ip[0] ,gWIZNETINFO.ip[1] ,gWIZNETINFO.ip[2] ,gWIZNETINFO.ip[3] );
    read_IP_info_to_flash(WIZ_Dest_IP,1);
    printf("read [Dest ] IP: %03d.%03d.%03d.%03d \r\n", WIZ_Dest_IP[0] ,WIZ_Dest_IP[1] ,WIZ_Dest_IP[2] ,WIZ_Dest_IP[3] );

    while(1){
      HAL_Delay(2500);
      printf_GREEN("please change the switch mod[0:3] and reset\r\n");
    }

  }else{
    printf( " mode pin = %d \r\n" ,HAL_GPIO_ReadPin(MOD0_GPIO_Port, MOD0_Pin) | HAL_GPIO_ReadPin(MOD1_GPIO_Port, MOD1_Pin)| 
                                    HAL_GPIO_ReadPin(MOD2_GPIO_Port, MOD2_Pin) |HAL_GPIO_ReadPin(MOD3_GPIO_Port, MOD3_Pin)); 

    read_IP_info_to_flash(gWIZNETINFO.ip,0);
    printf("read [W6300] IP: %03d.%03d.%03d.%03d \r\n", gWIZNETINFO.ip[0] ,gWIZNETINFO.ip[1] ,gWIZNETINFO.ip[2] ,gWIZNETINFO.ip[3] );
    read_IP_info_to_flash(WIZ_Dest_IP,1);
    printf("read [Dest ] IP: %03d.%03d.%03d.%03d \r\n", WIZ_Dest_IP[0] ,WIZ_Dest_IP[1] ,WIZ_Dest_IP[2] ,WIZ_Dest_IP[3] );
  }
  

  printf("W6300 test Program V%04d \r\n", RTLVERSiON);
  printf("Compile %s - %s \r\n", __DATE__, __TIME__);
  HAL_UART_Receive_IT(&huart2, &rxData, 1);
  //GPIO_PIN_SET,GPIO_PIN_RESET
  HAL_RCCEx_GetPLL2ClockFreq(&PLL2_Clk_data);
  printf("SET PLL2 P:%ld, Q:%ld, R:%ld \r\n", PLL2_Clk_data.PLL2_P_Frequency, PLL2_Clk_data.PLL2_Q_Frequency, PLL2_Clk_data.PLL2_R_Frequency);
  printf("QSPI CLK %d Mhz \r\n", (uint16_t)(PLL2_Clk_data.PLL2_R_Frequency / hospi1.Init.ClockPrescaler / 1000000));

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


  HAL_RCCEx_GetPLL2ClockFreq(&temp_PLL2_Clk_data);
  printf("QSPI CLK %dMhz \r\n", temp_PLL2_Clk_data.PLL2_R_Frequency / 2 / 1000000);
  SPI_CLK_SET(10);
  HAL_RCCEx_GetPLL2ClockFreq(&temp_PLL2_Clk_data);
  printf("QSPI CLK %dMhz \r\n", temp_PLL2_Clk_data.PLL2_R_Frequency / 2 / 1000000);

  W6300Initialze();

  //ctlwizchip(CW_SYS_UNLOCK, &syslock);
  printf("W6300Initialze_ok \r\n"); 

  printf("CHIP ID(%04x) = 0x%04x \r\n", _CIDR_, getCIDR());
  printf("VERSION(%04x) = 0x%04x \r\n", _VER_, getVER());
  int32_t cnt = 0;

/*Application code */

/* ES TEST*/
#if 1
  ESTEST();

  chip_hw_reset();
  W6300Initialze();
  
/* networ init */
  ctlnetwork(CN_SET_NETINFO, &gWIZNETINFO);
  
  for (i = 0; i < 8; i++)
  {
    printf("%d : max size = %d k \r\n", i, getSn_TxMAX(i));
  }
  print_network_information();
  ES_NET_PING_TEST( WIZ_Dest_IP);


  chip_hw_reset();
  W6300Initialze();
  ctlnetwork(CN_SET_NETINFO, &gWIZNETINFO);


  ES_NET_LOOPBACK_TEST( WIZ_Dest_IP);

  while (1)
  {
    /* code */
  }
#endif 


/* buffer read write TEST - for Wiz630io TEST*/
#if 0

  HAL_RCCEx_GetPLL2ClockFreq(&temp_PLL2_Clk_data);
  printf("QSPI CLK %dMhz \r\n", temp_PLL2_Clk_data.PLL2_R_Frequency / 2 / 1000000);
  SPI_CLK_SET(30);
  HAL_RCCEx_GetPLL2ClockFreq(&temp_PLL2_Clk_data);
  printf("QSPI CLK %dMhz \r\n", temp_PLL2_Clk_data.PLL2_R_Frequency / 2 / 1000000);


  HAL_Delay(1500);
  HAL_Delay(50);
  if(QSPI_MODE < 0x03)
  {
    printf("Software Mode set : QSPI %s\r\n",mode_char[QSPI_MODE]);
  }
  else
  {
    printf("Software Mode set : BUS %02x \r\n", QSPI_MODE);
  }

  while(1){
    int  len = 8;
    uint8_t buffer[10];
    /* TX TEST */
    for(uint16_t t = 1 ; t <= 10 ; t++ ) {
      memset(buffer, t , t);
      printf("[%02x]\r\n", t );
      uint8_t result =  ES_TEST_BUFFER_TEST_write_read( WIZCHIP_TXBUF_BLOCK(i), buffer, t);
      HAL_Delay(100);
      // memset(buffer, t , len);
      // ES_SOCKET_buffer_write_read( WIZCHIP_RXBUF_BLOCK(i), buffer, len);
    }
    HAL_Delay(4000);
  }
#endif 

/* networ init */
  ctlnetwork(CN_SET_NETINFO, &gWIZNETINFO);

  for (i = 0; i < 8; i++)
  {
    printf("%d : max size = %d k \r\n", i, getSn_TxMAX(i));
  }

  print_network_information();

  printf("RTL : %x\r\n",WIZCHIP_READ((_W6300_IO_BASE_ + (0x0004 << 8) + WIZCHIP_CREG_BLOCK)));
  printf("\r\n>");
  fflush(stdout);

#if 1
  HAL_RCCEx_GetPLL2ClockFreq(&temp_PLL2_Clk_data);
  printf("QSPI CLK %dMhz \r\n", temp_PLL2_Clk_data.PLL2_R_Frequency / 2 / 1000000);
  SPI_CLK_SET(30);
  HAL_RCCEx_GetPLL2ClockFreq(&temp_PLL2_Clk_data);
  printf("QSPI CLK %dMhz \r\n", temp_PLL2_Clk_data.PLL2_R_Frequency / 2 / 1000000);
#endif 


  set_loopback_mode_W6x00(AS_IPV4);

  printf ( "getSn_TXBUF_SIZE = %d KB \r\n " ,  getSn_TXBUF_SIZE(sn)); 
  if(QSPI_MODE < 0x03)
  {
    printf("Software Mode set : QSPI %s\r\n",mode_char[QSPI_MODE]);
  }
  else
  {
    printf("Software Mode set : BUS %02x \r\n", QSPI_MODE);
  }
  HAL_RCCEx_GetPLL2ClockFreq(&temp_PLL2_Clk_data);
  printf("QSPI CLK %dMhz \r\n", temp_PLL2_Clk_data.PLL2_R_Frequency / 2 / 1000000);
  printf("IP_mode = %d \r\n", check_loopback_mode_W6x00());


  while (1)
  {
    if ((retval = loopback_tcps(SOCKET, g_udp_buf_main, 5000)) < 0)
    {
      printf(" loopback_udps error : %d\n", retval);
      while (1)
          ;
    }
  }
}

// week_Function Redefined
int _write(int fd, char *str, int len) 
{

  for (int i = 0; i < len; i++)
  {
    HAL_UART_Transmit(&huart2, (uint8_t *)&str[i], 1, 0xFFFF);
  }
  return len; //
}

#ifndef STDIN_FILENO
#define STDIN_FILENO  0
#endif

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif
// _read() 함수 재정의
int _read(int file, char *ptr, int len)
{
    // scanf가 호출될 때 _read가 불려서 문자를 받게 됨
    if(file == STDIN_FILENO)
    {
        // 여기서는 간단히 1바이트씩 블로킹으로 수신한다고 가정
        HAL_StatusTypeDef status = HAL_UART_Receive(&huart2, (uint8_t *)ptr, 1, HAL_MAX_DELAY);
        if(status == HAL_OK) return 1; // 1바이트 읽었다고 알려줌
        else                 return 0; // 실패 시 0
    }
    return 0;
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

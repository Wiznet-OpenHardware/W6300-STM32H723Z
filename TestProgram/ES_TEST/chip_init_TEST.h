#ifndef chip_init_test_h
#define chip_init_test_h

#ifdef __cplusplus
extern "C"
{
#endif

#define printf_RED(text) printf("\033[0;31m%s\033[0m", text) // red
#define printf_GREEN(text) printf("\033[0;32m%s\033[0m", text) // green
#define printf_blue(text) printf("\033[0;34m%s\033[0m", text) // blue


#define PRINT_TEST_NAME() printf("========== test = [%s] ==========\r\n" ,__func__)
#define PRINT_TEST_RESULT_SUCCESSE() printf("\033[0;32m\t\t\t\t -> [%s] test result = Success \033[0m\r\n" ,__func__)
#define PRINT_TEST_RESULT_FAIL() printf("\033[0;31m\t\t\t\t -> [%s] test result = Fail \033[0m\r\n" ,__func__)

#define SUCCESS 0 
#define FAIL    1  
#define PRINT_RESULT(result) if(result == SUCCESS){PRINT_TEST_RESULT_SUCCESSE();}else{PRINT_TEST_RESULT_FAIL(); while(1){HAL_Delay(100); }}
#define PRINT_RESULT_noWhile(result) if(result == SUCCESS){PRINT_TEST_RESULT_SUCCESSE();}else{PRINT_TEST_RESULT_FAIL(); }
    


#define PHYMODE_AUTO 0x00
#define PHYMODE_100_FDX 0x04
#define PHYMODE_100_HDX 0x05
#define PHYMODE_10_FDX 0x06
#define PHYMODE_10_HDX 0x07





void ES_SW_Reset(void);

void ES_HW_Reset(void);

uint8_t ES_get_default_value(uint16_t addr);// 방법론적으로 어떻게 받아오는지 확인해보기 
uint8_t ES_check_default(uint16_t addr );

void ES_Set_Clk_25Mhz(void);
void ES_Set_Clk_100Mhz(void);

uint8_t register_read_compare(uint16_t addr , uint16_t value );
void ES_common_register_write(uint16_t addr, uint8_t *data, uint8_t len);

uint16_t ES_PHY_MDIO_READ_TEST(uint16_t addr );
void ESTEST(void) ;
#ifdef __cplusplus
}
#endif

#endif /* _CONFIG_DATA_H_ */

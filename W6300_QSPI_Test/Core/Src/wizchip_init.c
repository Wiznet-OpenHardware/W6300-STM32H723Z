#include "wizchip_init.h"


#define MEMORY_BARRIER() __asm volatile("dmb ish" ::: "memory"); 
#define MEMORY_BARRIER2() __asm volatile("" ::: "memory") ;
			// __asm volatile("dmb ish" ::: "memory");
OSPI_RegularCmdTypeDef com;


#if 0
void csEnable(void)
{
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
}

void csDisable(void)
{
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
}

void spiWriteByte(uint8_t tx)
{
	uint8_t rx;
	HAL_SPI_TransmitReceive(&hspi1, &tx, &rx, 1, 10);
}

uint8_t spiReadByte(void)
{
	uint8_t rx = 0, tx = 0xFF;
	HAL_SPI_TransmitReceive(&hspi1, &tx, &rx, 1, 10);
	return rx;
}
#else


#endif

static volatile uint8_t qspi_flag_rx = 0 ;
//Receive Complete Callback
//void HAL_OSPI_RxCpltCallback(OSPI_HandleTypeDef *hospi)  
//__weak void HAL_OSPI_TxCpltCallback(OSPI_HandleTypeDef *hospi) 

static volatile uint8_t qspi_flag_tx = 0 ;
//Transmit Complete Callback
//void HAL_OSPI_TxCpltCallback(OSPI_HandleTypeDef *hospi) 
//__weak void HAL_OSPI_TxCpltCallback(OSPI_HandleTypeDef *hospi) 

volatile uint32_t count_qspi_t = 0, count_qspi_r = 0 ;

//Receive Complete Callback
volatile void HAL_OSPI_RxCpltCallback(OSPI_HandleTypeDef *hospi) 
{
	MEMORY_BARRIER();
	// MEMORY_BARRIER2();
	qspi_flag_rx = 0;
}

//Transmit Complete Callback
volatile void HAL_OSPI_TxCpltCallback(OSPI_HandleTypeDef *hospi)
{
	MEMORY_BARRIER();
	// MEMORY_BARRIER2();
	qspi_flag_tx = 0;
}


void W6300Initialze(void)
{
	//W6100Reset();

#if _WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_SPI_
/* SPI method callback registration */
	#if defined SPI_DMA
	reg_wizchip_spi_cbfunc(W6300SpiReadByte, W6300SpiWriteByte, W6100SpiReadBurst, W6100SpiWriteBurst);
	#else
	reg_wizchip_spi_cbfunc(W6300SpiReadByte, W6300SpiWriteByte,0,0);
    //reg_wizchip_spi_cbfunc(wizchip_read, wizchip_write, wizchip_read_buf, wizchip_write_buf);
	#endif
	/* CS function register */
	#if defined SPI_DMA
	reg_wizchip_qspi_cbfunc(W6300SpiReadByte, W6300SpiWriteByte, W6100SpiReadBurst, W6100SpiWriteBurst);
	#else
	reg_wizchip_cs_cbfunc(W6300CsEnable, W6300CsDisable);
	reg_wizchip_qspi_cbfunc( qspi_read_buf, qspi_write_buf );	
	#endif 
	//reg_wizchip_bus_cbfunc(W6300BusReadByte, W6300BusWriteByte, W6100BusReadBurst, W6100BusWriteBurst);
#else

/* Indirect bus method callback registration */
	#if defined BUS_DMA
	reg_wizchip_bus_cbfunc(W6300BusReadByte, W6300BusWriteByte, W6100BusReadBurst, W6100BusWriteBurst);
	#else
	reg_wizchip_bus_cbfunc(W6300BusReadByte, W6300BusWriteByte, 0, 0);
	#endif
	//Add 2024-09-09
	/* CS function register */
	reg_wizchip_cs_cbfunc(W6300CsEnable, W6300CsDisable);
#endif
	uint8_t temp;
//	unsigned char W6300_AdrSet[2][8] = {{2, 2, 2, 2, 2, 2, 2, 2}, {2, 2, 2, 2, 2, 2, 2, 2}};
//	unsigned char W6300_AdrSet[2][8] = {{4, 4, 4, 4, 4, 4, 4, 4}, {4, 4, 4, 4, 4, 4, 4, 4}};
	unsigned char W6300_AdrSet[2][8] = {{4, 4, 4, 4, 4, 4, 4 ,4}, {4, 4, 4, 4, 4, 4, 4 ,4}};
	// unsigned char W6300_AdrSet[2][8] = {{2, 0, 0, 0, 0, 0, 0, 0}, {2, 0, 0, 0, 0, 0, 0, 0}};
	printf("PHY OK......\r\n");

    do
	{
		if (ctlwizchip(CW_GET_PHYLINK, (void *)&temp) == -1)
		{
			printf("Unknown PHY link status.\r\n");
		}
 	} while (temp == PHY_LINK_OFF);

	printf("PHY OK.\r\n");

	temp = IK_DEST_UNREACH;


	if (ctlwizchip(CW_INIT_WIZCHIP, (void *)W6300_AdrSet) == -1)
	{
		printf("W6300 initialized fail.\r\n");
	}else{
		printf("!!W6300 initialized OK!! .\r\n");
	}

	if (ctlwizchip(CW_SET_INTRMASK, &temp) == -1)
	{
		printf("W6300 interrupt\r\n");
	}

	printf("interrupt mask: %02x\r\n",getIMR());
}

//uint8_t Echo_Server_Process(uint8_t mode, uint16_t port);
uint8_t Get_W6300_main_IF_MODE(void)
{
	uint8_t value = 0;
	value |= (HAL_GPIO_ReadPin(MOD0_GPIO_Port, MOD0_Pin)<<0);
	value |= (HAL_GPIO_ReadPin(MOD1_GPIO_Port, MOD1_Pin)<<1);
	value |= (HAL_GPIO_ReadPin(MOD2_GPIO_Port, MOD2_Pin)<<2);
	value |= (HAL_GPIO_ReadPin(MOD3_GPIO_Port, MOD3_Pin)<<3);
    //W6300_mode = value;
	return value;
}

void FPGA_Reset(void)
 {
 	printf("FPGA reset\r\n");
 	HAL_GPIO_WritePin(RSTn_GPIO_Port, RSTn_Pin, GPIO_PIN_RESET);
 	HAL_Delay(500);
 	HAL_GPIO_WritePin(RSTn_GPIO_Port, RSTn_Pin, GPIO_PIN_SET);
 	HAL_Delay(500);
 	printf("FPGA reset complete!\r\n");
 }



//data write/read function list

void W6300BusWriteByte(uint32_t addr, iodata_t data)
{
}

iodata_t W6300BusReadByte(uint32_t addr)
{
 	return 0;
}


 void W6300SpiWriteByte(uint8_t tx)
 {
 }

 uint8_t W6300SpiReadByte(void)
 {
 	return 0;
 }



uint8_t qspi_read_buf(uint8_t op_code, uint32_t AddrSel, uint8_t *pbuf, uint16_t len)
{
    uint32_t OSPI_status = 0;
    uint8_t ret = 0;
	#if  QSPI_MODE == QSPI_MODE_QUAD 
	com.AddressMode = HAL_OSPI_ADDRESS_4_LINES;
	com.DataMode = HAL_OSPI_DATA_4_LINES;
	//com.InstructionMode = HAL_OSPI_INSTRUCTION_4_LINES;//QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...
	com.DummyCycles = 2;

    #elif  QSPI_MODE == QSPI_MODE_DUAL
	com.AddressMode = HAL_OSPI_ADDRESS_2_LINES;
	com.DataMode = HAL_OSPI_DATA_2_LINES;
	//com.InstructionMode = HAL_OSPI_INSTRUCTION_4_LINES;//QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...
	com.DummyCycles = 4	;

	#else  
	com.AddressMode = HAL_OSPI_ADDRESS_1_LINE;
	com.DataMode = HAL_OSPI_DATA_1_LINE;
	//com.InstructionMode = HAL_OSPI_INSTRUCTION_4_LINES;//QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...
	com.DummyCycles =  8	;
	#endif 	
	 
	com.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;//HAL_OSPI_INSTRUCTION_4_LINES;//QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...
	com.Instruction = op_code;//0xAB;    // Command
	com.AddressSize = HAL_OSPI_ADDRESS_16_BITS;
	//com.AddressMode = HAL_OSPI_ADDRESS_1_LINE;//HAL_OSPI_ADDRESS_4_LINES;//QSPI_ADDRESS_1_LINE;
	com.Address = AddrSel;//0x00000000;

	com.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytes = HAL_OSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytesSize = HAL_OSPI_ALTERNATE_BYTES_NONE;

	//com.DataMode = HAL_OSPI_DATA_1_LINE;//HAL_OSPI_DATA_4_LINES;
	//com.NbData = 1;

	com.DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE;
	//com.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	com.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;

	com.NbData = len;
	com.DQSMode = HAL_OSPI_DQS_DISABLE;
    #if 0
	if (HAL_OSPI_Command(&hospi1, &com, HAL_OSPI_TIMEOUT_DEFAULT_VALUE)
		  != HAL_OK)
	{
		printf("[%s > %s : %d]Cmd Error \r\n",__FILE__, __FUNCTION__, __LINE__ );
		return 1;
	}
	if (HAL_OSPI_Receive(&hospi1, pbuf, HAL_OSPI_TIMEOUT_DEFAULT_VALUE)
		!= HAL_OK)
	{
		printf("[%s > %s : %d]Recv Error \r\n",__FILE__, __FUNCTION__, __LINE__ );
		return 2;
	}
        #endif
#if 0
    while((OSPI_status=HAL_OSPI_GetState(&hospi1)) > 0)
    {
        hospi1.Instance->FCR = OSPI_status;
    }
#endif
	//if ((ret = HAL_OSPI_Command(&hospi1, &com, HAL_OSPI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK)
	if ((ret = HAL_OSPI_Command(&hospi1, &com, 100)) != HAL_OK)
	{
		printf("[%s > %s : %d]Cmd Error ret:%02X st:%X\r\n",__FILE__, __FUNCTION__, __LINE__ , ret, (uint16_t)OSPI_status);
		return 4;
	}
	//OSPI_status=HAL_OSPI_GetState(&hospi1);
	qspi_flag_rx = 1;
	if ((ret = HAL_OSPI_Receive_DMA(&hospi1, pbuf)) != HAL_OK)
	//if ((ret = HAL_OSPI_Receive_IT(&hospi1, pbuf)) != HAL_OK)
	{
		printf("[%s > %s : %d]Recv Error ret:%02X st:%X\r\n",__FILE__, __FUNCTION__, __LINE__, ret, (uint16_t)OSPI_status);
		return 5;
	}
#if 1
		while(qspi_flag_rx)
		{
			MEMORY_BARRIER();
			__WFI(); // Reduce CPU load by waiting
			// count_qspi_t ++ ; 
			// if(count_qspi_t  >= 1000000 ){
			// 	printf("[%s > %s : %d ] qspi_flag_tx Error ret:%02X, st:%X\r\n",__FILE__, __FUNCTION__, __LINE__, ret, (uint16_t)OSPI_status);
			// 	count_qspi_t = 0;
			// }
		}
		//count and return
		//Add 24.09.23 irina
		//if(count_qspi_r <=250000){
		//		printf("[%s > %s : %d ] qspi_flag_rx Error ret:%02X, st:%X\r\n",__FILE__, __FUNCTION__, __LINE__, ret, (uint16_t)OSPI_status);
		//		count_qspi_r = 0;
		//	}
		///
#endif
	//qspi_flag_rx = 1;
#if 0
    while((HAL_OSPI_GetState(&hospi1)) != HAL_OSPI_STATE_READY)
    {
        //hospi1.Instance->FCR = OSPI_status;
    	hospi1.State = 0;
    }
#endif
#if 0
	while((OSPI_status=HAL_OSPI_GetState(&hospi1)) == ( HAL_OSPI_STATE_BUSY_RX || HAL_OSPI_STATE_BUSY_AUTO_POLLING || HAL_OSPI_STATE_BUSY_MEM_MAPPED));
#endif
#if 0 // !!Do not enable this code , if you use optimization opthion, than will be crash  _lihan.
	while((OSPI_status=HAL_OSPI_GetState(&hospi1)) != HAL_OSPI_STATE_READY)
	{
		if((OSPI_status == HAL_OSPI_STATE_ABORT)||(OSPI_status == HAL_OSPI_STATE_ERROR))
		{
			while((OSPI_status = HAL_OSPI_Abort_IT(&hospi1)) != HAL_OK)
			{
				printf("HAL_OSPI_Abort_IT Error = 0x%02x\r\n", (uint16_t)OSPI_status);
			}
		}
	}
#endif
#if 0
	while(!((OSPI_status=hospi1.Instance->SR) & 0x13))
	{
		hospi1.Instance->FCR = OSPI_status;
	}
#endif
	return 0;
}

uint8_t qspi_write_buf(uint8_t op_code, uint32_t AddrSel, uint8_t *pbuf, uint16_t len)
{
    uint32_t OSPI_status = 0;
    uint8_t ret = 0;

	#if  QSPI_MODE == QSPI_MODE_QUAD 
	com.AddressMode = HAL_OSPI_ADDRESS_4_LINES;
	com.DataMode = HAL_OSPI_DATA_4_LINES;
	//com.InstructionMode = HAL_OSPI_INSTRUCTION_4_LINES;//QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...
	com.DummyCycles = 2	;

    #elif  QSPI_MODE == QSPI_MODE_DUAL
	com.AddressMode = HAL_OSPI_ADDRESS_2_LINES;
	com.DataMode = HAL_OSPI_DATA_2_LINES;
	//com.InstructionMode = HAL_OSPI_INSTRUCTION_4_LINES;//QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...
	com.DummyCycles = 4	;

	#else  
	com.AddressMode = HAL_OSPI_ADDRESS_1_LINE;
	com.DataMode = HAL_OSPI_DATA_1_LINE;
	//com.InstructionMode = HAL_OSPI_INSTRUCTION_4_LINES;//QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...
	com.DummyCycles = 8	;
	#endif 	
	 
	com.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;//HAL_OSPI_INSTRUCTION_4_LINES;//QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...
	com.Instruction = op_code;//0xAB;    // Command
	com.AddressSize = HAL_OSPI_ADDRESS_16_BITS;
	//com.AddressMode = HAL_OSPI_ADDRESS_1_LINE;//HAL_OSPI_ADDRESS_4_LINES;//QSPI_ADDRESS_1_LINE;
	com.Address = AddrSel;//0x00000000;
	
	com.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytes = HAL_OSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytesSize = HAL_OSPI_ALTERNATE_BYTES_NONE;

	//com.DataMode = HAL_OSPI_DATA_1_LINE;//HAL_OSPI_DATA_4_LINES;
	//com.NbData = 1;

	com.DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE;
	//com.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	com.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;

	com.NbData = len;
	//add 24.09.13
	com.DQSMode = HAL_OSPI_DQS_DISABLE;
    #if 0
	//printf("send data[%d]:%s\r\n >",len, data);
	if (HAL_OSPI_Command(&hospi1, &com, HAL_OSPI_TIMEOUT_DEFAULT_VALUE)
		!= HAL_OK)
	{
		printf("[%s > %s : %d]CMD Error \r\n",__FILE__, __FUNCTION__, __LINE__ );
		return 1;
	}
	if (HAL_OSPI_Transmit(&hospi1, pbuf, HAL_OSPI_TIMEOUT_DEFAULT_VALUE)
		!= HAL_OK)
	{
		printf("[%s > %s : %d]Send Error \r\n",__FILE__, __FUNCTION__, __LINE__ );
		return 2;
	}
        #endif
#if 0
    while((OSPI_status=HAL_OSPI_GetState(&hospi1)) > 0)
	{
	    hospi1.Instance->FCR = OSPI_status;
	}
#endif
	//if ((ret = HAL_OSPI_Command(&hospi1, &com, HAL_OSPI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK)
  if ((ret = HAL_OSPI_Command(&hospi1, &com, 100)) != HAL_OK)
	{
		printf("[%s > %s : %d]CMD Error ret:%02X st:%X\r\n",__FILE__, __FUNCTION__, __LINE__, ret, (uint16_t)OSPI_status);
		return 4;
	}
	qspi_flag_tx = 1;
	//TX_OFF();
	if ((ret = HAL_OSPI_Transmit_DMA(&hospi1, pbuf)) != HAL_OK)
	{
		printf("[%s > %s : %d]Send Error ret:%02X, st:%X\r\n",__FILE__, __FUNCTION__, __LINE__, ret, (uint16_t)OSPI_status);
		return 5;
	}

#if 1
	while(qspi_flag_tx)
	{
		// __asm volatile("" ::: "memory") ;
		// __asm volatile("dmb ish" ::: "memory");
		MEMORY_BARRIER();
		 __WFI(); // Reduce CPU load by waiting	
		// count_qspi_t ++ ; 
		// if(count_qspi_t  >= 1000000 ){
		// 	printf("[%s > %s : %d ] qspi_flag_tx Error ret:%02X, st:%X\r\n",__FILE__, __FUNCTION__, __LINE__, ret, (uint16_t)OSPI_status);
		// 	count_qspi_t = 0;
		// }
		// count and return
		// printf("TEST\r\n");
	}
#endif
	//TX_ON();
	//qspi_flag_tx = 1;
#if 0
	while((OSPI_status=HAL_OSPI_GetState(&hospi1)) > 4)
    {
        //hospi1.Instance->FCR = OSPI_status;
		//hospi1.State = 0;
    }
#endif
#if 0 //ADD 2024.09.25
	while((OSPI_status=HAL_OSPI_GetState(&hospi1)) == (HAL_OSPI_STATE_BUSY_CMD || HAL_OSPI_STATE_BUSY_TX || HAL_OSPI_STATE_BUSY_RX || HAL_OSPI_STATE_BUSY_AUTO_POLLING || HAL_OSPI_STATE_BUSY_MEM_MAPPED));

#endif
#if 1
	while((OSPI_status=HAL_OSPI_GetState(&hospi1)) != HAL_OSPI_STATE_READY)
	{
		if((OSPI_status == HAL_OSPI_STATE_ABORT)||(OSPI_status == HAL_OSPI_STATE_ERROR))
		{
			while((OSPI_status = HAL_OSPI_Abort_IT(&hospi1)) != HAL_OK)
			{
				printf("HAL_OSPI_Abort_IT Error = 0x%02x\r\n", (uint16_t)OSPI_status);
			}
		}
	}
#endif
#if 0
	while(!((OSPI_status=hospi1.Instance->SR) & 0x13))
		{
			hospi1.Instance->FCR = OSPI_status;
		}
#endif
	return 0;
}


void TRACE_ON(void)
 {
 	HAL_GPIO_WritePin(Trace_GPIO_Port, Trace_Pin, GPIO_PIN_RESET);
 }
void TRACE_OFF(void)
 {
 	HAL_GPIO_WritePin(Trace_GPIO_Port, Trace_Pin, GPIO_PIN_SET);
 }
 
void chip_hw_reset(void)
 {
   //printf("W6300 Hardware Reset \r\n" );
 	HAL_GPIO_WritePin(RSTn_GPIO_Port, RSTn_Pin, GPIO_PIN_RESET);
 	HAL_Delay(500);
 	HAL_GPIO_WritePin(RSTn_GPIO_Port, RSTn_Pin, GPIO_PIN_SET);
 	HAL_Delay(500);
 }
 //Add 2024-09-09
void chip_sw_reset(void)
 {
     printf("W6300 Software Reset \r\n" );
 }

void W6300CsEnable(void)
 {
 //	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_3, GPIO_PIN_RESET);
 	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_6, GPIO_PIN_RESET);
 	//Add 2024-09-09
 	//HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
 }

void W6300CsDisable(void)
{
	//HAL_GPIO_WritePin(GPIOF, GPIO_PIN_3, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_6, GPIO_PIN_SET);
	//Add 2024-09-09
	//HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

}
#if 0 
 char qspi_set_parameter(QSPI_Set_Data *init_data)
 {
     switch(init_data->mode)
     {
         case 's':
             com.AddressMode = HAL_OSPI_ADDRESS_1_LINE;
             com.DataMode = HAL_OSPI_DATA_1_LINE;//HAL_OSPI_DATA_4_LINES;
             break;
         case 'd':
             com.AddressMode = HAL_OSPI_ADDRESS_2_LINES;
             com.DataMode = HAL_OSPI_DATA_2_LINES;
             break;
         case 'q':
             com.AddressMode = HAL_OSPI_ADDRESS_4_LINES;
             com.DataMode = HAL_OSPI_DATA_4_LINES;
             break;
     }
     com.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;//HAL_OSPI_INSTRUCTION_4_LINES;//QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...
     com.Instruction = init_data->instruction;//0xAB;    // Command
     com.AddressSize = HAL_OSPI_ADDRESS_16_BITS;
     //com.AddressMode = HAL_OSPI_ADDRESS_1_LINE;//HAL_OSPI_ADDRESS_4_LINES;//QSPI_ADDRESS_1_LINE;
     com.Address = init_data->addr;//0x00000000;

     com.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
     com.AlternateBytes = HAL_OSPI_ALTERNATE_BYTES_NONE;
     com.AlternateBytesSize = HAL_OSPI_ALTERNATE_BYTES_NONE;

     com.DummyCycles = init_data->dummy;
     //com.DataMode = HAL_OSPI_DATA_1_LINE;//HAL_OSPI_DATA_4_LINES;
     //com.NbData = 1;

     com.DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE;
     //com.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
     com.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;
     return 0;
 }
 #endif 

 char send_spi_data(int len, unsigned char *data)
 {
 	uint8_t ret_data = 0;
     com.NbData = (uint32_t)len;
     printf("send data[%d]:%s\r\n >",len, data);
     if ((ret_data =HAL_OSPI_Command(&hospi1, &com, HAL_OSPI_TIMEOUT_DEFAULT_VALUE))
         != HAL_OK)
     {
         printf("[%s > %s : %d]CMD Error[%02x] \r\n",__FILE__, __FUNCTION__, __LINE__ , ret_data);
         return 1;
     }
     if ((ret_data = HAL_OSPI_Transmit(&hospi1, (uint8_t *)data, HAL_OSPI_TIMEOUT_DEFAULT_VALUE))
         != HAL_OK)
     {
         printf("[%s > %s : %d]Send Error[%02x] \r\n",__FILE__, __FUNCTION__, __LINE__ , ret_data);
         return 2;
     }
     printf("send complete!!\r\n");
     return 0;
 }
 char recv_spi_data(int len, unsigned char *data)
 {
     com.NbData = len;
     if (HAL_OSPI_Command(&hospi1, &com, HAL_OSPI_TIMEOUT_DEFAULT_VALUE)
           != HAL_OK)
     {
         printf("[%s > %s : %d]Cmd Error \r\n",__FILE__, __FUNCTION__, __LINE__ );
         return 1;
     }
     if (HAL_OSPI_Receive(&hospi1, data, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
     {
         printf("[%s > %s : %d]Recv Error \r\n",__FILE__, __FUNCTION__, __LINE__ );
         return 2;
     }
     return 0;
 }

//function list
uint32_t get_time(void)
{
	return HAL_GetTick();	
}
uint32_t get_us_time()
{
  	return HAL_GetTick();
}

void wiz_delay(uint32_t delay_time)
 {
   HAL_Delay(delay_time);
 }

void TX_ON(void)
{
	HAL_GPIO_WritePin(SPI_EN_GPIO_Port, SPI_EN_Pin, GPIO_PIN_RESET);
}
void TX_OFF(void)
{
	HAL_GPIO_WritePin(SPI_EN_GPIO_Port, SPI_EN_Pin, GPIO_PIN_SET);
}

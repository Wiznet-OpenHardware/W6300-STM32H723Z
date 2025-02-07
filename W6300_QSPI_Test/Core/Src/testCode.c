#include "wizchip_init.h"
#include "main.h"

#include "PHY_IP101G.h"

#include "W6300_TestProcess.h"

#include "W6300_TestProcess.h"
OSPI_RegularCmdTypeDef com;


uint8_t PHY_Scen_mode = 0, PHY_Scen_Seq = 0, PHY_OP_mode = 0;
uint8_t CLI_Answer = 0;
uint8_t data_buf[4096];

uint8_t w6300_qspi_read_buf(uint8_t op_code, uint32_t AddrSel, uint8_t *pbuf, uint16_t len); //for testcode
uint8_t w6300_qspi_write_buf(uint8_t op_code, uint32_t AddrSel, uint8_t *pbuf, uint16_t len); // for testcode   

uint8_t w6300_qspi_read_buf(uint8_t op_code, uint32_t AddrSel, uint8_t *pbuf, uint16_t len)
{
	uint8_t qspi_mode = (op_code >> 6) & 0x03;
	uint8_t temp_op = op_code &0xdf;
    uint8_t ret  = 0, ret1 = 0;
    uint32_t OSPI_status = 0, OSPI_error = 0;
	switch(qspi_mode)
	{
		case 0x00:	//single
			com.AddressMode = HAL_OSPI_ADDRESS_1_LINE;
			com.DataMode = HAL_OSPI_DATA_1_LINE;//HAL_OSPI_DATA_4_LINES;

			com.DummyCycles = 8;
			break;
		case 0x01:	//dual
			com.AddressMode = HAL_OSPI_ADDRESS_2_LINES;
			com.DataMode = HAL_OSPI_DATA_2_LINES;
			//com.InstructionMode = HAL_OSPI_INSTRUCTION_2_LINES;//QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...
			com.DummyCycles = 4;
			break;
		case 0x02:	//quad
			com.AddressMode = HAL_OSPI_ADDRESS_4_LINES;
			com.DataMode = HAL_OSPI_DATA_4_LINES;
			//com.InstructionMode = HAL_OSPI_INSTRUCTION_4_LINES;//QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...
			com.DummyCycles = 2;
			break;
		default :
			com.AddressMode = HAL_OSPI_ADDRESS_1_LINE;
			com.DataMode = HAL_OSPI_DATA_1_LINE;//HAL_OSPI_DATA_4_LINES;
			//com.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;//HAL_OSPI_INSTRUCTION_4_LINES;//QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...
			com.DummyCycles = 0;
			break;
	}
	com.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;//HAL_OSPI_INSTRUCTION_4_LINES;//QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...

	//com.Instruction = op_code;//0xAB;    // Commandtemp_op
	com.Instruction = temp_op;//0xAB;    // Command
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
    while((OSPI_status=HAL_OSPI_GetState(&hospi1)) > 4)
    {
        hospi1.Instance->FCR = OSPI_status;
        //OSPI_status=HAL_OSPI_GetState(&hospi1);
        //printf("r ospi status = %X\r\n", OSPI_status, OSPI_error);
        #if 0
        OSPI_error = HAL_OSPI_GetError(&hospi1);
    	printf("r ospi status = %X Error = %X\r\n", OSPI_status, OSPI_error);
         hospi1.Instance->FCR = OSPI_status;
        OSPI_status=HAL_OSPI_GetState(&hospi1);
        OSPI_error = HAL_OSPI_GetError(&hospi1); 
	    printf("r 2 ospi status = %X error = %X\r\n", OSPI_status, OSPI_error);
        #endif
    }
	if ((ret = HAL_OSPI_Command(&hospi1, &com, HAL_OSPI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK)
	{
	    ret1 = HAL_MDMA_GetState(&hospi1.hmdma);
		printf("[%s > %s : %d]Cmd Error ret:%02X ret1:%02X st:%d\r\n",__FILE__, __FUNCTION__, __LINE__ , ret, ret1, OSPI_status);
		return 4;
	}
          #if 0
	if (HAL_OSPI_Receive(&hospi1, pbuf, HAL_OSPI_TIMEOUT_DEFAULT_VALUE)
		!= HAL_OK)
	#else
    //OSPI_status=HAL_OSPI_GetState(&hospi1);
	if ((ret = HAL_OSPI_Receive_DMA(&hospi1, pbuf)) != HAL_OK)
		#endif
	{
	    ret1 = HAL_MDMA_GetState(&hospi1.hmdma);
		printf("[%s > %s : %d]Recv Error ret:%02X ret1:%02X st:%d\r\n",__FILE__, __FUNCTION__, __LINE__, ret, ret1, OSPI_status);
		return 5;
	}
    //OSPI_status=HAL_OSPI_GetState(&hospi1);
    //ret1 = HAL_MDMA_GetState(&hospi1.hmdma);
    while((OSPI_status=HAL_OSPI_GetState(&hospi1)) > 4)
    {
        hospi1.Instance->FCR = OSPI_status;
        ///OSPI_status=HAL_OSPI_GetState(&hospi1);
        #if 0
        OSPI_error = HAL_OSPI_GetError(&hospi1);
    	printf("r1 ospi status = %X Error = %X\r\n", OSPI_status, OSPI_error);
         hospi1.Instance->FCR = OSPI_status;
        OSPI_status=HAL_OSPI_GetState(&hospi1);
        OSPI_error = HAL_OSPI_GetError(&hospi1); 
	    printf("r1 2 ospi status = %X error = %X\r\n", OSPI_status, OSPI_error);
        #endif
    }
    //printf("dma status: %02x\r\n", ret);
	return qspi_mode;
}
uint8_t w6300_qspi_write_buf(uint8_t op_code, uint32_t AddrSel, uint8_t *pbuf, uint16_t len)
{
	uint8_t qspi_mode = (op_code >> 6) & 0x03;
	uint8_t temp_op = op_code | 0x20;
    uint8_t ret  = 0, ret1 = 0;
    uint32_t OSPI_status = 0, OSPI_error = 0;

	switch(qspi_mode)
	{
		case 0x00:	//single
			com.AddressMode = HAL_OSPI_ADDRESS_1_LINE;
			com.DataMode = HAL_OSPI_DATA_1_LINE;//HAL_OSPI_DATA_4_LINES;
			//com.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;//HAL_OSPI_INSTRUCTION_4_LINES;//QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...
			com.DummyCycles = 8;
			break;
		case 0x01:	//dual
			com.AddressMode = HAL_OSPI_ADDRESS_2_LINES;
			com.DataMode = HAL_OSPI_DATA_2_LINES;
			//com.InstructionMode = HAL_OSPI_INSTRUCTION_2_LINES;//QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...
			com.DummyCycles = 4;
			break;
		case 0x02:	//quad
			com.AddressMode = HAL_OSPI_ADDRESS_4_LINES;
			com.DataMode = HAL_OSPI_DATA_4_LINES;
			//com.InstructionMode = HAL_OSPI_INSTRUCTION_4_LINES;//QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...
			com.DummyCycles = 2;
			break;
		default :
			com.AddressMode = HAL_OSPI_ADDRESS_1_LINE;
			com.DataMode = HAL_OSPI_DATA_1_LINE;//HAL_OSPI_DATA_4_LINES;
			//com.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;//HAL_OSPI_INSTRUCTION_4_LINES;//QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...
			com.DummyCycles = 0;
			break;
	}
	com.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;//HAL_OSPI_INSTRUCTION_4_LINES;//QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...
	//com.Instruction = op_code;//0xAB;    // Command
	com.Instruction = temp_op;//0xAB;    // Command
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
	//printf("send data[%d]:%s\r\n >",len, data);
	while((OSPI_status=HAL_OSPI_GetState(&hospi1)) > 4)
	{
	    hospi1.Instance->FCR = OSPI_status;
        //OSPI_status=HAL_OSPI_GetState(&hospi1);
	#if 0
	    OSPI_error = HAL_OSPI_GetError(&hospi1); 
	    printf("w ospi status = %X error = %X\r\n", OSPI_status, OSPI_error);
        hospi1.Instance->FCR = OSPI_status;
        OSPI_status=HAL_OSPI_GetState(&hospi1);
        OSPI_error = HAL_OSPI_GetError(&hospi1); 
	    printf("w 2 ospi status = %X error = %X\r\n", OSPI_status, OSPI_error);
        #endif
	}
	if ((ret = HAL_OSPI_Command(&hospi1, &com, HAL_OSPI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK)
	{
	    ret1 = HAL_MDMA_GetState(&hospi1.hmdma);
		printf("[%s > %s : %d]CMD Error ret:%02X ret1:%02X st:%d\r\n",__FILE__, __FUNCTION__, __LINE__, ret, ret1, OSPI_status);
		return 4;
	}
        #if 0
	if (HAL_OSPI_Transmit(&hospi1, pbuf, HAL_OSPI_TIMEOUT_DEFAULT_VALUE)
		!= HAL_OK)
	#else
    //OSPI_status=HAL_OSPI_GetState(&hospi1);
	if ((ret = HAL_OSPI_Transmit_DMA(&hospi1, pbuf)) != HAL_OK)
		//HAL_MDMA_GetState
		#endif
	{
	    ret1 = HAL_MDMA_GetState(&hospi1.hmdma);
		printf("[%s > %s : %d]Send Error ret:%02X, ret1:%02X st:%d\r\n",__FILE__, __FUNCTION__, __LINE__, ret, ret1, OSPI_status);
		return 5;
	}
    //OSPI_status=HAL_OSPI_GetState(&hospi1);
	///ret1 = HAL_MDMA_GetState(&hospi1.hmdma);
	while((OSPI_status=HAL_OSPI_GetState(&hospi1)) > 4)
    {
            hospi1.Instance->FCR = OSPI_status;
    #if 0
        OSPI_error = HAL_OSPI_GetError(&hospi1);
    	printf("w1 ospi status = %X Error = %X\r\n", OSPI_status, OSPI_error);
         hospi1.Instance->FCR = OSPI_status;
        OSPI_status=HAL_OSPI_GetState(&hospi1);
        OSPI_error = HAL_OSPI_GetError(&hospi1); 
	    printf("w1 2 ospi status = %X error = %X\r\n", OSPI_status, OSPI_error);
        #endif
    }
    //printf("dma status: %02x\r\n", ret);
	return qspi_mode;
}


uint8_t PHY_IP101_01_SCEN(void)
{
  //control reg
  //uint16_t ret = 0;
  PHY_IP101_CONTR_REG *TempPHY_Ctrl;
  PHY_IP101_MDIX_CTRL_STA_REG *TempPHY_MCS_Reg;
  uint16_t temp_ctr_reg = 0;
  uint16_t temp_R_OP_Mode = 0;
  uint16_t tempCnt = 0;

  char FullHalf[2][5] = {"Half", "Full"};
  char SpeedDisp[2][5] ={"10M", "100M"};
  uint16_t temp_phy_ctrl[5]={0x3100, 0x2100, 0x2000, 0x0100, 0x0000};
  uint8_t phy_set_msg[5][5]={"Auto", "100F", "100H", "10F", "10H"};
  //                          Auto,   100F,   100H,   10F,   10H
  TempPHY_Ctrl = (PHY_IP101_CONTR_REG *)&temp_ctr_reg;
  TempPHY_MCS_Reg = (PHY_IP101_MDIX_CTRL_STA_REG *)&temp_R_OP_Mode;
  //Set auto
  switch(PHY_Scen_Seq)
  {
    case 0:
      printf("Please Set Link Partner Operation Mode '%s' next stage input 'yes' exit 'no'\r\n",phy_set_msg[PHY_OP_mode]);
      CLI_Answer = 0;
      PHY_Scen_Seq++;
    break;
    case 1:
      if(CLI_Answer == 1)
      {

        //start test
        PHY_address_set(PHY_ADDR);
        //set phy op mode
        PHY_write_register(PHY_IP101_CtrlReg, temp_phy_ctrl[PHY_OP_mode]);
        temp_ctr_reg = PHY_read_register(PHY_IP101_CtrlReg);
        printf("Set reg 0x%02x Val : 0x%04x -> 0x%04x \r\n", PHY_IP101_CtrlReg, temp_phy_ctrl[PHY_OP_mode], temp_ctr_reg);
        //
        //phy reset
        TempPHY_Ctrl->Reset = 1;
        printf("Set reset reg 0x%02x Val : 0x%04x \r\n", PHY_IP101_CtrlReg, temp_ctr_reg);
        PHY_write_register(PHY_IP101_CtrlReg, temp_ctr_reg);
        tempCnt = 0;
        while(TempPHY_Ctrl->Reset == 1)
        {
          HAL_Delay(100);
          temp_ctr_reg = PHY_read_register(PHY_IP101_CtrlReg);
          tempCnt++;
          if(tempCnt > 50)
          {
            printf("reset count Over Error \r\n");
            //reset Error
            return 0xE0;
          }
        }
        HAL_Delay(500);
        //link check
        printf("Check Link status\r\n");
        tempCnt = 0;
        while(PHY_IP101_Link_Check()==0)
        {
          HAL_Delay(100);
          tempCnt++;
          if(tempCnt > 50)
          {
            printf("Check Link Error %d \r\nBack to the previous Stage\r\n", tempCnt);
            PHY_Scen_Seq--;
            return 0xE1;
          }
        }
        temp_R_OP_Mode = PHY_IP101_Page_Read(0x0010, PHY_IP101_MDI_MDIX_CtrlSpecStaReg);
        printf("Link Ok 0x%04x , 0x%02x\r\n", temp_R_OP_Mode, TempPHY_MCS_Reg->OP_MODE_IND);
        if(TempPHY_MCS_Reg->OP_MODE_IND != 0)
        {
          printf("Link OP %s %s \r\n",SpeedDisp[(TempPHY_MCS_Reg->OP_MODE_IND&0x03)-1], FullHalf[(TempPHY_MCS_Reg->OP_MODE_IND>>2)&0x01]);
        }
        printf("Set loopback port 5000 test if test finish and next test input 'yes' exit 'no' \r\n");
        PHY_Scen_Seq++;
        CLI_Answer = 0;
      }
      else if(CLI_Answer == 2)
      {
        CLI_Answer = 0;
        PHY_Scen_mode = 0;
        PHY_Scen_Seq = 0;
        printf("Terminate Test Midway!!\r\n ");
        //finish Test
      }
    break;
  case 2: //loopback test
  //  loopback_tcps_t(0, data_buf, 5000, AS_IPV4);

    if(CLI_Answer == 1)
    {
      CLI_Answer = 0;
      printf("loopback test finish");
      close(0);
      PHY_Scen_Seq=0;
      PHY_OP_mode++;
      if(PHY_OP_mode > 4)
      {
        printf("PHY01 test Finish\r\n");
        PHY_Scen_mode = 0;
        PHY_Scen_Seq = 0;
        //end flag
      }
    }
    else if(CLI_Answer == 2)
    {
      CLI_Answer = 0;
      PHY_Scen_mode = 0;
      PHY_Scen_Seq = 0;
      printf("Terminate Test Midway!!\r\n ");
      close(0);
      //
    }
    break;
  }
  return 0;
}
uint8_t PHY_IP101_02_SCEN(void)
{
 // uint16_t ret = 0;
  static uint8_t MDIX_Mode = 0, cable_sel = 0;
  PHY_IP101_CONTR_REG *TempPHY_Ctrl;
  PHY_IP101_MDIX_CTRL_STA_REG *TempPHY_MCS_Reg;
  PHY_IP101_SPEC_CTRL_REG *TempPHY_SCR_Reg;
  PHY_IP101_STATUS_REG *TempPHY_SR_Reg;
  uint16_t temp_ctr_reg = 0, temp_R_OP_Mode = 0, temp_RW_MDIX = 0, temp_Stutus;
  uint16_t tempCnt = 0;
  TempPHY_Ctrl = (PHY_IP101_CONTR_REG *)&temp_ctr_reg;
  TempPHY_MCS_Reg = (PHY_IP101_MDIX_CTRL_STA_REG *)&temp_R_OP_Mode;
  TempPHY_SCR_Reg = (PHY_IP101_SPEC_CTRL_REG *)&temp_RW_MDIX;
  TempPHY_SR_Reg = (PHY_IP101_STATUS_REG *)&temp_Stutus;

  char MDIX_Sell[2][8] ={"Enabe", "Disable"};
  char FullHalf[2][5] = {"Half", "Full"};
  char SpeedDisp[2][5] ={"10M", "100M"};
  uint16_t temp_phy_ctrl[5]={0x3100, 0x2100, 0x2000, 0x0100, 0x0000};
  uint8_t phy_set_msg[5][5]={"Auto", "100F", "100H", "10F", "10H"};
  //                          Auto,   100F,   100H,   10F,   10H

  switch(PHY_Scen_Seq)
  {
    case 0:
      PHY_address_set(PHY_ADDR);
      MDIX_Mode = 0;
      PHY_OP_mode = 0;
      PHY_Scen_Seq++;
    break;
    case 1:
      printf("Set Auto-MDIX %s \r\n", MDIX_Sell[MDIX_Mode]);
      //printf("Set Auto DMIX And Auto Nego \r\n");
      //Auto-DMIX
      temp_RW_MDIX = PHY_IP101_Page_Read(0x0010, PHY_IP101_SpecCtrlReg);
      if(TempPHY_SCR_Reg->AUTO_MDIX_DIS != MDIX_Mode)
      {
        TempPHY_SCR_Reg->AUTO_MDIX_DIS = MDIX_Mode;
        PHY_IP101_Page_Write(0x0010, PHY_IP101_SpecCtrlReg, temp_RW_MDIX);
        temp_RW_MDIX = PHY_IP101_Page_Read(0x0010, PHY_IP101_SpecCtrlReg);
      }
      PHY_Scen_Seq++;
    break;
    case 2:
      //set phy op mode
      PHY_write_register(PHY_IP101_CtrlReg, temp_phy_ctrl[PHY_OP_mode]);
      temp_ctr_reg = PHY_read_register(PHY_IP101_CtrlReg);
      printf("Set reg 0x%02x Val : 0x%04x -> 0x%04x \r\n", PHY_IP101_CtrlReg, temp_phy_ctrl[PHY_OP_mode], temp_ctr_reg);
      //
      //phy reset
      TempPHY_Ctrl->Reset = 1;
      printf("Set reset reg 0x%02x Val : 0x%04x \r\n", PHY_IP101_CtrlReg, temp_ctr_reg);
      PHY_write_register(PHY_IP101_CtrlReg, temp_ctr_reg);
      tempCnt = 0;
      while(TempPHY_Ctrl->Reset == 1)
      {
        HAL_Delay(100);
        temp_ctr_reg = PHY_read_register(PHY_IP101_CtrlReg);
        tempCnt++;
        if(tempCnt > 50)
        {
          printf("reset count Over Error \r\n");
          //reset Error
          return 0xE0;
        }
      }
      printf("Please Set Link Partner Operation Mode '%s'\r\n",phy_set_msg[PHY_OP_mode]);
      printf("And Set Direct Cable connects to each other next stage input 'yes' exit 'no'\r\n");
      CLI_Answer = 0;
      cable_sel = 0;
      PHY_Scen_Seq++;
    break;
    case 3:
      if(CLI_Answer == 1)
      {
        printf("Check Link status\r\n");
        tempCnt = 0;
        while(PHY_IP101_Link_Check()==0)
        {
          HAL_Delay(100);
          tempCnt++;
          if(tempCnt > 50)
          {
            printf("Check Link Error %d \r\nBack to the previous Stage\r\n", tempCnt);
            PHY_Scen_Seq--;
            return 0xE1;
          }
        }
        //check Auto-Nego Enable and complete
        temp_Stutus = PHY_read_register(PHY_IP101_StatusReg);
        printf("Status[0x%04x] = Auto-Nego[%x], Acuto-NegoCom[%x], Link[%x]\r\n", temp_Stutus, TempPHY_SR_Reg->Auto_Nego, TempPHY_SR_Reg->Auto_Nego_Complete, TempPHY_SR_Reg->Link_Status);
        //check phy speed OP mode
        temp_R_OP_Mode = PHY_IP101_Page_Read(0x0010, PHY_IP101_MDI_MDIX_CtrlSpecStaReg);
        printf("Link Ok 0x%04x , 0x%02x\r\n", temp_R_OP_Mode, TempPHY_MCS_Reg->OP_MODE_IND);
        if(TempPHY_MCS_Reg->OP_MODE_IND != 0)
        {
          printf("Link OP %s %s \r\n",SpeedDisp[(TempPHY_MCS_Reg->OP_MODE_IND&0x03)-1], FullHalf[(TempPHY_MCS_Reg->OP_MODE_IND>>2)&0x01]);
        }
        CLI_Answer = 0;
        PHY_Scen_Seq++;
      }
      else if(CLI_Answer == 2)
      {
        MDIX_Mode = 0;
        CLI_Answer = 0;
        PHY_Scen_mode = 0;
        PHY_Scen_Seq = 0;
        printf("Terminate Test Midway!!\r\n ");
      }
    break;
    case 4: //loopback test
    loopback_tcps_t(0, data_buf, 5000, AS_IPV4);
    if(CLI_Answer == 1)
    {
      printf("loopback test finish\r\n");
      close(0);
      if(cable_sel == 0)
      {
        printf("Please Set Cross Cable connects to each other next stage input 'yes' exit 'no'\r\n");
        cable_sel++;
        PHY_Scen_Seq--;
      }
      else
      {
        cable_sel = 0;
        if(PHY_OP_mode<3)
        {
          PHY_OP_mode++;
          PHY_Scen_Seq = 2;
        }
        else if(MDIX_Mode == 0)
        {
          //change OP Mode
          PHY_OP_mode = 0;
          PHY_Scen_Seq = 1;
          MDIX_Mode++;
        }
        else
        {
          //finish test
          PHY_OP_mode = 0;
          PHY_Scen_Seq = 0;
          MDIX_Mode=0;
          PHY_Scen_mode = 0;
          printf("PHY02 test Finish\r\n");
        }
      }
      CLI_Answer = 0;
    }
    else if(CLI_Answer == 2)
    {
      MDIX_Mode = 0;
      CLI_Answer = 0;
      PHY_Scen_mode = 0;
      PHY_Scen_Seq = 0;
      printf("Terminate Test Midway!!\r\n ");
      close(0);
      //
    }
    break;
    case 5:
    break;
  }
  return 0;
}
#if 0 //hold
uint8_t PHY_IP101_03_SCEN(void)
{
  uint16_t Reg_Default[9]={0x3100, 0x7849, 0x0243, 0x0c54, 0x01e1, 0x0000, 0x0004, 0x2001, 0x0000};
  uint8_t Page16_Reg[7] = {16, 17, 18, 26, 27, 29, 30};
  uint8_t Page1_Reg[4] = {17, 18, 22, 23};

  uint16_t tempCnt = 0, temp_Rdata = 0;
  printf(" Reg | Default | Value \r\n");
  //read Reg
  for(tempCnt = 0; tempCnt < 9; tempCnt++)
  {
    temp_Rdata = PHY_read_register(tempCnt);
    printf(" %3d |  0x%04x | 0x%04x \r\n", tempCnt, Reg_Default[tempCnt], temp_Rdata);
  }

}
#endif
uint8_t PHY_IP101_04_SCEN(void)
{
  //uint16_t ret = 0;
  PHY_IP101_CONTR_REG *TempPHY_Ctrl;
  uint16_t temp_ctr_reg = 0;
  TempPHY_Ctrl = (PHY_IP101_CONTR_REG *)&temp_ctr_reg;
  switch(PHY_Scen_Seq)
  {
    case 0:
      printf("Please Set Power Down Mode input 'yes' exit 'no'\r\n");
      PHY_address_set(PHY_ADDR);
      CLI_Answer = 0;
      PHY_Scen_Seq++;
    break;
    case 1:
      if(CLI_Answer == 1)
      {
        temp_ctr_reg = PHY_read_register(PHY_IP101_CtrlReg);
        TempPHY_Ctrl->Pow_DN = 1;
        PHY_write_register(PHY_IP101_CtrlReg, temp_ctr_reg);
        temp_ctr_reg = PHY_read_register(PHY_IP101_CtrlReg);
        printf("PHY Power Down[0x%04x]=%d\r\n", temp_ctr_reg, TempPHY_Ctrl->Pow_DN);
        printf("Please Set Power Down Mode Disable input 'yes' exit 'no'\r\n");
        CLI_Answer = 0;
        PHY_Scen_Seq++;
      }
      else
      {
        CLI_Answer = 0;
        PHY_Scen_mode = 0;
        PHY_Scen_Seq = 0;
        printf("Terminate Test Midway!!\r\n ");
      }
    break;
    case 2:
      if(CLI_Answer == 1)
      {
        temp_ctr_reg = PHY_read_register(PHY_IP101_CtrlReg);
        TempPHY_Ctrl->Pow_DN = 0;
        PHY_write_register(PHY_IP101_CtrlReg, temp_ctr_reg);
        temp_ctr_reg = PHY_read_register(PHY_IP101_CtrlReg);
        printf("Power Down Disable status\r\n ");
        printf("PHY Power Down[0x%04x]=%d\r\n", temp_ctr_reg, TempPHY_Ctrl->Pow_DN);
        printf("PHY04 test Finish\r\n");
        CLI_Answer = 0;
        PHY_Scen_mode = 0;
        PHY_Scen_Seq = 0;
      }
      else
      {
        CLI_Answer = 0;
        PHY_Scen_mode = 0;
        PHY_Scen_Seq = 0;
        printf("Power Down Enable status\r\n ");
        temp_ctr_reg = PHY_read_register(PHY_IP101_CtrlReg);
        printf("PHY Power Down[0x%04x]=%d\r\n", temp_ctr_reg, TempPHY_Ctrl->Pow_DN);
        printf("PHY02 test Finish\r\n");
      }
    break;
  }
  return 0;
}

void PHY_Test_Process(void)
{
  if(PHY_Scen_mode != 0)
  {
    switch(PHY_Scen_mode)
    {
      case 1: //PHY01
        PHY_IP101_01_SCEN();
      break;
      case 2:
        PHY_IP101_02_SCEN();
      break;
    }
  }
}
char phy_test_func(char data)
{
 // int count_out = 0;
//  uint8_t temp_data[10];
  uint16_t temp_ret= 0;
  int cnt = 0;

  //

  PHY_IP101_STATUS_REG *PHY_S1;
  uint16_t TempReg = 0x7849;
  PHY_S1 = (PHY_IP101_STATUS_REG *)&TempReg;

  printf("0: %u %u %u %u \r\n", PHY_S1->Extended_Cap, PHY_S1->Jabber_Detect, PHY_S1->Link_Status, PHY_S1->Auto_Nego);
  printf("1: %u %u %u %u %u %u\r\n", PHY_S1->T4_100, PHY_S1->Full_100, PHY_S1->Half_100, PHY_S1->Full_10, PHY_S1->Half_10,PHY_S1->MF_Pream_Supp);
  //
  switch(data)
  {
    case 1:
    PHY_address_set(PHY_ADDR);
    for(cnt = 0; cnt < 9; cnt++)
    {
      PHY_read_register(cnt);
    }
    break;
    case 2:
    //auto-negotiation set test
    PHY_address_set(PHY_ADDR);

    temp_ret = PHY_read_register(0x01);
    printf("Status Register Read 0x%04x \r\n", temp_ret);
    temp_ret = PHY_read_register(0x04);
    printf("Auto-Negotiation Advertisement Register Read 0x%04x \r\n", temp_ret);
    PHY_write_register(0x04, 0x0000);
    printf("Auto-Negotiation Advertisement Register Write\r\n");
    temp_ret = PHY_read_register(0x04);
    printf("Auto-Negotiation Advertisement Register Read 0x%04x \r\n", temp_ret);
    temp_ret = PHY_read_register(0x01);
    printf("Status Register Read 0x%04x \r\n", temp_ret);
    break;
    case 3:
    //page test
    PHY_address_set(PHY_ADDR);
    for(cnt = 16; cnt < 19; cnt++)
    {
      printf("Page Read 0x%02x \r\n", cnt);
      PHY_write_register(0x14, 0x0010);
      temp_ret = PHY_read_register(cnt);
      printf("Read reg(%d)= 0x%04x\r\n", cnt, temp_ret);
    }
    for(cnt = 26; cnt < 28; cnt++)
    {
      printf("Page Read %02x \r\n", cnt);
      PHY_write_register(0x14, 0x0010);
      temp_ret = PHY_read_register(cnt);
      printf("Read reg(%d)= 0x%04x\r\n", cnt, temp_ret);
    }
    for(cnt = 29; cnt < 31; cnt++)
    {
      printf("Page Read %02x \r\n", cnt);
      PHY_write_register(0x14, 0x0010);
      temp_ret = PHY_read_register(cnt);
      printf("Read reg(%d)= 0x%04x\r\n", cnt, temp_ret);
    }
    break;
    case 4:
    //phy_division_reg(0x02);
    //auto-negotiation set test
    PHY_address_set(PHY_ADDR);
    temp_ret = PHY_read_register(0x01);
    printf("Status Register Read 0x%04x \r\n", temp_ret);
    temp_ret = PHY_read_register(0x04);
    printf("Auto-Negotiation Advertisement Register Read 0x%04x \r\n", temp_ret);
    PHY_write_register(0x04, 0x01E1);
    temp_ret = PHY_read_register(0x04);
    printf("Auto-Negotiation Advertisement Register Read 0x%04x \r\n", temp_ret);
    temp_ret = PHY_read_register(0x01);
    printf("Status Register Read 0x%04x \r\n", temp_ret);
    break;
    case 5:
    //reset
    PHY_address_set(PHY_ADDR);
    PHY_read_register(0x01);
    PHY_write_register(0x00, 0xb100);
    PHY_read_register(0x00);
    printf("phy reset \r\n");
    HAL_Delay(1000);
    PHY_address_set(0x07);
    PHY_read_register(0x00);
    PHY_read_register(0x01);
    break;
    case 6: //MMD Test
    PHY_address_set(PHY_ADDR);
    PHY_write_register(PHY_IP101_MMD_A_CtrlReg, 0x0003);
    PHY_write_register(PHY_IP101_MMD_A_AddrDataReg, 0x0014);
    PHY_write_register(PHY_IP101_MMD_A_CtrlReg, 0x4003);
    temp_ret = PHY_read_register(PHY_IP101_MMD_A_AddrDataReg);
    printf("MMD Access Addr Register 3.20 Read 0x%04x \r\n", temp_ret);

    PHY_address_set(PHY_ADDR);
    PHY_write_register(PHY_IP101_MMD_A_CtrlReg, 0x0007);
    PHY_write_register(PHY_IP101_MMD_A_AddrDataReg, 0x003C);
    PHY_write_register(PHY_IP101_MMD_A_CtrlReg, 0x4007);
    temp_ret = PHY_read_register(PHY_IP101_MMD_A_AddrDataReg);
    printf("MMD Access Addr Register 7.60 Read 0x%04x \r\n", temp_ret);

    PHY_address_set(PHY_ADDR);
    PHY_write_register(PHY_IP101_MMD_A_CtrlReg, 0x0007);
    PHY_write_register(PHY_IP101_MMD_A_AddrDataReg, 0x003C);
    PHY_write_register(PHY_IP101_MMD_A_CtrlReg, 0x4007);
    PHY_write_register(PHY_IP101_MMD_A_AddrDataReg, 0x0000);
    printf("MMD Access Addr Register 7.60 Write 0x0000 \r\n");

    PHY_address_set(PHY_ADDR);
    PHY_write_register(PHY_IP101_MMD_A_CtrlReg, 0x0007);
    PHY_write_register(PHY_IP101_MMD_A_AddrDataReg, 0x003C);
    PHY_write_register(PHY_IP101_MMD_A_CtrlReg, 0x4007);
    temp_ret = PHY_read_register(PHY_IP101_MMD_A_AddrDataReg);
    printf("MMD Access Addr Register 7.60 Read 0x%04x \r\n", temp_ret);
    break;
  }
  return 0;
}

//uint8_t Echo_Server_Process(uint8_t mode, uint16_t port)
//{
//
//    switch(mode )
//    {
//        case 4:
//            loopback_tcps(3, data_buf, port + 3, AS_IPV4);
//        case 3:
//            loopback_tcps(2, data_buf, port + 2, AS_IPV4);
//        case 2:
//            loopback_tcps(1, data_buf, port + 1, AS_IPV4);
//        case 1:
//            loopback_tcps(0, data_buf, port, AS_IPV4);
//            break;
//        case 5:
//            loopback_tcps_t(0, data_buf, port, AS_IPV4);
//            break;
//        case 6:
//            loopback_tcps_t(0, data_buf, port, AS_IPV4);
//            loopback_tcps_t(1, data_buf, port + 1, AS_IPDUAL);
//            loopback_tcps_t(2, data_buf, port + 2, AS_IPDUAL);
//            //loopback_tcpc_t(2, data_buf, WIZ_Dest_IP, port + 2, AS_IPV4); //TCP Client
//            //loopback_udps_t(2, data_buf, port + 2, AS_IPV4);
//            loopback_udps_t(3, data_buf, port + 3, AS_IPV4);
//            break;
//    }
//    return 0;
//}
//uint8_t Echo_Client_Process(uint8_t mode, uint16_t port)
void reg_loop_test(void)
{
  uint8_t temp_sn[4] = {0, 0, 0, 0};
  uint8_t temp_ret[4] = {0, 0, 0, 0};
  int ret = 0, t_ret = 0;
  int i, j, k, h;
  printf("ip test \r\n");
  for(i = 0; i < 256; i++)
  {
    temp_sn[0] = i;
    for(j = 0; j < 256; j++)
    {
      temp_sn[1] = j;
      for(k = 0; k < 256; k++)
      {
        temp_sn[2] = k;
        for(h = 0; h < 256; h++)
        {
          temp_sn[3] = h;
          setSIPR(temp_sn);
          getSIPR(temp_ret);
          ret = memcmp(temp_sn, temp_ret, 4);
          if(ret != 0)
          {
            t_ret = 1;
            printf("ret[%3d] 0x%02x %02x %02x %02x -> 0x%02x %02x %02x %02x \r\n",ret , temp_sn[0], temp_sn[1], temp_sn[2], temp_sn[3], temp_ret[0], temp_ret[1], temp_ret[2], temp_ret[3]);
          }
        }
      }
    }
  }
  printf("test result = %s \r\n", (t_ret == 0? "pass" : "fail"));
}


//
//void chip_reset_test(void)
//{
//	int ret = 0, count = 0;
//	uint8_t syslock = SYS_NET_LOCK;
//	uint32_t pre_time = 0, now_time = 0;
//	int i= 0, j;
//	//HAL_GetTick();
//	switch(QSPI_MODE)
//	{
//	case 0x00:
//			printf("spi mode = single \r\n");
//			break;
//	case 0x01:
//			printf("spi mode = dual \r\n");
//			break;
//	case 0x02:
//			printf("spi mode = quad \r\n");
//			break;
//	}
//	for(i=0; i< 10; i++)
//	{
//		printf("count %d s \r\n", i+1);
//		HAL_GPIO_WritePin(RSTn_GPIO_Port, RSTn_Pin, GPIO_PIN_RESET);
//		HAL_Delay(500);
//		HAL_GPIO_WritePin(RSTn_GPIO_Port, RSTn_Pin, GPIO_PIN_SET);
//		pre_time = HAL_GetTick();
//		HAL_Delay(500);
//	//	HAL_GPIO_WritePin(SPI_EN_GPIO_Port, SPI_EN_Pin, GPIO_PIN_SET);
//	//	HAL_Delay(500);
//	//	HAL_GPIO_WritePin(SPI_EN_GPIO_Port, SPI_EN_Pin, GPIO_PIN_RESET);
//		//HAL_Delay(500);
//		//HAL_Delay(1000);
//		for(j=0; j<i; j++)
//		{
//			HAL_Delay(1000);
//		}
//
//		ctlwizchip(CW_SYS_UNLOCK, &syslock);
//		now_time = HAL_GetTick();
//		ctlnetwork(CN_SET_NETINFO, &gWIZNETINFO);//wizchip_setnetinfo((wiz_NetInfo*)arg);
//		printf("delay time %d ms \r\n", now_time - pre_time);
//		ret = print_network_information_1();
//    count = 0;
//		while((ret != 0)&&(count < 30))
//		{
//      count++;
//			printf("repeat count = %d\r\n", count);
//			ctlnetwork(CN_SET_NETINFO, &gWIZNETINFO);
//			ret = print_network_information_1();
//		}
//    if(ret == 0 )
//      printf("data match & pass \r\n" );
//    else
//      printf("repeat count over\r\n" );
//	}
//}

#include "PHY_IP101G.h"
#include "main.h"
#include "W6300_TestProcess.h"

void PHY_Write_data(unsigned short Addr, unsigned char *Data, unsigned short Len)
{
  //qspi_write_buf(W6300_mode<<6|1<<5, Addr, Data, Len);
  qspi_write_buf(1<<5, Addr, Data, Len); //single
}
void PHY_Read_data(unsigned short Addr, unsigned char *Data, unsigned short Len)
{
  //qspi_read_buf(W6300_mode<<6, Addr, Data, Len);
  qspi_read_buf(0x00, Addr, Data, Len);  //single
}


uint16_t PHY_read_register(uint8_t reg_data)
{
	int count_out = 0;
	//uint8_t temp_data[2];
  TransUInt temp_data;
  //printf("Input Reg 0x%02x \r\n", reg_data);
	temp_data.Data8[0] = reg_data;  //status register
	//qspi_write_buf(W6300_mode<<6|1<<5, 0x3008, temp_data.Data8, 1);
  PHY_Write_data(PHY_PHYRAR, temp_data.Data8, 1); //PHYRAR
	temp_data.Data8[0] = 0x02;
	//qspi_write_buf(W6300_mode<<6|1<<5, 0x3014, temp_data.Data8, 1);//PHYACR
  PHY_Write_data(PHY_PHYACR, temp_data.Data8, 1);//PHYACR
	temp_data.Data8[0] = 0x00;
	//qspi_read_buf(W6300_mode<<6, 0x3014, temp_data.Data8, 1);//PHYACR
  PHY_Read_data(PHY_PHYACR, temp_data.Data8, 1);//PHYACR
	//printf("PHYACR read %d\r\n", temp_data.Data8[0]);
	while(temp_data.Data8[0] != 0x00)
	{
	  //qspi_read_buf(W6300_mode<<6, 0x3014, temp_data.Data8, 1); //PHYACR
    PHY_Read_data(PHY_PHYACR, temp_data.Data8, 1); //PHYACR
	  //printf("PHYACR read %d\r\n", temp_data.Data8[0]);
	  count_out++;
	  //HAL_Delay(100);
	  if(count_out > 20)
	  {
      printf("retry error %d\r\n", count_out);
      return 0xffff;
      break;
	  }
	}
	//qspi_read_buf(W6300_mode<<6, 0x3010, temp_data.Data8, 2); //PHYDOR
  PHY_Read_data(PHY_PHYDOR, temp_data.Data8, 2); //PHYDOR
	//printf("PHYDOR read status reg 0x%04x[%02x %02x] \r\n", temp_data.Data16, temp_data.Data8[0], temp_data.Data8[1]);
	return temp_data.Data16;
}
uint16_t PHY_write_register(uint8_t reg_data, uint16_t w_data)
{
	int count_out = 0;
	//uint8_t temp_data[2];
  TransUInt temp_data;
  temp_data.Data16 = w_data;
  //printf("Input Reg 0x%02x data: 0x%04x[%02x %02x]\r\n", reg_data, temp_data.Data16 , temp_data.Data8[0], temp_data.Data8[1]);
	temp_data.Data8[0] = reg_data;  //status register
	//qspi_write_buf(W6300_mode<<6|1<<5, 0x3008, temp_data.Data8, 1); // PHYRAR
  PHY_Write_data(PHY_PHYRAR, temp_data.Data8, 1); // PHYRAR
  HAL_Delay(100);
  temp_data.Data16 = w_data;
  //qspi_write_buf(W6300_mode<<6|1<<5, 0x300C, temp_data.Data8, 2); //PHYDIR
  PHY_Write_data(PHY_PHYDIR, temp_data.Data8, 2); //PHYDIR
	temp_data.Data8[0] = 0x01;
	//qspi_write_buf(W6300_mode<<6|1<<5, 0x3014, temp_data.Data8, 1);//PHYACR
  PHY_Write_data(PHY_PHYACR, temp_data.Data8, 1);//PHYACR
	temp_data.Data8[0] = 0x00;
	//qspi_read_buf(W6300_mode<<6, 0x3014, temp_data.Data8, 1);//PHYACR
  PHY_Read_data(PHY_PHYACR, temp_data.Data8, 1);//PHYACR
	//printf("PHYACR read %d\r\n", temp_data.Data8[0]);
	while(temp_data.Data8[0] != 0x00)
	{
	  //qspi_read_buf(W6300_mode<<6, 0x3014, temp_data.Data8, 1); //PHYACR
    PHY_Read_data(PHY_PHYACR, temp_data.Data8, 1); //PHYACR
	  //printf("PHYACR read %d\r\n", temp_data.Data8[0]);
	  count_out++;
	  //HAL_Delay(100);
	  if(count_out > 20)
	  {
      printf("retry error %d\r\n", count_out);
      return 0xffff;
      break;
	  }
	}
	return 0;
}

void PHY_address_set(uint8_t phy_address)
{
  TransUInt temp_data;
  temp_data.Data8[0] = phy_address;  //phy address
  //qspi_write_buf(W6300_mode<<6|1<<5, 0x3004, temp_data.Data8, 1); // PHYAR //phy address set reg
  PHY_Write_data(PHT_PHYAR, temp_data.Data8, 1); // PHYAR //phy address set reg
  temp_data.Data8[0] = 0x00;
	//qspi_read_buf(W6300_mode<<6, 0x3004, temp_data.Data8, 1);//PHYACR
  PHY_Read_data(PHT_PHYAR, temp_data.Data8, 1);//PHYACR
	//printf("PHYAR PHY Address set & read 0x%02x \r\n", temp_data.Data8[0]);
}

void PHY_division_reg(uint8_t phy_div)
{
  TransUInt temp_data;
  temp_data.Data8[0] = phy_div;  //phy address
  PHY_address_set(PHY_ADDR);
  //qspi_write_buf(W6300_mode<<6|1<<5, 0x3018, temp_data.Data8, 1); // PHYAR //phy address set reg
  PHY_Write_data(PHY_PHYDIVR, temp_data.Data8, 1); // PHY_PHYDIVR //phy address set reg
  temp_data.Data8[0] = 0x00;
	//qspi_read_buf(W6300_mode<<6, 0x3018, temp_data.Data8, 1);//PHYACR
  PHY_Read_data(PHY_PHYDIVR, temp_data.Data8, 1);//PHY_PHYDIVR
	printf("PHYDIVR REG SET & READ 0x%02x \r\n", temp_data.Data8[0]);
}


uint8_t PHY_IP101_Link_Check(void)
{
  unsigned short temp = 0;
  PHY_IP101_STATUS_REG *PHY_S1;
  PHY_address_set(PHY_ADDR);
  temp = PHY_read_register(PHY_IP101_StatusReg);
  PHY_S1 = (PHY_IP101_STATUS_REG *)&temp;
  return PHY_S1->Link_Status;
  #if 0
  uint8_t ret = 0;
  ret = (uint8_t)((PHY_read_register(PHY_StatusReg) & 0x04) >> 2);
  return ret;
  #endif
}

uint16_t PHY_IP101_Page_Read(uint16_t Page, uint8_t reg_data)
{
    uint16_t ret;
    PHY_address_set(PHY_ADDR);
    PHY_write_register(PHY_IP101_PageCtrlReg, Page);
    ret = PHY_read_register(reg_data);
    return ret;
}
void PHY_IP101_Page_Write(uint16_t Page, uint8_t reg_data, uint16_t w_data)
{
    PHY_address_set(PHY_ADDR);
    PHY_write_register(PHY_IP101_PageCtrlReg, Page);
    PHY_write_register(reg_data, w_data);
}

uint16_t PHY_IP101_MMD_Read(uint16_t Devad, uint16_t Addr)
{
    uint16_t ret = 0;
    PHY_address_set(PHY_ADDR);
    PHY_write_register(PHY_IP101_MMD_A_CtrlReg, Devad);
    PHY_write_register(PHY_IP101_MMD_A_AddrDataReg, Addr);
    PHY_write_register(PHY_IP101_MMD_A_CtrlReg, (Devad|0x4000));
    ret = PHY_read_register(PHY_IP101_MMD_A_AddrDataReg);
}

uint16_t PHY_IP101_MMD_Write(uint16_t Devad, uint16_t Addr, uint16_t w_data)
{
    PHY_address_set(PHY_ADDR);
    PHY_write_register(PHY_IP101_MMD_A_CtrlReg, Devad);
    PHY_write_register(PHY_IP101_MMD_A_AddrDataReg, Addr);
    PHY_write_register(PHY_IP101_MMD_A_CtrlReg, (Devad|0x4000));
    PHY_write_register(PHY_IP101_MMD_A_AddrDataReg, w_data);
}


uint8_t PHY_IP101_MOD_01(void)
{
    //
}

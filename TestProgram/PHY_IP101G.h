#ifndef __PHY_IP101G_H
#define __PHY_IP101G_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define PHY_ADDR 0x07

#define PHY_IP101_PageCtrlReg 20
#define PHY_IP101_CtrlReg     0
#define PHY_IP101_StatusReg   1
#define PHY_IP101_AutoNegoAdReg   4
#define PHY_IP101_AutoNegoLPA_Reg 5
#define PHY_IP101_AutoNegoExpReg  6
#define PHY_IP101_AutoNegoNPT_Reg 7
#define PHY_IP101_MMD_A_CtrlReg   13
#define PHY_IP101_MMD_A_AddrDataReg   14
//page 16
#define PHY_IP101_SpecCtrlReg     16
#define PHY_IP101_IntCtrlStaReg   17
#define PHY_IP101_StatusMonitReg  18
#define PHY_IP101_MDI_MDIX_CtrlSpecStaReg 30


//W6300 Reg Addr
#define PHT_PHYAR   0x3004
#define PHY_PHYRAR  0x3008
#define PHY_PHYACR  0x3014
#define PHY_PHYDOR  0x3010
#define PHY_PHYDIR  0x300C
#define PHY_PHYDIVR 0x3018

// Reg 00  Control Register
typedef struct PHY_IP101_CONTR_REG_t
{
    unsigned short Reserv : 7;
    unsigned short Collis_Test : 1;
    unsigned short Duplex_Mod : 1;
    unsigned short Restart_Auto_Nego : 1;
    unsigned short Isolate : 1;
    unsigned short Pow_DN : 1;
    unsigned short Auto_Nego_En : 1;
    unsigned short Speed_Sel : 1;
    unsigned short Loopback : 1;
    unsigned short Reset : 1;
}PHY_IP101_CONTR_REG;

// Reg 01  Status Register 
typedef struct PHY_IP101_STATUS_REG_t
{
  unsigned short Extended_Cap : 1;
  unsigned short Jabber_Detect : 1;
  unsigned short Link_Status : 1;
  unsigned short Auto_Nego : 1;
  unsigned short Remote_Fault : 1;
  unsigned short Auto_Nego_Complete : 1;
  unsigned short MF_Pream_Supp : 1;
  unsigned short Reserved : 4;
  unsigned short Half_10 : 1;
  unsigned short Full_10 : 1;
  unsigned short Half_100 : 1;
  unsigned short Full_100 : 1;
  unsigned short T4_100 : 1;
}PHY_IP101_STATUS_REG;

// Reg 04 Auto-Negotiation Advertisement Register
typedef struct PHY_IP101_A_NEGO_AD_REG_t
{
    unsigned short Selector : 5;
    unsigned short T_10 : 1;
    unsigned short TF_10 : 1;
    unsigned short TX_100 : 1;
    unsigned short TX_F100 : 1;
    unsigned short T4_100 : 1;
    unsigned short Pause : 1;
    unsigned short Asym_Pause : 1;
    unsigned short Reserv1 : 1;
    unsigned short RF : 1;
    unsigned short Reserv2 : 1;
    unsigned short NP : 1; 
}PHY_IP101_A_NEGO_AD_REG;

// Reg 05 Auto-Negotiation Link Partner Ability Register (ANLPAR)
typedef struct PHY_IP101_A_NEGO_LPA_REG_t
{
    unsigned short Selector : 5;
    unsigned short T_10 : 1;
    unsigned short TF_10 : 1;
    unsigned short TX_100 : 1;
    unsigned short TX_F100 : 1;
    unsigned short T4_100 : 1;
    unsigned short Pause : 1;
    unsigned short Asym_Pause : 1;
    unsigned short Reserve : 1;
    unsigned short RemoteFault : 1;
    unsigned short Ack : 1;
    unsigned short NextPage : 1;
}PHY_IP101_A_NEGO_LPA_REG;

// Page 16 Reg 16 UTP PHY Specific Control Register 
typedef struct PHY_IP101_SPEC_CTRL_REG_t
{
    unsigned short ANAL_OFF : 1;
    unsigned short LDPS_EN : 1;
    unsigned short REPEATER_MOD : 1;
    unsigned short Reserv1 : 2;
    unsigned short BYPASS_DSP_RESET : 1;
    unsigned short Reserv2 : 1;
    unsigned short NWAY_PSAVE_DIS : 1;
    unsigned short FEF_DIS : 1;
    unsigned short JABBER_EN : 1;
    unsigned short Reserv3 : 1;
    unsigned short AUTO_MDIX_DIS : 1;
    unsigned short RMII_V12 : 1;
    unsigned short RMII_V10 : 1;
    unsigned short Reserv4 : 2;
}PHY_IP101_SPEC_CTRL_REG;

//Page 16 Reg 30 PHY MDI/MDIX Control and Specific Status Register 
typedef struct PHY_IP101_MDIX_CTRL_STA_REG_t
{
    unsigned short OP_MODE_IND : 3;
    unsigned short FORCE_MDIX : 1;
    unsigned short Reserv1 : 4;
    unsigned short LINK_UP : 1;
    unsigned short Reserv2 : 7;
}PHY_IP101_MDIX_CTRL_STA_REG;

void PHY_Write_data(unsigned short Addr, unsigned char *Data, unsigned short Len);
void PHY_Read_data(unsigned short Addr, unsigned char *Data, unsigned short Len);

uint16_t PHY_read_register(uint8_t reg_data);
uint16_t PHY_write_register(uint8_t reg_data, uint16_t w_data);
void PHY_address_set(uint8_t phy_address);
uint8_t PHY_IP101_Link_Check(void);
uint16_t PHY_IP101_Page_Read(uint16_t Page, uint8_t reg_data);
void PHY_IP101_Page_Write(uint16_t Page, uint8_t reg_data, uint16_t w_data);
uint16_t PHY_IP101_MMD_Read(uint16_t Devad, uint16_t Addr);
uint16_t PHY_IP101_MMD_Write(uint16_t Devad, uint16_t Addr, uint16_t w_data);


#ifdef __cplusplus
}
#endif

#endif /* __W6300_TESTPROCESS_H */
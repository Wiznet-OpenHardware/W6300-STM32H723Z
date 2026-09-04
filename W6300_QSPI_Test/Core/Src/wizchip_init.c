/* W6300 BUS(FMC 8-bit indirect) 호스트 인터페이스 — NUCLEO-H723ZG + WIZ630MJ
 *
 * W6300 인다이렉트 버스 레지스터 (FMC Bank3 = 0x68000000 = _WIZCHIP_IO_BASE_):
 *   +0 (A[1:0]=0) IDM_AR0 : 주소 상위 바이트
 *   +1 (A[1:0]=1) IDM_AR1 : 주소 하위 바이트
 *   +2 (A[1:0]=2) IDM_BSR : 블록 셀렉트
 *   +3 (A[1:0]=3) IDM_DR  : 데이터. 읽기/쓰기마다 칩 내부 주소가 auto-increment 되므로
 *                           주소 3바이트를 한 번 쓰고 DR 을 연속 접근하면 버스트가 된다.
 */
#include "wizchip_init.h"
#include <stdio.h>

#define W6300_BUS_BASE   0x68000000UL
#define W6300_BUS_DR     (W6300_BUS_BASE + 3)

/* ---- 바이트 단위 접근 (w6300.c 의 WIZCHIP_READ/WRITE: 주소 3바이트 + 데이터 1바이트) ---- */
void W6300BusWriteByte(uint32_t addr, iodata_t data)
{
	if (HAL_SRAM_Write_8b(&hsram1, (uint32_t *)addr, &data, 1) != HAL_OK)
		printf("BusWriteError\r\n");
}

iodata_t W6300BusReadByte(uint32_t addr)
{
	uint8_t result = 0;
	if (HAL_SRAM_Read_8b(&hsram1, (uint32_t *)addr, &result, 1) != HAL_OK)
		printf("BusReadError\r\n");
	return result;
}

/* ---- 버스트 접근 (w6300.c 의 WIZCHIP_WRITE_BUF/READ_BUF: 소켓 TX/RX 버퍼) ----
   주소 3바이트 기록 후 데이터 포트(0x68000003)를 volatile 로 직접 연속 접근 (auto-increment) */
void W6300BusWriteBuf(uint32_t AddrSel, iodata_t *buf, int16_t len)
{
	W6300BusWriteByte(W6300_BUS_BASE + 0, (AddrSel >> 16) & 0xff);
	W6300BusWriteByte(W6300_BUS_BASE + 1, (AddrSel >>  8) & 0xff);
	W6300BusWriteByte(W6300_BUS_BASE + 2, (AddrSel >>  0) & 0xff);
	while (len-- > 0) *(volatile uint8_t *)W6300_BUS_DR = (uint8_t)(*buf++);
}

void W6300BusReadBuf(uint32_t AddrSel, iodata_t *buf, int16_t len)
{
	W6300BusWriteByte(W6300_BUS_BASE + 0, (AddrSel >> 16) & 0xff);
	W6300BusWriteByte(W6300_BUS_BASE + 1, (AddrSel >>  8) & 0xff);
	W6300BusWriteByte(W6300_BUS_BASE + 2, (AddrSel >>  0) & 0xff);
	while (len-- > 0) *buf++ = *(volatile uint8_t *)W6300_BUS_DR;
}

/* ---- 하드웨어 리셋 (RSTn = PF4) ---- */
void chip_hw_reset(void)
{
	printf("W6300 Hardware Reset\r\n");
	HAL_GPIO_WritePin(RSTn_GPIO_Port, RSTn_Pin, GPIO_PIN_RESET);
	HAL_Delay(500);
	HAL_GPIO_WritePin(RSTn_GPIO_Port, RSTn_Pin, GPIO_PIN_SET);
	HAL_Delay(500);
}

/* ---- 초기화: BUS 콜백 등록 → CIDR 확인 → PHY 링크 대기 → 소켓 버퍼/인터럽트 마스크 ---- */
void W6300Initialze(void)
{
	uint8_t  link = PHY_LINK_OFF, imr = IK_DEST_UNREACH;
	uint16_t cidr;
	uint32_t to;
	/* 소켓별 TX/RX 버퍼(KB). 소켓0 = 16KB (MACRAW 는 소켓0 전용이라 크게), 나머지 2KB. 합계 ≤ 32KB */
	uint8_t bufsize[2][8] = {{16, 2, 2, 2, 2, 2, 2, 2}, {16, 2, 2, 2, 2, 2, 2, 2}};

	/* BUS 인다이렉트 모드 콜백 등록.
	   W6300_IF_MODE(w6300.c) 는 레지스터 블록 인코딩 등 런타임 분기 기준이라 반드시 BUS_MODE 여야 함. */
	W6300_IF_MODE   = BUS_MODE;
	WIZCHIP.if_mode = _WIZCHIP_IO_MODE_BUS_INDIR_;
	reg_wizchip_bus_cbfunc(W6300BusReadByte, W6300BusWriteByte);
	/* 버스트 콜백은 구조체에 직접 등록. reg_wizchip_busbuf_cbfunc() 의 파라미터 프로토타입(4인자)이
	   실제 호출 규약(WIZCHIP.IF.BUS._read_data_buf: 3인자)과 달라 형 불일치 경고가 나기 때문. */
	WIZCHIP.IF.BUS._read_data_buf  = W6300BusReadBuf;
	WIZCHIP.IF.BUS._write_data_buf = W6300BusWriteBuf;

	/* 1) 인터페이스 통신 확인: CIDR 은 0x6300 이어야 함. 아니면 배선/MODE 핀/FMC 설정 문제 */
	cidr = getCIDR();
	printf("CIDR=0x%04x VER=0x%04x -> %s\r\n", cidr, getVER(),
	       (cidr == 0x6300) ? "BUS OK" : "BUS FAIL (check wiring / MODE pin / FMC)");

	/* 2) PHY 링크 대기 (최대 5초) */
	for (to = 0; to < 500; to++)
	{
		if (ctlwizchip(CW_GET_PHYLINK, (void *)&link) == -1) { printf("Unknown PHY link status.\r\n"); break; }
		if (link != PHY_LINK_OFF) break;
		HAL_Delay(10);
	}
	printf("PHY link: %s\r\n", (link != PHY_LINK_OFF) ? "UP" : "DOWN (timeout, proceeding)");

	/* 3) 소켓 버퍼 할당 + 인터럽트 마스크 */
	if (ctlwizchip(CW_INIT_WIZCHIP, (void *)bufsize) == -1) printf("W6300 init FAIL\r\n");
	else                                                    printf("W6300 init OK\r\n");
	if (ctlwizchip(CW_SET_INTRMASK, (void *)&imr) == -1)    printf("W6300 set IMR FAIL\r\n");
}

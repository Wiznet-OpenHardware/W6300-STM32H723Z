#ifndef WIZCHIP_INIT_H
#define WIZCHIP_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "wizchip_conf.h"

extern SRAM_HandleTypeDef hsram1;          /* FMC Bank3 (0x68000000) — main.c 에서 정의 */

/* W6300 BUS(FMC, 8-bit indirect) 호스트 인터페이스 */
void      W6300Initialze(void);            /* BUS 콜백 등록 → CIDR 확인 → PHY 링크 대기 → 버퍼/인터럽트 설정 */
void      chip_hw_reset(void);             /* RSTn 핀으로 W6300 하드웨어 리셋 */

/* 라이브러리(w6300.c) 콜백: 바이트 접근 / 버스트(버퍼) 접근 */
void      W6300BusWriteByte(uint32_t addr, iodata_t data);
iodata_t  W6300BusReadByte(uint32_t addr);
void      W6300BusWriteBuf(uint32_t AddrSel, iodata_t *buf, int16_t len);
void      W6300BusReadBuf(uint32_t AddrSel, iodata_t *buf, int16_t len);

#ifdef __cplusplus
}
#endif
#endif

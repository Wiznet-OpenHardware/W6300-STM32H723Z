# W6300 / NUCLEO-H723ZG 핀맵 (현재 펌웨어 기준)

> 출처: `Core/Src/main.c`(MX_GPIO_Init), `Core/Src/stm32h7xx_hal_msp.c`(FMC/OCTOSPI MSP), `Core/Inc/main.h`(핀 정의)
> 기준 펌웨어: USART3 디버그 + W6300 BUS(FMC), 단독 NUCLEO-H723ZG 부팅 확인본.
> ⚠️ 현재 `MX_FMC_Init()` / `MX_OCTOSPI1_Init()`는 **호출 안 됨(비활성)**. 핀 정의는 표에 남기고 상태는 "비고"에 표기.

---

## 1. 디버그 시리얼 (UART)

| 핀 | 신호 | 주변장치 | AF | 모드 | Pull | Speed | 상태 / 비고 |
|----|------|----------|----|------|------|-------|------|
| PD8 | USART3_TX | USART3 | AF7 | AF Push-Pull | NoPull | Very High | **VCP(COM) → ST-LINK**, `printf` 출력 (`_write`) |
| PD9 | USART3_RX | USART3 | AF7 | AF Push-Pull | NoPull | Very High | VCP(COM) ← ST-LINK |
| — | (USART2) | USART2 | — | — | — | — | `MX_USART2_UART_Init()`로 init만 됨. **GPIO 미설정 → 실제 핀 없음/비활성** |

---

## 2. W6300 제어 핀 (GPIO)

| 핀 | 신호 | 방향 | 모드 | Pull | Speed | 초기값 / 비고 |
|----|------|------|------|------|-------|------|
| PF4 | RSTn | Output | Push-Pull | NoPull | Low | **초기 0 = W6300 리셋 유지**. `chip_hw_reset()`에서 해제 |
| PF3 | IRQ | Input | EXTI Falling | Pull-Up | — | W6300 인터럽트. **EXTI3 사용** (NVIC EXTI3_IRQn) |
| PC4 | SPI_EN | Output | Push-Pull | NoPull | Very High | 초기 0 |
| PF15 | Trace | Output | Push-Pull | NoPull | Very High | 초기 0 |
| PC0 | MOD0 | Input | — | Pull-Down | — | 모드핀 (bit0, LSB) |
| PD3 | MOD1 | Input | — | Pull-Down | — | 모드핀 (bit1) |
| PG2 | MOD2 | Input | — | Pull-Down | — | 모드핀 (bit2) |
| PG3 | MOD3 | Input | — | Pull-Down | — | 모드핀 (bit3, MSB) |
| PE2 | MOD4 | Output | Push-Pull | NoPull | Low | 초기 0 |
| PE4 | MOD5 | Output | Push-Pull | NoPull | Low | 초기 0 |

> MOD0~3 = 입력(풀다운)으로 하드웨어 모드 읽기, MOD4/5 = 출력. `mode` 값은 `main()`에서 MOD0~3을 비트로 조합.

---

## 3. FMC 버스 — W6300 BUS 모드 (8-bit, Bank3 = 0x68000000)

모두 `AF12_FMC`, AF Push-Pull, NoPull, Very High. **현재 `MX_FMC_Init()` 주석처리 → 버스 비활성.**
단, `MX_GPIO_Init()`에서 **PF0/PF1/PD4/PD5는 AF12로 강제 설정**되어 있음(나머지 데이터/NE 핀은 FMC init 시에만 설정됨).

| 핀 | 신호 | 분류 | 비고 |
|----|------|------|------|
| PD14 | FMC_D0 | 데이터 | |
| PD15 | FMC_D1 | 데이터 | |
| PD0 | FMC_D2 | 데이터 | |
| PD1 | FMC_D3 | 데이터 | |
| PE7 | FMC_D4 | 데이터 | |
| PE8 | FMC_D5 | 데이터 | |
| PE9 | FMC_D6 | 데이터 | |
| PE10 | FMC_D7 | 데이터 | 8비트 버스 (D0~D7) |
| PF0 | FMC_A0 | 주소 | `MX_GPIO_Init`에서도 설정됨 |
| PF1 | FMC_A1 | 주소 | `MX_GPIO_Init`에서도 설정됨 |
| PD4 | FMC_NOE | 제어 | 읽기 스트로브(RD). `MX_GPIO_Init`에서도 설정됨 |
| PD5 | FMC_NWE | 제어 | 쓰기 스트로브(WR). `MX_GPIO_Init`에서도 설정됨 |
| PD7 | FMC_NE1 | 칩셀렉트 | NE1 |
| PG6 | FMC_NE3 | 칩셀렉트 | **NE3 = Bank3(0x68000000)** ← W6300 접근 베이스 |

---

## 4. OCTOSPI1 — W6300 QSPI 모드 (현재 미사용)

`HAL_OSPI_MspInit()`에 정의되어 있으나 **`MX_OCTOSPI1_Init()` 호출 없음 → 미사용.**

| 핀 | 신호 | AF | 모드 | Pull | Speed |
|----|------|----|------|------|-------|
| PF8 | OCTOSPIM_P1_IO0 | AF10 | AF PP | Pull-Down | Very High |
| PF9 | OCTOSPIM_P1_IO1 | AF10 | AF PP | Pull-Down | Very High |
| PF7 | OCTOSPIM_P1_IO2 | AF10 | AF PP | Pull-Down | Very High |
| PF6 | OCTOSPIM_P1_IO3 | AF10 | AF PP | Pull-Down | Very High |
| PF10 | OCTOSPIM_P1_CLK | AF9 | AF PP | NoPull | Very High |

---

## 5. 시스템 (코드에 명시 안 됨 / HAL·하드웨어 자동)

| 핀 | 신호 | 비고 |
|----|------|------|
| PH0 | OSC_IN | **HSE Bypass 8MHz (ST-LINK MCO)**. `RCC_HSE_BYPASS` 설정 시 HAL이 자동 구성 |
| PA13 | SWDIO | ST-LINK 디버그 (기본) |
| PA14 | SWCLK | ST-LINK 디버그 (기본) |

---

## 6. 충돌 / 주의 점검 (디버깅용)

- **USART3(PD8/PD9)는 FMC·W6300 제어핀·OCTOSPI 어느 핀과도 안 겹침** → 펌웨어 레벨에선 시리얼 핀 충돌 없음. 따라서 HAT 꽂을 때 시리얼이 끊기면 펌웨어가 아니라 **HAT 보드 배선/전원 쪽**을 봐야 함.
- **PF8/PF9 = OCTOSPI**, **PD8/PD9 = USART3** → 포트가 달라 무관(헷갈리기 쉬움).
- **PF4(RSTn) 초기 LOW** → 부팅 직후 W6300은 리셋 상태. 정상.
- **EXTI3**은 PF3(IRQ)에 묶임. MOD1(PD3)·MOD3(PG3)도 핀번호 3이지만 일반 입력이라 EXTI 충돌은 없음(단, EXTI line3 소스는 PF3 하나뿐).
- FMC 데이터버스는 **8비트(D0~D7)** 뿐. D8~D15(PD8/PD9 포함 영역)은 안 씀.

---

### AF / 약어 범례
- **AF7** = USART3, **AF12** = FMC, **AF10/AF9** = OCTOSPIM_P1
- **NOE** = Output Enable(읽기), **NWE** = Write Enable(쓰기), **NEx** = Chip-select(뱅크)
- VCP = Virtual COM Port (ST-LINK USB 시리얼)

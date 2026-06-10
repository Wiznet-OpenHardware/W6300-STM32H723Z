# W6300 펌웨어 → NUCLEO-H723ZG 인수인계서

> W6300 커스텀 보드용 펌웨어(`W6300_QSPI_TEST`)를 ST 평가보드 **NUCLEO-H723ZG + WIZ630MJ(HAT)** 환경으로 이식하고,
> **BUS / QSPI 두 인터페이스의 대역폭(throughput)을 측정**하는 작업의 현재 상태.
> 최종 갱신: 2026-06-10

---

## 1. 한 줄 요약

NUCLEO-H723ZG(HSE 8MHz) + WIZ630MJ(W6300) HAT에서, **하나의 바이너리로 BUS와 QSPI를 런타임 전환**하며 TCP 단방향 대역폭을 측정한다. UART·클럭·네트워크·모드전환 다 동작하고, **현재는 속도 튜닝 단계.**

**현재 측정값(2026-06-10):**

| | TX (장비→PC) | RX (PC→장비) | 무결성 |
|---|---|---|---|
| QSPI | 67.5 Mbps | 31.6 Mbps | 패턴 OK |
| BUS | 18.9 Mbps | 10.8 Mbps | 패턴 OK |

---

## 2. 하드웨어

| 항목 | 커스텀 보드(원본) | NUCLEO-H723ZG(현재) |
|------|------|------|
| MCU | STM32H723ZGT6 | STM32H723ZGT6 (동일) |
| HSE | 외부 **25MHz** 오실레이터 | **ST-LINK MCO 8MHz** (HSE Bypass, PH0/OSC_IN) |
| 디버그 UART | USART2 → CP2104 | **USART3 (PD8/PD9) → ST-LINK VCP** (115200) |
| W6300 | 보드에 직접 실장 | **WIZ630MJ 모듈을 HAT으로 연결** |
| 디버거 | 외부 | 온보드 STLINK-V3E (V3J10M7) |

- NUCLEO 점퍼: **JP2=[1-2] STLINK**, JP4=ON, JP5=[1-2] 3V3. USB는 CN1.
- W6300 IP `192.168.11.99`, PC(서버) `192.168.11.42:5000`.

---

## 3. 현재 동작 (런타임 듀얼 모드)

**핵심: PC0(MOD0) 스트랩 핀을 입력으로 읽어, 외부 점퍼가 정한 모드에 맞춰 FW가 페리페럴을 구성한다.**

- **PC0 = LOW → QSPI**, **PC0 = HIGH → BUS** (외부 점퍼가 W6300의 MODE0를 설정).
- `main()` 흐름:
  1. PC0가 LOW로 **안정(디바운스 0.5초)** → `setup_interface_mode(QSPI)` → `[QSPI][TX]` 측정 → `[QSPI][RX]` 측정
  2. 시리얼에 `버스로 바꿔주세요` 출력 → 조작자가 점퍼를 BUS로
  3. PC0가 HIGH로 안정 → `setup_interface_mode(BUS)` → `[BUS][TX]` → `[BUS][RX]`
- `setup_interface_mode(mode)` ([main.c](W6300_QSPI_Test/Core/Src/main.c)):
  - 이전 모드 페리페럴을 **먼저 끔**(`HAL_SRAM_DeInit`/`HAL_OSPI_DeInit`) → 공유핀(QD) High-Z
  - `chip_hw_reset()` (외부 점퍼 MODE0를 칩이 래치)
  - 모드별 `MX_FMC_Init()` 또는 `MX_OCTOSPI1_Init()`
  - `W6300Initialze()` (모드별 콜백 등록 + PHY 링크 대기) → 네트워크 설정

---

## 4. 클럭 구성 (NUCLEO 8MHz 기준)

```
HSE 8MHz (HSE_VALUE = 8000000 으로 맞춰야 함! ← 핵심)
 → PLL1: M=1, N=60, P=1 → SYSCLK 480MHz, HCLK 240MHz, PCLK1 120MHz
 → PLL2: M=5, N=120, R=10 → PLL2R = 19.2MHz  = FMC 커널 클럭(BUS)
 → QSPI(OCTOSPI): 소스 = HCLK(240MHz), prescaler=6 → SCLK 40MHz
```

- **QSPI는 PLL2R이 아니라 HCLK에 물려 있음** (`OspiClockSelection = RCC_OSPICLKSOURCE_HCLK`).
  → PLL2R(=FMC)와 분리됨. **PLL2R을 BUS 튜닝용으로 올려도 QSPI엔 영향 없음.**
- ⚠️ PLL2가 25MHz HSE 기준이라, 8MHz NUCLEO에선 **PLL2R = 19.2MHz로 원래(60MHz)의 1/3.**
  → **BUS(FMC)가 느린 근본 원인.** (원래 보드였으면 BUS가 ~3배 빨랐음)

---

## 5. 대역폭 측정 (요약, 상세는 PROTOCOL.md)

- PC = **TCP 서버**(`wiz_looptester.py`), 장비 = **클라이언트**. 한 세션 = **4 연결**: `[QSPI][TX]→[QSPI][RX]→[BUS][TX]→[BUS][RX]`.
- 패턴: `byte[i] = '0' + (i%10)`. 무결성 = 수신 패턴 일치 검사(err==0).
- **TX**(장비→PC): 장비가 N초 송신 후 close → PC가 EOF까지 바이트/시간 측정. (`tcp_measure_tx`)
- **RX**(PC→장비): PC가 M초 송신 후 `shutdown(WR)` → 장비가 EOF에서 `[RESULT]bytes=,ms=,err=` 회신. (`tcp_measure_rx`)
- 합격 = `Mbps ≥ Min` AND `err==0`.

---

## 6. 주요 수정 파일

| 파일 | 수정 내용 |
|------|----------|
| `Core/Inc/stm32h7xx_hal_conf.h`, `Core/Src/system_stm32h7xx.c` | **`HSE_VALUE` 25000000 → 8000000** (UART 깨짐 해결의 핵심) |
| `Core/Src/main.c` | `SystemClock_Config` 8MHz용(`#if 0`); `setup_interface_mode()`(런타임 모드전환); `tcp_measure_tx/rx`(측정); PC0=입력(MOD0 스트랩); `dest_ip={192,168,11,42}`; IP=.99; QSPI 클럭 검증 printf; `PeriphCommonClock_Config`의 `OspiClockSelection=HCLK` |
| `io6Library/Ethernet/wizchip_conf.h` | `QSPI_MODE = QSPI_MODE_QUAD`; `_WIZCHIP_IO_MODE_ = BUS_INDIR`(고정, 그래야 IO_BASE=0x68000000) |
| `io6Library/Ethernet/W6300/w6300.c/.h` | `W6300_IF_MODE`로 **런타임 BUS/QSPI 분기**(블록 매크로·READ/WRITE) |
| `Core/Src/wizchip_init.c` | 모드별 콜백 등록(if_mode 먼저 설정); PHY 링크 ~5초 타임아웃; **BUS 버퍼 직접 볼라타일 접근**(HAL-per-byte 제거); 소켓0 버퍼 **16KB TX/RX**(`W6300_AdrSet`) |
| `io6Library/Ethernet/socket.c` | **`send()` `#if 1`→`#if 0`** = 속도판(503, SENDOK 대기 없는 파이프라인) 활성화 |
| `Core/Src/stm32h7xx_hal_msp.c` | `HAL_OSPI_MspInit`에 PG6 = OCTOSPIM_P1_NCS(AF10) 추가 |

---

## 7. 핵심 교훈 / 함정 (시간 날린 것들)

1. **`HSE_VALUE`는 실제 HSE와 일치해야 함.** 25MHz로 둔 채 8MHz NUCLEO에 올리면 → 클럭/UART 보드율 다 틀어져 글자 깨짐. **이게 첫 번째 큰 함정.**
2. **PLL2는 25MHz HSE 전용 계수** → 8MHz NUCLEO에선 PLL2R=19.2MHz(1/3). FMC(BUS)·(원래)QSPI 다 느려짐. **QSPI는 HCLK로 분리해서 40MHz 확보.**
3. **모드 전환 시 이전 페리페럴을 칩 리셋 *전에* deinit.** 안 그러면 공유핀(QD)을 FMC/OSPI가 잡고 있어 W6300 부팅 시 **PHY 링크 실패**.
4. **`reg_wizchip_*_cbfunc`는 `WIZCHIP.if_mode` 무한 가드** 있음 → **if_mode 먼저 세팅 + 해당 모드만 등록.** 안 하면 등록에서 무한루프(멈춤).
5. **IP 충돌 주의.** 처음 .44로 했다가 다른 기기(Apple)와 충돌 → ping/TCP 들쭉날쭉. .99로 변경.
6. **`send()` 두 구현**: 455번(SENDOK 대기=직렬화, 느림) / 503번(파이프라인, 빠름). **`#if`로 503 선택해야 TX 빠름.** 455면 큰 청크에서 지연ACK 꼬리에 묶여 ~3Mbps로 폭락.
7. **BUS 직접 볼라타일 접근**(`*(volatile uint8_t*)0x68000003`)이 `HAL_SRAM_*_8b` per-byte보다 ~2배 빠름.
8. **DMA로는 BUS/QSPI 속도 못 올림** — FMC 사이클/OCTOSPI 클럭이 한계. DMA는 CPU 해방용일 뿐.
9. **TCP 측정 갭**: 장비가 TX 끝나면 **즉시 close**(EOF), RX는 EOF에서 **즉시 [RESULT]**. 안 그러면 PC가 타임아웃(4s/2s)만큼 대기 → 측정 부정확.

---

## 8. 남은 최적화 레버 / TODO

| 목표 | 방법 | 기대 |
|------|------|------|
| **RX ↑** (QSPI 31, BUS 10.8) | `recv()`에도 send()처럼 느린 `#if` 안전판 있는지 확인 → 속도판으로 | 둘 다 상승 (가성비 1순위) |
| **QSPI TX 67 → 90** | 청크 8000→8192 / `tx_sink.py`로 PC툴 cap 확인 / QSPI 클럭 40→60 | PHY(100M) 근접 |
| **BUS ↑** (~2배) | **FMC 타이밍 축소**: ADDSET 4→1, DATAST 2→1 (W6300 최소 펄스 스펙까지, 패턴 err=0 유지선 찾기) | ~35-40Mbps |
| **BUS 더 ↑** | **PLL2R 상향**(19.2→48~64MHz). OSPI를 HCLK로 분리했으므로 QSPI 영향 없음. FMC 타이밍 ns 재계산 필요 | ~50-80Mbps |
| **테스트 시간 ↓** | `chip_hw_reset` 500+500ms→축소, `TX_MS`(현 1000), PHY 대기 | 모드당 ~1초 절약 |

> 현재 FMC 타이밍(`MX_FMC_Init`): ADDSET=4, DATAST=2 (= write ~8사이클/바이트 ≈ 416ns @19.2MHz).

---

## 9. 빌드 / 플래시 / 테스트

1. **STM32CubeIDE에서 클린 빌드 → 플래시.** (socket.c/wizchip_init.c도 바뀌면 클린 권장)
2. PC: `wiz_looptester.py` 실행 → Bind `192.168.11.42:5000` → START SERVER. (RX Duration 1초 권장)
3. 장비: PC0 점퍼 **QSPI(LOW)** → 측정 → 시리얼 안내 보고 점퍼 **BUS(HIGH)** → 측정.
4. 부팅 로그에서 `SYSCLK=480000000`, `SCLK=40 MHz`(QSPI), `CIDR=0x6100` 확인.

복구(부팅 직후 죽어 디버거 안 붙을 때): CubeProgrammer → ST-LINK Mode **Under reset** → RST 누른 채 Connect → Full chip erase.

---

## 10. 환경 정보

- IDE: STM32CubeIDE (사용자 빌드), 보조: VS Code
- 프로젝트: `W6300_QSPI_TEST` · ST-LINK FW: V3J10M7
- 터미널: **115200, 8-N-1, flow None**
- 라이브러리: WIZnet ioLibrary (W6300, socket, wizchip_conf, loopback)
- 칩 ID: `CIDR=0x6100 VER=0x4661` (W6100류 값이지만 W6300 레지스터맵 호환, 통신 정상)
- 관련 문서: [PinMap](W6300_NUCLEO_PinMap.md) · 측정 프로토콜 [PROTOCOL.md](wiz_looptester/PROTOCOL.md) · PC 툴 요구사항 [REQUIREMENTS.md](wiz_looptester/REQUIREMENTS.md)

# W6300-STM32H723Z

WIZnet **W6300**(WIZ630MJ) Ethernet 컨트롤러를 **STM32H723**(NUCLEO-H723ZG)에서 구동하고,
**BUS(FMC) / QSPI(OCTOSPI) 두 호스트 인터페이스의 TCP 대역폭을 측정**하는 펌웨어 + PC 테스터.

## 무엇을 하나
- **하나의 바이너리로 BUS·QSPI 런타임 전환** — PC0(MOD0) 스트랩 핀이 가리키는 모드로 페리페럴(FMC/OCTOSPI) 구성.
- 모드별 **단방향 대역폭(TX/RX) + 패턴 무결성** 측정. PC가 TCP 서버, 장비가 클라이언트.
- 생산품(WIZ630MJ) 불량 판정용 GUI 테스터(`wiz_looptester.py`) 포함.

## 현재 측정값 (NUCLEO 8MHz, 2026-06-10)
| | TX (장비→PC) | RX (PC→장비) |
|---|---|---|
| QSPI (SCLK 40MHz) | 67.5 Mbps | 31.6 Mbps |
| BUS (FMC 19.2MHz, 8-bit) | 18.9 Mbps | 10.8 Mbps |

> QSPI 클럭은 HCLK 분리로 40MHz 확보. BUS는 NUCLEO의 PLL2R(19.2MHz)이 원래 보드(60MHz)의 1/3이라 느림 — FMC 타이밍/클럭 튜닝 여지 있음.

## 문서
| 문서 | 내용 |
|------|------|
| [W6300_NUCLEO_인수인계서.md](W6300_NUCLEO_인수인계서.md) | **여기부터 읽기.** 이식 전체 상황·클럭·모드전환·교훈·남은 TODO |
| [W6300_NUCLEO_PinMap.md](W6300_NUCLEO_PinMap.md) | 핀맵 (UART / W6300 제어 / FMC / OCTOSPI / 공유핀) |
| [wiz_looptester/PROTOCOL.md](wiz_looptester/PROTOCOL.md) | PC↔장비 측정 통신 규약 (헤더 / 패턴 / TX·RX / RESULT) |
| [wiz_looptester/REQUIREMENTS.md](wiz_looptester/REQUIREMENTS.md) | PC 테스터(`wiz_looptester.py`) 요구사항·이력 |

## 빠른 실행
1. STM32CubeIDE 클린 빌드 → 플래시.
2. PC: `wiz_looptester.py` → Bind `192.168.11.42:5000` → START SERVER.
3. 장비: PC0 점퍼 **QSPI(LOW)** → 측정 → 안내 보고 **BUS(HIGH)** → 측정.

- 장비 IP `192.168.11.99`, PC 서버 `192.168.11.42:5000`, 디버그 UART 115200(USART3→ST-LINK VCP).

## 핵심 주의 (자세히는 인수인계서)
- **`HSE_VALUE`는 실제 HSE(8MHz)와 일치**해야 함 (안 그러면 클럭·UART 다 틀어짐).
- 모드 전환 시 **이전 페리페럴 deinit → 칩 리셋** 순서 (공유핀 High-Z, PHY 링크용).
- `send()`는 속도판(socket.c `#if 0`, 파이프라인)을 써야 TX 빠름.

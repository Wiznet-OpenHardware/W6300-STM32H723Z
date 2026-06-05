# W6300 펌웨어 → NUCLEO-H723ZG 이식 인수인계서

> 작성 목적: W6300 커스텀 보드용 펌웨어(`W6300_QSPI_TEST`)를 ST 평가보드 **NUCLEO-H723ZG**에 올려서 동작/디버그 메시지를 확인하는 작업의 진행 상황과 남은 과제 정리.

---

## 1. 한 줄 요약

원래 **W6300 커스텀 보드**(STM32H723 + W6300 칩 직접 실장)용 펌웨어를, 하드웨어가 다른 **NUCLEO-H723ZG**에 이식 중. 같은 MCU(STM32H723)지만 클럭 소스·UART 경로·주변 하드웨어가 모두 달라서, 그 차이를 코드에서 맞추는 작업.

**현재 상태:** MCU 부팅 OK, USART3 → ST-LINK VCP(COM) 출력까지 도달. 단, **출력 문자가 깨짐(보드율 불일치 추정)** — 이게 마지막 미해결 과제.

---

## 2. 하드웨어 두 보드 차이 (핵심)

| 항목 | 커스텀 보드 (원본) | NUCLEO-H723ZG (이식 대상) |
|------|------|------|
| MCU | STM32H723ZGT6 | STM32H723ZGT6 (동일) |
| 클럭(HSE) | 외부 25MHz 오실레이터 | **ST-LINK MCO 8MHz (HSE Bypass)** |
| 디버그 UART | USART2 → CP2104 USB-시리얼 칩 | **USART3 (PD8/PD9) → ST-LINK VCP** |
| W6300 칩 | FMC 버스(0x68000000)로 연결, 실장됨 | **없음** |
| 디버거 | 외부 | 온보드 STLINK-V3E |

> 결론: 커스텀 보드 코드를 그대로 구우면 (1) 클럭이 안 맞아 부팅 직후 멈추고, (2) UART 경로가 달라 COM에 안 뜨고, (3) W6300 초기화에서 없는 칩을 기다리다 멈춤.

---

## 3. NUCLEO 보드 설정 (UM2407 매뉴얼 기준, 확정 사실)

- **전원 점퍼 JP2 = [1-2] STLINK** (기본값). USB만 쓸 땐 무조건 이 위치.
  - ⚠️ JP2를 VIN/5V로 두고 USB만 꽂으면 enumeration 불안정 → 굽기 실패(FAIL.TXT) 발생. 이 문제로 한참 헤맸음. **JP2는 STLINK가 정답.**
- **JP4 (IDD measurement) = ON** (빠지면 MCU 전원 끊김)
- **JP5 (MCU Power) = [1-2] 3V3** (기본값)
- **USB는 CN1 (ST-LINK Micro USB)** 에 연결. CN13(User USB)은 OTG용이라 디버그와 무관.
- **HSE 클럭:** 기본 ST-LINK MCO 8MHz, PF0/PH0(OSC_IN)로 주입. 관련 SB: SB45 ON, SB44/SB46 OFF (기본값).
- **VCP(COM):** USART3(PD8=TX, PD9=RX)에 연결. 관련 SB12/SB19 ON (기본값).
- **COM LED(LD4) 기본색은 빨강.** 통신 시 녹색 깜빡. 빨강 자체는 정상.

---

## 4. ST-LINK 펌웨어 상태

- 현재 버전: **V3J10M7** (업그레이드 완료함. M3 → M7)
- "M7"의 M = Mass Storage 포함 → 그래서 윈도우에 **NOD_H723ZG 이동식 디스크가 뜨는 건 정상**. 무시해도 됨.
- VCP + Mass Storage + Debug가 한 USB에 같이 뜨는 composite device 구조라 COM과 디스크가 동시에 보이는 게 맞음.
- 업그레이드 도구: STM32CubeProgrammer → Firmware Upgrade → Open in update mode → Upgrade.

---

## 5. 지금까지 한 코드 수정 (main.c)

원본은 W6300 커스텀 보드용. 아래를 수정해서 NUCLEO에서 부팅 + COM 출력까지 도달시킴.

### 5-1. 클럭: 25MHz → 8MHz (HSE Bypass)
`SystemClock_Config()`에서 `#if 1`을 **`#if 0`** 으로 바꿔, 8MHz용 블록(`#else`)이 활성화되게 함.

```c
// 활성화된 8MHz 설정 (목표 SYSCLK 480MHz)
RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;   // MCO 주입이므로 BYPASS
RCC_OscInitStruct.PLL.PLLM = 1;                // 8/1 = 8MHz
RCC_OscInitStruct.PLL.PLLN = 60;               // 8*60 = 480MHz (VCO, WIDE 범위 192~836 안)
RCC_OscInitStruct.PLL.PLLP = 1;                // 480MHz SYSCLK
RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;  // 입력 8MHz → RANGE_3 (8~16MHz)
RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
```

> 참고: 원본은 25MHz 기준 550MHz였음. 8MHz로는 550이 정수로 안 떨어져 480MHz로 낮춤. 디버그 출력엔 무관.
> ⚠️ VCO WIDE 범위는 STM32H723 데이터시트상 192~836MHz. 480은 OK. (1100MHz로 잘못 잡으면 PLL 미잠금 → 부팅 직후 사망 → 이 실수도 한 번 했음.)

### 5-2. W6300/FMC 관련 비활성화 (NUCLEO엔 없는 하드웨어)
`main()`에서 아래 주석 처리:
```c
//  MPU_Config_FMC_Region();
//  MX_FMC_Init();
```
그리고 `while(1)` 이후의 W6300 초기화·루프백 코드 전체를 건너뛰고, 부팅 확인용 최소 루프만 남김:
```c
while (1)
{
    printf("alive %d\r\n", i++);
    HAL_Delay(500);
}
```

### 5-3. 디버그 출력: USART2 → USART3
- `huart3` 핸들 추가, `MX_USART3_UART_Init()` 함수 추가(115200, 8-N-1).
- `MX_GPIO_Init()`에 PD8/PD9를 USART3로 설정 + `__HAL_RCC_USART3_CLK_ENABLE()` 추가:
```c
__HAL_RCC_USART3_CLK_ENABLE();
GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
```
- `main()`에서 `MX_USART3_UART_Init();` 호출.
- printf 출력 경로 `_write()`를 `huart2` → **`huart3`** 으로 변경.

> ⚠️ 정리 필요(빌드는 되지만 지저분함):
> - 파일 상단에 `UART_HandleTypeDef huart2;`가 중복 선언됨 → 위쪽 추가분에서 한 줄 삭제 권장.
> - `main()`에서 `MX_USART2_UART_Init();`이 두 번 호출됨 → 한 번 삭제 권장.

---

## 6. 굽는 절차 (MCU가 멈췄을 때 복구 포함)

부팅 직후 죽는 펌웨어를 한 번 구우면 디버거가 안 붙어서 "FAILED to reset/halt the target MCU"가 뜸. 이때 복구:

1. STM32CubeProgrammer 실행
2. 우측 ST-LINK 설정 → **Mode: `Under reset`**
3. 보드 **RST 버튼 누른 채 → Connect → 버튼 떼기**
4. 붙으면 **Full chip erase**
5. (CubeIDE에서 구울 거면 Debug Config → Debugger → Connection을 Under reset으로 두면 편함)

---

## 7. 현재 증상 & 남은 과제 (제일 중요)

### 증상
- 빌드 OK, 굽기 OK, MCU 부팅 OK, USART3 → COM 출력까지 도달.
- 터미널(115200, 8-N-1)에 **깨진 문자(`���`)만 출력됨.**

### 해석
글자가 깨져 나온다 = **하드웨어/배선/포트는 정상, 보드율(baud rate)만 불일치.**
MCU가 실제로 내보내는 속도와 PC 터미널 속도가 다름. 즉 **실제 시스템 클럭이 의도한 480MHz가 아닐 가능성**이 큼 (USART3은 APB1 클럭 기준으로 보드율 생성).

### 다음에 할 일 (우선순위 순)
1. **터미널 보드율을 바꿔가며 정상 출력 지점 찾기:** 230400, 57600, 9600 등 시도. 정상 나오는 보드율 ÷ 115200 비율 = 실제 클럭 / 의도 클럭. 이 비율로 어디가 틀어졌는지 역산.
2. **실제 클럭 확인:** 정상 보드율 찾은 후 `printf("SYSCLK=%lu\r\n", HAL_RCC_GetSysClockFreq());` 로 실제 값 확인.
3. 클럭이 의도와 다르면 → PLL 계수(5-1) 재검토. 또는 가장 확실하게는 **CubeMX(.ioc)에서 NUCLEO 기준 클럭 설정을 자동 생성해 그 `SystemClock_Config` 값을 그대로 이식**하는 것 권장(손계산보다 안전).
4. USART3 동작 자체는 확인됨(문자가 뜨므로). `HAL_UART_Transmit(&huart3, ...)` 직접 호출로도 동일하게 깨지면 클럭 문제 확정.

---

## 8. 주의사항 / 함정 모음 (시간 날린 것들)

- **JP2를 STLINK 아닌 곳(VIN/5V)에 두지 말 것.** USB만 쓸 땐 STLINK. (VIN/5V는 외부전원 줄 때 + 전원 시퀀스 지켜야 함)
- **NOD_ 이동식 디스크는 정상.** ST-LINK 펌웨어 문제 아님. 무시.
- **PLL VCO는 192~836MHz(WIDE) 안에 들 것.** 범위 넘으면 부팅 직후 사망.
- **이 코드는 W6300 칩이 있어야 본 기능(네트워크 루프백) 동작.** NUCLEO엔 칩이 없으므로, NUCLEO에서는 "MCU 부팅 + UART 출력 확인"까지가 한계. 실제 W6300 기능 테스트는 커스텀 보드에서 해야 함.
- **printf는 `_write()`를 통해 나감** (이 프로젝트는 `__io_putchar`가 아니라 `_write` 사용). 출력 UART 바꾸려면 `_write` 안의 핸들을 수정.
- 필요시 `setvbuf(stdout, NULL, _IONBF, 0);`를 `HAL_Init()` 직후 추가해 printf 버퍼링 끄기.

---

## 9. 환경 정보

- IDE: STM32CubeIDE → (이관 대상) VS Code
- 프로젝트명: `W6300_QSPI_TEST`
- ST-LINK FW: V3J10M7
- 디버그 터미널 설정: **115200, 8bit, No parity, 1 stop, flow control None**
- 라이브러리: WIZnet ioLibrary (loopback, wizchip_conf, socket 등)

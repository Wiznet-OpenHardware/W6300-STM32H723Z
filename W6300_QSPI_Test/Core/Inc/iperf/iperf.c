#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "iperf.h"
#include "stm32h7xx.h"
#include "stm32h7xx_hal_tim.h"



TIM_HandleTypeDef htim2;


void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM2)
    {
        // TIM2 클럭 활성화
        __HAL_RCC_TIM2_CLK_ENABLE();
        
        // 만약 인터럽트 사용 시, NVIC 설정도 여기서 추가할 수 있습니다.
        // HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
        // HAL_NVIC_EnableIRQ(TIM2_IRQn);
    }
}

void MX_TIM2_Init(void)
{
       HAL_TIM_Base_DeInit(&htim2);
    /* TIM2 초기화 구조체 설정 */
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = ( 275000000 / 1000000) - 1; // 1MHz, 즉 1µs tick
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 0xFFFFFFFF;  // 최대값 (32비트)
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
    {
        // 초기화 실패 시 처리
       // Error_Handler();
    }

    /* 타이머 시작 (폴링 모드로 동작) */
    if (HAL_TIM_Base_Start(&htim2) != HAL_OK)
    {
        // 시작 실패 시 처리
      //  Error_Handler();
    }
    HAL_TIM_Base_MspInit(&htim2); 
   
}



// 현재 시간을 마이크로초 단위로 반환
uint32_t get_time_us( ) {
   return (uint32_t)__HAL_TIM_GET_COUNTER(&htim2);

}

// Stats 구조체 초기화 함수
void stats_init(Stats *stats, uint32_t pacing_timer_ms) {
    stats->pacing_timer_us = pacing_timer_ms * 1000;
    stats->running = false;
    stats->t0 = 0;
    stats->t1 = 0;
    stats->t3 = 0;
    stats->nb0 = 0;
    stats->nb1 = 0;
    stats->np0 = 0;
    stats->np1 = 0;
}

// Stats 시작 함수
void stats_start(Stats *stats) {
    stats->running = true;
    stats->t0 = stats->t1 = get_time_us();
    stats->nb0 = stats->nb1 = 0;
    stats->np0 = stats->np1 = 0;
    printf("Interval           Transfer     Bitrate\n");
}

// Stats 업데이트 함수
void stats_update(Stats *stats, bool final) {
    if (!stats->running) return;

    uint32_t t2 = get_time_us();
    uint32_t dt = t2 - stats->t1;  // 마지막 업데이트 이후 경과 시간

    if (final || dt > stats->pacing_timer_us) {
        double ta = (stats->t1 - stats->t0) / 1e6;  // 이전 시간 간격 시작
        double tb = (t2 - stats->t0) / 1e6;         // 현재 시간 간격 종료
        double transfer_mbits = (stats->nb1 * 8) / 1e6 / (dt / 1e6);  // Mbps 계산

        printf("%5.2f-%-5.2f sec %8u Bytes  %5.2f Mbits/sec\n",
               ta, tb, stats->nb1, transfer_mbits);

        stats->t1 = t2;  // 타이머 갱신
        stats->nb1 = 0;  // 주기별 데이터 초기화
        stats->np1 = 0;

    }
}

// Stats 중지 함수
void stats_stop(Stats *stats) {
    if (!stats->running) return;

    stats_update(stats, true);  // 마지막 업데이트
    stats->running = false;

    stats->t3 = get_time_us();  // 종료 시간 기록
    uint32_t total_time_us = stats->t3 - stats->t0;
    double total_time_s = total_time_us / 1e6;
    double transfer_mbits = (stats->nb0 * 8) / 1e6 / total_time_s;

    printf("------------------------------------------------------------\n");
    printf("Total: %5.2f sec %8lu Bytes  %5.2f Mbits/sec\n",
           total_time_s, stats->nb0, transfer_mbits);
}

// Stats 데이터 추가 함수
void stats_add_bytes(Stats *stats, uint32_t n) {
    if (!stats->running) return;

    stats->nb0 += n;  // 총 바이트 수 증가
    stats->nb1 += n;  // 주기별 바이트 수 증가
    stats->np0 += 1;  // 총 패킷 수 증가
    stats->np1 += 1;  // 주기별 패킷 수 증가

}

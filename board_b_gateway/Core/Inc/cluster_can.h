/**
 * @file    cluster_can.h
 * @brief   VW Passat B6 계기판 CAN 인터페이스
 * @author  ⑤ 미정
 * @board   보드B (게이트웨이와 같은 보드 - CAN2로 계기판 직결)
 *
 *   역할:
 *     - 계기판이 요구하는 CAN 메시지 포맷으로 RPM/Speed/Temp 송신
 *     - keep-alive (ABS/에어백 경고등 끄기)
 *     - 경고등 제어 (Gateway가 이상 감지 시)
 *
 *   TODO(⑤):
 *     [ ] VW Passat B6 CAN DB 정확한 비트 배치 검증 (브랜드별 편차 주의)
 *     [ ] keep-alive 초기화 시퀀스 구현
 *     [ ] 게이트웨이와 공유 데이터 (최신 RPM/Speed) 접근 설계
 */

#ifndef CLUSTER_CAN_H
#define CLUSTER_CAN_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void Cluster_Init(void);

/** 계기판 초기화 시퀀스 (바늘 풀스윙 + 경고등 소등 keep-alive) */
void Cluster_SendInitSequence(void);

/** 외부에서 최신 값을 주입하면 내부적으로 주기 송신 */
void Cluster_SetRPM(uint16_t rpm);
void Cluster_SetSpeed(uint16_t kmh);
void Cluster_SetCoolantTemp(int8_t celsius);
void Cluster_SetWarning(uint8_t on);

/** FreeRTOS Task - 주기적으로 최신 값을 CAN2에 송신 */
void Cluster_Task(void *argument);

#ifdef __cplusplus
}
#endif

#endif /* CLUSTER_CAN_H */

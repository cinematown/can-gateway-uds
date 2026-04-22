/**
 * @file    uds_server.h
 * @brief   UDS (ISO 14229) 진단 서버 - SID 0x22 ReadDataByIdentifier
 * @author  ④ 은빈
 * @board   보드C
 *
 *   스코프 (2주):
 *     - SID 0x22 ReadDataByIdentifier
 *     - Single Frame (ISO-TP 미사용, 데이터 ≤ 7 bytes)
 *     - NRC 0x13 (잘못된 길이), 0x31 (존재하지 않는 DID)
 *     - UART CLI 로 테스터 대체 (예: `read_did F40C`)
 *
 *   스코프 아웃:
 *     - ISO-TP 멀티프레임
 *     - SID 0x10 (세션), 0x27 (보안), 0x2E (Write)
 *
 *   TODO(④):
 *     [ ] DID 테이블 (VIN, RPM, Speed, Temp)
 *     [ ] SID 디스패처
 *     [ ] NRC 응답 생성 함수
 *     [ ] UART CLI 파서
 *     [ ] 현재 값 공급: CAN2에서 RPM/Speed 메시지 수신하여 내부 캐시
 */

#ifndef UDS_SERVER_H
#define UDS_SERVER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void UDS_Init(void);

/** CAN2 RX 에서 UDS 요청 수신 → 응답 송신 Task */
void UDS_Task(void *argument);

/** UART CLI Task (read_did <hex> 형식 명령) */
void UDS_CliTask(void *argument);

/** CAN2 에 흐르는 RPM/Speed/Temp 메시지를 내부 캐시에 반영 (Gateway 쪽에서 호출하거나, UDS 보드가 직접 수신) */
void UDS_UpdateCache_RPM(uint16_t rpm);
void UDS_UpdateCache_Speed(uint16_t kmh);
void UDS_UpdateCache_Temp(int8_t celsius);

#ifdef __cplusplus
}
#endif

#endif /* UDS_SERVER_H */

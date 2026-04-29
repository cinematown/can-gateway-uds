#ifndef PROTOCOL_IDS_H
#define PROTOCOL_IDS_H

/* --- CAN 메시지 ID 정의 (Board A -> Board B) --- */
#define CAN_ID_ENGINE_DATA          0x100  // RPM, Speed, Temp 등 실시간 데이터
#define CAN_ID_DASHBOARD_CTRL       0x200  // 계기판 제어 신호

/* --- UDS 진단 ID 정의 (Board B <-> Board C) --- */
#define CAN_ID_UDS_REQ_BOARD_B      0x714  // 진단기(Tester) -> Gateway 요청
#define CAN_ID_UDS_RESP_BOARD_B     0x77E  // Gateway -> 진단기 응답

/* --- UDS DID (Data Identifier) 정의 --- */
#define UDS_DID_VIN                 0xF190
#define UDS_DID_RPM                 0xF40C
#define UDS_DID_SPEED               0xF40D
#define UDS_DID_TEMP                0xF40E

#endif
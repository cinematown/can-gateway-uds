#include "uds_server.h"
#include "can_bsp.h"
#include "uart.h"
#include "cli.h"

/**
 * @brief UDS 서버 초기화
 */
void UDS_Server_Init(void) {
    // 1. CAN 필터 설정 (계기판 응답 ID인 0x7E8만 골라 받기 위해)
    // 이 함수는 can_bsp.c에 구현되어 있어야 합니다.
    
    // 2. 초기 환영 메시지 (디버그용)
    cliPrintf("UDS Server Initialized (Monitoring ID: 0x7E8)\r\n");
}

/**
 * @brief UDS 응답 메시지 처리 루틴
 */
void UDS_Server_Process(void) {
    CAN_RxMessage_t rxMsg;

    // CAN 메시지가 도착했는지 확인
    if (CAN_BSP_GetRxMessage(&rxMsg) == HAL_OK) {
        
        // 계기판의 응답 ID (0x7E8)인 경우
        if (rxMsg.id == CAN_ID_UDS_RESP_BOARD_B) {  //0x77E
            
            /* UDS Frame 구조 (표준 CAN 기준)
             * data[0]: PCI (Protocol Control Information) - 보통 데이터 길이를 나타냄
             * data[1]: Service ID (SID) - 0x62 (Positive Response for 0x22)
             * data[2~3]: DID (Data Identifier)
             * data[4~]: 실제 데이터 (Raw Value)
             */

            // 서비스 ID가 Read Data By Identifier의 긍정 응답(0x62)인지 확인
            if (rxMsg.data[1] == 0x62) {
                
                // 어떤 데이터에 대한 응답인지 DID 확인 (예: 0x0D00 - Speed)
                uint16_t receivedDID = (rxMsg.data[2] << 8) | rxMsg.data[3];

                if (receivedDID == 0x0D00) { // Speed 데이터인 경우
                    uint8_t rawValue = rxMsg.data[4]; 
                    
                    // 출력 1: 계기판이 보내준 생 데이터
                    cliPrintf("\r\n> (출력) \"Raw Response: 0x%02X\"\r\n", rawValue);
                    
                    // 출력 2: 물리 값 변환 (예: 1단위당 1km/h라고 가정)
                    float physicsValue = (float)rawValue; 
                    cliPrintf("> (출력) \"변환된 값: %.1f km/h\"\r\n", physicsValue);
                    cliPrintf("CLI> "); // 다시 프롬프트 출력
                }
            }
            // 부정 응답(Negative Response) 처리 (0x7F)
            else if (rxMsg.data[1] == 0x7F) {
                cliPrintf("\r\n> (출력) UDS Negative Response! Error Code: 0x%02X\r\n", rxMsg.data[3]);
                cliPrintf("CLI> ");
            }
        }
    }
}
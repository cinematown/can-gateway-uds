#include "uds_client.h"
#include "cli.h"
#include "main.h"
#include "uart.h"
#include "cmsis_os2.h"
#include <string.h>

#define ISOTP_TYPE_MASK     0xF0u
#define ISOTP_SF            0x00u
#define ISOTP_FF            0x10u
#define ISOTP_CF            0x20u
#define ISOTP_FC            0x30u
#define ISOTP_CF_SN_MASK    0x0Fu

typedef struct
{
    uint32_t req_id;
    uint32_t resp_id;
    uint16_t last_did;

    uint8_t  rx_buf[UDS_PAYLOAD_MAX_LEN];
    uint16_t rx_total_len;
    uint16_t rx_offset;
    uint8_t  next_sn;
    bool     receiving_multi;
    bool     response_complete;

    uint32_t tx_count;
    uint32_t rx_count;
    uint32_t err_count;
    uint32_t last_tx_ms;
    uint32_t last_rx_ms;
    bool     log_on;
} UDS_ClientContext_t;

static UDS_ClientContext_t g_client;

static void print_can_frame(const char *tag, const CAN_RxMessage_t *msg)
{
    cliPrintf("%s BUS=CAN%u ID=0x%03lX DLC=%u DATA=",
              tag,
              (unsigned int)msg->bus,
              (unsigned long)msg->id,
              (unsigned int)msg->dlc);
    for (uint8_t i = 0u; i < msg->dlc && i < 8u; i++)
    {
        cliPrintf("%02X ", msg->data[i]);
    }
    cliPrintf("\r\n");
}

static void print_payload(const char *tag, const uint8_t *data, uint16_t len)
{
    cliPrintf("%s LEN=%u DATA=", tag, (unsigned int)len);
    for (uint16_t i = 0u; i < len; i++)
    {
        cliPrintf("%02X ", data[i]);
    }
    cliPrintf("\r\n");
}

static void print_ascii_if_printable(const uint8_t *data, uint16_t len)
{
    bool printable = true;

    if (data == NULL || len == 0u)
    {
        return;
    }

    for (uint16_t i = 0u; i < len; i++)
    {
        if (data[i] < 0x20u || data[i] > 0x7Eu)
        {
            printable = false;
            break;
        }
    }

    if (printable)
    {
        cliPrintf("[CLUSTER ASCII] ");
        for (uint16_t i = 0u; i < len; i++)
        {
            cliPrintf("%c", data[i]);
        }
        cliPrintf("\r\n");
    }
}

static void decode_uds_payload(const uint8_t *payload, uint16_t len)
{
    uint16_t did;

    if (payload == NULL || len == 0u)
    {
        return;
    }

    if (payload[0] == UDS_NEG_RESP && len >= 3u)
    {
        cliPrintf("[CLUSTER NRC] SID=0x%02X NRC=0x%02X\r\n", payload[1], payload[2]);
        return;
    }

    if (payload[0] == UDS_POS_READ_DID && len >= 3u)
    {
        did = ((uint16_t)payload[1] << 8) | payload[2];
        cliPrintf("[CLUSTER DID] DID=0x%04X VALUE_LEN=%u\r\n",
                  did,
                  (unsigned int)(len - 3u));
        if (len > 3u)
        {
            print_payload("[CLUSTER VALUE]", &payload[3], (uint16_t)(len - 3u));
            print_ascii_if_printable(&payload[3], (uint16_t)(len - 3u));
        }
        return;
    }

    if (payload[0] == UDS_POS_DIAGNOSTIC_SESSION_CONTROL && len >= 2u)
    {
        cliPrintf("[CLUSTER SESSION OK] type=0x%02X\r\n", payload[1]);
        return;
    }

    if (payload[0] == UDS_POS_TESTER_PRESENT && len >= 2u)
    {
        cliPrintf("[CLUSTER TESTER PRESENT OK] sub=0x%02X\r\n", payload[1]);
        return;
    }
}

static void reset_isotp_rx_state(void)
{
    memset(g_client.rx_buf, 0, sizeof(g_client.rx_buf));
    g_client.rx_total_len = 0u;
    g_client.rx_offset = 0u;
    g_client.next_sn = 1u;
    g_client.receiving_multi = false;
}

static void reset_rx_state(void)
{
    reset_isotp_rx_state();
    g_client.response_complete = false;
}

static bool send_flow_control_cts(void)
{
    uint8_t tx[8] = {0};
    HAL_StatusTypeDef ret;

    tx[0] = 0x30u;
    tx[1] = 0x00u;
    tx[2] = 0x00u;

    ret = CAN_BSP_Send(g_client.req_id, tx, 8u);
    if (ret == HAL_OK)
    {
        cliPrintf("[CLUSTER FC TX] ID=0x%03lX DATA=30 00 00 00 00 00 00 00\r\n",
                  (unsigned long)g_client.req_id);
        return true;
    }

    g_client.err_count++;
    cliPrintf("[CLUSTER FC TX FAIL] id=0x%03lX ret=%d\r\n",
              (unsigned long)g_client.req_id,
              (int)ret);
    return false;
}

void UDS_Client_Init(void)
{
    memset(&g_client, 0, sizeof(g_client));
    g_client.req_id = CAN_ID_CLUSTER_UDS_REQ;
    g_client.resp_id = CAN_ID_CLUSTER_UDS_RESP;
    g_client.last_did = UDS_DID_VIN;
    g_client.log_on = false;
    reset_rx_state();
}

void UDS_Client_SetTarget(uint32_t req_id, uint32_t resp_id)
{
    g_client.req_id = req_id & 0x7FFu;
    g_client.resp_id = resp_id & 0x7FFu;
    reset_rx_state();
}

void UDS_Client_GetStatus(UDS_ClientStatus_t *out)
{
    if (out != NULL)
    {
        out->req_id = g_client.req_id;
        out->resp_id = g_client.resp_id;
        out->last_did = g_client.last_did;
        out->tx_count = g_client.tx_count;
        out->rx_count = g_client.rx_count;
        out->err_count = g_client.err_count;
        out->log_on = g_client.log_on;
    }
}

void UDS_Client_SetCanLog(bool on)
{
    g_client.log_on = on;
}

bool UDS_Client_GetCanLog(void)
{
    return g_client.log_on;
}

bool UDS_Client_ReadDID(uint16_t did)
{
    uint8_t tx[8] = {0};
    HAL_StatusTypeDef ret;

    reset_rx_state();

    tx[0] = 0x03u;
    tx[1] = UDS_SID_READ_DID;
    tx[2] = (uint8_t)((did >> 8) & 0xFFu);
    tx[3] = (uint8_t)(did & 0xFFu);

    ret = CAN_BSP_Send(g_client.req_id, tx, 8u);
    if (ret == HAL_OK)
    {
        g_client.last_did = did;
        g_client.tx_count++;
        g_client.last_tx_ms = HAL_GetTick();

        cliPrintf("[CLUSTER UDS TX] ID=0x%03lX DATA=03 22 %02X %02X 00 00 00 00\r\n",
                  (unsigned long)g_client.req_id,
                  tx[2],
                  tx[3]);
        cliPrintf("[WAIT RESP] ID=0x%03lX\r\n", (unsigned long)g_client.resp_id);
        return true;
    }

    g_client.err_count++;
    cliPrintf("[CLUSTER UDS TX FAIL] id=0x%03lX ret=%d\r\n",
              (unsigned long)g_client.req_id,
              (int)ret);
    return false;
}


/* 팀원 코드 스타일: CAN1로 ReadDID 요청만 단순 송신 */
void UDS_Request_ReadData(uint32_t canId, uint16_t did)
{
    uint8_t txData[8] = {0};
    HAL_StatusTypeDef status;

    txData[0] = 0x03u;
    txData[1] = UDS_SID_READ_DID;
    txData[2] = (uint8_t)((did >> 8) & 0xFFu);
    txData[3] = (uint8_t)(did & 0xFFu);

    status = CAN_BSP_Send(canId, txData, 8u);
    if (status == HAL_OK)
    {
        if ((canId & 0x7FFu) == g_client.req_id)
        {
            g_client.last_did = did;
            g_client.tx_count++;
            g_client.last_tx_ms = HAL_GetTick();
        }
    }
    else
    {
        g_client.err_count++;
        cliPrintf("[CAN TX Error] Code: %d\r\n", (int)status);
    }
}

static void drain_uart_rx(void)
{
    while (uartAvailable(0) > 0u)
    {
        (void)uartRead(0);
    }
}

static void print_team_style_rx(const CAN_RxMessage_t *rxMsg)
{
    cliPrintf("[RX] ID=0x%03lX DLC=%u Data=",
              (unsigned long)rxMsg->id,
              (unsigned int)rxMsg->dlc);

    for (uint8_t i = 0u; i < rxMsg->dlc && i < 8u; i++)
    {
        cliPrintf("%02X ", rxMsg->data[i]);
    }
    cliPrintf("\r\n");
}

static void print_team_style_hit(const CAN_RxMessage_t *rxMsg)
{
    cliPrintf(">>> [HIT] 0x%03lX 수신 성공!\r\n", (unsigned long)g_client.resp_id);
    cliPrintf(">>> Data: ");
    for (uint8_t i = 0u; i < 8u; i++)
    {
        cliPrintf("%02X ", (i < rxMsg->dlc) ? rxMsg->data[i] : 0u);
    }
    cliPrintf("\r\n");
}

/*
 * 팀원 성공 코드와 비슷한 방식의 진단 루프입니다.
 * 차이점: hcan1 직접 접근 대신 기존 CAN_BSP_Send/CAN_BSP_Read을 사용하므로 CAN1로 동작합니다.
 */
void UDS_Client_ExecuteDiagnostic(uint16_t did, const char *label)
{
    CAN_RxMessage_t rxMsg;
    bool saved_log = g_client.log_on;
    bool is_passive;
    uint32_t target_id;

    if (label == NULL) label = "DID";

    switch (did)
    {
        case UDS_DID_RPM:
            is_passive = true;
            target_id  = UDS_DID_RPM;
            break;
        case UDS_DID_SPEED:
            is_passive = true;
            target_id  = UDS_DID_SPEED;
            break;
        case UDS_DID_COOLANT:
            is_passive = true;
            target_id  = UDS_DID_COOLANT;
            break;
        case UDS_DID_ALL:
            is_passive = true;
            target_id  = UDS_DID_ALL;  // 0x0000 = 모든 ID
            break;
        default:
            is_passive = false;
            target_id  = g_client.resp_id;  // 0x77E (UDS 방식)
            break;
    }

    osDelay(10u);
    drain_uart_rx();
    g_client.log_on = false;

    cliPrintf("\r\n[START] %s 진단 시작 - 아무 키나 누르면 종료\r\n", label);

    for (;;)
    {
        uint32_t start_ms;

        reset_rx_state();

        if (is_passive)
        {
            if (did == UDS_DID_ALL)
                cliPrintf("[LISTEN] 모든 CAN 프레임 수신 중...\r\n");
            else
                cliPrintf("[LISTEN] ID=0x%03lX 수신 대기 중...\r\n", (unsigned long)target_id);
        }
        else
        {
            UDS_Request_ReadData(g_client.req_id, did);
            cliPrintf("[TX] Request %s (DID: 0x%04X) via ID: 0x%03lX\r\n",
                      label, did, (unsigned long)g_client.req_id);
        }

        start_ms = HAL_GetTick();
        while ((HAL_GetTick() - start_ms) < 3000u)
        {
            while (CAN_BSP_Read(&rxMsg, 0u))
            {
                // ALL 모드: 모든 프레임 출력 후 계속
                if (did == UDS_DID_ALL)
                {
                    cliPrintf("[RX] ID=0x%03lX DLC=%u Data=",
                              (unsigned long)rxMsg.id, rxMsg.dlc);
                    for (uint8_t i = 0u; i < rxMsg.dlc; i++)
                        cliPrintf("%02X ", rxMsg.data[i]);
                    cliPrintf("\r\n");
                }
                // 특정 ID 모드: target_id 일치할 때만 처리
                else if (rxMsg.id == target_id)
                {
                    cliPrintf("[RX] ID=0x%03lX DLC=%u Data=",
                              (unsigned long)rxMsg.id, rxMsg.dlc);
                    for (uint8_t i = 0u; i < rxMsg.dlc; i++)
                        cliPrintf("%02X ", rxMsg.data[i]);
                    cliPrintf("\r\n");

                    if (is_passive)
                    {
                        switch (did)
                        {
                            case UDS_DID_RPM: {
                                uint16_t rpm = (uint16_t)rxMsg.data[0] |
                                               ((uint16_t)rxMsg.data[1] << 8);
                                cliPrintf("[RESULT] RPM = %u\r\n", rpm);
                                break;
                            }
                            case UDS_DID_SPEED: {
                                uint16_t speed = (uint16_t)rxMsg.data[0] |
                                                 ((uint16_t)rxMsg.data[1] << 8);
                                cliPrintf("[RESULT] SPEED = %u km/h\r\n", speed);
                                break;
                            }
                            case UDS_DID_COOLANT: {
                                cliPrintf("[RESULT] TEMP = %u C\r\n", rxMsg.data[0]);
                                break;
                            }
                            default:
                                break;
                        }
                        drain_uart_rx();
                        g_client.log_on = saved_log;
                        cliPrintf("[DONE] %s 수신 완료\r\n", label);
                        return;
                    }
                    else
                    {
                        UDS_Client_OnCanRx(&rxMsg);
                        if (g_client.response_complete)
                        {
                            drain_uart_rx();
                            g_client.log_on = saved_log;
                            cliPrintf("[DONE] %s 진단 응답 수신 완료\r\n", label);
                            return;
                        }
                    }
                }
            }

            // 아무 키나 누르면 종료 (ALL 모드 포함)
            if (uartAvailable(0) > 0u)
            {
                (void)uartRead(0);
                drain_uart_rx();
                g_client.log_on = saved_log;
                cliPrintf("\r\n[STOP] 진단 종료\r\n");
                return;
            }

            osDelay(1u);
        }

        // ALL 모드는 3초 타임아웃 없이 계속 (키 입력으로만 종료)
        if (did != UDS_DID_ALL)
            cliPrintf("[WAIT] 3초 경과, 재시도...\r\n\r\n");
    }
}

void UDS_Execute_Diagnostic(uint16_t did, const char *label)
{
    UDS_Client_ExecuteDiagnostic(did, label);
}
void UDS_Client_OnCanRx(const CAN_RxMessage_t *msg)
{
    uint8_t pci_type;

    if (msg == NULL || msg->bus != CAN_BSP_BUS_CAN1)
    {
        return;
    }

    if (g_client.log_on)
    {
        print_can_frame("[CAN1 RX]", msg);
    }

    if (msg->id != g_client.resp_id || msg->dlc == 0u)
    {
        return;
    }

    g_client.rx_count++;
    g_client.last_rx_ms = HAL_GetTick();
    pci_type = msg->data[0] & ISOTP_TYPE_MASK;

    if (pci_type == ISOTP_SF)
    {
        uint8_t len = msg->data[0] & 0x0Fu;
        if (len == 0u || len > 7u || len > (msg->dlc - 1u))
        {
            g_client.err_count++;
            cliPrintf("[CLUSTER RESP ERR] invalid single frame len=%u\r\n", len);
            return;
        }

        print_payload("[CLUSTER UDS RESP]", &msg->data[1], len);
        decode_uds_payload(&msg->data[1], len);
        g_client.response_complete = true;
        reset_isotp_rx_state();
        return;
    }

    if (pci_type == ISOTP_FF)
    {
        uint16_t total_len;
        uint8_t copy_len;

        if (msg->dlc < 8u)
        {
            g_client.err_count++;
            cliPrintf("[CLUSTER RESP ERR] invalid first frame dlc=%u\r\n", msg->dlc);
            return;
        }

        total_len = (uint16_t)(((uint16_t)(msg->data[0] & 0x0Fu) << 8) | msg->data[1]);
        if (total_len == 0u || total_len > UDS_PAYLOAD_MAX_LEN)
        {
            g_client.err_count++;
            cliPrintf("[CLUSTER RESP ERR] payload too long=%u max=%u\r\n",
                      total_len,
                      (unsigned int)UDS_PAYLOAD_MAX_LEN);
            reset_rx_state();
            return;
        }

        reset_rx_state();
        g_client.receiving_multi = true;
        g_client.rx_total_len = total_len;
        g_client.next_sn = 1u;

        copy_len = (total_len >= 6u) ? 6u : (uint8_t)total_len;
        memcpy(&g_client.rx_buf[0], &msg->data[2], copy_len);
        g_client.rx_offset = copy_len;

        cliPrintf("[CLUSTER FF RX] total_len=%u copied=%u -> auto FlowControl CTS\r\n",
                  (unsigned int)total_len,
                  (unsigned int)copy_len);

        (void)send_flow_control_cts();
        return;
    }

    if (pci_type == ISOTP_CF)
    {
        uint8_t sn;
        uint16_t remain;
        uint8_t copy_len;

        if (!g_client.receiving_multi)
        {
            g_client.err_count++;
            cliPrintf("[CLUSTER RESP ERR] unexpected CF\r\n");
            return;
        }

        sn = msg->data[0] & ISOTP_CF_SN_MASK;
        if (sn != g_client.next_sn)
        {
            g_client.err_count++;
            cliPrintf("[CLUSTER RESP ERR] SN mismatch rx=%u exp=%u\r\n", sn, g_client.next_sn);
            reset_rx_state();
            return;
        }

        remain = g_client.rx_total_len - g_client.rx_offset;
        copy_len = (remain > 7u) ? 7u : (uint8_t)remain;
        if (copy_len > (msg->dlc - 1u))
        {
            g_client.err_count++;
            cliPrintf("[CLUSTER RESP ERR] CF dlc too short\r\n");
            reset_rx_state();
            return;
        }

        memcpy(&g_client.rx_buf[g_client.rx_offset], &msg->data[1], copy_len);
        g_client.rx_offset += copy_len;
        g_client.next_sn = (uint8_t)((g_client.next_sn + 1u) & 0x0Fu);

        if (g_client.rx_offset >= g_client.rx_total_len)
        {
            cliPrintf("[CLUSTER MF COMPLETE] total_len=%u\r\n", (unsigned int)g_client.rx_total_len);
            print_payload("[CLUSTER UDS RESP]", g_client.rx_buf, g_client.rx_total_len);
            decode_uds_payload(g_client.rx_buf, g_client.rx_total_len);
            g_client.response_complete = true;
            reset_isotp_rx_state();
        }
        return;
    }

    if (pci_type == ISOTP_FC)
    {
        cliPrintf("[CLUSTER FC RX] DATA=%02X %02X %02X\r\n",
                  msg->data[0], msg->data[1], msg->data[2]);
        return;
    }

    g_client.err_count++;
    cliPrintf("[CLUSTER RESP ERR] unknown PCI=0x%02X\r\n", msg->data[0]);
}

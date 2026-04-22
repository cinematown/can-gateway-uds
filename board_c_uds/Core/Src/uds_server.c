/**
 * @file    uds_server.c
 * @brief   UDS 서버 구현 (STUB)
 * @author  ④ 은빈
 */

#include "uds_server.h"
#include "can_bsp.h"
#include "can_db.h"
#include <string.h>
#include <stdio.h>

/* TODO: extern CAN_HandleTypeDef hcan2; */

/* ========================================================================== */
/*  내부 캐시 (현재 값)                                                        */
/* ========================================================================== */

static uint16_t s_cache_rpm   = 0U;
static uint16_t s_cache_speed = 0U;
static int8_t   s_cache_temp  = 25;

/* VIN 예시 (17자, ISO-TP 없이는 7바이트까지만 한 프레임 전송 가능 → 일부만 구현) */
static const char s_vin[17] = "KNAGT41FBC5000001";

/* ========================================================================== */
/*  DID 테이블                                                                */
/* ========================================================================== */

typedef struct {
    uint16_t did;
    uint8_t  length;
    /* 실제 구현에서는 getter 함수 포인터로 바꾸는 것이 깔끔 */
} UDS_DID_Entry_t;

static const UDS_DID_Entry_t s_did_table[] = {
    { DID_VIN,   17 },   /* 주의: Single Frame 한계로 잘라서 전송 or NRC */
    { DID_RPM,   2  },
    { DID_SPEED, 2  },
    { DID_TEMP,  1  },
};
#define DID_TABLE_SIZE (sizeof(s_did_table) / sizeof(s_did_table[0]))

static int find_did(uint16_t did, uint8_t *out_len)
{
    for (size_t i = 0; i < DID_TABLE_SIZE; i++) {
        if (s_did_table[i].did == did) {
            *out_len = s_did_table[i].length;
            return 0;
        }
    }
    return -1;
}

/* ========================================================================== */
/*  응답 생성                                                                 */
/* ========================================================================== */

static void send_positive_response(uint16_t did, const uint8_t *data, uint8_t len)
{
    CAN_Msg_t resp = { .id = CAN_ID_UDS_RESP, .dlc = 8 };
    memset(resp.data, 0, 8);
    /* ISO-TP Single Frame: byte[0] = 0x0N (N = PCI length) */
    uint8_t pci_len = 3 + len;  /* SID(1) + DID(2) + data */
    if (pci_len > 7) {
        /* Single Frame 초과 — 이번 스코프에서는 NRC 로 처리 */
        resp.data[0] = 0x03;
        resp.data[1] = SID_NEGATIVE_RESP;
        resp.data[2] = SID_READ_DATA_BY_ID;
        resp.data[3] = NRC_REQUEST_OUT_OF_RANGE;
    } else {
        resp.data[0] = pci_len;
        resp.data[1] = SID_READ_DATA_BY_ID | SID_POSITIVE_RESP_MASK; /* 0x62 */
        resp.data[2] = (uint8_t)(did >> 8);
        resp.data[3] = (uint8_t)(did & 0xFF);
        for (uint8_t i = 0; i < len; i++) {
            resp.data[4 + i] = data[i];
        }
    }
    /* TODO: CAN_BSP_Send(&hcan2, &resp); */
    (void)resp;
}

static void send_negative_response(uint8_t req_sid, uint8_t nrc)
{
    CAN_Msg_t resp = { .id = CAN_ID_UDS_RESP, .dlc = 8 };
    memset(resp.data, 0, 8);
    resp.data[0] = 0x03;                   /* PCI length */
    resp.data[1] = SID_NEGATIVE_RESP;      /* 0x7F */
    resp.data[2] = req_sid;
    resp.data[3] = nrc;
    /* TODO: CAN_BSP_Send(&hcan2, &resp); */
    (void)resp;
}

/* ========================================================================== */
/*  요청 처리                                                                 */
/* ========================================================================== */

static void handle_read_data_by_id(const CAN_Msg_t *req)
{
    /* req->data[0] = PCI length, [1] = SID 0x22, [2:3] = DID */
    uint8_t pci_len = req->data[0];
    if (pci_len != 3) {
        send_negative_response(SID_READ_DATA_BY_ID, NRC_INCORRECT_LENGTH);
        return;
    }
    uint16_t did = ((uint16_t)req->data[2] << 8) | req->data[3];

    uint8_t len;
    if (find_did(did, &len) != 0) {
        send_negative_response(SID_READ_DATA_BY_ID, NRC_REQUEST_OUT_OF_RANGE);
        return;
    }

    uint8_t payload[8] = {0};
    switch (did) {
        case DID_VIN:
            /* Single Frame 한계 — 처음 4자만 반환 (실제로는 ISO-TP 필요) */
            memcpy(payload, s_vin, 4);
            len = 4;
            break;
        case DID_RPM:
            payload[0] = (uint8_t)(s_cache_rpm >> 8);
            payload[1] = (uint8_t)(s_cache_rpm & 0xFF);
            break;
        case DID_SPEED:
            payload[0] = (uint8_t)(s_cache_speed >> 8);
            payload[1] = (uint8_t)(s_cache_speed & 0xFF);
            break;
        case DID_TEMP:
            payload[0] = (uint8_t)s_cache_temp;
            break;
        default:
            send_negative_response(SID_READ_DATA_BY_ID, NRC_REQUEST_OUT_OF_RANGE);
            return;
    }
    send_positive_response(did, payload, len);
}

/* ========================================================================== */

void UDS_Init(void)
{
    s_cache_rpm = 0U;
    s_cache_speed = 0U;
    s_cache_temp = 25;
}

void UDS_Task(void *argument)
{
    (void)argument;
    CAN_Msg_t req;

    for (;;) {
        /* TODO: xQueueReceive(can2_rx_queue, &req, portMAX_DELAY); */
        memset(&req, 0, sizeof(req));

        if (req.id != CAN_ID_UDS_REQ) {
            /* TODO: RPM/Speed/Temp 메시지면 캐시 갱신 */
            if (req.id == CAN_ID_RPM) {
                uint16_t raw = (uint16_t)req.data[2] | ((uint16_t)req.data[3] << 8);
                UDS_UpdateCache_RPM(DECODE_RPM(raw));
            }
            break; /* STUB */
        }

        /* SID 디스패치 */
        uint8_t sid = req.data[1];
        switch (sid) {
            case SID_READ_DATA_BY_ID:
                handle_read_data_by_id(&req);
                break;
            default:
                send_negative_response(sid, NRC_SERVICE_NOT_SUPPORTED);
                break;
        }
        break; /* STUB: 루프 폭주 방지 */
    }
}

/* ========================================================================== */
/*  UART CLI (간단 파서)                                                      */
/* ========================================================================== */

void UDS_CliTask(void *argument)
{
    (void)argument;

    /* TODO:
     *   - UART RX line buffer 구현 (인터럽트 or DMA)
     *   - "read_did <HEX>" 파싱
     *   - CAN_ID_UDS_REQ 메시지 생성 → CAN_BSP_Send(&hcan2, ...)
     *   - 응답 대기 후 UART 출력
     *
     *   예시 출력:
     *     > read_did F40C
     *     [TX] 7DF  22 F4 0C
     *     [RX] 7E8  62 F4 0C 0D AC  → RPM = 3500
     */
    for (;;) {
        break; /* STUB */
    }
}

/* ========================================================================== */

void UDS_UpdateCache_RPM(uint16_t rpm)        { s_cache_rpm = rpm; }
void UDS_UpdateCache_Speed(uint16_t kmh)      { s_cache_speed = kmh; }
void UDS_UpdateCache_Temp(int8_t celsius)     { s_cache_temp = celsius; }

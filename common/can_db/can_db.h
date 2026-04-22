/**
 * @file    can_db.h
 * @brief   CAN Database - 전 보드 공유 CAN ID 및 신호 정의
 * @note    ⚠️ 이 파일은 팀장 승인 없이 수정 금지 ⚠️
 *          모든 보드(A/B/C)가 이 파일을 include하여 ID 일치성을 보장함.
 *          하드코딩된 CAN ID 사용 금지 — 반드시 이 매크로를 사용할 것.
 */

#ifndef COMMON_CAN_DB_H
#define COMMON_CAN_DB_H

#include <stdint.h>

/* ========================================================================== */
/*  Powertrain Bus (CAN1) — 보드A → 보드B                                    */
/* ========================================================================== */

#define CAN_ID_RPM              0x280U  /* 엔진 회전수 */
#define CAN_ID_SPEED            0x1A0U  /* 차량 속도 */
#define CAN_ID_COOLANT          0x288U  /* 냉각수 온도 */

/* 송신 주기 (ms) */
#define PERIOD_RPM_MS           50U
#define PERIOD_SPEED_MS         100U
#define PERIOD_COOLANT_MS       1000U

/* ========================================================================== */
/*  Diagnostic Bus (CAN2) — 보드B ↔ 보드C ↔ 계기판                           */
/* ========================================================================== */

#define CAN_ID_WARNING          0x480U  /* Gateway → Cluster: 경고등 제어 */
#define CAN_ID_AIRBAG_OFF       0x050U  /* Cluster: 에어백 경고등 off */

/* UDS (ISO 14229) */
#define CAN_ID_UDS_REQ          0x7DFU  /* Tester(CLI) → ECU */
#define CAN_ID_UDS_RESP         0x7E8U  /* ECU → Tester */

/* ========================================================================== */
/*  VW Passat B6 계기판 인코딩                                                 */
/* ========================================================================== */

/* RPM: 0x280, byte[2:3] = rpm × 4 */
#define ENCODE_RPM(rpm_value)       ((uint16_t)((rpm_value) * 4U))
#define DECODE_RPM(raw_value)       ((uint16_t)((raw_value) / 4U))

/* Speed: 0x1A0, byte[2:3] = speed × 100 (0.01 km/h 단위) */
#define ENCODE_SPEED(kmh_value)     ((uint16_t)((kmh_value) * 100U))
#define DECODE_SPEED(raw_value)     ((uint16_t)((raw_value) / 100U))

/* Coolant Temp: 0x288, byte[3] = temp + 40 */
#define ENCODE_TEMP(celsius)        ((uint8_t)((celsius) + 40))
#define DECODE_TEMP(raw_value)      ((int8_t)((raw_value) - 40))

/* ========================================================================== */
/*  UDS DID (Data Identifier) - ISO 14229                                     */
/* ========================================================================== */

#define DID_VIN                 0xF190U  /* Vehicle Identification Number */
#define DID_RPM                 0xF40CU  /* Current RPM */
#define DID_SPEED               0xF40DU  /* Current Speed */
#define DID_TEMP                0xF40EU  /* Coolant Temp */

/* UDS SID */
#define SID_READ_DATA_BY_ID     0x22U
#define SID_POSITIVE_RESP_MASK  0x40U   /* Response SID = Request SID + 0x40 */
#define SID_NEGATIVE_RESP       0x7FU

/* UDS NRC (Negative Response Code) */
#define NRC_INCORRECT_LENGTH    0x13U
#define NRC_REQUEST_OUT_OF_RANGE 0x31U
#define NRC_SERVICE_NOT_SUPPORTED 0x11U

/* ========================================================================== */
/*  임계값 (Gateway 이상 감지)                                                */
/* ========================================================================== */

#define RPM_THRESHOLD_WARNING   5500U   /* 이 값 초과 시 경고등 ON */
#define SPEED_MAX_KMH           260U    /* 물리적 최대 속도 */

/* ========================================================================== */
/*  Warning Message (0x480) 비트필드                                           */
/* ========================================================================== */

#define WARN_BIT_RPM_OVER       (1U << 0)  /* byte[0] bit0: RPM 초과 경고 */
#define WARN_BIT_OVERHEAT       (1U << 1)  /* byte[0] bit1: 과열 경고 */
#define WARN_BIT_GENERAL        (1U << 2)  /* byte[0] bit2: 일반 경고 */

/* ========================================================================== */
/*  공통 프로토타입                                                            */
/* ========================================================================== */

/**
 * @brief 모든 보드에서 사용하는 CAN 메시지 구조체.
 *        HAL 비의존적 표현을 위해 중립적으로 정의.
 */
typedef struct {
    uint32_t id;        /* 11-bit standard ID */
    uint8_t  data[8];
    uint8_t  dlc;       /* 0 ~ 8 */
} CAN_Msg_t;

#endif /* COMMON_CAN_DB_H */

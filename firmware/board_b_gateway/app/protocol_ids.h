#ifndef PROTOCOL_IDS_H
#define PROTOCOL_IDS_H

#define CAN_ID_ENGINE_DATA           0x100U
#define CAN_ID_DASHBOARD_CTRL        0x200U
#define CAN_ID_BODY_STATUS           0x390U
#define CAN_ID_UDS_REQ_BOARD_B       0x7E0U
#define CAN_ID_UDS_RESP_BOARD_B      0x7E8U

/* --- CAN 메시지 ID 정의 (Board A -> Board B) --- */

/*
 * CAN_ID_ENGINE_DATA payload layout (8 bytes)
 *
 *  byte0-1 : RPM      (uint16_t, little-endian)
 *  byte2-3 : Speed    (uint16_t, little-endian)
 *  byte4   : Coolant  (uint8_t)
 *  byte5   : Status   (uint8_t)
 *            bit0    : IGN ON
 *            bit1~7  : Alive Counter
 *  byte6   : Reserved
 *  byte7   : Reserved
 */

#define CAN_ID_ENGINE_DATA               0x100U
#define CAN_ENGINE_DATA_DLC              8U
#define CAN_DLC_BODY_STATUS              8U

#define CAN_ENGINE_DATA_RPM_IDX          0U
#define CAN_ENGINE_DATA_SPEED_IDX        2U
#define CAN_ENGINE_DATA_COOLANT_IDX      4U
#define CAN_ENGINE_DATA_STATUS_IDX       5U
#define CAN_ENGINE_DATA_RESERVED0_IDX    6U
#define CAN_ENGINE_DATA_RESERVED1_IDX    7U

#define CAN_ENGINE_STATUS_IGN_MASK       0x01U
#define CAN_ENGINE_STATUS_ALIVE_SHIFT    1U
#define CAN_ENGINE_STATUS_ALIVE_MASK     0xFEU

static inline uint16_t CAN_GetU16LE(const uint8_t *buf, uint8_t index)
{
    return ((uint16_t)buf[index + 1U] << 8) | buf[index];
}

#endif

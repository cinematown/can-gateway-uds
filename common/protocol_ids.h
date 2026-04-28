#ifndef PROTOCOL_IDS_H
#define PROTOCOL_IDS_H

#define CAN_ID_ENGINE_DATA           0x100U
#define CAN_ID_DASHBOARD_CTRL        0x200U
#define CAN_ID_UDS_REQ_BOARD_B       0x7E0U
#define CAN_ID_UDS_RESP_BOARD_B      0x7E8U

/*
 * CAN_ID_ENGINE_DATA payload layout (8 bytes)
 *  byte0-1 : RPM      (uint16_t, little-endian)
 *  byte2-3 : Speed    (uint16_t, little-endian)
 *  byte4   : Coolant  (uint8_t)
 *  byte5   : Reserved
 *  byte6   : Reserved
 *  byte7   : Reserved
 */
#define CAN_ENGINE_DATA_DLC           8U
#define CAN_ENGINE_DATA_RPM_IDX       0U
#define CAN_ENGINE_DATA_SPEED_IDX     2U
#define CAN_ENGINE_DATA_COOLANT_IDX   4U
#define CAN_ENGINE_DATA_RESERVED0_IDX 5U
#define CAN_ENGINE_DATA_RESERVED1_IDX 6U
#define CAN_ENGINE_DATA_RESERVED2_IDX 7U

static inline uint16_t CAN_GetU16LE(const uint8_t *buf, uint8_t index)
{
    return ((uint16_t)buf[index + 1U] << 8) | buf[index];
}

#endif

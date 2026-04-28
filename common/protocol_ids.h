#ifndef PROTOCOL_IDS_H
#define PROTOCOL_IDS_H

#include "signal_db.h"

#define CAN_ID_ENGINE_RPM           SIGNAL_DB_CAN_ID_MOTOR_1
#define CAN_ID_ENGINE_SPEED         SIGNAL_DB_CAN_ID_BREMSE_1
#define CAN_ID_ENGINE_COOLANT       SIGNAL_DB_CAN_ID_MOTOR_2
#define CAN_ID_ENGINE_KEEPALIVE     0x300u
#define CAN_ID_ENGINE_DATA          CAN_ID_ENGINE_RPM
#define CAN_ID_DASHBOARD_CTRL       0x200u
#define CAN_ID_WARNING              0x480u

#define CAN_ID_UDS_REQ_BOARD_B      0x714u
#define CAN_ID_UDS_RESP_BOARD_B     0x77Eu

#define UDS_DID_VIN                 0xF190u
#define UDS_DID_RPM                 0xF40Cu
#define UDS_DID_SPEED               0xF40Du
#define UDS_DID_TEMP                0xF40Eu

#endif

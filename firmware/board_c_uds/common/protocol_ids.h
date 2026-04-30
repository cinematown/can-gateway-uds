#ifndef PROTOCOL_IDS_H
#define PROTOCOL_IDS_H

/*
 * Board C -> Instrument Cluster UDS communication IDs
 * CAN1 is used in this project.
 */
#define CAN_ID_CLUSTER_UDS_REQ      0x714u  /* Board C -> Cluster request */
#define CAN_ID_CLUSTER_UDS_RESP     0x77Eu  /* Cluster -> Board C response */

/* UDS DID (Data Identifier) values used for cluster diagnosis */
#define UDS_DID_PART_NUMBER         0xF187u /* Vehicle manufacturer spare part number */
#define UDS_DID_SW_NUMBER           0xF188u /* ECU software number */
#define UDS_DID_SW_VERSION          0xF189u /* ECU software version number */
#define UDS_DID_SERIAL_NUMBER       0xF18Cu /* ECU serial number */
#define UDS_DID_VIN                 0xF190u /* VIN */
#define UDS_DID_HW_NUMBER           0xF191u /* ECU hardware number */
#define UDS_DID_SYSTEM_NAME         0xF197u /* System name */

/* Optional dynamic signal DIDs. These may not be supported by the real cluster. */
#define UDS_DID_RPM                 0x280u /* Engine RPM */
#define UDS_DID_SPEED               0x1A0u /* Vehicle speed */
#define UDS_DID_COOLANT             0xF40Eu /* Coolant temperature */
#define UDS_DID_TEMP                UDS_DID_COOLANT

#define CAN_ID_RPM_DATA             UDS_DID_RPM      // 0x280
#define CAN_ID_SPEED_DATA           UDS_DID_SPEED    // 0x1A0
#define CAN_ID_COOLANT_DATA         UDS_DID_COOLANT 

#define UDS_DID_ALL                 0x0000u
#endif /* PROTOCOL_IDS_H */

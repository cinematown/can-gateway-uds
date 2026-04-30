#ifndef UDS_DEFS_H
#define UDS_DEFS_H

#include <stdint.h>

/* UDS service IDs used by Board C as a client */
#define UDS_SID_DIAGNOSTIC_SESSION_CONTROL  0x10u
#define UDS_SID_READ_DID                    0x22u
#define UDS_SID_TESTER_PRESENT              0x3Eu

/* Positive response IDs */
#define UDS_POS_DIAGNOSTIC_SESSION_CONTROL  0x50u
#define UDS_POS_READ_DID                    0x62u
#define UDS_POS_TESTER_PRESENT              0x7Eu

/* Negative response */
#define UDS_NEG_RESP                        0x7Fu

/* ISO-TP reassembly buffer size for cluster responses */
#define UDS_PAYLOAD_MAX_LEN                 64u

#endif /* UDS_DEFS_H */

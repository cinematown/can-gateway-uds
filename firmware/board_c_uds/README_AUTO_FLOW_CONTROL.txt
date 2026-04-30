Board C - CAN1 Cluster UDS Client with Automatic ISO-TP Flow Control

Purpose
- Board C works as a UDS tester/client.
- Board C sends UDS ReadDataByIdentifier requests to the instrument cluster.
- Request CAN ID : 0x714
- Response CAN ID: 0x77E
- CAN bus        : CAN1

Dedicated commands
- read vin      -> DID F190, VIN
- read part     -> DID F187, manufacturer spare part number
- read sw       -> DID F188, ECU software number
- read swver    -> DID F189, software version
- read serial   -> DID F18C, ECU serial number
- read hw       -> DID F191, hardware number
- read system   -> DID F197, system name

Automatic Flow Control
- If the cluster response is a Single Frame, the payload is decoded immediately.
- If the cluster response is a First Frame, for example:
    ID=0x77E DATA=10 xx ...
  the code automatically sends Flow Control CTS:
    ID=0x714 DATA=30 00 00 00 00 00 00 00
- Then it receives Consecutive Frames:
    ID=0x77E DATA=21 ...
    ID=0x77E DATA=22 ...
- After the full ISO-TP payload is assembled, it prints:
    [CLUSTER MF COMPLETE]
    [CLUSTER UDS RESP]
    [CLUSTER ASCII]

Recommended test
1. cluster_info
2. read vin
3. read part
4. read sw
5. read swver

Success examples
- Positive response: 62 F1 90 ...
- Negative response: 7F 22 31
  This still means CAN/UDS communication succeeded, but the DID was not supported or allowed.

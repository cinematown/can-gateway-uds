#include "gateway_body_bridge.h"

/*
 * Body Board bridge placeholder.
 *
 * CAN1 also carries Body Board messages such as turn signals, doors, lamps,
 * and other cluster-visible body states. Keep this module separate from the
 * engine bridge so each board protocol can evolve independently.
 */
void GatewayBodyBridge_OnRx(const CAN_RxMessage_t *rx_msg)
{
    (void)rx_msg;

    /*
     * TODO: Add Body Board protocol parsing here once the CAN IDs and payload
     * layout are fixed. Expected outputs may include VW body/cluster frames
     * such as 0x470.
     */
}

void GatewayBodyBridge_Task10ms(void)
{
    /*
     * TODO: Add periodic body/cluster keep-alive transmission here if the
     * target cluster requires body state frames to be refreshed.
     */
}

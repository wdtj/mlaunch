#include "zb.h"

/*
 Network Watchdog Timeout. Set/read the network watchdog timeout value. If NW is
 set > 0, the router will monitor communication from the coordinator (or data collector)
 and leave the network if it cannot communicate with the coordinator for 3 NW periods.
 The timer is reset each time data is received from or sent to a coordinator, or if a many-
 to-one broadcast is received.

 0 - 0x64FF [x 1 minute] (up to over 17 days)
 */

void zb_nw(unsigned char fid, unsigned char time)
{
    zb_write2("%c%cNW", ZB_AT_COMMAND, fid);
}


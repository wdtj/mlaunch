#include "zb.h"

/*
 Received Signal Strength. This command reports the received signal strength of the
 last received RF data packet. The DB command only indicates the signal strength of the
 last hop. It does not provide an accurate quality measurement for a multihop link. DB can
 be set to 0 to clear it. The DB command value is measured in -dBm. For example if DB
 returns 0x50, then the RSSI of the last packet received was
 -80dBm. As of 2x6x firmware, the DB command value is also updated when an APS
 acknowledgment is received.

 Observed range for
 XBee-PRO: 0x1A - 0x58
 For XBee: 0x 1A - 0x5C

 */
void zb_db(unsigned char fid)
{
    zb_write2("%c%cDB", ZB_AT_COMMAND, fid);
}


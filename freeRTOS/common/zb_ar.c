#include "zb.h"

/*

 Aggregate Routing Notification. Set/read time between consecutive aggregate route
 broadcast messages. If used, AR may be set on only one device to enable many-to-one
 routing to the device. Setting AR to 0 only sends one broadcast. AR is in units of 10
 seconds.

 */
void zb_ar(unsigned char fid, char mode)
{
    zb_write2("%c%cAR%c", ZB_AT_COMMAND, fid, mode);
}


#include "zb.h"

/*
 Operating Channel. Read the channel number used for transmitting and receiving
 between RF modules. Uses 802.15.4 channel numbers. A value of 0 means the device
 has not joined a PAN and is not operating on any channel.

 XBee  0, 0x0B - 0x1A (Channels 11-26)
 XBee-PRO (S2) 0, 0x0B - 0x18 (Channels 11-24)
 XBee-PRO (S2B) 0, 0x0B - 0x19 (Channels 11-25)

 */
void zb_ch(unsigned char fid)
{
    zb_write2("%c%cCH", ZB_AT_COMMAND, fid);
}


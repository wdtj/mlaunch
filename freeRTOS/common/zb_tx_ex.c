#include "zb.h"
#include "string.h"

/*
 Frame Type: 0x11

 Allows ZigBee application layer fields (endpoint and cluster ID) to be specified for a data transmission.

 Similar to the ZigBee Transmit Request, but also requires ZigBee application layer addressing fields to be
 specified (endpoints, cluster ID, profile ID). An Explicit Addressing Request API frame causes the module to
 send data as an RF packet to the specified destination, using the specified source and destination endpoints,
 cluster ID, and profile ID.

 The 64-bit destination address should be set to 0x000000000000FFFF for a broadcast transmission (to all
 devices). The coordinator can be addressed by either setting the 64-bit address to all 0x00s and the 16-bit
 address to 0xFFFE, OR by setting the 64-bit address to the coordinator's 64-bit address and the 16-bit address
 to 0x0000. For all other transmissions, setting the 16-bit address to the correct 16-bit address can help improve
 performance when transmitting to multiple destinations. If a 16-bit address is not known, this field should be
 set to 0xFFFE (unknown). The Transmit Status frame (0x8B) will indicate the discovered 16-bit address, if
 successful.

 The broadcast radius can be set from 0 up to NH. If set to 0, the value of NH specifies the broadcast radius
 (recommended). This parameter is only used for broadcast transmissions.

 The maximum number of payload bytes can be read with the NP command. Note: if source routing is used, the
 RF payload will be reduced by two bytes per intermediate hop in the source route.
 */
void zb_tx_ex(unsigned char fid, zbAddr dest, zbNetAddr nad,
              unsigned char src, unsigned char dst,
              unsigned short clust,
              unsigned short prof,
              unsigned char radius, char options, char *data, int size)
{
    zb_write2("%c%c%a%n%c%c%c%c%c%c%c%c%S",
              ZB_TRANSMIT_EXPLICIT, fid, dest, nad,
              src, dst,
              (char)(clust >> 8 & 0xff), (char)(clust & 0xff),
              (char)(prof >> 8 & 0xff), (char)(prof & 0xff),
              radius,
              options,
              size,
              data);

}


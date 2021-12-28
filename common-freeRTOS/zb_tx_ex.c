#include "zb.h"
#include "string.h"

void zb_tx_ex(unsigned char fid, zbAddr dest, zbNetAddr nad, unsigned char src,
        unsigned char dst, unsigned short clust, zbProfile prof,
        unsigned char radius, char *data, int size)
{
    struct
    {
        unsigned char frameType;
        unsigned char fid;
        zbAddr dest;
        zbNetAddr nad;
        unsigned char src;
        unsigned char dst;
        unsigned char clust[2];
        zbProfile profile;
        unsigned char radius;
        unsigned char fill;
        unsigned char data[72];
    } block =
        { ZB_TRANSMIT_EXPLICIT, fid };

    block.dest = dest;
    block.nad = nad;
    block.src = src;
    block.dst = dst;
    block.clust[0] = (unsigned char) (clust >> 8 & 0xff);
    block.clust[1] = (unsigned char) (clust & 0xff);
    block.profile = prof;
    block.radius = radius;
    block.fill = 0;
    memcpy(&block.data, data, size);
    zb_write((unsigned char *) &block, size + 20);
}


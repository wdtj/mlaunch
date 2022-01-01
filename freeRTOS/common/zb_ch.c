#include "zb.h"

void zb_ch(unsigned char fid)
{
    struct
    {
        unsigned char frameType;
        unsigned char frameId;
        unsigned char cmd[2];
    } block =
        {
        ZB_AT_COMMAND, fid,
            { 'C', 'H' } };

    zb_write((char *) &block, sizeof block);
}


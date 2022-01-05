#include "zb.h"

/*
 Software Reset. Reset module. Responds immediately with an OK status, and then
 performs a software reset about 2 seconds later.
 */

void zb_fr(unsigned char fid)
{
    zb_write2("%c%cFR", ZB_AT_COMMAND, fid);
}


#include "zb.h"

void zb_fr(unsigned char fid)
{
  struct {
    unsigned char frameType;
    unsigned char frameId;
    unsigned char cmd[2];
  } block = {
    ZB_AT_COMMAND,
    fid,
    {'F', 'R'}
  };

  zb_write((unsigned char *)&block, sizeof block);
}


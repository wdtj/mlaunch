#include "zb.h"

void zb_wr(unsigned char fid)
{
	struct {
		unsigned char frameType;
		unsigned char frameId;
		unsigned char cmd[2];
		} block = {
		ZB_AT_COMMAND,
		fid,
		{'W', 'R'}
	};

	zb_write((unsigned char *)&block, sizeof block);
}


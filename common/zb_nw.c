#include "zb.h"

void zb_nw(unsigned char fid, unsigned char time)
{
	struct {
		unsigned char frameType;
		unsigned char frameId;
		unsigned char cmd[2];
		unsigned char time;
		} block = {
		ZB_AT_COMMAND,
		fid,
		{'N', 'W'},
		time
	};

	zb_write((unsigned char *)&block, sizeof block);
}


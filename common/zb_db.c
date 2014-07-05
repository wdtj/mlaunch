#include "zb.h"

void zb_db(unsigned char fid)
{
	struct {
		unsigned char frameType;
		unsigned char frameId;
		unsigned char cmd[2];
		} block = {
		ZB_AT_COMMAND,
		fid,
		{'D', 'B'}
	};

	zb_write((unsigned char *)&block, sizeof block);
}


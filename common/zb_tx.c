#include "zb.h"
#include "string.h"

bool zb_txBusy=false;

void zb_tx(unsigned char fid, 
           zbAddr dest, 
           zbNetAddr nad, 
           unsigned char bcast, 
           unsigned char radius, 
           char *data, 
           int size)
{
  struct {
    unsigned char frameType;
    unsigned char fid;
    zbAddr dest;
    zbNetAddr nad;
    unsigned char radius;
    unsigned char opt; 
    unsigned char data[72]; 
  } block={ZB_TRANSMIT_REQUEST, fid}; 

  zb_txBusy=true;
  
  block.dest=dest;
  block.nad=nad;
  block.radius=radius;
  block.opt=bcast;
  memcpy(&block.data, data, size); 
  zb_write((unsigned char *)&block, size+14); 
}

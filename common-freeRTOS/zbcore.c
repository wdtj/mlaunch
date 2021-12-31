#include "zb.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

void (*zbWritePtr)(char *buff, int count);
void (*zbReceivedPktPtr)(unsigned char *pkt, unsigned int length);

void zbInit(void (*zbWrite)(char *buff, int count),
void (*zbReceivedPkt)(unsigned char *pkt, unsigned int length))
{
    zbWritePtr = zbWrite;
    zbReceivedPktPtr = zbReceivedPkt;
}

bool fe = false;
bool cse = true;

/*
  This function takes a single character and through a finite state machine
  demarshalls and determines if a full xbee packet has been received.  If it 
  is, it returns a pointer to the packet.  If not it returns NULL and waits 
  for more characters.
*/
void zbReceive(unsigned char ch)
{
    static enum {
        readEsc, readLenMsb, readLenLsb, readPkt, readCksum
    } portState = readEsc;

    static unsigned int length;
    static unsigned int count;
    static unsigned char cksum;

    static unsigned char pkt[256];
    static unsigned char *ptr = pkt;

    switch(portState) {
    case readEsc:
        if(ch == 0x7e) {
            portState = readLenMsb;
        } else {
            fe = true;
        }
        break;

    case readLenMsb:
        length = ch << 8;
        portState = readLenLsb;
        break;

    case readLenLsb:
        length += ch;

        count = length;
        cksum = 0;
        portState = readPkt;
        break;

    case readPkt:
        *ptr = ch;
        ptr++;
        cksum += ch;
        count--;
        if(count == 0) {
            portState = readCksum;
        }
        break;

    case readCksum:
        cksum = 0xff - cksum;
        if(ch != cksum) {
            cse = true;
        }
        portState = readEsc;

        (*zbReceivedPktPtr)(pkt, length);
        ptr = pkt;
        break;
    }
}

int zb_write(char *block, int size)
{
    char cksum = 0;
    unsigned long i;

    (*zbWritePtr)("\x7e", 1);
    (*zbWritePtr)(((char *) &size) + 1, 1);
    (*zbWritePtr)(((char *) &size), 1);

    for(i = 0; i < size; ++i) {
        cksum += *(block + i);
    }
    cksum = 0xff - cksum;

    (*zbWritePtr)(block, size);
    (*zbWritePtr)(&cksum, 1);

    return 0;
}

/*
 *  Node Identifier. Set/read a string identifier. The register only accepts printable ASCII
 *  data. In AT Command Mode, a string cannot start with a space. A carriage return ends
 *  the command. A command will automatically end when maximum bytes for the string
 *  have been entered. This string is returned as part of the ND (Node Discover) command.
 *  This identifier is also used with the DN (Destination Node) command. In AT command
 *  mode, an ASCII comma (0x2C) cannot be used in the NI string
 */
void zb_ni(unsigned char fid, char *string)
{
    struct {
        unsigned char frameType;
        unsigned char frameId;
        unsigned char cmd[2];
        char ni[20];
    } block = {
        ZB_AT_COMMAND, fid,
        { 'N', 'I' }
    };
    if (string)
    {
        strncpy(block.ni, string, sizeof block.ni);
        zb_write((char *) &block, 4 + strlen(string));
    }
    else
    {
        zb_write((char *) &block, 4);
    }
}

void zb_nj(unsigned char fid, unsigned char join)
{
    struct {
        unsigned char frameType;
        unsigned char frameId;
        unsigned char cmd[2];
        unsigned char join;
    } block = {
        ZB_AT_COMMAND, fid,
        { 'N', 'J' }, join
    };

    zb_write((char *) &block, sizeof block);
}

void zb_nr(unsigned char fid, unsigned char reset)
{
    struct {
        unsigned char frameType;
        unsigned char frameId;
        unsigned char cmd[2];
        unsigned char reset;
    } block = {
        ZB_AT_COMMAND, fid,
        { 'N', 'R' }, reset
    };

    zb_write((char *) &block, sizeof block);
}

void zb_nd(unsigned char fid)
{
    struct {
        unsigned char frameType;
        unsigned char frameId;
        unsigned char cmd[2];
    } block = {
        ZB_AT_COMMAND, fid,
        { 'N', 'D' }
    };

    zb_write((char *) &block, sizeof block);
}

void zb_jn(unsigned char fid, unsigned char notify)
{
    struct {
        unsigned char frameType;
        unsigned char frameId;
        unsigned char cmd[2];
        unsigned char notify;
    } block = {
        ZB_AT_COMMAND, fid,
        { 'J', 'N' }, notify
    };

    zb_write((char *) &block, sizeof block);
}

void zb_ao(unsigned char fid, unsigned char option)
{
    struct {
        unsigned char frameType;
        unsigned char frameId;
        unsigned char cmd[2];
        unsigned char option;
    } block = {
        ZB_AT_COMMAND, fid,
        { 'A', 'O' }, option
    };

    zb_write((char *) &block, sizeof block);
}

void zb_jv(unsigned char fid, unsigned char v)
{
    struct {
        unsigned char frameType;
        unsigned char frameId;
        unsigned char cmd[2];
        unsigned char v;
    } block = {
        ZB_AT_COMMAND, fid,
        { 'J', 'V' }, v
    };

    zb_write((char *) &block, sizeof block);
}

void zb_id(unsigned char fid, unsigned long v)
{
    struct {
        unsigned char frameType;
        unsigned char frameId;
        unsigned char cmd[2];
        unsigned long long v;
    } block = {
        ZB_AT_COMMAND, fid,
        { 'I', 'D' }, v
    };

    zb_write((char *) &block, sizeof block);
}

void zb_vr(unsigned char fid)
{
    struct {
        unsigned char frameType;
        unsigned char frameId;
        unsigned char cmd[2];
    } block = {
        ZB_AT_COMMAND, fid,
        { 'V', 'R' }
    };

    zb_write((char *) &block, sizeof block);
}

void zb_no(unsigned char fid, unsigned char opt)
{
    struct {
        unsigned char frameType;
        unsigned char frameId;
        unsigned char cmd[2];
        unsigned char opt;
    } block = {
        ZB_AT_COMMAND, fid,
        { 'N', 'O' }, opt
    };

    zb_write((char *) &block, sizeof block);
}

int zbAddrCmp(zbAddr addr1, zbAddr addr2)
{
    return addr1.addr64[0] == addr2.addr64[0]
           && addr1.addr64[1] == addr2.addr64[1]
           && addr1.addr64[2] == addr2.addr64[2]
           && addr1.addr64[3] == addr2.addr64[3]
           && addr1.addr64[4] == addr2.addr64[4]
           && addr1.addr64[5] == addr2.addr64[5]
           && addr1.addr64[6] == addr2.addr64[6]
           && addr1.addr64[7] == addr2.addr64[7];
}

void zbAddrZero(zbAddr addr)
{
    addr.addr64[0] = 0;
    addr.addr64[1] = 0;
    addr.addr64[2] = 0;
    addr.addr64[3] = 0;
    addr.addr64[4] = 0;
    addr.addr64[5] = 0;
    addr.addr64[6] = 0;
    addr.addr64[7] = 0;
}


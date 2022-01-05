#include "zb.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

void (*zbWritePtr)(char *buff, int count);
void (*zbReceivedPktPtr)(unsigned char *pkt, unsigned int length);

void zbInit(
    void (*zbWrite)(char *buff, int count),
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


void zb_write2(char *format, ...)
{
    char ch;
    char *string;
    int length;

    va_list argp;
    va_start(argp, format);

    int size = 0;

    char *ptr = format;
    while(*ptr != '\0') {
        if(*ptr == '%') {
            ptr++;
            switch(*ptr) {
            case '%':
                size++;
                break;
            case 'c':
                size++;
                ch = va_arg(argp, int);
                break;
            case 's':
                string = va_arg(argp, char *);
                size += strlen(string);
                break;
            }
        } else {
            size++;
        }
        ptr++;
    }
    va_end(argp);

    va_start(argp, format);

    char cksum = 0;
    unsigned long i;

    (*zbWritePtr)("\x7e", 1);

    (*zbWritePtr)(((char *) &size) + 1, 1);
    (*zbWritePtr)(((char *) &size), 1);

    while(*format != '\0') {
        if(*format == '%') {

            format++;
            switch(*format) {
            case '%':
                (*zbWritePtr)("%", 1);
                cksum += '%';
                break;
            case 'c':
                ch = va_arg(argp, int);
                (*zbWritePtr)(&ch, 1);
                cksum += ch;
                break;
            case 's':
                string = va_arg(argp, char *);
                length = strlen(string);
                (*zbWritePtr)(string, length);
                for(i = 0; i < length; ++i) {
                    cksum += *(string + i);
                }

                break;
            }
        } else {
            (*zbWritePtr)(format, 1);
            cksum += *format;
        }
        format++;
    }
    cksum = 0xff - cksum;
    (*zbWritePtr)(&cksum, 1);

    va_end(argp);
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
    if(strlen(string) > 20) {
        string[20] = '\0';
    }
    zb_write2("%c%cNI%s", ZB_AT_COMMAND, fid, string);
}

void zb_nj(unsigned char fid, unsigned char join)
{
    zb_write2("%c%cNJ%c", ZB_AT_COMMAND, fid, join);
}

void zb_nr(unsigned char fid, unsigned char reset)
{
    zb_write2("%c%cNR%c", ZB_AT_COMMAND, fid, reset);
}

void zb_nd(unsigned char fid)
{
    zb_write2("%c%cND", ZB_AT_COMMAND, fid);
}

void zb_jn(unsigned char fid, unsigned char notify)
{
    zb_write2("%c%cJN%c", ZB_AT_COMMAND, fid, notify);
}

void zb_ao(unsigned char fid, unsigned char option)
{
    zb_write2("%c%cAO%c", ZB_AT_COMMAND, fid, option);
}

void zb_jv(unsigned char fid, unsigned char verify)
{
    zb_write2("%c%cJV%c", ZB_AT_COMMAND, fid, verify);
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


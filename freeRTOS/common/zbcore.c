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

void cksumCalc(char *cksum, char *block, int length)
{
    while(length-- > 0) {
        *cksum += *(block++);
    }
}

void zb_write2(char *format, ...)
{
    char ch;
    char *string;
    int length;

    va_list argp;
    va_start(argp, format);

    /*
    The API packets require a length at the start, we calculate that here by
    walking through the request format.
    */
    int size = 0;

    char *ptr = format;
    while(*ptr != '\0') {
        if(*ptr == '%') {
            ptr++;
            switch(*ptr) {
            case 'a': {
                    zbAddr addr = va_arg(argp, zbAddr);
                    size += sizeof addr;
                }
                break;
            case 'n': {
                    zbNetAddr netAddr = va_arg(argp, zbNetAddr);
                    size += sizeof netAddr;
                }
                break;
            case 'c':
                size++;
                ch = va_arg(argp, int);
                break;
            case 's':
                string = va_arg(argp, char *);
                size += strlen(string);
                break;
            case 'S':
                length = va_arg(argp, int);
                string = va_arg(argp, char *);
                size += length;
            }
        } else {
            size++;
        }
        ptr++;
    }
    va_end(argp);

    va_start(argp, format);

    char cksum = 0;

    (*zbWritePtr)("\x7e", 1);

    (*zbWritePtr)(((char *) &size) + 1, 1);
    (*zbWritePtr)(((char *) &size), 1);

    while(*format != '\0') {
        if(*format == '%') {

            format++;
            switch(*format) {
            case 'a': {
                    zbAddr addr = va_arg(argp, zbAddr);
                    (*zbWritePtr)((char *)&addr, sizeof addr);
                    cksumCalc(&cksum, (char *)&addr, sizeof addr);
                    break;
                }
            case 'n': {
                    zbNetAddr netAddr = va_arg(argp, zbNetAddr);
                    (*zbWritePtr)((char *)&netAddr, sizeof netAddr);
                    cksumCalc(&cksum, (char *)&netAddr, sizeof netAddr);
                }
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
                cksumCalc(&cksum, string, length);
                break;
            case 'S':
                length = va_arg(argp, int);
                string = va_arg(argp, char *);
                (*zbWritePtr)(string, length);
                cksumCalc(&cksum, string, length);
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
  Node Identifier. Set/read a string identifier. The register only accepts printable ASCII
  data. In AT Command Mode, a string cannot start with a space. A carriage return ends
  the command. A command will automatically end when maximum bytes for the string
  have been entered. This string is returned as part of the ND (Node Discover) command.
  This identifier is also used with the DN (Destination Node) command. In AT command
  mode, an ASCII comma (0x2C) cannot be used in the NI string
 */
void zb_ni(unsigned char fid, char *string)
{
    if(strlen(string) > 20) {
        string[20] = '\0';
    }
    zb_write2("%c%cNI%s", ZB_AT_COMMAND, fid, string);
}

/*
 Node Join Time. Set/Read the time that a Coordinator/Router allows nodes to join.
 This value can be changed at run time without requiring a Coordinator or Router to
 restart. The time starts once the Coordinator or Router has started. The timer is reset
 on power-cycle or when NJ changes.
 For an end device to enable rejoining, NJ should be set less than 0xFF on the device
 that will join. If NJ < 0xFF, the device assumes the network is not allowing joining and
 first tries to join a network using rejoining. If multiple rejoining attempts fail, or if
 NJ=0xFF, the device will attempt to join using association.
 Note: Setting the NJ command will not cause the radio to broadcast the new value of
 NJ out to the network via a Mgmt_Permit_Joining_req; this value is transmitted by
 setting CB=2. See the command description for CB for more information.
 */
void zb_nj(unsigned char fid, unsigned char join)
{
    zb_write2("%c%cNJ%c", ZB_AT_COMMAND, fid, join);
}

/*
 Network Reset. Reset network layer parameters on one or more modules within a PAN.
 Responds immediately with an “OK” then causes a network restart. All network
 configuration and routing information is consequently lost.
 If NR = 0: Resets network layer parameters on the node issuing the command.
 If NR = 1: Sends broadcast transmission to reset network layer parameters on all nodes
 in the PAN
 */
void zb_nr(unsigned char fid, unsigned char reset)
{
    zb_write2("%c%cNR%c", ZB_AT_COMMAND, fid, reset);
}

/*
 Node Discover. Discovers and reports all RF modules found. The following information
 is reported for each module discovered.
  SH<CR>
  SL<CR>
  NI<CR> (Variable length)
  PARENT_NETWORK ADDRESS (2 Bytes)<CR>
  DEVICE_TYPE<CR> (1 Byte: 0=Coord, 1=Router, 2=End Device)
  STATUS<CR> (1 Byte: Reserved)
  PROFILE_ID<CR> (2 Bytes)
  MANUFACTURER_ID<CR> (2 Bytes)
  <CR>
 After (NT * 100) milliseconds, the command ends by returning a <CR>. ND also accepts
 a Node Identifier (NI) as a parameter (optional). In this case, only a module that matches
 the supplied identifier will respond.
 If ND is sent through the API, each response is returned as a separate
 AT_CMD_Response packet. The data consists of the above listed bytes without the
 carriage return delimiters. The NI string will end in a "0x00" null character. The radius of
 the ND command is set by the BH command.
*/
void zb_nd(unsigned char fid)
{
    zb_write2("%c%cND", ZB_AT_COMMAND, fid);
}

/*
 Node Join Time. Set/Read the time that a Coordinator/Router allows nodes to join.
 This value can be changed at run time without requiring a Coordinator or Router to
 restart. The time starts once the Coordinator or Router has started. The timer is reset
 on power-cycle or when NJ changes.
 For an end device to enable rejoining, NJ should be set less than 0xFF on the device
 that will join. If NJ < 0xFF, the device assumes the network is not allowing joining and
 first tries to join a network using rejoining. If multiple rejoining attempts fail, or if
 NJ=0xFF, the device will attempt to join using association.
 Note: Setting the NJ command will not cause the radio to broadcast the new value of
 NJ out to the network via a Mgmt_Permit_Joining_req; this value is transmitted by
 setting CB=2. See the command description for CB for more information.
*/
void zb_jn(unsigned char fid, unsigned char notify)
{
    zb_write2("%c%cJN%c", ZB_AT_COMMAND, fid, notify);
}

/*
 API Options. Configure options for API. Current options select the type of receive API
 frame to send out the Uart for received RF data packets

 0 - Default receive API indicators enabled
 1 - Explicit Rx data indicator API frame enabled (0x91)
 3 - enable ZDO passthrough of ZDO requests to the UART which are not supported by the stack,
     as well as Simple_Desc_req, Active_EP_req, and Match_Desc_req
 */
void zb_ao(unsigned char fid, unsigned char option)
{
    zb_write2("%c%cAO%c", ZB_AT_COMMAND, fid, option);
}

/*
 Channel Verification. Set/Read the channel verification parameter. If JV=1, a router
 will verify the coordinator is on its operating channel when joining or coming up from a
 power cycle. If a coordinator is not detected, the router will leave its current channel and
 attempt to join a new PAN. If JV=0, the router will continue operating on its current
 channel even if a coordinator is not detected.
 */
void zb_jv(unsigned char fid, unsigned char verify)
{
    zb_write2("%c%cJV%c", ZB_AT_COMMAND, fid, verify);
}

/*
 Extended PAN ID. Set/read the 64-bit extended PAN ID. If set to 0, the coordinator will
 select a random extended PAN ID, and the router / end device will join any extended
 PAN ID. Changes to ID should be written to non-volatile memory using the WR
 command to preserve the ID setting if a power cycle occurs
 */
void zb_id(unsigned char fid, unsigned long id)
{
    zb_write2("%c%cID%c", ZB_AT_COMMAND, fid, id);
}

/*
 Firmware Version. Read firmware version of the module.
 The firmware version returns 4 hexadecimal values (2 bytes) "ABCD". Digits ABC are
 the main release number and D is the revision number from the main release. "B" is a
 variant designator.

 XBee and XBee-PRO ZB modules return:
 0x2xxx versions.

 XBee and XBee-PRO ZNet modules return:
 0x1xxx versions. ZNet firmware is not compatible with ZB firmware
 */
void zb_vr(unsigned char fid)
{
    zb_write2("%c%cVR", ZB_AT_COMMAND, fid);
}

/*
 Network Discovery options. Set/Read the options value for the network discovery
 command. The options bitfield value can change the behavior of the ND (network
 discovery) command and/or change what optional values are returned in any received
 ND responses or API node identification frames. Options include:
 0x01 = Append DD value (to ND responses or API node identification frames)
 002 = Local device sends ND response frame when ND is issued.
 */
void zb_no(unsigned char fid, unsigned char opt)
{
    zb_write2("%c%cNO%c", ZB_AT_COMMAND, fid, opt);
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


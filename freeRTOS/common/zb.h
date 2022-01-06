#ifndef ZB_HEADER

#include <stdbool.h>

#ifdef  __cplusplus
extern "C"
{
#endif
//#define ZB_DEBUG(level, s) if ((level) <= zb_debug) {zb_log##s;}

#define ZB_AT_COMMAND 0x08
#define ZB_AT_COMMAND_QUEUE 0x09
#define ZB_TRANSMIT_REQUEST 0x10
#define ZB_TRANSMIT_EXPLICIT 0x11
#define ZB_REMOTE_COMMAND_REQUEST 0x17
#define ZB_SOURCE_ROUTE 0x21

#define ZB_AT_COMMAND_RESPONSE 0x88
#define ZB_MODEM_STATUS 0x8A
#define ZB_TRANSMIT_STATUS 0x8B
#define ZB_RECEIVE_PACKET 0x90
#define ZB_EXPLICIT_RX_INDICATOR 0x91
#define ZB_IO_DATA_SAMPLE 0x92
#define ZB_SENSOR_READ 0x94
#define ZB_NODE_IDENTIFICATION 0x95
#define ZB_REMOTE_COMMAND_RESPONSE 0x97
#define ZB_FIRMWARE_UPDATE 0xA0
#define ZB_ROUTE_RECORD 0xA1
#define ZB_MANY_TO_ONE_ROUTE_REQUEST_INDICATOR 0xA3

/* Substructures */

/* factory assigned 64 bit address (AKA MAC) */
typedef struct zbAddr
{
    unsigned char addr64[8];
} zbAddr;

int zbAddrCmp(zbAddr addr1, zbAddr addr2);
void zbAddrZero(zbAddr addr);

/* Coordinator assigned 16 bit address */
typedef struct zbNetAddr
{
    unsigned char addr16[2];
} zbNetAddr;

#define zbNetAddrZero(addr) { addr.addr16[0]=0; addr.addr16[1]=0; }
#define zbNetAddrCmp(addr1, addr2) { addr1.addr16[0]=addr2.addr16[0] && addr1.addr16[1]=addr2.addr16[1];  }


/* Frame types */

/* FrameType ZB_AT_COMMAND (0x08) */
typedef struct zbATCommand
{
    unsigned char frameId;
    unsigned char cmd[2];
    unsigned char data[72];
} zbATCommand;

/* FrameType ZB_AT_COMMAND_QUEUE (0x09) */
typedef struct zbATCommandQueue
{
    unsigned char frameId;
    unsigned char cmd[2];
    unsigned char data[72];
} zbATCommandQueue;

/* FrameType ZB_TRANSMIT_REQUEST (0x10) */
typedef struct zbTx
{
    unsigned char frameId;
    zbAddr dest;
    zbNetAddr nad;
    unsigned char radius;
    unsigned char opt;
    unsigned char data[72];
} zbTx;

/* FrameType ZB_TRANSMIT_EXPLICIT (0x11)*/
typedef struct zbExpTx
{
    unsigned char frameId;
    zbAddr dest;
    zbNetAddr nad;
    unsigned char src;
    unsigned char dst;
    unsigned short cluster;
    unsigned short profile;
    unsigned char radius;
    unsigned char opt;
    unsigned char data[72];
} zbExpTx;

/* FrameType ZB_REMOTE_COMMAND_REQUEST (0x17) */
typedef struct zbRemoteCommand
{
    unsigned char frameId;
    zbAddr dest;
    zbNetAddr nad;
    unsigned char opt;
    unsigned char cmd[2];
    unsigned char data[72];
} zbRemoteCommand;

/* FrameType ZB_SOURCE_ROUTE (0x21) */
typedef struct zbSourceRoute
{
    unsigned char frameId;
    zbAddr dest;
    zbNetAddr nad;
    unsigned char opt;
    unsigned char numAddr;
    zbNetAddr naddr[];
} zbSourceRoute;

/* FrameType ZB_AT_COMMAND_RESPONSE (0x88) */

#define ZB_AT_STATUS_OK 0
#define ZB_AT_STATUS_ERROR 1
#define ZB_AT_STATUS_INV_CMD 2
#define ZB_AT_STATUS_INV_PARAM 3
#define ZB_AT_STATUS_TX_FAIL 4

typedef struct zbATResponse
{
    unsigned char frameId;
    unsigned char cmd[2];
    unsigned char status;
    union
    {
        unsigned char data[72];
        struct
        {
            unsigned char my[2];
            unsigned char sh[4];
            unsigned char sl[4];
            unsigned char ni[20];
        };
    };
} zbATResponse;

struct ATNDData
{
    zbNetAddr netAddr;
    zbAddr addr;
    unsigned char ni[20];
};


/* FrameType ZB_MODEM_STATUS (0x8A) */

enum status
{
    zb_mdm_hwrst = 0,
    zb_mdm_wdrst = 1,
    zb_mdm_assoc = 2,
    zb_mdm_disassoc = 3,
    zb_mdm_start = 6
};

typedef struct zbModemStatus
{
    enum status status;
} zbModemStatus;

/* FrameType ZB_TRANSMIT_STATUS (0x8B) */
typedef struct zbTransmitStatus
{
    unsigned char frameId;
    zbNetAddr nad;
    unsigned char retry;
    unsigned char delStatus;
    unsigned char disStatus;
} zbTxStatus;

/* FrameType ZB_RECEIVE_PACKET (0x90) */
typedef struct zbRx
{
    zbAddr dest;
    zbNetAddr nad;
    unsigned char opt;
    unsigned char data[72];
} zbRx;

/* FrameType ZB_EXPLICIT_RX_INDICATOR (0x91) */
typedef struct zbExpRx
{
    zbAddr dest;
    zbNetAddr nad;
    unsigned char src;
    unsigned char dst;
    unsigned short cluster;
    unsigned short profile;
    unsigned char opt;
    unsigned char data[72];
} zbExpRx;

/* FrameType ZB_IO_DATA_SAMPLE (0x92) */
typedef struct zbSample
{
    unsigned char addr64[8];
    unsigned char addr16[2];
    unsigned char opt;
    unsigned char numSamples;
    unsigned char digitalMask[2];
    unsigned char analogMask;
    unsigned char data[72];
} zbSample;

/* FrameType ZB_SENSOR_READ (0x94) */
typedef struct zbSensor
{
    unsigned char addr64[8];
    unsigned char addr16[2];
    unsigned char opt;
    unsigned char sensorMask;
    unsigned char data[72];
} zbSensor;

/* FrameType ZB_NODE_IDENTIFICATION (0x95) */
typedef struct zbNID
{
    zbAddr dest;
    zbNetAddr destNad;
    unsigned char opt;
    zbNetAddr remNetAddr;
    zbAddr remAddr;
    unsigned char ni[20];
    unsigned char parent[2];
    unsigned char type;
    unsigned char src;
    unsigned short profile;
    unsigned char manufacture[2];
} zbNID;

/* FrameType ZB_REMOTE_COMMAND_RESPONSE (0x97) */
typedef struct zbRemResponse
{
    unsigned char frameId;
    unsigned char addr64[8];
    unsigned char addr16[2];
    unsigned char cmd[2];
    unsigned char status;
    unsigned char data[72];
} zbRemResponse;

/* FrameType ZB_FIRMWARE_UPDATE 0xA0) */
typedef struct zbFirmwareUpdate
{
    zbAddr src;
    zbNetAddr destNad;
    unsigned char opt;
    unsigned char bootMsgType;
    unsigned char blockNo;
    zbAddr target;
} zbFirmwareUpdate;

/* FrameType ZB_ROUTE_RECORD (0xA1) */
typedef struct zbRR
{
    zbAddr dest;
    zbNetAddr destNad;
    unsigned char opt;
    unsigned char numAddr;
    zbNetAddr route[1];
} zbRR;

/* FrameType ZB_MANY_TO_ONE_ROUTE_REQUEST_INDICATOR (0xA3) */
typedef struct zbManyToOneRouteRequestIndicator
{
    zbAddr dest;
    zbNetAddr destNad;
} zbManyToOneRouteRequestIndicator;

typedef struct zbFrame
{
    unsigned char esc;
    unsigned char pktlen[2];
    unsigned char frameType;
    union
    {
        struct zbATCommand zbATCommand;
        struct zbATCommandQueue zbATCommandQueue;
        struct zbTx zbTx;
        struct zbExpTx zbExpTx;
        struct zbRemoteCommand zbRemoteCommand;
        struct zbSourceRoute zbSourceRoute;

        struct zbATResponse zbATResponse;
        struct zbModemStatus zbModemStatus;
        struct zbTransmitStatus zbTransmitStatus;
        struct zbRx zbRX;
        struct zbExpRx zbExpRx;
        struct zbSample zbSample;
        struct zbSensor zbSensor;
        struct zbNID zbNID;
        struct zbRemResponse zbRemResponse;
        struct zbFirmwareUpdate zbFirmwareUpdate;
        struct zbRR zbRR;
        struct zbManyToOneRouteRequestIndicator zbManyToOneRouteRequestIndicator;
    };
    unsigned char cksum;
} zbFrame;

typedef struct zbPkt
{
    unsigned char frameType;
    union
    {
        struct zbATCommand zbATCommand;
        struct zbATCommandQueue zbATCommandQueue;
        struct zbTx zbTx;
        struct zbExpTx zbExpTx;
        struct zbRemoteCommand zbRemoteCommand;
        struct zbSourceRoute zbSourceRoute;

        struct zbATResponse zbATResponse;
        struct zbModemStatus zbModemStatus;
        struct zbTransmitStatus zbTransmitStatus;
        struct zbRx zbRX;
        struct zbExpRx zbExpRx;
        struct zbSample zbSample;
        struct zbSensor zbSensor;
        struct zbNID zbNID;
        struct zbRemResponse zbRemResponse;
        struct zbFirmwareUpdate zbFirmwareUpdate;
        struct zbRR zbRR;
        struct zbManyToOneRouteRequestIndicator zbManyToOneRouteRequestIndicator;
    };
} zbPkt;

void zb_tx(unsigned char fid, zbAddr dest, zbNetAddr nad, unsigned char bcast,
        unsigned char radius, char *data, int size);

void zb_tx_ex(unsigned char fid, zbAddr dest, zbNetAddr nad, unsigned char src,
        unsigned char dst, unsigned short clust, unsigned short prof,
        unsigned char radius, char options, char *data, int size);

void zbInit(
        void (*zbWrite)(char *buff, int count),
        void (*zbReceivedPkt)(unsigned char *pkt, unsigned int length));
void zbReceive(unsigned char ch);

void zb_settimeout(int time);
void zb_timeout();
void zb_flush();
void zb_abort(char *msg);
int zb_write(char *block, int size);
void zb_write2(char *format, ...);

void zb_ni(unsigned char fid, char *string);
void zb_nj(unsigned char fid, unsigned char join);
void zb_nr(unsigned char fid, unsigned char reset);
void zb_nd(unsigned char fid);
void zb_ao(unsigned char fid, unsigned char option);
void zb_jv(unsigned char fid, unsigned char verify);
void zb_jn(unsigned char fid, unsigned char notify);
void zb_id(unsigned char fid, unsigned long v);
void zb_fr(unsigned char fid);
void zb_ch(unsigned char fid);
void zb_wr(unsigned char fid);
void zb_db(unsigned char fid);
void zb_nw(unsigned char fid, unsigned char time);
void zb_vr(unsigned char fid);
void zb_no(unsigned char fid, unsigned char opt);

#ifdef  __cplusplus
}
#endif

#define ZB_HEADER
#endif

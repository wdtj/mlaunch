/*
 * linkFSM.c
 *
 * Created: 6/30/2014 5:40:18 PM
 *  Author: waynej
 */

#include "pad30.h"
#include "../common/config.h"
#include "../common/uart.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <string.h>

#include "../common/zb.h"

volatile enum LINKSTATE
{
    SEND_NI,
    NI_SENT,
    SEND_JV,
    JV_SENT,
    SEND_JN,
    JN_SENT,
    SEND_NW,
    NW_SENT,
    SEND_WR,
    WR_SENT,
    SEND_FR,
    FR_SENT,
    SEND_CH,
    CH_SENT,
    SEND_DB,
    DB_SENT,
    RESET,
    READY
} linkState;

char ni[21];
unsigned char channel;
unsigned char signalStrength;
unsigned int linkTimer = 0;
unsigned int statTimer = 0;

void linkFSMinit(const char const * nodeName)
{
    strcpy(ni, nodeName);
    linkState = SEND_NI;
}

void linkFSMtimer(void)
{
    if (linkTimer != 0)
    {
        linkTimer--;
        if (linkTimer == 1)
        {
            linkState = SEND_NI;
        } else
        {
            linkTimer--;
        }
    }

    if (statTimer != 0)
    {
        if (statTimer == 1)
        {
            linkState = SEND_DB;
            statTimer = 0;
        } else
        {
            statTimer--;
        }
    }

}

void linkFSMpkt(zbPkt *pkt)
{
    switch (pkt->frameType)
    {
    case ZB_MODEM_STATUS:
    {
        struct zbModemStatus *ms = &pkt->zbModemStatus;
        switch (ms->status)
        {
        case zb_mdm_assoc:
        case zb_mdm_start:
            linkState = SEND_CH;
            linkTimer = 0;
            break;

        case zb_mdm_disassoc:
        case zb_mdm_hwrst:
        case zb_mdm_wdrst:
            linkState = RESET;
            statTimer = 0;
            linkTimer = 0;
            break;
        }
    }
        break;

    case ZB_TRANSMIT_STATUS:
    {
//			struct zbTransmitStatus *ts=&pkt->zbTransmitStatus;
    }
        break;

    case ZB_AT_COMMAND_RESPONSE:
    {
        struct zbATResponse *cr = &pkt->zbATResponse;

        switch (linkState)
        {
        case NI_SENT:
            linkState = SEND_JV;
            linkTimer = 0;
            break;

        case JV_SENT:
            linkState = SEND_JN;
            linkTimer = 0;
            break;

        case JN_SENT:
            linkState = SEND_NW;
            linkTimer = 0;
            break;

        case NW_SENT:
            linkState = SEND_WR;
            linkTimer = 0;
            break;

        case WR_SENT:
            linkState = SEND_FR;
            linkTimer = 0;
            break;

        case CH_SENT:
            channel = cr->data[0];
            linkState = SEND_DB;
            linkTimer = 0;
            break;

        case DB_SENT:
            signalStrength = cr->data[0];
            linkState = READY;
            linkTimer = 0;
            statTimer = 10000 / TIMER0_PERIOD;
            break;

        default:
            break;
        }
    }
        break;

    case ZB_NODE_IDENTIFICATION:
    {
        break;
    }

    default:
#if defined(DEBUG)
        while(false)
        {}
#endif
        break;
    }
}

void linkFSMToDo(void)
{
    switch (linkState)
    {
    case SEND_NI:
        zb_ni(1, ni);
        linkState = NI_SENT;
        linkTimer = 1000 / TIMER0_PERIOD;
        break;

    case SEND_JV:
        zb_jv(2, 1);
        linkState = JV_SENT;
        linkTimer = 1000 / TIMER0_PERIOD;
        break;

    case SEND_JN:
        zb_jn(3, 1);
        linkState = JN_SENT;
        linkTimer = 1000 / TIMER0_PERIOD;
        break;

    case SEND_NW:
        zb_nw(4, 1);
        linkState = NW_SENT;
        linkTimer = 1000 / TIMER0_PERIOD;
        break;

    case SEND_WR:
        zb_wr(5);
        linkState = WR_SENT;
        linkTimer = 1000 / TIMER0_PERIOD;
        break;

    case SEND_FR:
        zb_fr(6);
        linkState = FR_SENT;
        linkTimer = 10000 / TIMER0_PERIOD;
        break;

    case SEND_CH:
        zb_ch(7);
        linkState = CH_SENT;
        linkTimer = 1000 / TIMER0_PERIOD;
        break;

    case SEND_DB:
        zb_db(8);
        linkState = DB_SENT;
        linkTimer = 1000 / TIMER0_PERIOD;
        break;

    default:
        break;
    }
}

enum LINK_STATUS linkFSMStatus(void)
{
    switch (linkState)
    {
    case SEND_NI:
        return MODEM_INIT;
    case READY:
    case SEND_CH:
    case CH_SENT:
    case SEND_DB:
    case DB_SENT:
        return MODEM_READY;
    default:
        return MODEM_RESET;
    }
}

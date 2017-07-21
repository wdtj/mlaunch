/*
 * linkFSM.c
 *
 * Created: 6/30/2014 5:40:18 PM
 *  Author: waynej
 */

#include "pad30.h"
#include "../../common/config.h"
#include "../../common/uart.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <string.h>

#include "../../common/zb.h"

volatile enum LINKSTATE
{
    SEND_NI1,
    NI_SENT1,
    SEND_JV,
    JV_SENT,
    SEND_JN,
    JN_SENT,
    SEND_NW,
    NW_SENT,
    SEND_WR,
    WR_SENT,
    SEND_NI2,
    NI_SENT2,
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

/* Initialization sequence 
 NI1 with node id

 */
void linkFSMinit(const char const * nodeName)
{
    strcpy(ni, nodeName);
    linkState = SEND_NI1;
}

void linkFSMset()
{
    linkState = NI_SENT1;
}

void linkFSMtimer(void)
{
    if (linkTimer != 0)
    {
        linkTimer--;
        if (linkTimer == 1)
        {
            linkState = SEND_NI1;
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
        case NI_SENT1:
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
            linkState = SEND_NI2;
            linkTimer = 0;
            break;

        case NI_SENT2:
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
#if defined(myDEBUG)
        while(false)
        {}
#endif
        break;
    }
}

void linkFSMToDo(void)
{
    // If we have data from the modem, go get it
    while (uart_rxReady() > 0)
    {
        unsigned char ch = uart_getchar(NULL);

#if defined(TRACE)
        *(trace_buffer_ptr++)=ch;
        if (trace_buffer_ptr==trace_buffer_end)
        {
            trace_buffer_ptr=trace_buffer;
        }
#endif

        zbReceivedChar(ch);
        if (uart_fe)
        {
            uart_fe = 0;
        }
        ASSERT(uart_doe || uart_pe || uart_roe);
    }

    switch (linkState)
    {
    // Reset Node Identifier.
    case SEND_NI1:
        zb_ni(1, ni);
        linkState = NI_SENT1;
        linkTimer = 1000 / TIMER0_PERIOD;
        break;

        // Set Channel Verification on.
        // Will verify the coordinator is on its operating channel when joining or
        // coming up from a power cycle. If a coordinator is not detected, the
        // router will leave its current channel and attempt to join a new PAN.
    case SEND_JV:
        zb_jv(2, 1);
        linkState = JV_SENT;
        linkTimer = 1000 / TIMER0_PERIOD;
        break;

        // Set Join Notification on.
        // The module will transmit a broadcast node identification packet on
        // power up and when joining
    case SEND_JN:
        zb_jn(3, 1);
        linkState = JN_SENT;
        linkTimer = 1000 / TIMER0_PERIOD;
        break;

        // Set Network Watchdog Timeout.
        // Will monitor communication from the coordinator (or data collector)
        // and leave the network if it cannot communicate with the coordinator for
        // 3 NW periods.
    case SEND_NW:
        zb_nw(4, 1);
        linkState = NW_SENT;
        linkTimer = 1000 / TIMER0_PERIOD;
        break;

        // Write. Write parameter values to non-volatile memory so that parameter
        // modifications persist through subsequent resets.
    case SEND_WR:
        zb_wr(5);
        linkState = WR_SENT;
        linkTimer = 1000 / TIMER0_PERIOD;
        break;

        // Reset Node Identifier.
    case SEND_NI2:
        zb_ni(6, ni);
        linkState = NI_SENT2;
        linkTimer = 1000 / TIMER0_PERIOD;
        break;

        // Set Software Reset. Reset module. Responds immediately with an OK
        // status, and then performs a software reset about 2 seconds later.
    case SEND_FR:
        zb_fr(7);
        linkState = FR_SENT;
        linkTimer = 10000 / TIMER0_PERIOD;
        break;

        // Read Operating Channel. Read the channel number used for transmitting
        // and receiving between RF modules. Uses 802.15.4 channel numbers. A value
        // of 0 means the device has not joined a PAN and is not operating on any
        // channel.
    case SEND_CH:
        zb_ch(8);
        linkState = CH_SENT;
        linkTimer = 1000 / TIMER0_PERIOD;
        break;

        // Received Signal Strength. This command reports the received signal
        // strength of the last received RF data packet. The DB command only
        // indicates the signal strength of the last hop. It does not provide an
        // accurate quality measurement for a multihop link. DB can be set to 0
        // to clear it. The DB command value is measured in -dBm.
    case SEND_DB:
        zb_db(9);
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
    case SEND_NI1:
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

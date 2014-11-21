/*
 * commFSM.c
 *
 * Created: 2/27/2014 7:38:11 PM
 *  Author: waynej
 */ 

#include <stdbool.h>
#include "../common/zb.h"
#include "../common/OLED.h"
#include "controller30.h"

void linkData( zbRx* rxPkt, unsigned int length );
void printpkt(zbRx* rxPkt, unsigned int length);

volatile enum status status=0;

volatile enum linkStatus {
	reset,
	waitForReset,
	discovery,
	ready
} linkStatus;

unsigned int discoveryTimer=0;

void linkStatusReset( zbPkt * pkt )
{
	switch(pkt->frameType)
	{
		case ZB_AT_COMMAND_RESPONSE:
		{
#if defined(myDEBUG)
			struct zbATResponse *resp=&pkt->zbATResponse;
			OLED_XYprintf(0, 3, "AT%c%c", resp->cmd[0], resp->cmd[1]);
			OLED_clearEOL();
#endif
		}
		break;
		
		case ZB_MODEM_STATUS:
		{
			struct zbModemStatus *ms=&pkt->zbModemStatus;
			int mss=ms->status;
			switch (mss)
			{
			  case zb_mdm_start:
				OLED_XYprintf(0, 1, "Waiting for network");
				OLED_clearEOL();
				discoveryTimer=30000/TIMER0_PERIOD;
				zb_nd(1);
				linkStatus=waitForReset;
				break;
			  case zb_mdm_disassoc:
				OLED_XYprintf(0, 1, "Disassociated");
				OLED_clearEOL();
				break;
			  case zb_mdm_assoc:
			    OLED_XYprintf(0, 1, "Joined Net");
			    OLED_clearEOL();
			    linkStatus=ready;
			  break;
			}
			break;
		}
		break;
		
		case ZB_RECEIVE_PACKET:
			OLED_XYprintf(0, 3, "RX");
			OLED_clearEOL();
		break;
		
		case ZB_NODE_IDENTIFICATION:
			OLED_XYprintf(0, 3, "NI");
			OLED_clearEOL();
		break;
		
		default:
			OLED_XYprintf(0, 3, "Unknown %x", pkt->frameType);
			OLED_clearEOL();
		break;
	}
}

struct ATNDData
{
	zbNetAddr netAddr;
	zbAddr addr;
	unsigned char ni[20];
};

void linkStatusDiscovery( zbPkt * pkt )
{
	switch(pkt->frameType)
	{
		case ZB_AT_COMMAND_RESPONSE:
		{
			struct zbATResponse *atr=&pkt->zbATResponse;
			if (atr->cmd[0]=='N' && atr->cmd[1]=='D')
			{
				struct ATNDData *nid=(struct ATNDData *)atr->data;
				padDiscovered(nid->addr, nid->netAddr, nid->ni);
			}
#if defined(myDEBUG)
			else
			{
				OLED_XYprintf(0, 3, "AT%c%c", atr->cmd[0], atr->cmd[1]);
				OLED_clearEOL();
			}
#endif
		}
		break;
		
		case ZB_NODE_IDENTIFICATION:
		{
			struct zbNID *nid=&pkt->zbNID;
			padDiscovered(nid->remAddr, nid->remNetAddr, nid->ni);
		}
		break;
		
		case ZB_MODEM_STATUS:
		{
			struct zbModemStatus *ms=&pkt->zbModemStatus;
			int mss=ms->status;
			switch (mss)
			{
			  case zb_mdm_start:
				OLED_XYprintf(0, 3, "Modem Start");
				OLED_clearEOL();
				break;
			  case zb_mdm_disassoc:
				OLED_XYprintf(0, 3, "Modem Disassociated");
				OLED_clearEOL();
				break;
			}
			break;
		}
		break;
		case ZB_TRANSMIT_STATUS:
		break;
		default:
			OLED_XYprintf(0, 3, "Unknown %x", pkt->frameType);
			OLED_clearEOL();
			break;
	}
}

void linkStatusReady( zbPkt * pkt, unsigned int length )
{
	switch(pkt->frameType)
	{
		case ZB_RECEIVE_PACKET:
			linkData(&pkt->zbRX, length-(((char *)&pkt->zbRX)-((char *)pkt)));
		break;
		case ZB_NODE_IDENTIFICATION:
		{
			struct zbNID *nid=&pkt->zbNID;
			padDiscovered(nid->remAddr, nid->remNetAddr, nid->ni);
		}
		break;
	}
}

void linkPkt(unsigned char *data, unsigned int length)
{
	zbPkt *pkt=(zbPkt *)data;
	switch (linkStatus)
	{
		case reset:
		linkStatusReset(pkt);
		break;

		case waitForReset:
		linkStatusDiscovery(pkt);
		break;
		
		case discovery:
		linkStatusDiscovery(pkt);
		break;
		
		case ready:
		linkStatusReady(pkt, length);
		break;
	}
}

void linkData( zbRx* rxPkt, unsigned int length )
{
	int dataLength=length-(((char *)rxPkt->data)-((char *)rxPkt));
//	OLED_XYprintf(0, 1, "PKT %16s", rxPkt->data);
//	OLED_XYprintf(0, 2, "PKT %16s", rxPkt->data+16);
	switch (rxPkt->data[0])
	{
		case 'S':
		{
			statusMessage((char *)rxPkt->data, dataLength);
		}
		break;
		default:
#if defined(myDEBUG)
#warning debug pn
			printpkt(rxPkt, dataLength);
			while(true){}
#endif
		break;
	}
}

void printpkt(zbRx* rxPkt, unsigned int length)
{
	OLED_XYprintf(0, 1, "PKT '");
	unsigned char *ptr=rxPkt->data;
	unsigned int lineLength=(length>16?16:length);
	length-=lineLength;
	while(lineLength)
	{
		OLED_printf("%c", *ptr++);
		lineLength--;
	}
	if (length==0)
	{
		OLED_printf("'");
		return;
	}
	lineLength=(length>16?16:length);
	while(lineLength)
	{
		OLED_printf("%c", *ptr++);
		lineLength--;
	}
}

void linkStart(bool notInitialized)
{
	if (notInitialized)
	{
		zb_ni(0, "Controller30");
		OLED_XYprintf(0, 1, "Net Reset");
		OLED_clearEOL();
		linkStatus=reset;
		zb_nr(1, 1);
	}
	else
	{
		OLED_XYprintf(0, 1, "Discovering Network");
		OLED_clearEOL();
		linkStatus=discovery;
		discoveryTimer=6000/TIMER0_PERIOD;
		zb_nd(1);
	}
}

void linkTimer(void)
{
	switch (linkStatus)
	{
		case reset:
		break;
		
		case waitForReset:
		discoveryTimer--;
	
		if ((discoveryTimer%(1000/TIMER0_PERIOD))==0)
		{
			OLED_XYprintf(0, 2, "%d ", discoveryTimer/(1000/TIMER0_PERIOD));
		}
		
		if (discoveryTimer==0)
		{
			OLED_XYprintf(0, 1, "Discovering Network");
			OLED_clearEOL();
			linkStatus=discovery;
			discoveryTimer=6000/TIMER0_PERIOD;
			zb_nd(1);
		}
		break;

		case discovery:
		discoveryTimer--;
	
		if ((discoveryTimer%(1000/TIMER0_PERIOD))==0)
		{
			OLED_XYprintf(0, 2, "%d", discoveryTimer/(1000/TIMER0_PERIOD));
		}
		
		if (discoveryTimer==0)
		{
			linkStatus=ready;
		}
		break;
		
		case ready:
		break;
	}
	
}

bool isLinkReady()
{
	return (linkStatus==ready);
}


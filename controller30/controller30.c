/*
 * controller30.c
 *
 * Created: 2/26/2014 7:34:55 PM
 *  Author: waynej
 */ 


#include "controller30.h"
#include "../common/config.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <string.h>

#include "../common/led.h"
#include "../common/OLED.h"
#include "../common/adc.h"
#include "../common/uart.h"
#include "../common/zb.h"
#include "../common/Timer0.h"
#include "../common/Timer1.h"
#include "linkFSM.h"
#include "switchFSM.h"

void init(void);
int zbWrite(void *buff, unsigned int count);
void timer0(void);
void enable(int sw);
void disable(int sw);
void padFSMTimer(void);
void sendEnable( int sw );
void sendDisable( int sw );
void sendLaunch( int sw );
void sendIdent( zbAddr addr, zbNetAddr netAddr );
void sendAssign( int sw );

FILE xbee = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

char rxBuffer[200]="";
char txBuffer[200]="";

unsigned int greenLeds=0;
unsigned int redLeds=0;
unsigned int yellowLeds=0;

volatile struct padInfo pads[]=
{
    {
        IDLE,
        '1'
    },
    {
        IDLE,
        '2'
    },
    {
        IDLE,
        '3'
    },
    {
        IDLE,
        '4'
    },
    {
        IDLE,
        '5'
    },
    {
        IDLE,
        '6'
    },
    {
        IDLE,
        '7'
    },
    {
        IDLE,
        '8'
    }
};

struct NewPad newPad[8];

/* Flag to force manual initialization */
bool notInitialized=false;

int main(void)
{
    bool launchPressed=false;
    
    init();

    LED_output(0x0, 0x0, 0x0);            /* Turn off LEDs */

    OLED_display(true, false, false);    /* Display on, Cursor off, Blink off */
    OLED_functionSet(true, 0);            /* 8 bit, English/Japanese char set */
    OLED_entryModeSet(true, false);        /* Increment, no shift */
    OLED_cursorDisplayShift(false, true); /* Shift cursor, Right */

    OLED_clearDisplay();                /* Blank display */

    OLED_XYprintf(0, 0, "mLaunch 3.0");
    OLED_XYprintf(0, 2, "Copyright 2016 by");
    OLED_XYprintf(0, 3, "Wayne Johnson");

    OLED_clearLine(2);
    OLED_clearLine(3);
    
#if defined(iDEBUG)
        notInitialized=true;
#else
    if (PINB!=0xFF)
    {
        notInitialized=true;
        OLED_XYprintf(0, 3, "Reinitializing");
    }
#endif

    OLED_XYprintf(0, 1, "Comm Init");
    OLED_clearEOL();

    /* Flash the pad lights to be pretty */
    for(int i=0; i<10; ++i)
    {
        unsigned int bit=1<<i;
        LED_output(bit, bit>>1, bit>>2);
        _delay_ms(100);
        LED_output(0x0, 0x0, 0x0);
    }

    linkInit(notInitialized);

    OLED_clearDisplay();
    
    /* main loop */
    bool displayCluttered=false;
    while(1)
    {
        bool allOpen=true;
        
        linkFSM();
        
        if (isLinkReady())
        {
        
            for(int sw=0; sw<8; ++sw)
            {
                if(!pads[sw].discovered)
                {
                    continue;
                }
            
                if (isClosed(sw))
                {
                    allOpen=false;
                    if (pads[sw].padState == IDLE)
                    {
                        enable(sw);
                    } 
                    else if (pads[sw].padState == ENABLED)
                    {
                        disable(sw);
                    }
                }
                else
                {
                    if (pads[sw].padState == PRESSED2ENABLE)
                    {
                        pads[sw].padState=ENABLED;
                    } 
                    else if (pads[sw].padState == PRESSED2DISABLE)
                    {
                        pads[sw].padState=IDLE;
                    }
                }
            
                if (pads[sw].newStatus)
                {
                    if (pads[sw].padState == PRESSED2ENABLE)
                    {
                        OLED_clearDisplay();
                        OLED_XYprintf(0,0, "Pad %d", sw+1);
                        if (pads[sw].contResistance==-1)
                        {
                            OLED_XYprintf(0,1, "Resistance=OPEN", pads[sw].contResistance);
                        }
                        else
                        {
                            OLED_XYprintf(0,1, "Resistance=%d", pads[sw].contResistance);
                        }

                        OLED_XYprintf(0,2, "Batt=%d.%d", pads[sw].batt/100, pads[sw].batt%100);
                        displayCluttered=true;
                        pads[sw].newStatus=false;
                    }
                }
            
                if (pads[sw].padState == ENABLED ||
                    pads[sw].padState == PRESSED2ENABLE)
                {
                    if (pads[sw].statusValid)
                    {
                        if (pads[sw].contState==0)
                        {
                            redLeds|=_BV(sw);
                            greenLeds&=~_BV(sw);
                            yellowLeds&=~_BV(sw);
                        }
                        else
                        {
                            redLeds&=~_BV(sw);
                            greenLeds|=_BV(sw);
                            yellowLeds&=~_BV(sw);
                        }
                    }
                    else
                    {
                        redLeds&=~_BV(sw);
                        greenLeds&=~_BV(sw);
                        yellowLeds|=_BV(sw);
                    }
                }

                if (pads[sw].padState == IDLE)
                {
                    redLeds&=~_BV(sw);
                    greenLeds&=~_BV(sw);
                    yellowLeds&=~_BV(sw);
                }

        if (pads[sw].sendEnable)
        {
          sendEnable(sw);
          pads[sw].sendEnable=false;
        }

        if (pads[sw].sendLaunch)
        {
          sendLaunch(sw);
          pads[sw].sendLaunch=false;
        }

            }
            LED_output(redLeds, greenLeds, yellowLeds);
        
            if (allOpen == true && displayCluttered)
            {
          OLED_clearLine(0);
          OLED_clearLine(1);
          OLED_clearLine(2);
                OLED_XYprintf(0, 0, "Ready");
                displayCluttered=false;
            }
        
            if (isClosed(8) && !launchPressed)
            {
                launchPressed=true;
                for (int pad=0; pad<8; ++pad)
                {
                    if (pads[pad].padState==ENABLED)
                    {
                        pads[pad].padState=PAD_LAUNCH;
                        pads[pad].enableTimer=0;
                    }
                }
            }
            else if (!isClosed(8) && launchPressed)
            {
                launchPressed=false;
                for (int pad=0; pad<8; ++pad)
                {
                    if (pads[pad].padState==PAD_LAUNCH)
                    {
                        sendDisable(pad);
                        pads[pad].padState=IDLE;
                    }
                }
            }
        }
    }
}

void init(void)
{
    DDRB=0x0;        // Set switches as input
    PORTB=0xff;

    DDRC&=~0x7f;    // Set Launch Switch
    PORTC|=0x80;
    
    DDRC|=_BV(1);
    DDRC|=_BV(6);

    LED_init();
    OLED_init();
    
    timer0_init(CS_1024, timer0);                    // Timer0 uses system clock/1024

#if(F_CPU/1024/1000*TIMER0_PERIOD >=256)
#error timer speed too slow
#endif

    timer0_set(F_CPU/1024/1000*TIMER0_PERIOD);        // Timer0 is 20 ms

    timer1_init_normal(CS_1024);                    // Timer1 uses system clock/1024
    timer1_set_normal();

    uart_txBuff(txBuffer, sizeof txBuffer);
    uart_rxBuff(rxBuffer, sizeof rxBuffer);

    uart_init(UART_BAUD, NULL);
    
    zbInit(&zbWrite, &linkPkt);

    sei();                                // enable interrupts
}

int zbWrite(void *buff, unsigned int count)
{
    return fwrite(buff, count, 1, &xbee);
}

#define xUnitTest
#ifdef UnitTest
static int utTimer=0;
#endif

volatile static unsigned int maxSwitch=0;
volatile static unsigned int maxPad=0;
volatile static unsigned int maxLink=0;

void timer0(void)
{
    
#ifdef UnitTest
    utTimer++;
    if (utTimer==1000/TIMER0_PERIOD)
    {
        PORTC&=~_BV(1);
    }
    else if (utTimer==3000/TIMER0_PERIOD)
    {
        PORTC|=_BV(1);
        utTimer=0;
    }
#endif
    
    unsigned int timer=TCNT1;
    switchFSMtimer();
    maxSwitch=MAX(maxSwitch, TCNT1-timer);

    timer=TCNT1;
    padFSMTimer();
    maxPad=MAX(maxPad, TCNT1-timer);
    
    timer=TCNT1;
    linkTimer();
    maxLink=MAX(maxLink, TCNT1-timer);

#if(F_CPU/1024/1000*TIMER0_PERIOD >=256)
#error timer speed too slow
#endif
    timer0_set(F_CPU/1024/1000*TIMER0_PERIOD);
}

void enable(int sw)
{
    pads[sw].padState=PRESSED2ENABLE;
    redLeds&=~_BV(sw);
    yellowLeds|=_BV(sw);
    greenLeds&=~_BV(sw);
    LED_output(redLeds, greenLeds, yellowLeds);    
    pads[sw].enableTimer=0;
    pads[sw].statusTimer=5000;
    pads[sw].newStatus=false;
}


void disable(int sw)
{
    pads[sw].padState=PRESSED2DISABLE;    
    redLeds&=~_BV(sw);
    yellowLeds&=~_BV(sw);
    greenLeds&=~_BV(sw);
    LED_output(redLeds, greenLeds, yellowLeds);
    pads[sw].statusValid=false;

    sendDisable(sw);
}

void sendEnable( int sw )
{
    char msg[]="En";
    msg[1]=pads[sw].padId;
    zb_tx(0, pads[sw].addr, pads[sw].netAddr, 0, 0, msg, 2);
}

void sendDisable( int sw )
{
    char msg[]="Rn";
    msg[1]=pads[sw].padId;
    zb_tx(0, pads[sw].addr, pads[sw].netAddr, 0, 0, msg, 2);
}

void sendLaunch( int sw )
{
    char msg[]="Ln";
    msg[1]=pads[sw].padId;
    zb_tx(0, pads[sw].addr, pads[sw].netAddr, 0, 0, msg, 2);
}

void sendAssign( int sw )
{
    char msg[]="Ann";
    msg[1]=pads[sw].padId;
    msg[2]=pads[sw+1].padId;
    zb_tx(0, pads[sw].addr, pads[sw].netAddr, 0, 0, msg, 3);
}

void sendIdent( zbAddr addr, zbNetAddr netAddr )
{
    char msg[]="I";
    zb_tx(0, addr, netAddr, 0, 0, msg, 1);
}

void sendUnident( zbAddr addr, zbNetAddr netAddr )
{
    char msg[]="X";
    zb_tx(0, addr, netAddr, 0, 0, msg, 1);
}

void padFSMTimer(void)
{
    for(int pad=0; pad<8; ++pad)
    {
        switch (pads[pad].padState)
        {
        case PRESSED2ENABLE:
        case ENABLED:
            {
                pads[pad].enableTimer--;
                if (pads[pad].enableTimer<=0)
                {
                    pads[pad].sendEnable=true;
                    pads[pad].enableTimer=2000/TIMER0_PERIOD;
                }
                
                pads[pad].statusTimer--;
                if (pads[pad].statusTimer<=0)
                {
                    pads[pad].statusValid=false;
                }
            }
        break;
        case IDLE:
        case PRESSED2DISABLE:
        break;
        case PAD_LAUNCH:
            pads[pad].enableTimer--;
            if (pads[pad].enableTimer<=0)
            {
        pads[pad].sendLaunch=true;
                pads[pad].enableTimer=1000/TIMER0_PERIOD;
            }
        break;
        }
    }
}

void statusMessage(char * data,int length)
{
    unsigned char padnum;
    int contState, launchState;
    int pad;
    int contResistance, batt, batt1, batt2;
    
    data[length-1]='\0';
    
    sscanf(data, "S%c e%d l%d Cr%d Vb%d.%d", 
        &padnum, &contState, &launchState, &contResistance, &batt1, &batt2);

    batt=batt1*100+batt2;

    pad=padnum-'1';
    if (pad<8)
    {
        pads[pad].contState=contState;
        pads[pad].launchState=launchState;
        pads[pad].contResistance=contResistance;
        pads[pad].batt=batt;
        pads[pad].statusValid=true;
        pads[pad].newStatus=true;
        pads[pad].statusTimer=5000/TIMER0_PERIOD;
    }
}

// Add pad to ToBeAssigned list
void addToNew( unsigned char * ni, zbNetAddr netAddr, zbAddr addr )
{
    // find a slot in newPad
    for(int i=0; i<8; ++i)
    {
        // Don't duplicate new pads in the list
        if (newPad[i].used==true && memcmp(&newPad[i].netAddr, &netAddr, sizeof netAddr)==0)
        {
            return;
        }

        /* Add info to new pad list */
        if (newPad[i].used==false)
        {
            newPad[i].addr=addr;
            newPad[i].netAddr=netAddr;
            newPad[i].used=true;
#if defined(DEBUG)
            OLED_XYprintf(0, 1, "%02x%02x %02x%02x %02x%02x %02x%02x",
            newPad[i].addr.addr64[0],
            newPad[i].addr.addr64[1],
            newPad[i].addr.addr64[2],
            newPad[i].addr.addr64[3],
            newPad[i].addr.addr64[4],
            newPad[i].addr.addr64[5],
            newPad[i].addr.addr64[6],
            newPad[i].addr.addr64[7]);
            OLED_XYprintf(0, 3, "New pad");
            OLED_clearEOL();
#endif
            return;
        }
    }
    ASSERT("New pad overrun");
}

void padDiscovered( zbAddr addr, zbNetAddr netAddr, unsigned char *ni )
{
    /* If it's not one of ours, ignore it */
    if (ni[0] != 'P' || ni[1] != 'A' || ni[2] != 'D') return;

    unsigned char padId0=ni[3];
    unsigned char padId1=ni[4];

    /* OK, it's one of ours,  is it assigned a pad id? */
    if (!notInitialized && padId0>'0' && padId0<'9' && padId1>'0' && padId1<'9' )
    {
        /* Yes, add it to the discovered pads */
        
        unsigned int padIndex0=padId0-'0'-1;
        unsigned int padIndex1=padId1-'0'-1;
        
        OLED_XYprintf(0, 3, "Discovered %c&%c", padId0, padId1);
        OLED_clearEOL();
        
        /* Is there another pad of that number registered? */
        if (pads[padId0].discovered || pads[padId1].discovered)
        {
            if (pads[padIndex0].padId != padId0 || pads[padIndex1].padId != padId1)
            {
                /* Yes, reassign it */
                addToNew(ni, netAddr, addr);
                return;
            }
        }
        
        /* Ok we add it */
        pads[padIndex0].addr=addr;
        pads[padIndex0].netAddr=netAddr;
        
        pads[padIndex1].addr=addr;
        pads[padIndex1].netAddr=netAddr;

        pads[padIndex0].discovered=true;
        pads[padIndex1].discovered=true;
        
        pads[padIndex0].discoveredAck=true;
        greenLeds|=_BV(padIndex0);
        pads[padIndex1].discoveredAck=true;
        greenLeds|=_BV(padIndex1);

        LED_output(redLeds, greenLeds, yellowLeds);
    }
    else
    {
        /* Unassigned pad */
        addToNew(ni, netAddr, addr);
    }
}

void padReady()
{
    OLED_clearLine(0);
    OLED_clearLine(1);
    OLED_clearLine(2);
    OLED_XYprintf(0, 0, "Ready");
    
    redLeds=0;
    yellowLeds=0;
    greenLeds=0;
    LED_output(redLeds, greenLeds, yellowLeds);    
}

void initNewPads()
{
    bool newPadFound=false;
    unsigned int greenLeds=0;        /* Local led status */
    unsigned int redLeds=0;
    unsigned int yellowLeds=0;
    
    /* Any new pads? */
    for(int i=0; i<8; ++i)
    {
        if(newPad[i].used)
        {
            newPadFound=true;
        }
    }
    
    /* If none, go home */
    if (!newPadFound)
    {
        return;
    }
    
    OLED_clearDisplay();

    /* Set the LEDs for the currently assigned pads */
    for(int i=0; i<8; ++i)
    {
        if (pads[i].discovered==true)
        {
            greenLeds|=_BV(i);
        }
    }
    LED_output(redLeds, greenLeds, yellowLeds);
    
    /* Go through the new pads and accept assignment */
    for(int i=0; i<8; ++i)
    {
        bool wait=false;
        if(newPad[i].used)    /* new pad? */
        {
            /* Send an ident signal to the pad */
            sendIdent(newPad[i].addr, newPad[i].netAddr); 
            
            OLED_XYprintf(0, 0, "Unassigned Device:");
            OLED_XYprintf(0, 1, "%02x%02x %02x%02x %02x%02x %02x%02x", 
            newPad[i].addr.addr64[0],
            newPad[i].addr.addr64[1],
            newPad[i].addr.addr64[2],
            newPad[i].addr.addr64[3],
            newPad[i].addr.addr64[4],
            newPad[i].addr.addr64[5],
            newPad[i].addr.addr64[6],
            newPad[i].addr.addr64[7]);
            OLED_XYprintf(0, 3, "Select Pad");
            
            /* Wait till a button is pushed */
            wait=true;
            while(wait)
            {
                /* Cycle through all the buttons */
                for(int sw=0; sw<8; ++sw)
                {
                    if(isClosed(sw))
                    {
                        int pair=sw/2*2;
                        pads[pair].addr=newPad[i].addr;
                        pads[pair].netAddr=newPad[i].netAddr;
                        pads[pair].discovered=true;
                        pads[pair+1].addr=newPad[i].addr;
                        pads[pair+1].netAddr=newPad[i].netAddr;
                        pads[pair+1].discovered=true;
                        sendAssign(pair);
                        greenLeds|=_BV(pair);
                        greenLeds|=_BV(pair+1);
                        LED_output(redLeds, greenLeds, yellowLeds);
                        newPad[i].used=0;
                        wait=false;
                        while(isClosed(sw)) {}
                        break;
                    }
                }
                /* If the launch button is pushed, ignore the pad */
                if(isClosed(8))
                {
                    sendUnident(newPad[i].addr, newPad[i].netAddr);
                    newPad[i].used=0;
                    wait=false;
                    while(isClosed(8)) {}
                }
            }
        }
    }
    
    OLED_clearLine(0);
    OLED_clearLine(1);
    OLED_clearLine(2);
    OLED_clearLine(3);

}


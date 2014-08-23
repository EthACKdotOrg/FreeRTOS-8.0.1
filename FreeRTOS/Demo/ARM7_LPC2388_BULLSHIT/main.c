/*
 * main.c
 *
 *  Created on: Aug 17, 2014
 *      Author: Aelia
 *
 * This file was created for Ethack and is released under GPLv3.
 * See the LICENSE file for more information about the GPLv3.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "flash.h"
#include "lpc23xx.h"
#include "mmc.h"

#define mainFLASH_PRIORITY                  ( tskIDLE_PRIORITY + 2 )
#define mainMMC_PRIORITY                    ( tskIDLE_PRIORITY + 4 )
#define mainUSB_PRIORITY                    ( tskIDLE_PRIORITY + 10 )

/* Constants to setup the PLL. */
#define mainPLL_MUL			( ( unsigned long ) ( 24 - 1 ) ) // Multiply PLL Clock by 24 to ahve 12*24 = 288
#define mainPLL_DIV			( ( unsigned long ) 0x0000 ) // do not divide the PLL input
#define mainUSB_CLK_DIV     ( ( unsigned long ) 0x0002 ) // Divide the PLL clock by 3 for USB to have 96
#define mainCPU_CLK_DIV		( ( unsigned long ) 0x000b ) // Divide Fcco by 12 to have 288 / 12 = 24
#define mainPLL_ENABLE		( ( unsigned long ) 0x0001 )
#define mainPLL_CONNECT		( ( ( unsigned long ) 0x0002 ) | mainPLL_ENABLE )
#define mainPLL_FEED_BYTE1	( ( unsigned long ) 0xaa )
#define mainPLL_FEED_BYTE2	( ( unsigned long ) 0x55 )
#define mainPLL_LOCK		( ( unsigned long ) 0x4000000 )
#define mainPLL_CONNECTED	( ( unsigned long ) 0x2000000 )
#define mainOSC_ENABLE		( ( unsigned long ) 0x20 )
#define mainOSC_STAT		( ( unsigned long ) 0x40 )
#define mainOSC_SELECT		( ( unsigned long ) 0x01 )

void prvSetupHardware(void) {
	/* Disable the PLL. */
	PLLCON = 0;
	PLLFEED = mainPLL_FEED_BYTE1;
	PLLFEED = mainPLL_FEED_BYTE2;

	/* Configure clock source. */
	SCS |= mainOSC_ENABLE;
	while( !( SCS & mainOSC_STAT ) );
	CLKSRCSEL = mainOSC_SELECT;

	/* Setup the PLL to multiply the XTAL input by 36/3. */
	PLLCFG = ( mainPLL_MUL | mainPLL_DIV );
	PLLFEED = mainPLL_FEED_BYTE1;
	PLLFEED = mainPLL_FEED_BYTE2;

	/* Turn on and wait for the PLL to lock... */
	PLLCON = mainPLL_ENABLE;
	PLLFEED = mainPLL_FEED_BYTE1;
	PLLFEED = mainPLL_FEED_BYTE2;
	CCLKCFG = mainCPU_CLK_DIV;
	while( !( PLLSTAT & mainPLL_LOCK ) );

	/* Connecting the clock. */
	PLLCON = mainPLL_CONNECT;
	PLLFEED = mainPLL_FEED_BYTE1;
	PLLFEED = mainPLL_FEED_BYTE2;
	while( !( PLLSTAT & mainPLL_CONNECTED ) );

	/*
	This code is commented out as the MAM does not work on the original revision
	LPC2368 chips.  If using Rev B chips then you can increase the speed though
	the use of the MAM.

	Setup and turn on the MAM.  Three cycle access is used due to the fast
	PLL used.  It is possible faster overall performance could be obtained by
	tuning the MAM and PLL settings.
	MAMCR = 0;
	MAMTIM = mainMAM_TIM_3;
	MAMCR = mainMAM_MODE_FULL;
	*/
	USBCLKCFG = 2;
	vParTestInitialise();
}

int
main(void) {
	dma_init();
	mmc_init(mainMMC_PRIORITY);
	usb_init(mainUSB_PRIORITY);
	vStartLEDFlashTasks(mainFLASH_PRIORITY);
    vStartQueuePeekTasks();
    vStartDynamicPriorityTasks();
	/* Start the scheduler. */
	vTaskStartScheduler();
}

void vApplicationTickHook(void) {
	/* TODO put the sanity check here */
}

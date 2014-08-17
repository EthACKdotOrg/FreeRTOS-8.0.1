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

#define mainFLASH_PRIORITY                  ( tskIDLE_PRIORITY + 2 )

static void prvSetupHardware(void) {
	/* Disable the PLL. */
//	PLLCON = 0;
//	PLLFEED = mainPLL_FEED_BYTE1;
//	PLLFEED = mainPLL_FEED_BYTE2;

	/* Configure clock source. */
//	SCS |= mainOSC_ENABLE;
//	while( !( SCS & mainOSC_STAT ) );
//	CLKSRCSEL = mainOSC_SELECT;

	/* Setup the PLL to multiply the XTAL input by 4. */
//	PLLCFG = ( mainPLL_MUL | mainPLL_DIV );
//	PLLFEED = mainPLL_FEED_BYTE1;
//	PLLFEED = mainPLL_FEED_BYTE2;

	/* Turn on and wait for the PLL to lock... */
//	PLLCON = mainPLL_ENABLE;
//	PLLFEED = mainPLL_FEED_BYTE1;
//	PLLFEED = mainPLL_FEED_BYTE2;
//	CCLKCFG = mainCPU_CLK_DIV;
//	while( !( PLLSTAT & mainPLL_LOCK ) );

	/* Connecting the clock. */
//	PLLCON = mainPLL_CONNECT;
//	PLLFEED = mainPLL_FEED_BYTE1;
//	PLLFEED = mainPLL_FEED_BYTE2;
//	while( !( PLLSTAT & mainPLL_CONNECTED ) );

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

	/* Setup the led's on the MCB2300 board */
	vParTestInitialise();
}

int main(void)  __attribute__((noreturn));

int
main(void) {
	prvSetupHardware();
	/* TODO put the clock initialization here */
	/* TODO put the USB initialization here */
	/* TODO put the MMC initialization here */
    vStartQueuePeekTasks();
    vStartDynamicPriorityTasks();
	/* Start the scheduler. */
	vTaskStartScheduler();
	while(1);
}

void vApplicationTickHook(void) {
	/* TODO put the sanity check here */
}

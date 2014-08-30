/*
 * uart_driver.c
 *
 *  Created on: Aug 30, 2014
 *      Author: Aelia
 *
 * This file was created for Ethack and is released under GPLv3.
 * See the LICENSE file for more information about the GPLv3.
 */

#include "uart.h"
#include "lpc23xx.h"

void
uart1_init(enum uart_speed speed) {
	PINSEL0 &= ~UART1_PINSEL0_MASK;
	PINSEL0 |= UART1_PINSEL0_TXD;
	PINSEL1 &= ~UART1_PINSEL1_MASK;
	PINSEL1 |= UART1_PINSEL1_RXD;
	U1LCR &= ~UART_LCR_WORDLEN_MASK;
	U1LCR |= UART_LCR_WORDLEN8;
	U1LCR &= ~UART_LCR_STOPBIT_MASK;
	U1LCR |= UART_LCR_STOPBIT1;
	U1LCR &= ~UART_LCR_PARITY_MASK;
	U1LCR |= UART_LCR_PARITY_NONE;
	U1LCR &= UART_LCR_DLAB_MASK;
	U1LCR |= UART_LCR_DLAB_PERMIT;
	switch (speed) {
	case UART_SPEED_INVALID:
		return;
	case UART_SPEED_115200:
		U1DLL = UART_DLL_115200;
		U1DLM = UART_DLM_115200;
		U1FDR = UART_FDR_DIVIDE_115200 | (UART_FDR_MULTIPLY_11500<<4);
		break;
	}
	U1LCR &= UART_LCR_DLAB_MASK;
	U1LCR |= UART_LCR_DLAB_DISALLOW;
}

void
uart1_putchar(uint8_t chr) {
	U1THR = chr;
	while (!U1LSR&UART_LSR_TEMT);
}

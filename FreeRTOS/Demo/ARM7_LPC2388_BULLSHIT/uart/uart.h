/*
 * uart.h
 *
 *  Created on: Aug 30, 2014
 *      Author: Aelia
 *
 * This file was created for Ethack and is released under GPLv3.
 * See the LICENSE file for more information about the GPLv3.
 */

#ifndef UART_H_
#define UART_H_
#include "stdint.h"

#define UART1_PINSEL0_TXD      (1<<31)
#define UART1_PINSEL1_RXD      (1<<0)
#define UART1_PINSEL0_MASK     (1<<30|1<<31)
#define UART1_PINSEL1_MASK     (1<<0|1<<1)
#define UART_LCR_WORDLEN_MASK  (1<<0|1<<1)
#define UART_LCR_WORDLEN5      (0        )
#define UART_LCR_WORDLEN6      (1<<0     )
#define UART_LCR_WORDLEN7      (     1<<1)
#define UART_LCR_WORDLEN8      (1<<0|1<<1)
#define UART_LCR_STOPBIT_MASK  (1<<2)
#define UART_LCR_STOPBIT1      (0)
#define UART_LCR_STOPBIT2      (1<<2)
#define UART_LCR_PARITY_MASK   (1<<3|1<<4|1<<5)
#define UART_LCR_PARITY_NONE   (0        )
#define UART_LCR_PARTIY_EVEN   (1<<3|1<<4)
#define UART_LCR_PARTIY_ODD    (1<<3     )
#define UART_LCR_DLAB_MASK     (1<<7)
#define UART_LCR_DLAB_PERMIT   (1<<7)
#define UART_LCR_DLAB_DISALLOW (0)
#define UART_LSR_TEMT          (1<<6)

enum uart_speed {
	UART_SPEED_115200,
	UART_SPEED_INVALID,
};

/*!
 * These values are computed for a CCLK of 24MHz.
 * The UART speed is computed with:
 *                                  CCLK
 *  UART_SPEED = -----------------------------------------------
 *               16 * (256 * DLM + DLL) * (1 + FDR_DIV/ FDR_MUL)
 * \{
 */
#define UART_FDR_MULTIPLY_11500 8
#define UART_FDR_DIVIDE_115200  5
#define UART_DLL_115200         8
#define UART_DLM_115200         0
/*! \} */

/*!
 *
 */
void uart1_init(enum uart_speed);
/*!
 *
 */
void uart1_putchar(uint8_t chr);

#endif /* UART_H_ */

/*
 * dma.h
 *
 *  Created on: Aug 23, 2014
 *      Author: Aelia
 *
 * This file was created for Ethack and is released under GPLv3.
 * See the LICENSE file for more information about the GPLv3.
 */

#ifndef DMA_H_
#define DMA_H_
#include "stdbool.h"
#include "stdint.h"

enum dma_mode {
	DMA_READ,
	DMA_WRITE,
};

enum dma_channel {
	DMA_CHANNEL0,
	DMA_CHANNEL1,
	DMA_CHANNEL_BOUND,
};

enum dma_state {
	DMA_STATE_SUCCESS,
	DMA_STATE_TIMEOUT,
	DMA_STATE_ERROR,
};

typedef void (*dma_callback)(enum dma_state);

void dma_init(void);
bool dma_start(enum dma_channel channel, enum dma_mode, uint32_t *buffer);
enum dma_channel dma_reserve_channel(dma_callback callback);

#endif /* DMA_H_ */

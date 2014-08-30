/*
 * dma_driver.c
 *
 *  Created on: Aug 23, 2014
 *      Author: Aelia
 *
 * This file was created for Ethack and is released under GPLv3.
 * See the LICENSE file for more information about the GPLv3.
 */
#include "stddef.h"
#include "dma.h"

static struct dma_registrations {
	dma_callback callback;
} registrations[DMA_CHANNEL_BOUND] = {0};

void
dma_init(void) {
	// TODO
}

bool
dma_start(enum dma_channel channel, enum dma_mode mode, uint32_t *buffer) {
	return false;
}

enum dma_channel
dma_reserve_channel(dma_callback callback) {
	int i;
	for (i = 0; i < DMA_CHANNEL_BOUND; i++) {
		if (registrations[i].callback == NULL) {
			registrations[i].callback = callback;
			return i;
		}
	}
	return DMA_CHANNEL_BOUND;
}

// TODO ISR that redirects to the registration callback

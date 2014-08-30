/*
 * mmc_driver.c
 *
 *  Created on: Aug 23, 2014
 *      Author: Aelia
 *
 * This file was created for Ethack and is released under GPLv3.
 * See the LICENSE file for more information about the GPLv3.
 */

#include "mmc.h"
#include "lpc23xx.h"
#include "stddef.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "dma.h"

#define MMC_TO_WRITE_QUEUE_LENGTH 5
#define MMC_TO_READ_QUEUE_LENGTH  5
#define MMC_STACK_SIZE 1024

static TaskHandle_t mmcTask;
static QueueHandle_t mmcToWriteQueue;
static QueueHandle_t mmcToReadQueue;
static QueueSetHandle_t mmcReadWriteSet;
static uint32_t cardRCA = 0;
static enum dma_channel dma_channel;
static bool error = false;

/*!
 * Try to set the speed of the bus to the specified speed.
 */
static void
mmc_set_speed(enum mmc_speed speed) {
	   /* Set a MCI clock speed to desired value. */
	   int i,clkdiv;
	   MCI_CLOCK = (MCI_CLOCK &
			   ~MCI_CLOCK_CONTROL_CLKDIV_MASK) |
			   MCI_CLOCK_CONTROL_ENABLE |
			   MCI_CLOCK_CONTROL_PWRSAVE |
			   speed;
	   /* To be sure that everything is perfectly stable, in theory it should be 3MCLK + 2PCLK */
	   vTaskDelay(10 / portTICK_PERIOD_MS);
}

/*!
 * Send a command and check the response (if any).
 * \return true on success, false on error
 */
static bool
mmc_send_command(enum mmc_command cmd, uint32_t arg, enum mmc_response_type resp_type, uint32_t *rp) {
	uint32_t cmd_data = cmd;
	uint32_t status = 0;

	cmd_data |= MMC_COMMAND_TRANSMITTER_DIRECTION;
	cmd_data |= resp_type;
   /* Send the command. */
	MCI_ARGUMENT = arg;
	MCI_COMMAND  = cmd_data;
	if (resp_type == MMC_RESPONSE_NONE) {
		while (MCI_STATUS & MCI_COMMAND_ACTIVE);
		MCI_CLEAR = MCI_CLEAR_MASK;
		return true;
	}
	while (true) {
		status = MCI_STATUS;
		if (status & MCI_STATUS_CMD_TIMEOUT) {
			MCI_CLEAR = status & MCI_CLEAR_MASK;
			return false;
		}
		if (status & MCI_STATUS_CMD_CRC_FAIL) {
			MCI_CLEAR = status & MCI_CLEAR_MASK;
			if ((
						cmd == MMC_COMMAND_SEND_OP_COND ||
						cmd == MMC_COMMAND_SEND_APP_OP_COND||
						cmd == MMC_COMMAND_STOP_TRANSMISSION
					)) {
				MCI_COMMAND = MMC_COMMAND_GO_IDLE_STATE;
				break;
			}
			return false;
		}
		if (status & MCI_STATUS_CMD_RESP_END) {
			MCI_CLEAR = status & MCI_CLEAR_MASK;
			break;
		}
	}
	if ((MCI_RESP_CMD & MCI_COMMAND_MASK) != cmd) {
		if (
					(cmd != MMC_COMMAND_SEND_OP_COND)     &&
					(cmd != MMC_COMMAND_SEND_APP_OP_COND) &&
					(cmd != MMC_COMMAND_ALL_SEND_CID)     &&
					(cmd != MMC_COMMAND_SEND_CSD)
				) {
			return false;
		}
	}
	if (rp == NULL) {
		/* Response pointer undefined. */
		return false;
	}
	/* Read MCI response registers */
	rp[0] = MCI_RESP0;
	if (resp_type == MMC_RESPONSE_LONG) {
		rp[1] = MCI_RESP1;
		rp[2] = MCI_RESP2;
		rp[3] = MCI_RESP3;
	}
	return true;
}

static bool
mmc_detect_sd_card(void) {
	int i;
	uint32_t response[1];
	MCI_POWER &= ~MCI_POWER_CONTROL_OPENDRAIN;
	for (i = 0; i < 500; i++) {
		if (mmc_send_command(MMC_COMMAND_APP_CMD,
				0,
				MMC_RESPONSE_SHORT,
				response) && (response[0] & SD_STATUS_ACMD_ENABLE)) {
			if (mmc_send_command(MMC_COMMAND_SEND_APP_OP_COND,
						SD_OCR_POWER_32DV_TO_33DV|SD_OCR_POWER_33DV_TO_34DV,
						MMC_RESPONSE_SHORT,
						response) &&
					(response[0] & SD_OCR_POWER_UP_STATUS)) {
				return true;
			}
		}
	}
	return false;
}

static bool
mmc_detect_mmc_card(void) {
	uint32_t response[1];
	int i;
	MCI_POWER |= MCI_POWER_CONTROL_OPENDRAIN;
	/* First try really hard to detect an MMC, send CMD1. */
	for (i = 0; i < 100; i++) {
		if (mmc_send_command(MMC_COMMAND_SEND_OP_COND, MCI_ARGUMENT_OCR_INDEX, MMC_RESPONSE_SHORT, response)) {
			return true;
		}
	}
	return false;
}

static void
mmc_read_cid(void) {
	int i;
	uint32_t response[4];
	for (i = 0; i < 20; i++) {
		if (mmc_send_command(MMC_COMMAND_ALL_SEND_CID, 0, MMC_RESPONSE_LONG, response)) {
			// TODO write identifiers to debug
		}
	}
}

static bool
mmc_set_address(bool has_mmc) {
	/* Set Relative Address, send CMD3 after CMD2. */
	int i;
	uint32_t arg = has_mmc?0x00010000:0;
	uint32_t response[1];
	for (i = 0; i < 20; i++) {
		if (mmc_send_command(MMC_COMMAND_SEND_RELATIVE_ADDR, arg, MMC_RESPONSE_SHORT, response)) {
			if ((response[0] & SD_COMMAND_STATE_MASK) == SD_COMMAND_STATUS_IDENT|SD_COMMAND_STATUS_READY_FOR_DATA) {
				cardRCA = response[0] >> 16; // save the new RCA to address the card in the future
			}
			if (has_mmc) {
				cardRCA = arg>>16;
			}
			return true;
		}
	}
	return false;
}

static bool
mmc_select_card(void) {
	/* Select the Card, send CMD7 after CMD9, inter-change state */
	/* between STBY and TRANS after this command. */
	int i;
	uint32_t arg = cardRCA << 16;
	uint32_t response[1];
	for (i = 0; i < 200; i++) {
		if (mmc_send_command(MMC_COMMAND_SELECT_CARD, arg, MMC_RESPONSE_SHORT, response)) {
			if ((response[0] & MMC_COMMAND_SEND_RELATIVE_ADDR) == SD_COMMAND_STATUS_STBY|SD_COMMAND_STATUS_READY_FOR_DATA) {
				return true;
			}
		}
	}
	return false;
}

/*!
 * Try to detect the MMC/SD card and set the maximum working speed
 * \return true on success, false on error
 */
static bool
mmc_detect_card() {
	int i;
	uint32_t response[1];

	bool has_mmc = false;
	mmc_set_speed(MMC_SPEED_400KHZ);
	/* the MMC bus should still be initializing from the hardware init */
	vTaskDelay(100 / portTICK_PERIOD_MS);
	/* Power on the Flash Card. */
	MCI_POWER |= MCI_POWER_CONTROL_CTRL_POWER_ON;
	/* wait until the card is powered on */
	vTaskDelay(100 / portTICK_PERIOD_MS);

	/* Reset the card, send CMD0. */
	mmc_send_command(MMC_COMMAND_GO_IDLE_STATE, 0, MMC_RESPONSE_NONE, NULL);
	if (mmc_detect_mmc_card()) {
		has_mmc = true;
	}
	if (has_mmc || mmc_detect_sd_card()) {
		mmc_read_cid();
		if (!mmc_set_address(has_mmc)) {
			return false;
		}
		if (has_mmc) {
			MCI_POWER &= ~MCI_POWER_CONTROL_OPENDRAIN;
		}
		mmc_set_speed(MMC_SPEED_24MHZ); // try max speed immediately ?
		if (!mmc_select_card()) {
			return false;
		}
		if (!has_mmc) {
			MCI_CLOCK |= MCI_CLOCK_CONTROL_WIDEBUS;
			vTaskDelay(100 / portTICK_PERIOD_MS);
			if (!mmc_send_command(MMC_COMMAND_SET_ACMD_BUS_WIDTH, SD_COMMAND_ARG_BUS_WIDTH_4BITS, MMC_RESPONSE_SHORT, response) ||
					((response[0] & SD_COMMAND_STATE_MASK) != SD_COMMAND_STATUS_TRAN|SD_COMMAND_STATUS_READY_FOR_DATA)) {
				MCI_CLOCK &= ~MCI_CLOCK_CONTROL_WIDEBUS;
				vTaskDelay(100 / portTICK_PERIOD_MS);
				if (!mmc_send_command(MMC_COMMAND_SET_ACMD_BUS_WIDTH, SD_COMMAND_ARG_BUS_WIDTH_1BIT, MMC_RESPONSE_SHORT, response)) {
					return false;
				}
			}
		}
		for (i = 0; i < 20; i++) {
			/* Send ACMD6 command to set the bus width. */
			if (!mmc_send_command(MMC_COMMAND_SET_BLOCK_LEN, MMC_BLOCK_SIZE, MMC_RESPONSE_SHORT, response)) {
				continue;
			}
			if ((response[0] & SD_COMMAND_STATE_MASK) == SD_COMMAND_STATUS_TRAN|SD_COMMAND_STATUS_READY_FOR_DATA) {
				return true;
			}
		}
	}
	return false;
}

static bool
mmc_wait_for_tran_state(void) {
	// TODO
	return false;
}

static bool
mmc_cmd_read_block(uint32_t block, uint32_t count) {
	// TODO
	return false;
}

static bool
mmc_cmd_write_block(uint32_t block, uint32_t count) {
	// TODO
	return false;
}

static bool
mmc_write_sector(uint32_t sector, uint32_t *buffer, uint32_t cnt) {
	int i;
	uint32_t response[1];
	if (!mmc_wait_for_tran_state()) {
		/* Card not in TRAN state. */
		return false;
	}
	/* Write a 512 byte sector to Flash Card. */
	if (!mmc_wait_for_tran_state()) {
		/* Card not in TRAN state. */
		return false;
	}
	if (!mmc_cmd_write_block(sector, cnt)) {
		/* Command Failed. */
		return false;
	}
	for (i = 0; i < cnt; i++) {
		MCI_DATA_TMR = MCI_DATA_RW_TOUT_VALUE;
		MCI_DATA_LEN = MMC_BLOCK_SIZE;
		dma_start(DMA_WRITE, dma_channel, buffer + i * MMC_BLOCK_SIZE);
		MCI_DATA_CTRL = (MMC_BLOCK_SIZE_POWER << 4) | MCI_CONTROL_DMA_ENABLE | MCI_CONTROL_ENABLE;
		vTaskSuspend(mmcTask);
		while (MCI_STATUS != (MCI_STATUS_DATA_END | MCI_STATUS_DATA_BLK_END)) {
			if (MCI_STATUS & (MCI_STATUS_DATA_CRC_FAIL | MCI_STATUS_DATA_TIMEOUT)) {
				if (!mmc_send_command(MMC_COMMAND_STOP_TRANSMISSION, 0, MMC_RESPONSE_SHORT, response) ||
						((response[0] & SD_COMMAND_STATE_MASK) != SD_COMMAND_STATUS_IDLE|SD_COMMAND_STATUS_READY_FOR_DATA)) {
					return false;
				}
				/* If error while Data Block sending occured. */
				if (!mmc_send_command(MMC_COMMAND_STOP_TRANSMISSION, 0, MMC_RESPONSE_SHORT, response) ||
						((response[0] & SD_COMMAND_STATE_MASK) != SD_COMMAND_STATUS_IDLE|SD_COMMAND_STATUS_READY_FOR_DATA)) {
					return false;
				}
				/* Write request Failed. */
				return false;
			}
		}
	}
	if (!mmc_send_command(MMC_COMMAND_STOP_TRANSMISSION, 0, MMC_RESPONSE_SHORT, response) ||
			((response[0] & SD_COMMAND_STATE_MASK) != SD_COMMAND_STATUS_IDLE|SD_COMMAND_STATUS_READY_FOR_DATA)) {
		return false;
	}
	return true;
}

static bool
mmc_read_sector(uint32_t sector, uint32_t *buffer, uint32_t cnt) {
   /* Read one or more 512 byte sectors from Flash Card. */
   int i;
   if (!mmc_wait_for_tran_state()) {
      /* Card not in TRAN state. */
      return false;
   }
   if (!mmc_cmd_read_block(sector, cnt)) {
      /* Command Failed. */
      return false;
   }
   /* Set MCI Transfer registers. */
   MCI_DATA_TMR  = MCI_DATA_RD_TOUT_VALUE;
   MCI_DATA_LEN  = cnt * MMC_BLOCK_SIZE;

   /* Start DMA Peripheral to Memory transfer. */
   dma_start(dma_channel, DMA_READ, buffer);
   MCI_DATA_CTRL = (MMC_BLOCK_SIZE_POWER << 4) | MCI_CONTROL_DMA_ENABLE | MCI_CONTROL_DIRECTION_READ | MCI_CONTROL_ENABLE;
   dma_start(dma_channel, DMA_READ, buffer);
   return true;
}

static
portTASK_FUNCTION(mmc_driver_task, pvParameters) {
	union {
		struct mmc_datawrite_request write;
		struct mmc_dataread_request read;
	} data;
	QueueSetMemberHandle_t requested_queue;
	uint32_t response;
	static bool initialized = false;
	if (error) {
		initialized = false;
		// TODO debug write message about init retry due to error
	}
	if (!initialized) {
		if (!mmc_detect_card()) {
			error = true;
		} else {
			initialized = true;
		}
	}
	while (!error) {
		requested_queue = xQueueSelectFromSet(mmcReadWriteSet, portMAX_DELAY);
		if (requested_queue == mmcToWriteQueue) {
			/* wait indefinitely for a message */
			xQueueReceive(mmcToWriteQueue, &data.write, portMAX_DELAY);
			if (!mmc_send_command(MMC_COMMAND_WRITE_BLOCK,
					data.write.block * MMC_BLOCK_SIZE,
					MMC_RESPONSE_SHORT,
					&response)) {
				error = true;
				break;
			}
			if (!mmc_write_sector(data.write.block, data.write.data, 1)) {
				error = true;
			} else {
				vTaskSuspend(mmcTask);
			}
			data.write.callback(error);
			/* the write is finished now, switching to the next command */
		}
		if (requested_queue == mmcToReadQueue) {
			xQueueReceive(mmcToReadQueue, &data.read, portMAX_DELAY);
			if (!mmc_read_sector(data.read.block, data.read.data, 1)) {
				error = true;
			} else {
				vTaskSuspend(mmcTask);
			}
			data.read.callback(error, data.read.data);
			/*the read is finished now, switching to the next command */
		}
	}
	/* on error exit the mmcTask */
	vTaskDelete(mmcTask);
}

static void
mmc_dma_callback(enum dma_state state) {
	switch (state) {
	case DMA_STATE_ERROR:
	case DMA_STATE_TIMEOUT:
		error = true;
		/* no break */
	case DMA_STATE_SUCCESS:
		vTaskResume(mmcTask);
		break;
	}
}

void
mmc_init(int taskpriority) {
	PCONP |= MCI_PCONP_ENABLE;
	/* be sure that the MMC/SD module power control is enabled */
	SCS |= MCI_SCS_POWER;
	PINSEL1 &= MCI_PINSEL1_CLEAR;
	PINSEL1 |= MCI_PINSEL1_SET;
	PINSEL4 &= MCI_PINSEL4_CLEAR;
	PINSEL4 |= MCI_PINSEL4_SET;
	/* Clear all pending interrupts. */
	MCI_COMMAND   = 0;
	MCI_DATA_CTRL = 0;
	MCI_CLEAR     = 0x7FF;
	/* Power up, switch on VCC for the Flash Card. */
	MCI_POWER  = MCI_POWER_CONTROL_CTRL_POWER_UP;
	xTaskCreate(mmc_driver_task, "MMC", MMC_STACK_SIZE, NULL, taskpriority, &mmcTask);
	mmcToWriteQueue = xQueueCreate(4, sizeof(struct mmc_datawrite_request));
	mmcToReadQueue  = xQueueCreate(4, sizeof(struct mmc_datawrite_request));
	configASSERT(mmcTask);
	configASSERT(mmcToReadQueue);
	configASSERT(mmcToWriteQueue);
	dma_channel = dma_reserve_channel(mmc_dma_callback);
	mmcReadWriteSet = xQueueCreateSet(MMC_TO_WRITE_QUEUE_LENGTH + MMC_TO_READ_QUEUE_LENGTH);
	configASSERT(mmcReadWriteSet);
}

bool
mmc_post_read_request(struct mmc_dataread_request *req) {
	if (xQueueSendToBack(mmcToReadQueue, req, portMAX_DELAY) != pdTRUE) {
		return false;
	}
	return true;
}

bool
mmc_post_write_request(struct mmc_datawrite_request *req) {
	if (xQueueSendToBack(mmcToWriteQueue, req, portMAX_DELAY) != pdTRUE) {
		return false;
	}
	return true;
}

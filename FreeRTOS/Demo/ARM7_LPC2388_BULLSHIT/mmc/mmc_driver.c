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
#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

/*!
 * Send a command and check the response (if any).
 * \return true on success, false on error
 */
static bool
mmc_send_command(enum mmc_command cmd, uint32_t arg, enum mmc_response_type resp_type, uint32_t *rp) {
	uint32_t cmd_data = cmd;
	uint32_t status = 0;

	cmd_data |= MMC_COMMAND_FLAG;
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

/*!
 * Try to detect the MMC/SD card and set the maximum working speed
 * \return true on success, false on error
 */
static bool
mmc_detect_card() {
	int i;
	/* the MMC bus should still be initializing from the hardware init */
	vTaskDelay(portTICK_PERIOD_MS * 100);
	/* Power on the Flash Card. */
	MCI_POWER |= 0x01;
	/* wait until the card is powered on */
	vTaskDelay(portTICK_PERIOD_MS * 100);

	/* Reset the card, send CMD0. */
	mmc_send_command(MMC_COMMAND_GO_IDLE_STATE, 0, MMC_RESPONSE_NONE, NULL);
	// TODO detect MMC or SD card plus its speed
	return false;
}

/*!
 * Try to set the speed of the bus to the specified speed.
 */
static void
mmc_set_speed(enum mmc_speed high_speed) {
}

static
portTASK_FUNCTION(mmc_driver_task, pvParameters) {
	static bool initialized = false, error = false;
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
		// TODO read from incoming queue and write to MMC via DMA
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
	MCI_POWER  = 0x02;
	mmc_set_speed(MMC_SPEED_400KHZ);
	xTaskCreate(mmc_driver_task, "MMC", configMINIMAL_STACK_SIZE, NULL, taskpriority, NULL);
}


/*
 * mmc.h
 *
 *  Created on: Aug 23, 2014
 *      Author: Aelia
 *
 * This file was created for Ethack and is released under GPLv3.
 * See the LICENSE file for more information about the GPLv3.
 */

/*
 * For BULLSHIT we use the LPC2388 here is the description
 * of the pins for MMC/SD interface on that chip:
 * 	MCICLK  out 85 (P0[19])
 * 	MCICMD  in  83 (P0[20])
 * 	MCIPWR  out 82 (P0[21])
 * 	MCIDAT0 out 80 (P0[22])
 * 	MCIDAT1 out 75 (P2[11])
 * 	MCIDAT2 out 73 (P2[12])
 * 	MCIDAT3 out 71 (P2[13])
 */

#define MMC_BLOCK_SIZE_POWER 9
#define MMC_BLOCK_SIZE (2<<MMC_BLOCK_SIZE_POWER)
#if MMC_BLOCK_SIZE_POWER > 11
#error "MMC_BLOCK_SIZE_POWER cannot be greater than 11 (MMC_BLOCK_SIZE > 2048bit)"
#endif
#include "stdbool.h"
#include "stdint.h"
#include "FreeRTOS.h"
#include "queue.h"

#ifndef MMC_H_
#define MMC_H_

/*!
 * These values are not correct!
 * These values are dependent on the CPU speed and are computed for a CCLK of 24MHz.
 */
enum mmc_speed {
	MMC_SPEED_400KHZ = 240,
	MMC_SPEED_20MHZ  = 1,
	MMC_SPEED_24MHZ  = 0,
};

enum mci_status {
	MCI_STATUS_CMD_CRC_FAIL    = 0x00000001,
	MCI_STATUS_DATA_CRC_FAIL   = 0x00000002,
	MCI_STATUS_CMD_TIMEOUT     = 0x00000004,
	MCI_STATUS_DATA_TIMEOUT    = 0x00000008,
	MCI_STATUS_TX_UNDERRUN     = 0x00000010,
	MCI_STATUS_RX_OVERRUN      = 0x00000020,
	MCI_STATUS_CMD_RESP_END    = 0x00000040,
	MCI_STATUS_CMD_SENT        = 0x00000080,
	MCI_STATUS_DATA_END        = 0x00000100,
	MCI_STATUS_START_BIT_ERR   = 0x00000200,
	MCI_STATUS_DATA_BLK_END    = 0x00000400,
	MCI_STATUS_CMD_ACTIVE      = 0x00000800,
	MCI_STATUS_TX_ACTIVE       = 0x00001000,
	MCI_STATUS_RX_ACTIVE       = 0x00002000,
	MCI_STATUS_TX_HALF_EMPTY   = 0x00004000,
	MCI_STATUS_RX_HALF_FULL    = 0x00008000,
	MCI_STATUS_TX_FIFO_FULL    = 0x00010000,
	MCI_STATUS_RX_FIFO_FULL    = 0x00020000,
	MCI_STATUS_TX_FIFO_EMPTY   = 0x00040000,
	MCI_STATUS_RX_FIFO_EMPTY   = 0x00080000,
	MCI_STATUS_TX_DATA_AVAIL   = 0x00100000,
	MCI_STATUS_RX_DATA_AVAIL   = 0x00200000,
};
typedef void (*mmc_write_callback)(bool has_error);
typedef void (*mmc_read_callback)(bool has_error, uint32_t *data);

struct mmc_datawrite_request {
	uint32_t block;
	uint32_t data[MMC_BLOCK_SIZE / sizeof(uint32_t)];
	mmc_write_callback callback;
};

struct mmc_dataread_request {
	uint32_t block;
	uint32_t data[MMC_BLOCK_SIZE / sizeof(uint32_t)];
	mmc_read_callback callback;
};

enum mmc_command {
	MMC_COMMAND_GO_IDLE_STATE       = 0 ,      /* Resetthe card to idle state  (MMC,SD) */
	MMC_COMMAND_SEND_OP_COND        = 1 ,      /* Send Op.Cond. Register       (MMC)    */
	MMC_COMMAND_ALL_SEND_CID        = 2 ,      /* Send Card CID number         (MMC,SD) */
	MMC_COMMAND_SEND_RELATIVE_ADDR   = 3 ,      /* Set Relative Address         (MMC,SD) */
	MMC_COMMAND_SET_ACMD_BUS_WIDTH  = 6 ,
	MMC_COMMAND_SELECT_CARD         = 7 ,      /* Select/Deselect the Card     (MMC,SD) */
	MMC_COMMAND_SEND_CSD            = 9 ,      /* Send Card Specific Data      (MMC,SD) */
	MMC_COMMAND_SEND_CID            = 10,      /* Send Card Identification Data(MMC,SD) */
	MMC_COMMAND_STOP_TRANSMISSION   = 12,      /* Stop Read or Write transm.   (MMC,SD) */
	MMC_COMMAND_SEND_STATUS         = 13,      /* Send Status Resiger          (MMC,SD) */
	MMC_COMMAND_SET_BLOCK_LEN       = 16,      /* Set Block Length in bytes    (MMC,SD) */
	MMC_COMMAND_READ_BLOCK          = 17,      /* Read a Single Block          (MMC,SD) */
	MMC_COMMAND_READ_MULT_BLOCK     = 18,      /* Read Multiple Blocks         (MMC,SD) */
	MMC_COMMAND_WRITE_BLOCK         = 24,      /* Write a Block                (MMC,SD) */
	MMC_COMMAND_WRITE_MULT_BLOCK    = 25,      /* Write Multiple Blocks        (MMC,SD) */
	MMC_COMMAND_SEND_APP_OP_COND    = 41,      /* Send App. Op.Cond Register       (SD) */
	MMC_COMMAND_APP_CMD             = 55,      /* App.Specific Command follow  (MMC,SD) */
};

enum mmc_response_type {
	MMC_RESPONSE_NONE  =    0,
	MMC_RESPONSE_SHORT = 0x40,
	MMC_RESPONSE_LONG  = 0xC0,
};

#define MMC_COMMAND_TRANSMITTER_DIRECTION   (0x400)
#define MCI_DATA_RD_TOUT_VALUE  (10*(24000000/1000))  /* ~10ms  at 24MHz SD clock */
#define MCI_DATA_RW_TOUT_VALUE  (200*(24000000/1000)) /* ~200ms at 24MHz SD clock */

#define SD_STATUS_ACMD_ENABLE            0x00000020
#define SD_COMMAND_STATE_MASK            (SD_COMMAND_STATUS_READY_FOR_DATA|SD_COMMAND_STATUS_PRG|SD_COMMAND_STATUS_DIS) // 7 is 1 + 2 + 4, with 8 they span all possible values of the 4 bits of state
#define SD_COMMAND_STATUS_READY_FOR_DATA (1<<8)
#define SD_COMMAND_STATUS_IDLE           (0<<9)
#define SD_COMMAND_STATUS_READY          (1<<9)
#define SD_COMMAND_STATUS_IDENT          (2<<9)
#define SD_COMMAND_STATUS_STBY           (3<<9)
#define SD_COMMAND_STATUS_TRAN           (4<<9)
#define SD_COMMAND_STATUS_DATA           (5<<9)
#define SD_COMMAND_STATUS_RCV            (6<<9)
#define SD_COMMAND_STATUS_PRG            (7<<9)
#define SD_COMMAND_STATUS_DIS            (8<<9)

#define SD_COMMAND_ARG_BUS_WIDTH_1BIT       0
#define SD_COMMAND_ARG_BUS_WIDTH_4BITS     10

#define SD_OCR_POWER_16DV_TO_17DV  (1<< 4)
#define SD_OCR_POWER_17DV_TO_18DV  (1<< 5)
#define SD_OCR_POWER_18DV_TO_19DV  (1<< 6)
#define SD_OCR_POWER_19DV_TO_20DV  (1<< 7)
#define SD_OCR_POWER_20DV_TO_21DV  (1<< 8)
#define SD_OCR_POWER_21DV_TO_22DV  (1<< 9)
#define SD_OCR_POWER_22DV_TO_23DV  (1<<10)
#define SD_OCR_POWER_23DV_TO_24DV  (1<<11)
#define SD_OCR_POWER_24DV_TO_25DV  (1<<12)
#define SD_OCR_POWER_25DV_TO_26DV  (1<<13)
#define SD_OCR_POWER_26DV_TO_27DV  (1<<14)
#define SD_OCR_POWER_27DV_TO_28DV  (1<<15)
#define SD_OCR_POWER_28DV_TO_29DV  (1<<16)
#define SD_OCR_POWER_29DV_TO_30DV  (1<<17)
#define SD_OCR_POWER_30DV_TO_31DV  (1<<18)
#define SD_OCR_POWER_31DV_TO_32DV  (1<<19)
#define SD_OCR_POWER_32DV_TO_33DV  (1<<20)
#define SD_OCR_POWER_33DV_TO_34DV  (1<<21)
#define SD_OCR_POWER_34DV_TO_35DV  (1<<22)
#define SD_OCR_POWER_35DV_TO_36DV  (1<<23)
#define SD_OCR_POWER_UP_STATUS     (1<<31)

#define MCI_INTERRUPT_LINE  (2<<24)
#define MCI_PINSEL1_CLEAR   (~(1<< 6 | 1<< 8 | 1<<10 | 1<<12))
#define MCI_PINSEL4_CLEAR   (~(1<<22 | 1<<24 | 1<<26))
#define MCI_PINSEL1_P0_19   (1<< 7)
#define MCI_PINSEL1_P0_20   (1<< 9)
#define MCI_PINSEL1_P0_21   (1<<11)
#define MCI_PINSEL1_P0_22   (1<<13)
#define MCI_PINSEL1_SET     (MCI_PINSEL1_P0_19|MCI_PINSEL1_P0_20|MCI_PINSEL1_P0_21|MCI_PINSEL1_P0_22)
#define MCI_PINSEL4_P2_11   (1<<23)
#define MCI_PINSEL4_P2_12   (1<<25)
#define MCI_PINSEL4_P2_13   (1<<27)
#define MCI_PINSEL4_SET     (MCI_PINSEL4_P2_11|MCI_PINSEL4_P2_12|MCI_PINSEL4_P2_13)
#define MCI_PCONP_ENABLE    (1<<28)
#define MCI_PCLK_SEL1       (      1<<24)
#define MCI_SCS_POWER       (1<<8)
#define MCI_COMMAND_MASK    (0x3F)
#define MCI_COMMAND_ACTIVE  (0x00000800)
#define MCI_CLEAR_MASK      (0x7FF)

#define MCI_CONTROL_ENABLE         (1<<0)
#define MCI_CONTROL_DIRECTION_READ (1<<1)
#define MCI_CONTROL_MODE_STREAM    (1<<2)
#define MCI_CONTROL_DMA_ENABLE     (1<<3)

#define MCI_CLOCK_CONTROL_CLKDIV_MASK 0xFF
#define MCI_CLOCK_CONTROL_ENABLE      (1<< 8)
#define MCI_CLOCK_CONTROL_PWRSAVE     (1<< 9)
#define MCI_CLOCK_CONTROL_BYPASS      (1<<10)
#define MCI_CLOCK_CONTROL_WIDEBUS     (1<<11)

#define MCI_POWER_CONTROL_CTRL_CLEAR_MASK (1<<0 | 1<<1)
#define MCI_POWER_CONTROL_CTRL_POWER_OFF  (0)
#define MCI_POWER_CONTROL_CTRL_POWER_UP   (1<<1)
#define MCI_POWER_CONTROL_CTRL_POWER_ON   (1<<0 | 1<<1)
#define MCI_POWER_CONTROL_OPENDRAIN       (1<<6)
#define MCI_POWER_CONTROL_ROD             (1<<7)

#define MCI_ARGUMENT_OCR_INDEX            0x00FF8000

void mmc_init(int taskpriority);
bool mmc_post_read_request(struct mmc_dataread_request *req);
bool mmc_post_write_request(struct mmc_datawrite_request *req);

#endif /* MMC_H_ */

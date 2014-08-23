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


#ifndef MMC_H_
#define MMC_H_

enum mmc_speed {
	MMC_SPEED_400KHZ,
	MMC_SPEED_24MHZ,
};

enum mmc_command {
	MMC_COMMAND_GO_IDLE_STATE       = 0 ,      /* Resetthe card to idle state  (MMC,SD) */
	MMC_COMMAND_SEND_OP_COND        = 1 ,      /* Send Op.Cond. Register       (MMC)    */
	MMC_COMMAND_ALL_SEND_CID        = 2 ,      /* Send Card CID number         (MMC,SD) */
	MMC_COMMAND_SET_RELATIVE_ADDR   = 3 ,      /* Set Relative Address         (MMC,SD) */
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
	MMC_RESPONSE_NONE,
	MMC_RESPONSE_SHORT = 0x40,
	MMC_RESPONSE_LONG = 0xC0,
};
#define MMC_COMMAND_FLAG   (0x400)

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


void mmc_init(int taskpriority);

#endif /* MMC_H_ */

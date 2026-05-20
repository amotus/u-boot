/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2025 Dimonoff
 */

#ifndef _IMX6_EEPROM_H_
#define _IMX6_EEPROM_H_ 1

/* Even if we use I2C2 bus, use bus index 0 since we only enable I2C2. */
#define IMX6_EEPROM_I2C_BUS	0
#define IMX6_EEPROM_I2C_ADDR	0x50

#define WHILE_NOT_EQUAL_INDEX	241
#define WHILE_EQUAL_INDEX	242
#define WHILE_AND_INDEX		243
#define WHILE_NOT_AND_INDEX	244
#define DELAY_10USEC_INDEX	245
#define LAST_COMMAND_INDEX	255

#define MAX_DEFAULT_ADDRS_INDEX		200
#define MAX_DEFAULT_VALUES_INDEX	200
#define MAX_CUSTOM_ADDRESSES	32
#define MAX_CUSTOM_VALUES	32
#define MAX_NUM_OF_COMMANDS	150

#define EEPROM_WAIT_COMMAND_DELAY_US	100  /* Delay in us between retries. */
#define EEPROM_WAIT_COMMAND_MAX_TRY	1000 /* Maximum number of retries. */

struct cmd {
	u8 addr;
	u8 index;
};

int imx6_eeprom_dram_init(u32 *custom_addr_val, struct cmd custom_cmd[]);
int imx6_eeprom_display_infos(const char *path);

#endif /* _IMX6_EEPROM_H_ */

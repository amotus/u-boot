/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2025 Dimonoff
 */

#ifndef _IMX6_EEPROM_H_
#define _IMX6_EEPROM_H_ 1

#define MAX_COMMON_ADDRS_INDEX	200
#define MAX_COMMON_VALUES_INDEX	200
#define MAX_CUSTOM_ADDRESSES	32
#define MAX_CUSTOM_VALUES	32
#define MAX_NUM_OF_COMMANDS	150

struct cmd {
	u8 addr;
	u8 index;
};

int imx6_eeprom_display_infos(const char *path);

#endif /* _IMX6_EEPROM_H_ */

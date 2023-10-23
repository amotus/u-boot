// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2020 Variscite Ltd.
 * Copyright 2023 DimOnOff Inc.
 */

#include <dm.h>
#include <i2c_eeprom.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>

#include "som_eeprom.h"

DECLARE_GLOBAL_DATA_PTR;

#define VAR_IMX8_EEPROM_MAGIC	0x384D /* "8M" */

#if defined(CONFIG_DISPLAY_BOARDINFO)

void display_som_infos(struct var_imx8_eeprom_info *info)
{
	char partnumber[sizeof(info->partnumber) +
			sizeof(info->partnumber2) + 1];
	char assembly[sizeof(info->assembly) + 1];
	char date[sizeof(info->date) + 1];

	/* Read first part of P/N. */
	memcpy(partnumber, info->partnumber, sizeof(info->partnumber));

	/* Read second part of P/N. */
	if (info->eeprom_version >= 3)
		memcpy(partnumber + sizeof(info->partnumber), info->partnumber2,
		       sizeof(info->partnumber2));

	memcpy(assembly, info->assembly, sizeof(info->assembly));
	memcpy(date, info->date, sizeof(info->date));

	/* Make sure strings are null terminated. */
	partnumber[sizeof(partnumber) - 1] = '\0';
	assembly[sizeof(assembly) - 1] = '\0';
	date[sizeof(date) - 1] = '\0';

	printf("SOM board: P/N: %s, Assy: %s, Date: %s\n"
	       "           Wifi: %s, EthPhy: %s, Rev: %d\n",
	       partnumber, assembly, date,
	       info->features & VAR_EEPROM_F_WIFI ? "yes" : "no",
	       info->features & VAR_EEPROM_F_ETH ? "yes" : "no",
	       info->somrev);
}

#endif /* CONFIG_DISPLAY_BOARDINFO */

int var_read_som_eeprom(struct var_imx8_eeprom_info *info)
{
	const char *path = "eeprom-som";
	struct udevice *dev;
	int ret, off;

	off = fdt_path_offset(gd->fdt_blob, path);
	if (off < 0) {
		pr_err("%s: fdt_path_offset() failed: %d\n", __func__, off);
		return off;
	}

	ret = uclass_get_device_by_of_offset(UCLASS_I2C_EEPROM, off, &dev);
	if (ret) {
		pr_err("%s: uclass_get_device_by_of_offset() failed: %d\n",
		       __func__, ret);
		return ret;
	}

	ret = i2c_eeprom_read(dev, 0, (uint8_t *)info,
			      sizeof(struct var_imx8_eeprom_info));
	if (ret) {
		pr_err("%s: i2c_eeprom_read() failed: %d\n", __func__, ret);
		return ret;
	}

	if (htons(info->magic) != VAR_IMX8_EEPROM_MAGIC) {
		/* Do not fail if the content is invalid */
		pr_err("Board: Invalid board info magic: 0x%08x, expected 0x%08x\n",
		       htons(info->magic), VAR_IMX8_EEPROM_MAGIC);
	}

#if defined(CONFIG_BOARD_TYPES)
	gd->board_type = info->features;
#endif /* CONFIG_BOARD_TYPES */

	return 0;
}

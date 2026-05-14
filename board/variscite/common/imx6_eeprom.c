// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015-2019 Variscite Ltd.
 * Copyright (C) 2019 Parthiban Nallathambi <parthitce@gmail.com>
 * Copyright (C) 2021 Marc Ferland, Amotus Solutions Inc., <ferlandm@amotus.ca>
 */

#include <linux/compiler.h>
#include <linux/printk.h>
#include <linux/sizes.h>
#include <linux/string.h>

#if defined(CONFIG_XPL_BUILD)
#include <i2c.h>
#else
#include <dm.h>
#include <i2c_eeprom.h>
#include <linux/libfdt.h>
#endif /* CONFIG_XPL_BUILD */

#include "imx6_eeprom.h"

/* length of strings stored in the eeprom */
#define IMX6_PN_LEN   16
#define IMX6_ASSY_LEN 16
#define IMX6_DATE_LEN 12

/* eeprom content, 512 bytes */
struct imx6_eeprom_info {
	u32 magic;
	u8 partnumber[IMX6_PN_LEN];
	u8 assy[IMX6_ASSY_LEN];
	u8 date[IMX6_DATE_LEN];
	u32 custom_addr_val[MAX_CUSTOM_ADDRESSES];
	struct cmd custom_cmd[MAX_NUM_OF_COMMANDS];
	u8 res[33];
	u8 som_info;
	u8 ddr_size;
	u8 crc;
} __attribute__ ((__packed__));

#define IMX6_INFO_STORAGE_GET(n) ((n) & 0x3)
#define IMX6_INFO_WIFI_GET(n)    ((n) >> 2 & 0x1)
#define IMX6_INFO_REV_GET(n)     ((n) >> 3 & 0x3)
#define IMX6_DDRSIZE(n)          ((n) * SZ_128M)
#define IMX6_INFO_MAGIC          0x32524156

#if defined(CONFIG_XPL_BUILD)

static int imx6_eeprom_read(const char *path, struct imx6_eeprom_info *info)
{
	int ret;
	(void) path;

	i2c_set_bus_num(IMX6_EEPROM_I2C_BUS);

	ret = i2c_probe(IMX6_EEPROM_I2C_ADDR);
	if (ret) {
		printf("%s: I2C EEPROM probe failed\n", __func__);
		return ret;
	}

	ret = i2c_read(IMX6_EEPROM_I2C_ADDR, 0, 1, (uint8_t *)info,
		       sizeof(struct imx6_eeprom_info));
	if (ret) {
		printf("%s: EEPROM read failed ret=%d\n", __func__, ret);
		return ret;
	}

	return 0;
}

#else

static int imx6_eeprom_get_device(const char *path, struct udevice **dev)
{
	int ret, off;

	off = fdt_path_offset(gd->fdt_blob, path);
	if (off < 0) {
		pr_err("%s: fdt_path_offset() failed: %d\n", __func__, off);
		return off;
	}

	ret = uclass_get_device_by_of_offset(UCLASS_I2C_EEPROM, off, dev);
	if (ret) {
		pr_err("%s: uclass_get_device_by_of_offset() failed: %d\n", __func__, ret);
		return ret;
	}

	return 0;
}

static int imx6_eeprom_read(const char *path, struct imx6_eeprom_info *info)
{
	struct udevice *dev;
	int ret;

	ret = imx6_eeprom_get_device(path, &dev);
	if (ret)
		return ret;

	ret = i2c_eeprom_read(dev, 0, (uint8_t *)info,
			      sizeof(struct imx6_eeprom_info));
	if (ret) {
		printf("%s: i2c_eeprom_read() failed: %d\n", __func__, ret);
		return ret;
	}

	return 0;
}

#endif /* CONFIG_XPL_BUILD */

int imx6_eeprom_get_infos(const char *path, struct imx6_eeprom_info *info)
{
	int ret;

	ret = imx6_eeprom_read(path, info);
	if (ret) {
		printf("%s: imx6_eeprom_read() failed: %d\n", __func__, ret);
		return ret;
	}

	if (info->magic != IMX6_INFO_MAGIC) {
		printf("Board: Invalid board info magic: 0x%08x, expected 0x%08x\n",
		       info->magic, IMX6_INFO_MAGIC);
		/* do not fail if the content is invalid */
		return 0;
	}

	return 0;
}

#if defined(CONFIG_XPL_BUILD)

int imx6_eeprom_dram_init(u32 *custom_addr_val, struct cmd custom_cmd[])
{
	struct imx6_eeprom_info info;
	int ret;

	ret = imx6_eeprom_get_infos(NULL, &info);
	if (ret) {
		printf("%s: imx6_eeprom_get_infos() failed: %d\n", __func__, ret);
		return ret;
	}

	if (info.magic != IMX6_INFO_MAGIC)
		return 0; /* do not fail if the content is invalid */

	memcpy(custom_addr_val, info.custom_addr_val, sizeof(info.custom_addr_val));
	memcpy(custom_cmd, info.custom_cmd, sizeof(info.custom_cmd));

	return 0;
}

#else

static const char *som_info_storage_to_str(u8 som_info)
{
	switch (IMX6_INFO_STORAGE_GET(som_info)) {
	case 0x0: return "none (SD only)";
	case 0x1: return "NAND";
	case 0x2: return "eMMC";
	default: return "unknown";
	}
}

static const char *som_info_rev_to_str(u8 som_info)
{
	switch (IMX6_INFO_REV_GET(som_info)) {
	case 0x0: return "2.4G LWB";
	case 0x1: return "5G LW5";
	case 0x2: return "5G IW611";
	case 0x3: return "5G IW612";
	default: return "unknown";
	}
}

int imx6_eeprom_display_infos(const char *path)
{
	struct imx6_eeprom_info info;
	int ret;

	ret = imx6_eeprom_get_infos(path, &info);
	if (ret) {
		printf("%s: imx6_eeprom_get_infos() failed: %d\n", __func__, ret);
		return ret;
	}

	if (info.magic != IMX6_INFO_MAGIC)
		return 0; /* do not fail if the content is invalid */

	/* make sure strings are null terminated */
	info.partnumber[IMX6_PN_LEN - 1] = '\0';
	info.assy[IMX6_ASSY_LEN - 1] = '\0';
	info.date[IMX6_DATE_LEN - 1] = '\0';

	printf("Board: PN: %s, Assy: %s, Date: %s\n"
	       "       Storage: %s, Wifi: %s, DDR: %d MiB, Rev: %s\n",
	       info.partnumber,
	       info.assy,
	       info.date,
	       som_info_storage_to_str(info.som_info),
	       IMX6_INFO_WIFI_GET(info.som_info) ? "yes" : "no",
	       IMX6_DDRSIZE(info.ddr_size) / SZ_1M,
	       som_info_rev_to_str(info.som_info));

	return 0;
}

#endif /* CONFIG_XPL_BUILD */

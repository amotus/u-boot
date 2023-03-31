// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 Collabora Ltd.
 * Copyright 2018-2020 Variscite Ltd.
 * Copyright 2023 DimOnOff Inc.
 */

#include <env.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <asm/arch/sys_proto.h>
#include <dt-bindings/gpio/gpio.h>
#include <linux/libfdt.h>
#include <linux/errno.h>

#include "som_eeprom.h"

DECLARE_GLOBAL_DATA_PTR;

int board_late_init(void)
{
	if (is_usb_boot()) {
		/*
		 * Force use of default env when booting from USB (MFG mode).
		 * Using an existing environment in eMMC can cause problems in
		 * MFG mode when trying to reflash U-Boot and rootfs.
		 */
		env_set_default("MFG mode", 0);
	}

	return 0;
}

int board_mmc_get_env_dev(int devno)
{
	return devno;
}

int checkboard(void)
{
	int rc;
	struct var_imx8_eeprom_info *info;

	info = malloc(sizeof(struct var_imx8_eeprom_info));
	if (!info)
		return -ENOMEM;

	rc = var_read_som_eeprom(info);
	if (rc)
		return rc;

#if defined(CONFIG_DISPLAY_BOARDINFO)
	display_som_infos(info);
#endif /* CONFIG_DISPLAY_BOARDINFO */

	return 0;
}

static int insert_gpios_prop(void *blob, int node, const char *prop,
			     unsigned int phandle, u32 gpio, u32 flags)
{
	fdt32_t val[3] = { cpu_to_fdt32(phandle), cpu_to_fdt32(gpio),
			   cpu_to_fdt32(flags) };
	return fdt_setprop(blob, node, prop, &val, sizeof(val));
}

static int configure_phy_reset_gpios(void *blob)
{
	int node;
	int phynode;
	int ret;
	u32 handle;
	u32 gpio;
	u32 flags;
	char path[1024];
	const char *eth_alias = "ethernet0";

	snprintf(path, sizeof(path), "%s/mdio/ethernet-phy@4",
		 fdt_get_alias(blob, eth_alias));

	phynode = fdt_path_offset(blob, path);
	if (phynode < 0) {
		pr_err("%s(): unable to locate PHY node: %s\n", __func__, path);
		return 0;
	}

	if (gd_board_type() & VAR_EEPROM_F_ETH) {
		snprintf(path, sizeof(path), "%s",
			 fdt_get_alias(blob, "gpio0")); /* Alias to gpio1 */
		gpio = 9;
		flags = GPIO_ACTIVE_LOW;
	} else {
		snprintf(path, sizeof(path), "%s/gpio@20",
			 fdt_get_alias(blob, "i2c1")); /* Alias to i2c2 */
		gpio = 5;
		flags = GPIO_ACTIVE_HIGH;
	}

	node = fdt_path_offset(blob, path);
	if (node < 0) {
		pr_err("%s(): unable to locate GPIO node: %s\n", __func__,
		       path);
		return 0;
	}

	handle = fdt_get_phandle(blob, node);
	if (handle < 0) {
		pr_err("%s(): unable to locate GPIO controller handle: %s\n",
		       __func__, path);
	}

	ret = insert_gpios_prop(blob, phynode, "reset-gpios",
				handle, gpio, flags);
	if (ret < 0) {
		pr_err("%s(): failed to set reset-gpios property\n", __func__);
		return ret;
	}

	return 0;
}

#if defined(CONFIG_OF_BOARD_FIXUP)
int board_fix_fdt(void *blob)
{
	/* Fix U-Boot device tree: */
	return configure_phy_reset_gpios(blob);
}
#endif /* CONFIG_OF_BOARD_FIXUP */

#if defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	/* Fix kernel device tree: */
	return configure_phy_reset_gpios(blob);
}
#endif /* CONFIG_OF_BOARD_SETUP */

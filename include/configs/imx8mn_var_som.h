/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 Collabora Ltd.
 */

#ifndef __IMX8MN_VAR_SOM_H
#define __IMX8MN_VAR_SOM_H

#include <linux/sizes.h>
#include <linux/stringify.h>
#include <asm/arch/imx-regs.h>

#define CFG_SYS_UBOOT_BASE	\
	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

#define PART_A_ID    1
#define PART_B_ID    2

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 2) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na) \

#include <config_distro_bootcmd.h>

#define MEM_LAYOUT_ENV_SETTINGS \
	"scriptaddr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"kernel_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"ramdisk_addr_r=0x43800000\0" \
	"fdt_addr_r=0x43000000\0" \
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"fastboot_partition_alias_all=" \
		__stringify(CONFIG_FASTBOOT_FLASH_MMC_DEV) ".0:0\0" \
	"fastboot_partition_alias_bootloader=" \
		__stringify(CONFIG_FASTBOOT_FLASH_MMC_DEV) ".1:0\0" \
	"fastboot_addr=" __stringify(CONFIG_FASTBOOT_BUF_ADDR) "\0" \
	"emmc_dev=" __stringify(CONFIG_FASTBOOT_FLASH_MMC_DEV) "\0" \
	"emmc_ack=1\0" \
	"pxefile_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"distro_bootpart=" __stringify(PART_A_ID) "\0" \
	"part_a_id=" __stringify(PART_A_ID) "\0" \
	"part_b_id=" __stringify(PART_B_ID) "\0" \

#define ALT_BOOTCMD \
	"altbootcmd=" \
		"echo Rollback to previous rootfs; " \
		"if test x${distro_bootpart} = x${part_a_id}; then " \
			"setenv distro_bootpart ${part_b_id}; " \
		"else " \
			"setenv distro_bootpart ${part_a_id}; " \
		"fi; " \
		"setenv bootcount 0; " \
		"saveenv; " \
		"boot\0" \

#define BOARD_BOOTCOMMAND \
	"board_bootcmd=" \
		"mmc dev ${emmc_dev}; " \
		"run check_gpt_sig; " \
		"gpio read back_button gpio@20_1; " \
		"if test x${back_button} = x0; then " \
			"run mfg_bootcmd; " \
		"elif test ${gpt_sig_valid} != 1; then " \
			"echo No partition table found...; " \
			"run mfg_bootcmd; " \
		"else " \
			"run distro_bootcmd; " \
		"fi\0" \

/* Initial environment variables */
#define CFG_EXTRA_ENV_SETTINGS \
	MEM_LAYOUT_ENV_SETTINGS \
	ALTBOOTCMD \
	BOOTENV

/* Link Definitions */

#define CFG_SYS_INIT_RAM_ADDR	0x40000000
#define CFG_SYS_INIT_RAM_SIZE	SZ_512K

#define CFG_SYS_SDRAM_BASE		0x40000000
#define PHYS_SDRAM			0x40000000
#define PHYS_SDRAM_SIZE			SZ_1G /* 1GB DDR */

/* USDHC */
#define CFG_SYS_FSL_ESDHC_ADDR	0

#endif /* __IMX8MN_VAR_SOM_H */

/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Board configuration file for Variscite DART-6UL Evaluation Kit
 * Copyright (C) 2019 Parthiban Nallathambi <parthitce@gmail.com>
 */
#ifndef __DART_6UL_H
#define __DART_6UL_H

#include <linux/sizes.h>
#include <linux/stringify.h>
#include "mx6_common.h"

/* NAND pin conflicts with usdhc2 */
#ifdef CONFIG_CMD_NAND
#define CFG_SYS_FSL_USDHC_NUM        1
#else
#define CFG_SYS_FSL_USDHC_NUM        2
#endif

#ifdef CONFIG_CMD_NET
#define CFG_FEC_ENET_DEV		0
#endif

/* Environment settings */

/* Environment in SD */
#define MMC_ROOTFS_DEV			0
#define MMC_ROOTFS_PART			2

/* Console configs */
#define CFG_MXC_UART_BASE		UART1_BASE

/* MMC Configs */

#define CFG_SYS_FSL_ESDHC_ADDR	USDHC2_BASE_ADDR

/* I2C configs */

/* Miscellaneous configurable options */

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR
#define PHYS_SDRAM_SIZE			SZ_512M

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define PART_A_ID    1
#define PART_B_ID    2

#define GPT_SIG_VALID "aa550000"

#define UMS_CMD \
	"mfg_ums=" \
		"echo Start UMS...; " \
		"ums 0 mmc ${emmc_dev}\0" \

#define FASTBOOT_CMD \
	"mfg_fastboot=" \
		"echo Starting fastboot...; " \
		"fastboot 0\0" \

#define CHECK_GPT_SIG_CMD \
	"check_gpt_sig=" \
		"mmc read ${temp_addr} 0 1; " \
		"setexpr gpt_sig_addr ${temp_addr} + 1FC; " \
		"if itest.l *${gpt_sig_addr} != " __stringify(GPT_SIG_VALID) "; then " \
			"setenv gpt_sig_valid 0; " \
		"else " \
			"setenv gpt_sig_valid 1; " \
		"fi\0" \

/*
 * Erase eMMC partition table. When using UMS mode, this will prevent
 * the OS from mounting any previously defined partitions.
 * eMMC partition table offset = 0.
 * Note: we cannot use "mmc erase" command, because the "Erase Group Size"
 * is 512 KB, as reported by "mmc info".
 */
#define MMC_DEL_PART_TABLE_CMD \
	"mmc_del_part_table=" \
		"mmc dev ${emmc_dev}; " \
		"mw.b ${fastboot_addr} 0 200; " \
		"mmc write ${fastboot_addr} 0 1\0" \

/*
 * Erase eMMC environment. For initial programming, this will prevent
 * U-Boot from using/loading an old environment from eMMC.
 */
#define MMC_DEL_ENVIRONMENT_CMD \
	"mmc_del_environment=" \
		"mmc dev ${emmc_dev}; " \
		"mw.b ${fastboot_addr} 0 200; " \
		"setexpr mmc_env_offset_blk " __stringify(CONFIG_ENV_OFFSET) " / 200; " \
		"mmc write ${fastboot_addr} ${mmc_env_offset_blk} 1\0" \

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 0) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

#define MEM_LAYOUT_ENV_SETTINGS \
	"scriptaddr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"kernel_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"temp_addr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
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
	"console=ttymxc0\0" \
	"boot_prefixes=/boot\0" \
	"distro_bootpart=" __stringify(PART_A_ID) "\0" \
	"part_a_id=" __stringify(PART_A_ID) "\0" \
	"part_b_id=" __stringify(PART_B_ID) "\0" \

#define MFG_BOOTCMD \
	"mfg_bootcmd=" \
		"run mfg_fastboot; " \
		"run mfg_ums\0" \

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
		"if test ${boot_from_usb} = 1; then " \
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
	UMS_CMD \
	FASTBOOT_CMD \
	CHECK_GPT_SIG_CMD \
	MMC_DEL_PART_TABLE_CMD \
	MMC_DEL_ENVIRONMENT_CMD \
	ALT_BOOTCMD \
	BOARD_BOOTCOMMAND \
	MFG_BOOTCMD \
	BOOTENV

#endif /* __DART_6UL_H */

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015-2019 Variscite Ltd.
 * Copyright (C) 2019 Parthiban Nallathambi <parthitce@gmail.com>
 */

#include <config.h>
#include <init.h>
#include <spl.h>
#include <asm/arch/clock.h>
#include <asm/io.h>
#include <asm/arch/mx6-ddr.h>
#include <asm/arch/mx6-pins.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/arch/crm_regs.h>
#include <asm/sections.h>
#include <fsl_esdhc_imx.h>
#include <linux/delay.h>

#include "../common/imx6_eeprom.h"

#ifdef SPL_DEBUG
#define spl_debug(M, ...) printf(M, ##__VA_ARGS__)
#else
#define spl_debug(M, ...)
#endif

#define UART_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

static iomux_v3_cfg_t const uart1_pads[] = {
	MX6_PAD_UART1_TX_DATA__UART1_DCE_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_UART1_RX_DATA__UART1_DCE_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static void setup_iomux_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
}

static struct mx6ul_iomux_grp_regs mx6_grp_ioregs = {
	.grp_addds = 0x00000030,
	.grp_ddrmode_ctl = 0x00020000,
	.grp_b0ds = 0x00000030,
	.grp_ctlds = 0x00000030,
	.grp_b1ds = 0x00000030,
	.grp_ddrpke = 0x00000000,
	.grp_ddrmode = 0x00020000,
	.grp_ddr_type = 0x000c0000,
};

static struct mx6ul_iomux_ddr_regs mx6_ddr_ioregs = {
	.dram_dqm0 = 0x00000030,
	.dram_dqm1 = 0x00000030,
	.dram_ras = 0x00000030,
	.dram_cas = 0x00000030,
	.dram_odt0 = 0x00000030,
	.dram_odt1 = 0x00000030,
	.dram_sdba2 = 0x00000000,
	.dram_sdclk_0 = 0x00000008,
	.dram_sdqs0 = 0x00000038,
	.dram_sdqs1 = 0x00000030,
	.dram_reset = 0x00000030,
};

static struct mx6_mmdc_calibration mx6_mmcd_calib = {
	.p0_mpwldectrl0 = 0x00000000,
	.p0_mpdgctrl0   = 0x414C0158,
	.p0_mprddlctl   = 0x40403A3A,
	.p0_mpwrdlctl   = 0x40405A56,
};

struct mx6_ddr_sysinfo ddr_sysinfo = {
	.dsize = 0,
	.cs_density = 20,
	.ncs = 1,
	.cs1_mirror = 0,
	.rtt_wr = 2,
	.rtt_nom = 1,		/* RTT_Nom = RZQ/2 */
	.walat = 1,		/* Write additional latency */
	.ralat = 5,		/* Read additional latency */
	.mif3_mode = 3,		/* Command prediction working mode */
	.bi_on = 1,		/* Bank interleaving enabled */
	.sde_to_rst = 0x10,	/* 14 cycles, 200us (JEDEC default) */
	.rst_to_cke = 0x23,	/* 33 cycles, 500us (JEDEC default) */
};

static struct mx6_ddr3_cfg mem_ddr = {
	.mem_speed = 800,
	.density = 4,
	.width = 16,
	.banks = 8,
	.rowaddr = 15,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1375,
	.trcmin = 4875,
	.trasmin = 3500,
};

static void ccgr_init(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	writel(0xFFFFFFFF, &ccm->CCGR0);
	writel(0xFFFFFFFF, &ccm->CCGR1);
	writel(0xFFFFFFFF, &ccm->CCGR2);
	writel(0xFFFFFFFF, &ccm->CCGR3);
	writel(0xFFFFFFFF, &ccm->CCGR4);
	writel(0xFFFFFFFF, &ccm->CCGR5);
	writel(0xFFFFFFFF, &ccm->CCGR6);
	writel(0xFFFFFFFF, &ccm->CCGR7);
	/* Enable Audio Clock for SOM codec */
	writel(0x01130100, (long *)CCM_CCOSR);
}

static void spl_legacy_dram_init(void)
{
	printf("DDR LEGACY configuration\n");
	mx6ul_dram_iocfg(mem_ddr.width, &mx6_ddr_ioregs, &mx6_grp_ioregs);
	mx6_dram_cfg(&ddr_sysinfo, &mx6_mmcd_calib, &mem_ddr);
}

#define USDHC_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_22K_UP  | PAD_CTL_SPEED_LOW |		\
	PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)
static iomux_v3_cfg_t const usdhc1_pads[] = {
	MX6_PAD_SD1_CLK__USDHC1_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_CMD__USDHC1_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DATA0__USDHC1_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DATA1__USDHC1_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DATA2__USDHC1_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DATA3__USDHC1_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
};

#ifndef CONFIG_NAND_MXS
static iomux_v3_cfg_t const usdhc2_pads[] = {
	MX6_PAD_NAND_RE_B__USDHC2_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NAND_WE_B__USDHC2_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NAND_DATA00__USDHC2_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NAND_DATA01__USDHC2_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NAND_DATA02__USDHC2_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NAND_DATA03__USDHC2_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NAND_DATA04__USDHC2_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NAND_DATA05__USDHC2_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NAND_DATA06__USDHC2_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NAND_DATA07__USDHC2_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
};
#endif

static struct fsl_esdhc_cfg usdhc_cfg[] = {
	{
		.esdhc_base = USDHC1_BASE_ADDR,
		.max_bus_width = 4,
	},
#ifndef CONFIG_NAND_MXS
	{
		.esdhc_base = USDHC2_BASE_ADDR,
		.max_bus_width = 8,
	},
#endif
};

int board_mmc_getcd(struct mmc *mmc)
{
	return 1;
}

int board_mmc_init(struct bd_info *bis)
{
	int i, ret;

	for (i = 0; i < CFG_SYS_FSL_USDHC_NUM; i++) {
		switch (i) {
		case 0:
			SETUP_IOMUX_PADS(usdhc1_pads);
			usdhc_cfg[i].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
			break;
#ifndef CONFIG_NAND_MXS
		case 1:
			SETUP_IOMUX_PADS(usdhc2_pads);
			usdhc_cfg[i].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
			break;
#endif
		default:
			printf("Warning - USDHC%d controller not supporting\n",
			       i + 1);
			return 0;
		}

		ret = fsl_esdhc_initialize(bis, &usdhc_cfg[i]);
		if (ret) {
			printf("Warning: failed to initialize mmc dev %d\n", i);
			return ret;
		}
	}

	return 0;
}

void board_boot_order(u32 *spl_boot_list)
{
       u32 boot_dev = spl_boot_device();

       if (boot_dev == BOOT_DEVICE_MMC1)
               /* boot from MMC2 (eMMC) and skip uSD */
               boot_dev = BOOT_DEVICE_MMC2;
       spl_boot_list[0] = boot_dev;
}

#define I2C_PAD_CTRL    (PAD_CTL_PKE | PAD_CTL_PUE |			\
			 PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |	\
			 PAD_CTL_DSE_40ohm | PAD_CTL_HYS |		\
			 PAD_CTL_ODE)

static struct i2c_pads_info i2c2_pads_info = {
	.scl = {
		.i2c_mode  = MX6_PAD_CSI_HSYNC__I2C2_SCL | MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = MX6_PAD_CSI_HSYNC__GPIO4_IO20 | MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(4, 20),
	},
	.sda = {
		.i2c_mode  = MX6_PAD_CSI_VSYNC__I2C2_SDA | MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = MX6_PAD_CSI_VSYNC__GPIO4_IO19 | MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(4, 19),
	},
};

static u32 get_address_by_index(u8 index, const u32 *default_addresses, const u32 *custom_addresses)
{
	if (index >= MAX_DEFAULT_ADDRS_INDEX)
		return custom_addresses[index - MAX_DEFAULT_ADDRS_INDEX];

	return default_addresses[index];
}

static u32 get_value_by_index(u8 index, const u32 *default_values, const u32 *custom_values)
{
	if (index >= MAX_DEFAULT_VALUES_INDEX)
		return custom_values[index - MAX_DEFAULT_VALUES_INDEX];

	return default_values[index];
}

static int handle_commands(const struct cmd eeprom_cmd[],
			   const u32 *default_addresses, const u32 *default_values,
			   const u32 *custom_addresses, const u32 *custom_values)
{
	u32 address, value;
	volatile u32 *reg_ptr;
	u8 wait_idx = 0;
	int i = 0;

	while (i < MAX_NUM_OF_COMMANDS) {
		spl_debug("Command[%03d] addr=%03d,  index=%03d\n", i, eeprom_cmd[i].addr,
			 eeprom_cmd[i].index);

		if (eeprom_cmd[i].addr == LAST_COMMAND_INDEX)
			return 0;

		if (eeprom_cmd[i].index == DELAY_10USEC_INDEX) {
			/* Delay for Value * 10 uSeconds */
			spl_debug("  Delay %d microseconds\n", eeprom_cmd[i].index * 10);
			udelay((int)(eeprom_cmd[i].index * 10));
			++i;
			continue;
		}

		/*
		 * Check for a wait index.
		 * A wait index means "next command is a wait command".
		 */
		switch (eeprom_cmd[i].addr) {
		case WHILE_NOT_EQUAL_INDEX:
		case WHILE_EQUAL_INDEX:
		case WHILE_AND_INDEX:
		case WHILE_NOT_AND_INDEX:
			/* Save wait index and go to next command */
			wait_idx = eeprom_cmd[i].addr;
			++i;
			break;
		}

		/* Get address and value */
		address = get_address_by_index(eeprom_cmd[i].addr, default_addresses,
					       custom_addresses);
		value = get_value_by_index(eeprom_cmd[i].index, default_values, custom_values);
		reg_ptr = (u32 *)address;

		if (wait_idx != 0) {
			bool done = false;
			int try = 0;

			do {
				udelay(EEPROM_WAIT_COMMAND_DELAY_US);

				switch (wait_idx) {
				case WHILE_NOT_EQUAL_INDEX:
					spl_debug("  Wait !=\n");
					if (!(*reg_ptr != value))
						done = true;
					break;
				case WHILE_EQUAL_INDEX:
					spl_debug("  Wait ==\n");
					if (!(*reg_ptr == value))
						done = true;
					break;
				case WHILE_AND_INDEX:
					spl_debug("  Wait and\n");
					if (!(*reg_ptr & value))
						done = true;
					break;
				case WHILE_NOT_AND_INDEX:
					spl_debug("  Wait !and\n");
					if (*reg_ptr & value)
						done = true;
					break;
				}

				if (++try > EEPROM_WAIT_COMMAND_MAX_TRY) {
					pr_err("%s() wait failure: reg=$%08X, value=$%08X\n",
					       __func__, address, value);
					done = true;
				}
			} while (!done);

			wait_idx = 0;
		} else {
			if (address == 0x021B0020 && value == 0x00007800)
				value = 0x00000800;

			/* This is a regular set command (non-wait) */
			spl_debug("  [$%08X] = $%08X\n", address, value);
			*reg_ptr = value;
		}

		++i;
	}

	return 0;
}

/*
 * Fills custom_addresses & custom_values, from custom_addresses_values
 */
static void load_custom_data(u32 *custom_addresses, u32 *custom_values, const u32 *custom_addr_val)
{
	int i, j = 0;

	for (i = 0; i < MAX_CUSTOM_ADDRESSES; i++) {
		if (custom_addr_val[i] == 0)
			break;

		spl_debug("%s(): eeprom_addresses[%02d] = $%08X\n", __func__, i, custom_addr_val[i]);
		custom_addresses[i] = custom_addr_val[i];
	}

	i++;
	if (i > MAX_CUSTOM_ADDRESSES)
		return;

	j = 0;
	for (; i < MAX_CUSTOM_VALUES; i++) {
		if (custom_addr_val[i] == 0)
			break;
		custom_values[j] = custom_addr_val[i];
		spl_debug("%s(): eeprom_values[%02d] = $%08X\n", __func__, j, custom_addr_val[i]);
		j++;
	}
}

static int spl_eeprom_dram_init(const u32 *eeprom_addr_val, const struct cmd eeprom_cmd[])
{
	/*
	 * The eeprom contains commands with
	 * 1 byte index to a default address in this array, and
	 * 1 byte index to a default value in the next array - to write to the address.
	 */
	const u32 default_addresses[] = {
		#include "addresses.inc"
	};

	const u32 default_values[] = {
		#include "values.inc"
	};

	/*
	 * Some commands in the eeprom contain higher indices,
	 * to custom addresses and values which are not present in the default arrays,
	 * and it also contains an array of the custom addresses and values themselves.
	 */
	u32 custom_addresses[MAX_CUSTOM_ADDRESSES] = {0};
	u32 custom_values[MAX_CUSTOM_VALUES] = {0};

	load_custom_data(custom_addresses, custom_values, eeprom_addr_val);

	return handle_commands(eeprom_cmd, default_addresses, default_values,
			       custom_addresses, custom_values);
}

void board_init_f(ulong dummy)
{
	int ret;
	u32 eeprom_addr_val[MAX_CUSTOM_ADDRESSES];
	struct cmd eeprom_cmd[MAX_NUM_OF_COMMANDS];

	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	ccgr_init();

	/* setup GP timer */
	timer_init();

	setup_iomux_uart();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

	setup_i2c(IMX6_EEPROM_I2C_BUS, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c2_pads_info);

	/* DDR initialization */
	ret = imx6_eeprom_dram_init(eeprom_addr_val, eeprom_cmd);
	if (ret == 0) {
		ret = spl_eeprom_dram_init(eeprom_addr_val, eeprom_cmd);
	}

	if (ret)
		spl_legacy_dram_init();
	else
		printf("DDR eeprom configuration\n");
}

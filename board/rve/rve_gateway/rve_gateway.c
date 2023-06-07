// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 Collabora Ltd.
 */

#include <env.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>

static void setup_fec(void)
{
	struct iomuxc_gpr_base_regs *gpr =
		(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;

	/* Use 125M anatop REF_CLK1 for ENET1, not from external */
	clrsetbits_le32(&gpr->gpr[1], 0x2000, 0);
}

int board_init(void)
{
	if (IS_ENABLED(CONFIG_FEC_MXC))
		setup_fec();

	return 0;
}

int board_late_init(void)
{
	if (is_usb_boot()) {
		/*
		 * Force use of default env when booting from USB (MFG mode).
		 * Using an existing environment in eMMC can cause problems in
		 * MFG mode when trying to reflash U-Boot and rootfs.
		 */
		env_set_default("MFG mode", 0);

                env_set("bootcmd", "run mfg_bootcmd");
                env_set("bootdelay", "0");
        }

	return 0;
}

int board_mmc_get_env_dev(int devno)
{
	return devno;
}

/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018-2020 Variscite Ltd.
 * Copyright 2023 DimOnOff Inc.
 */

#ifndef _SOM_EEPROM_
#define _SOM_EEPROM_

/* Optional SOM features flags. */
#define VAR_EEPROM_F_WIFI		BIT(0)
#define VAR_EEPROM_F_ETH		BIT(1) /* Ethernet PHY on SOM. */
#define VAR_EEPROM_F_AUDIO		BIT(2)
#define VAR_EEPROM_F_MX8M_LVDS		BIT(3) /* i.MX8MM, i.MX8MN, i.MX8MQ only */
#define VAR_EEPROM_F_MX8Q_SOC_ID	BIT(3) /* 0 = i.MX8QM, 1 = i.MX8QP */
#define VAR_EEPROM_F_NAND		BIT(4)

/* Number of DRAM adjustment tables. */
#define DRAM_TABLES_NUM 7

struct var_imx8_eeprom_info {
	u16 magic;
	u8 partnumber[3];         /* Part number */
	u8 assembly[10];          /* Assembly number */
	u8 date[9];               /* Build date */
	u8 mac[6];                /* MAC address */
	u8 somrev;
	u8 eeprom_version;
	u8 features;              /* SOM features */
	u8 dramsize;              /* DRAM size */
	u8 off[DRAM_TABLES_NUM + 1]; /* DRAM table offsets */
	u8 partnumber2[5];        /* Part number 2 */
} __packed;

void display_som_infos(struct var_imx8_eeprom_info *info);

int var_read_som_eeprom(struct var_imx8_eeprom_info *info);

#endif /* _SOM_EEPROM_ */

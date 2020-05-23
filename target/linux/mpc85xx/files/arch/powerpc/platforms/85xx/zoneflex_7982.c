// SPDX-License-Identifier: GPL-2.0+

/*
 * Rufus ZoneFlex 7982 Board Setup
 *
 * Copyright (C) 2019 David Bauer <mail@david-bauer.net>
 *
 * Based on:
 *   p1024rdb.c:
 *      P1024 RDB Board Setup
 *      Copyright 2011 Freescale Semiconductor Inc.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <linux/stddef.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/of_platform.h>

#include <asm/time.h>
#include <asm/machdep.h>
#include <asm/pci-bridge.h>
#include <mm/mmu_decl.h>
#include <asm/prom.h>
#include <asm/udbg.h>
#include <asm/mpic.h>

#include <sysdev/fsl_soc.h>
#include <sysdev/fsl_pci.h>

#include "mpc85xx.h"

void __init ruckus_zoneflex_7982_pic_init(void)
{
	struct mpic *mpic;

	mpic = mpic_alloc(NULL, 0,
	  MPIC_BIG_ENDIAN | MPIC_SINGLE_DEST_CPU,
	  0, 256, " OpenPIC  ");

	BUG_ON(mpic == NULL);
	mpic_init(mpic);
}

/*
 * Setup the architecture
 */
static void __init ruckus_zoneflex_7982_setup_arch(void)
{
	if (ppc_md.progress)
		ppc_md.progress("ruckus_zoneflex_7982_setup_arch()", 0);

	fsl_pci_assign_primary();

	pr_info("ZoneFlex 7982 from Rufus\n");
}

machine_arch_initcall(ruckus_zoneflex_7982, mpc85xx_common_publish_devices);

/*
 * Called very early, device-tree isn't unflattened
 */
static int __init ruckus_zoneflex_7982_probe(void)
{
	if (of_machine_is_compatible("ruckus,zoneflex-7982"))
		return 1;
	return 0;
}

define_machine(ruckus_zoneflex_7982) {
	.name			= "P1024 RDB",
	.probe			= ruckus_zoneflex_7982_probe,
	.setup_arch		= ruckus_zoneflex_7982_setup_arch,
	.init_IRQ		= ruckus_zoneflex_7982_pic_init,
#ifdef CONFIG_PCI
	.pcibios_fixup_bus	= fsl_pcibios_fixup_bus,
	.pcibios_fixup_phb      = fsl_pcibios_fixup_phb,
#endif
	.get_irq		= mpic_get_irq,
	.calibrate_decr		= generic_calibrate_decr,
	.progress		= udbg_progress,
};

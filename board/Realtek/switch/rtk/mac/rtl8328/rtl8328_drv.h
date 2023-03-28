/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * Purpose : Related definition of the RTL8328 driver.
 *
 * Feature : RTL8328 driver
 *
 */

#ifndef	__RTL8328_DRV_H__
#define	__RTL8328_DRV_H__

/*
 * Include Files
 */
#include <rtk_switch.h>

/*
 * Function Declaration
 */
extern void rtl8328_setPhyReg(int portid, int page, int reg, unsigned int val);
extern void rtl8328_getPhyReg(int portid, int page, int reg, unsigned int *val);
extern void rtl8328_setPhyRegByMask(unsigned int port_mask, int page, int reg, unsigned int val);
extern void rtl8328_phyPowerOn(void);
extern void rtl8328_phyPowerOff(void);
extern void rtl8328_phyReset(const rtk_switch_model_t *pModel);

#endif	/* __RTL8328_DRV_H__ */


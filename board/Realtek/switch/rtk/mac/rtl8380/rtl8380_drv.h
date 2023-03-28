/*
 * Copyright(c) Realtek Semiconductor Corporation, 2012
 * All rights reserved.
 *
 * Purpose : Related definition of the RTL8380 driver.
 *
 * Feature : RTL8380 driver
 *
 */

#ifndef	__RTL8380_DRV_H__
#define	__RTL8380_DRV_H__

/*
 * Include Files
 */
#include <rtk_switch.h>

/*
 * Function Declaration
 */
extern int rtl8380_setPhyReg(int portid, int page, int reg, unsigned int val);
extern int rtl8380_getPhyReg(int portid, int page, int reg, unsigned int *val);
extern int rtl8380_setPhyRegByMask(unsigned long long port_mask, int page, int reg, unsigned int val);
extern void rtl8380_phyPowerOn(void);
extern void rtl8380_phyPowerOff(void);
extern void rtl8380_phyReset(const rtk_switch_model_t *pModel);
extern void rtl8380_phyPowerOn_except_serdes_fiber(void);
void rtl8380_phyPortPowerOn(int mac_id);
#endif	/* __RTL8380_DRV_H__ */


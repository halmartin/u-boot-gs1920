/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * Purpose : Related definition of the RTL8328 driver for RTK command.
 *
 * Feature : RTL8328 driver for RTK command
 *
 */

#ifndef	__RTL8328_RTK_H__
#define	__RTL8328_RTK_H__

#include <rtk_switch.h>

extern const rtk_switch_model_t *gSwitchModel;
extern const rtk_mac_drv_t *gMacDrv;

extern void rtk_default(void);

extern void rtk_comboport_copper(void);
extern void rtk_comboport_fiber(void);

#ifdef CONFIG_EEE
extern void rtk_eee_on(const rtk_switch_model_t *pModel);
extern void rtk_eee_off(const rtk_switch_model_t *pModel);
#endif

extern void rtk_network_on(void);
extern void rtk_network_off(void);

extern void rtk_linkdown_powersaving_patch(void);

extern void rtk_l2testmode_on();
extern void rtk_l2testmode_off();

#endif	/* __RTL8328_RTK_H__ */


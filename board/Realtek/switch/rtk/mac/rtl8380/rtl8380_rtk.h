/*
 * Copyright(c) Realtek Semiconductor Corporation, 2012
 * All rights reserved.
 *
 * Purpose : Related definition of the RTL8328 driver for RTK command.
 *
 * Feature : RTL8328 driver for RTK command
 *
 */

#ifndef	__RTL8380_RTK_H__
#define	__RTL8380_RTK_H__

#include <rtk_switch.h>

#ifdef CONFIG_EEE
#if (defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B) || defined(CONFIG_RTL8212B))
#include <rtk/phy/conf/conftypes.h>
#endif
#endif


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

void rtk_phyPortPowerOn(int mac_idx);

extern void rtk_linkdown_powersaving_patch(void);

extern void rtk_port_isolation_on(void);
extern void rtk_port_isolation_off(void);

extern void rtk_phy_selfLoop_on(int portId);
extern void rtk_phy_selfLoop_off(int portId);
extern int32 rtk_portLinkStatus_get(int portId);
extern void rtk_smiRead(uint32 phyad, uint32 regad, uint32* pData);
extern void rtk_smiWrite(uint32 phyad, uint32 regad, uint32 data);
extern void rtk_sys_led_on(void);
extern void rtk_sys_led_off(void);
extern void rtk_sys_led_blink(void);

extern void rtk_l2testmode_on(void);
extern void rtk_l2testmode_off(void);

extern void rtk_mac_polling_enable(int port);
extern void rtk_mac_polling_disable(int port);
extern void rtk_serdes_fiber_watchdog(int port);

extern void rtk_fiberRxWatchdog(void);

extern void rtk_portIsolation(int srcPort,int destPort);
extern void rtk_portIsolationCPUgoto(int port);
extern void rtk_portIsolationToCPU(int port);
extern void rtk_saLearning(int state);
extern int rtk_portlink_get(int unit,int port,int *link);


#endif	/* __RTL8380_RTK_H__ */


/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * Purpose : Related implementation of the RTL8214 PHY driver.
 *
 * Feature : RTL8214 PHY driver
 *
 */

#ifndef __RTL8214FC_H__
#define __RTL8214FC_H__

/*
 * Include Files
 */

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */

/*
 * Macro Definition
 */

/*
 * Function Declaration
 */
#if defined(CONFIG_RTL8390)
extern void rtl8214fc_rtl8390_config(Tuint8 macId, Tuint8 portNum);
#endif
extern void rtl8214fc_phyPowerOff(int macId);
extern void rtl8214fc_rtl8380_config(Tuint8 phyid);

extern void rtk_8214fc_dumpTop(Tuint8 macId);

#endif  /* __RTL8214FC_H__ */


/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * Purpose : Related implementation of the RTL8218 PHY driver.
 *
 * Feature : RTL8218 PHY driver
 *
 */

#ifndef	__RTL8218B_H__
#define	__RTL8218B_H__

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
extern void rtl8218b_rtl8390_config(Tuint8 macId);
extern void rtl8218fb_rtl8390_config(Tuint8 macId);
extern void rtl8218b_rtl8380_config(Tuint8 phyid);
extern void rtl8218fb_rtl8380_config(Tuint8 phyid);
extern void rtl8218b_phyPowerOff(int macId);
extern void rtl8218fb_media_set(int portid, int media);

extern void rtl8214fc_media_set(int portid, int media);
extern void rtl8214fc_media_get(int portid, int *media);

extern void rtl8214fc_fiber_watchdog(int port);

#endif	/* __RTL8218B_H__ */


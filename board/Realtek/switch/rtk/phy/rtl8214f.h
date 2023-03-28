/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * Purpose : Related implementation of the RTL8214F PHY driver.
 *
 * Feature : RTL8214F PHY driver
 *
 */

#ifndef	__RTL8214F_H__
#define	__RTL8214F_H__

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
extern void rtl8214f_media_set(int portid, int media);
extern void rtl8214f_media_get(int portid, int *media);
extern void rtl8214fb_media_set(int portid, int media);
extern void rtl8214fb_media_get(int portid, int *media);
extern void rtl8214f_phyPowerOn(int portid);
extern void rtl8214f_phyPowerOff(int portid);
extern void rtl8214f_config(Tuint8 macId);
extern void rtl8214fb_phyPowerOn(int portid);
extern void rtl8214fb_phyPowerOff(int portid);
extern void rtl8214fb_config(Tuint8 macId);
extern void rtl8328_rtl8214f_config(Tuint8 macId);

#endif	/* __RTL8214F_H__ */

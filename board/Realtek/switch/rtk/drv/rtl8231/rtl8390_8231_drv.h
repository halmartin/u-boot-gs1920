/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 30020 $
 * $Date: 2012-06-18 15:09:58 +0800 (Mon, 18 Jun 2012) $
 *
 * Purpose : Definition those public APIs to acess RTL8231 from RTL8390 family.
 *
 * Feature : The file have include the following module and sub-modules
 *            1) mdc read & write
 */

#ifndef	__RTL8390_RTL8231_DRV_H__
#define	__RTL8390_RTL8231_DRV_H__

#include <rtk_switch.h>

extern void rtl8390_rtl8231_init(void);
extern int rtl8390_rtl8231_read(uint32 phy_id, uint32 reg_addr, uint32 *pData);
extern int rtl8390_rtl8231_write(uint32 phy_id, uint32 reg_addr, uint32 data);
#endif	/* __RTL8390_RTL8231_DRV_H__ */


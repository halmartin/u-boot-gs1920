/*
 * Copyright(c) Realtek Semiconductor Corporation, 2012
 * All rights reserved.
 *
 * Purpose :
 *
 * Feature :
 *
 */

#ifndef __RTL8390_INIT_H__
#define __RTL8390_INIT_H__

/*
 * Include Files
 */
#include <rtk_type.h>
#include <rtk_switch.h>


/*
 * Function Declaration
 */
extern void rtl8390_config(const rtk_switch_model_t *pModel);

void rtl8390_sfp_speed_set(int port, int speed);

#endif  /* __RTL8390_INIT_H__ */


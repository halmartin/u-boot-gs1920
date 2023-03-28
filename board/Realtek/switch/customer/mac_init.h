/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * Purpose : Related definition of the customer MAC init for U-Boot.
 *
 * Feature : customer MAC init function
 *
 */

#ifndef __CUSTOMER_MAC_INIT_H__
#define __CUSTOMER_MAC_INIT_H__

/*
 * Include Files
 */
#include <rtk_type.h>
#include <rtk_switch.h>


/*
 * Function Declaration
 */
extern void customer_mac_config_init(const rtk_switch_model_t *pModel);
extern void customer_phy_config_init(const rtk_switch_model_t *pModel);
extern void customer_mac_misc_config_init(const rtk_switch_model_t *pModel);

#endif  /* __CUSTOMER_MAC_INIT_H__ */


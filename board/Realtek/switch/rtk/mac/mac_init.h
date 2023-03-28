/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * Purpose : Related definition of the MAC init for U-Boot.
 *
 * Feature : MAC init function
 *
 */

#ifndef __RTK_MAC_INIT_H__
#define __RTK_MAC_INIT_H__

/*
 * Include Files
 */
#include <rtk_type.h>
#include <rtk_switch.h>


/*
 * Function Declaration
 */

/* Function Name:
 *      mac_drv_init
 * Description:
 *      Initial the mac driver
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      0 - Success
 * Note:
 *      None
 */
extern int mac_drv_init(int chip_index, rtk_mac_drv_t **ppMacDrv);

/* Function Name:
 *      chip_config
 * Description:
 *      Config code of the chip.
 * Input:
 *      pModel - Switch model information
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
extern void chip_config(const rtk_switch_model_t *pModel);

#endif  /* __RTK_MAC_INIT_H__ */


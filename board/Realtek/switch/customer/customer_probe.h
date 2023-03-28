/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * Purpose : Related definition of the customer board probe for U-Boot.
 *
 * Feature : customer board probe function
 *
 */

#ifndef	__CUSTOMER_PROBE_H__
#define	__CUSTOMER_PROBE_H__

/*
 * Include Files
 */
#include <rtk_switch.h>


/*
 * Function Declaration
 */

/* Function Name:
 *      customer_board_probe
 * Description:
 *      Probe the customer board
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      0 - Success
 * Note:
 *      None
 */
extern int customer_board_probe(rtk_switch_model_t **pSwitchModel);

#endif	/* __CUSTOMER_PROBE_H__ */

/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * Purpose : Related definition of the RTK board probe for U-Boot.
 *
 * Feature : RTK board probe function
 *
 */

#ifndef	__RTK_PROBE_H__
#define	__RTK_PROBE_H__

/*
 * Include Files
 */
#include <rtk_switch.h>


/*
 * Function Declaration
 */

/* Function Name:
 *      rtk_board_probe
 * Description:
 *      Probe the rtk board
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      0 - Success
 * Note:
 *      None
 */
extern int rtk_board_probe(rtk_switch_model_t **pSwitchModel);

#endif	/* __RTK_PROBE_H__ */

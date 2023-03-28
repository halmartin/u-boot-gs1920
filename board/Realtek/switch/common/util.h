/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * Purpose : Related definition of switch util for U-Boot.
 *
 * Feature : switch util function
 *
 */

#ifndef __UTIL_H__
#define __UTIL_H__

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

/* Function Name:
 *      rtk_portIdxFromMacId
 * Description:
 *      Get PHY index from MAC port id
 * Input:
 *      macId   - MAC port id
 * Output:
 *      portIdx - port index relates MAC port id
 * Return:
 *      None
 * Note:
 *      None
 */
int rtk_portIdxFromMacId(int macId, int *portIdx);

#endif  /* __UTIL_H__ */

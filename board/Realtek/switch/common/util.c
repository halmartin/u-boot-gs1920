/*
 * Copyright(c) Realtek Semiconductor Corporation, 2013
 * All rights reserved.
 *
 * Purpose : Related switch common util.
 *
 * Feature : switch util function
 *
 */


/*
 * Include Files
 */
#include <rtk_switch.h>

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */
extern const rtk_switch_model_t *gSwitchModel;
extern const rtk_mac_drv_t *gMacDrv;

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
int rtk_portIdxFromMacId(int macId, int *portIdx)
{
    int idx;

    if (gSwitchModel == NULL)
        return -1;

    if (gMacDrv == NULL)
        return -1;

    for (idx = 0; idx < gSwitchModel->port.count; ++idx)
    {
        if (gSwitchModel->port.list[idx].mac_id == macId)
        {
            *portIdx = idx;
            return 0;
        }
    }

    return -1;
}

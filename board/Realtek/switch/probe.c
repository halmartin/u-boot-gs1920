/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * Purpose : Related definition of the board probe for U-Boot.
 *
 * Feature : board probe function
 *
 */


/*
 * Include Files
 */
#include <config.h>
#include <rtk_switch.h>
#if defined(CONFIG_RTK_BOARD)
#include <rtk/rtk_probe.h>
#endif
#if defined(CONFIG_CUSTOMER_BOARD)
#include <customer/customer_probe.h>
#endif

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */
int gSysBoardModel;

/*
 * Macro Definition
 */

/*
 * Function Declaration
 */

/* Function Name:
 *      board_probe
 * Description:
 *      Probe the board
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      0 - Success
 * Note:
 *      None
 */
int board_probe(rtk_switch_model_t **pSwitchModel)
{
#if defined(CONFIG_CUSTOMER_BOARD)
    customer_board_probe(pSwitchModel);
    if (*pSwitchModel != NULL)
        return 0;
#endif

#if defined(CONFIG_RTK_BOARD)
    rtk_board_probe(pSwitchModel);

    if (*pSwitchModel != NULL)
        return 0;
#endif

    return 0;
} /* end of board_probe */

int board_model_set(int modelId)
{
    gSysBoardModel = modelId;

    return 0;
}

int board_model_get(int *modelId)
{
    *modelId = gSysBoardModel;

    return 0;
}


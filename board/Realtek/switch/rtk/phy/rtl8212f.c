/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * Purpose : Related implementation of the RTL8212F PHY driver.
 *
 * Feature : RTL8212F PHY driver
 *
 */


/*
 * Include Files
 */
#include <rtk_switch.h>
#include <rtk_osal.h>
#include <rtk_debug.h>
//#include <rtk/phy/conf/conftypes.h>
//#include <rtk/phy/conf/conf_rtl8214fb.c>
#include "rtl8328_asicregs.h"

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
 *      rtl8328_rtl8212f_config
 * Description:
 *      Configuration code for RTL8212F for RTL8328.
 * Input:
 *      phyId - the phyid of PHY0 of the RTL8212F
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8328_rtl8212f_config(void)
{
    MEM32_WRITE(0xBB010000, 0x6002B807);
    MEM32_WRITE(0xBB010008, 0x13CCD80);
    MEM32_WRITE(0xBB040014, 0x3FFFFFF);    

    gMacDrv->drv_miim_write(24, 14, 17, 0x1200);
    gMacDrv->drv_miim_write(25, 14, 17, 0x8F00);

    return;
} /* end of rtl8328_rtl8212f_config */


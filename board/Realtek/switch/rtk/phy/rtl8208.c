/*
 * Copyright(c) Realtek Semiconductor Corporation, 2012
 * All rights reserved.
 *
 * Purpose : Related implementation of the RTL8208 PHY driver.
 *
 * Feature : RTL8208 PHY driver
 *
 */


/*
 * Include Files
 */
#include <rtk_switch.h>
#include <rtk_osal.h>
#include <rtk_debug.h>
#include <rtk/phy/conf/conftypes.h>
#include <rtk/phy/conf/conf_rtl8208.c>

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */
extern const rtk_mac_drv_t *gMacDrv;

/*
 * Macro Definition
 */
#ifndef VALUE_CHG
#define VALUE_CHG(var,mask,val)     do { var = ((var & ~(mask)) | ((val) & (mask))); } while(0)
#endif

#define RTL8208_CHIP_ID	0x8

/*
 * Function Declaration
 */

/*
 * Function Declaration
 */
/* Function Name:
 *      rtl8218_config
 * Description:
 *      Configuration code for RTL8218.
 * Input:
 *      phyId - the phyid of PHY0 of the RTL8218
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8208_config(Tuint8 phyid)
{
    int rtl8208_phy0_macid = phyid;
    unsigned int val;
    int i, port_id;

	/*Check the 8208 internal chip ID*/
    /*Page0 Reg3   dft:0xc882  bit[9:4] modle number:0x8  [3:0] revision number:0x2 */
	gMacDrv->drv_miim_read(phyid, 0, 3, &val);
    val = (val >> 4) & 0x3f;

	if(val != RTL8208_CHIP_ID)
    {
		OSAL_PRINTF("### Error chip ID = 0x%08x ###\n", val);
		return;
	}

    OSAL_PRINTF("### RTL8208 config - MAC ID = %d ###\n", rtl8208_phy0_macid);

    /* Per-Chip */
    for (i=0; i<(sizeof(rtl8208_perchip)/sizeof(confcode_prv_t)); i++)
    {
        gMacDrv->drv_miim_write(rtl8208_phy0_macid + rtl8208_perchip[i].phy, \
        gMacDrv->miim_max_page, (int)rtl8208_perchip[i].reg, rtl8208_perchip[i].val);
    }

    /* Per-Port */
    for(port_id = 0; port_id < 8; port_id++)
    {
        for (i=0; i<(sizeof(rtl8208_perport)/sizeof(confcode_rv_t)); i++)
        {
            gMacDrv->drv_miim_write(rtl8208_phy0_macid + port_id, \
            gMacDrv->miim_max_page, (int)rtl8208_perport[i].reg, rtl8208_perport[i].val);
        }
    }

    return;
} /* end of rtl8208_config */

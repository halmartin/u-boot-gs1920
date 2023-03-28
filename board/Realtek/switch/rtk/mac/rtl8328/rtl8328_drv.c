/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * Purpose : Related implementation of the RTL8328 board for U-Boot.
 *
 * Feature : RTL8328 platform
 *
 */


/*
 * Include Files
 */
#include <rtk_osal.h>
#include <rtk_switch.h>
#include <rtk_debug.h>
#include <init.h>
#include "rtl8328_asicregs.h"

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
 *      rtl8328_setPhyReg
 * Description:
 *      Set PHY register.
 * Input:
 *      portid - Port number (0~28)
 *      page   - PHY page (0~127)
 *      reg    - PHY register (0~31)
 *      val    - Read data
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8328_setPhyReg(int portid, int page, int reg, unsigned int val)
{
    unsigned int value;
    unsigned int try = 200000;

    /* check parameter range */
    portid += (gSwitchModel->port.offset);
    portid = (portid > 28)? 28 : portid;
    page &= 0x7F;
    reg &= 0x1F;
    val &= 0xFFFF;

    /* Make sure indirect access PHY is idle */
    while (MEM32_READ(0xbb040008) & 1)
    {
        if ((try--) == 0)
        {
            return; /* FAILED */
        }
    }

    /* Indirect access PHY register with MAC */
    MEM32_WRITE(0xbb040010, val);
    value = (portid << 27) | (reg << 22) | (page << 15) | 0x3;  /* Write + CMD */
    MEM32_WRITE(0xbb040008, value); /* Write + CMD */

    /* Busy waiting */
    while ((MEM32_READ(0xbb040008) & 0x1) == 0x1);
} /* end of rtl8328_setPhyReg */

/* Function Name:
 *      rtl8328_getPhyReg
 * Description:
 *      Get PHY register.
 * Input:
 *      portid - Port number (0~28)
 *      page   - PHY page (0~127)
 *      reg    - PHY register (0~31)
 *      val    - Read data
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8328_getPhyReg(int portid, int page, int reg, unsigned int *val)
{
    unsigned int value;
    unsigned int try = 200000;

    /* check parameter range */
    portid += (gSwitchModel->port.offset);
    portid &= 0x1F;
    page &= 0x7F;
    reg &= 0x1F;

    /* Make sure indirect access PHY is idle */
    while (MEM32_READ(0xbb040008) & 1)
    {
        if ((try--) == 0)
        {
            return; /* FAILED */
        }
    }

    value = (portid << 27) | (reg << 22) | (page << 15) | 0x7;  /* Read + CMD */
    MEM32_WRITE( 0xbb040008, value);

    /* Busy waiting */
    while ((MEM32_READ(0xbb040008) & 0x1) == 0x1);

    *val = MEM32_READ(0xbb040010) & 0xFFFF;
} /* end of rtl8328_getPhyReg */

/* Function Name:
 *      rtl8328_setPhyRegByMask
 * Description:
 *      Set PHY register by portmask.
 * Input:
 *      port_mask - Port mask (0~28)
 *      page      - PHY page (0~127)
 *      reg       - PHY register (0~31)
 *      val       - Read data
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8328_setPhyRegByMask(unsigned long long port_mask, int page, int reg, unsigned int val)
{
    unsigned int value;
    unsigned int try = 200000;

    /* check parameter range */
    port_mask &= 0x0FFFFFFF;
    page &= 0x7F;
    reg &= 0x1F;
    val &= 0xFFFF;

    /* Make sure indirect access PHY is idle */
    while (MEM32_READ(0xbb040008) & 1)
    {
        if ((try--) == 0)
        {
            return; /* FAILED */
        }
    }

    /* Indirect access PHY register with MAC */
    MEM32_WRITE(0xbb040010, val);
    MEM32_WRITE(0xbb04000C, port_mask);
    value = (reg << 22) | (page << 15) | 0xB;   /* Write Portmask + CMD */
    MEM32_WRITE(0xbb040008, value);

    /* Busy waiting */
    while ((REG32(0xbb040008) & 0x1) == 0x1);
} /* end of rtl8328_setPhyRegByMask */

/* Function Name:
 *      rtl8328_phyPowerOn
 * Description:
 *      Power-On PHY.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8328_phyPowerOn(void)
{
    int i;
    int phy_idx;
    int val;

    if (gSwitchModel == NULL)
        return;

    if (gMacDrv == NULL)
        return;

    for (i=0; i<gSwitchModel->port.count; i++)
    {   /* power-on all ports */
        phy_idx = gSwitchModel->port.list[i].phy_idx;

        switch (gSwitchModel->phy.list[phy_idx].chip)
        {
        case RTK_CHIP_RTL8214F:
            rtl8214f_phyPowerOn(i);
            break;

        case RTK_CHIP_RTL8214FB:
        case RTK_CHIP_RTL8214B:
        case RTK_CHIP_RTL8212B:
            rtl8214fb_phyPowerOn(i);
            break;

        case RTK_CHIP_RTL8208:
        case RTK_CHIP_RTL8208D:
        case RTK_CHIP_RTL8214:
        default:
            gMacDrv->drv_miim_read(i, 0, 0, &val);
            gMacDrv->drv_miim_write(i, 0, 0, val & ~(0x1 << 11));
            break;
        }
    }
    return;
} /* end of rtl8328_phyPowerOn */

/* Function Name:
 *      rtl8328_phyPowerOff
 * Description:
 *      Power-Off PHY.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8328_phyPowerOff(void)
{
    int i;
    int phy_idx;
    int val;

    if (gSwitchModel == NULL)
        return;

    if (gMacDrv == NULL)
        return;

    for (i=0; i<gSwitchModel->port.count; i++)
    {   /* power-off all ports */
        phy_idx = gSwitchModel->port.list[i].phy_idx;

        switch (gSwitchModel->phy.list[phy_idx].chip)
        {
        case RTK_CHIP_RTL8214F:
            rtl8214f_phyPowerOff(i);
            break;

        case RTK_CHIP_RTL8214FB:
        case RTK_CHIP_RTL8214B:
        case RTK_CHIP_RTL8212B:
            rtl8214fb_phyPowerOff(i);
            break;

        case RTK_CHIP_RTL8208:
        case RTK_CHIP_RTL8208D:
        case RTK_CHIP_RTL8214:
        default:
            gMacDrv->drv_miim_read(i, 0, 0, &val);
            gMacDrv->drv_miim_write(i, 0, 0, val | (0x1 << 11));
            break;
        }
    }
    return;
} /* end of rtl8328_phyPowerOff */

/* Function Name:
 *      rtl8328_phyReset
 * Description:
 *      Reset PHY through GPIOF.
 * Input:
 *      pModel - pointer to switch model of platform
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8328_phyReset(const rtk_switch_model_t *pModel)
{
    return;
} /* end of rtl8328_phyReset */

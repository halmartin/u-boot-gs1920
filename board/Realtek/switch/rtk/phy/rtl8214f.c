/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * Purpose : Related implementation of the RTL8214F PHY driver.
 *
 * Feature : RTL8214F PHY driver
 *
 */


/*
 * Include Files
 */
#include <rtk_switch.h>
#include <rtk_osal.h>
#include <rtk_debug.h>
#include <common/util.h>
#include <rtk/phy/conf/conftypes.h>
#include <rtk/phy/conf/conf_rtl8214fb.c>
#include <configs/autoconf.h>


/*
 * Symbol Definition
 */
#define MEDIATYPE_COPPER    (0)
#define MEDIATYPE_FIBER     (1)
#define MEDIATYPE_COPPER_AUTO   (2)
#define MEDIATYPE_FIBER_AUTO    (3)

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
 *      rtl8214f_media_set
 * Description:
 *      Set meida of port.
 * Input:
 *      portid - Port number (0~28)
 *      media  - Media
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8214f_media_set(int portid, int media)
{
    unsigned int _RTL8214F_Phy0Id;
    int phy_idx, portIdx;
    unsigned int val;
    int power = 0;
    unsigned int oldVal;

    if (gSwitchModel == NULL)
        return;

    if (gMacDrv == NULL)
        return;

    if ((media != MEDIATYPE_COPPER) && (media != MEDIATYPE_FIBER))
        return;

    if (rtk_portIdxFromMacId(portid, &portIdx) < 0)
        return;

    phy_idx = gSwitchModel->port.list[portIdx].phy_idx;
    if (gSwitchModel->phy.list[phy_idx].chip != RTK_CHIP_RTL8214F)
        return;

    _RTL8214F_Phy0Id = gSwitchModel->phy.list[phy_idx].mac_id;

    switch (media) {
    case MEDIATYPE_COPPER:
        gMacDrv->drv_miim_read(_RTL8214F_Phy0Id + 1, 8, 16, &val);
        if ((val & (0x1000 << (portid - _RTL8214F_Phy0Id))) == 0x0000) {
            return;
        }
        gMacDrv->drv_miim_read(portid, 9, 24, &val);
        if ((val & (0x3 << 2)) == (0x2 << 2)) {
            power = 1;
            val |= (0x1 << 2);
            gMacDrv->drv_miim_write(portid, 9, 24, val);

            gMacDrv->drv_miim_read(portid, 0, 0, &oldVal);
            gMacDrv->drv_miim_write(portid, 0, 0, oldVal | 0x1200);

            val &= ~(0x1 << 3);
            gMacDrv->drv_miim_write(portid, 9, 24, val);

            gMacDrv->drv_miim_write(portid, 0, 0, oldVal);
        }
        gMacDrv->drv_miim_read(_RTL8214F_Phy0Id + 1, 8, 16, &val);
        val &= ~(0x1000 << (portid - _RTL8214F_Phy0Id));
        gMacDrv->drv_miim_write(_RTL8214F_Phy0Id + 1, 8, 16, val);
        if (power) {
            gMacDrv->drv_miim_read(portid, 0, 0, &val);
            val &= ~(0x1 << 11);
            gMacDrv->drv_miim_write(portid, 0, 0, val);
        }
        break;

    case MEDIATYPE_FIBER:
        gMacDrv->drv_miim_read(_RTL8214F_Phy0Id + 1, 8, 16, &val);
        if ((val & (0x1000 << (portid - _RTL8214F_Phy0Id))) != 0x0000) {
            return;
        }
        gMacDrv->drv_miim_read(portid, 0, 0, &val);
        if ((val & (0x1 << 11)) == 0x0) {
            power = 1;
            val |= (0x1 << 11);
            gMacDrv->drv_miim_write(portid, 0, 0, val);
        }
        gMacDrv->drv_miim_read(_RTL8214F_Phy0Id + 1, 8, 16, &val);
        val |= (0x1000 << (portid - _RTL8214F_Phy0Id));
        gMacDrv->drv_miim_write(_RTL8214F_Phy0Id + 1, 8, 16, val);
        if (power) {
            gMacDrv->drv_miim_read(portid, 9, 24, &val);
            val |= (0x1 << 3);
            val &= ~(0x1 << 2);
            gMacDrv->drv_miim_write(portid, 9, 24, val);
        }
        break;

    default:
        return;
    }
} /* end of rtl8214f_media_set */

/* Function Name:
 *      rtl8214f_media_get
 * Description:
 *      Get meida of port.
 * Input:
 *      portid - Port number (0~28)
 *      media  - Media
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8214f_media_get(int portid, int *media)
{
    unsigned int _RTL8214F_Phy0Id;
    int phy_idx, portIdx;
    unsigned int val;

    if (gSwitchModel == NULL)
        return;

    if (gMacDrv == NULL)
        return;

    if (NULL == media)
        return;

    if (rtk_portIdxFromMacId(portid, &portIdx) < 0)
        return;

    phy_idx = gSwitchModel->port.list[portIdx].phy_idx;
    if (gSwitchModel->phy.list[phy_idx].chip != RTK_CHIP_RTL8214F) {
        *media = MEDIATYPE_COPPER;
        return;
    }

    _RTL8214F_Phy0Id = gSwitchModel->phy.list[phy_idx].mac_id;

    gMacDrv->drv_miim_read(_RTL8214F_Phy0Id + 1, 8, 16, &val);
    if ((val & (0x1000 << (portid - _RTL8214F_Phy0Id))) == 0x0000) {
        /* copper mode */
        *media = MEDIATYPE_COPPER;
    } else {
        /* fiber mode */
        *media = MEDIATYPE_FIBER;
    }
    return;
} /* end of rtl8214f_media_get */

/* Function Name:
 *      rtl8214fb_media_set
 * Description:
 *      Set meida of port.
 * Input:
 *      portid - Port number (0~28)
 *      media  - Media
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8214fb_media_set(int portid, int media)
{
    unsigned int phyReg0, is_phyDown;
    int phy_idx, portIdx;
    unsigned int val;

    if (gSwitchModel == NULL)
        return;

    if (gMacDrv == NULL)
        return;

    if ((media != MEDIATYPE_COPPER) && (media != MEDIATYPE_FIBER) &&
        (media != MEDIATYPE_COPPER_AUTO) && (media != MEDIATYPE_FIBER_AUTO))
        return;

    if (rtk_portIdxFromMacId(portid, &portIdx) < 0)
        return;

    phy_idx = gSwitchModel->port.list[portIdx].phy_idx;
    if (gSwitchModel->phy.list[phy_idx].chip != RTK_CHIP_RTL8214FB &&
        gSwitchModel->phy.list[phy_idx].chip != RTK_CHIP_RTL8214B &&
        gSwitchModel->phy.list[phy_idx].chip != RTK_CHIP_RTL8212B)
        return;

    switch (media) {
    case MEDIATYPE_COPPER:
        gMacDrv->drv_miim_read(portid, 8, 30, &val);
        if ((((val >> 13) & 0x1) == 1) && (((val >> 12) & 0x1) == 0))
        {
            /* no change media */
            return;
        }
        /* If the original media is PORT_MEDIA_FIBER, need to apply
         * reg0.bit11 power down state from copper to fiber
         */
        if ((((val >> 13) & 0x1) == 1) && (((val >> 12) & 0x1) == 1))
        {
            gMacDrv->drv_miim_read(portid, 0, 0, &phyReg0);
            is_phyDown = (phyReg0 >> 11) & 0x1;

            /* turn-on copper */
            val &= ~(1<<12); /* select UTP */
            val |= (0x2000); /* disable auto-sensing */
            gMacDrv->drv_miim_write(portid, 8, 30, val);
            gMacDrv->drv_miim_read(portid, 0, 0, &phyReg0);
            phyReg0 &= 0xF7FF;
            phyReg0 |= (is_phyDown << 11);
            gMacDrv->drv_miim_write(portid, 0, 0, phyReg0);
        }
        else
        {
            /* turn-on copper */
            val &= ~(1<<12); /* select UTP */
            val |= (0x2000); /* disable auto-sensing */
            gMacDrv->drv_miim_write(portid, 8, 30, val);
        }
        /* Force pattern */
        gMacDrv->drv_miim_write(portid, 8, 31, 0x000F);
        gMacDrv->drv_miim_write(portid,15, 30, 0x001C);
        gMacDrv->drv_miim_write(portid,15, 21, 0x88D2);
        gMacDrv->drv_miim_write(portid,15, 31, 0x0008);
        /* force power */
        gMacDrv->drv_miim_write(portid, 8, 31, 0x000F);
        gMacDrv->drv_miim_write(portid,15, 30, 0x001E);
        gMacDrv->drv_miim_write(portid,15, 21, 0xC129);
        gMacDrv->drv_miim_write(portid,15, 31, 0x0008);
        break;

    case MEDIATYPE_FIBER:
        gMacDrv->drv_miim_read(portid, 8, 30, &val);
        if ((((val >> 13) & 0x1) == 1) && (((val >> 12) & 0x1) == 1))
        {
            /* no change media */
            return;
        }
        /* If the original media is PORT_MEDIA_COPPER, need to apply
         * reg0.bit11 power down state from copper to fiber
         */
        if ((((val >> 13) & 0x1) == 1) && (((val >> 12) & 0x1) == 0))
        {
            gMacDrv->drv_miim_read(portid, 0, 0, &phyReg0);
            is_phyDown = (phyReg0 >> 11) & 0x1;

            /* turn-on fiber */
            val |= (0x3000); /* disable auto-sensing + select FIBER */
            gMacDrv->drv_miim_write(portid, 8, 30, val);
            gMacDrv->drv_miim_read(portid, 0, 0, &phyReg0);
            phyReg0 &= 0xF7FF;
            phyReg0 |= (is_phyDown << 11);
            gMacDrv->drv_miim_write(portid, 0, 0, phyReg0);
        }
        else
        {
            /* turn-on fiber */
            val |= (0x3000); /* disable auto-sensing + select FIBER */
            gMacDrv->drv_miim_write(portid, 8, 30, val);
        }
        /* Default setting */
        gMacDrv->drv_miim_write(portid, 8, 31, 0x000F);
        gMacDrv->drv_miim_write(portid,15, 30, 0x001C);
        gMacDrv->drv_miim_write(portid,15, 21, 0x88D0);
        gMacDrv->drv_miim_write(portid,15, 31, 0x0008);
        /* Default setting */
        gMacDrv->drv_miim_write(portid, 8, 31, 0x000F);
        gMacDrv->drv_miim_write(portid,15, 30, 0x001E);
        gMacDrv->drv_miim_write(portid,15, 21, 0xC178);
        gMacDrv->drv_miim_write(portid,15, 31, 0x0008);
        break;

    case MEDIATYPE_COPPER_AUTO:
        gMacDrv->drv_miim_read(portid, 8, 30, &val);
        if ((((val >> 13) & 0x1) == 0) && (((val >> 5) & 0x1) == 1))
        {
            /* no change media */
            return;
        }
        /* turn-on copper */
        val &= ~(1<<13); /* enable auto-sensing */
        val |= (1<<5); /* preferred copper */
        gMacDrv->drv_miim_write(portid, 8, 30, val);
        /* Default setting */
        gMacDrv->drv_miim_write(portid, 8, 31, 0x000F);
        gMacDrv->drv_miim_write(portid,15, 30, 0x001C);
        gMacDrv->drv_miim_write(portid,15, 21, 0x88D0);
        gMacDrv->drv_miim_write(portid,15, 31, 0x0008);
        /* Default setting */
        gMacDrv->drv_miim_write(portid, 8, 31, 0x000F);
        gMacDrv->drv_miim_write(portid,15, 30, 0x001E);
        gMacDrv->drv_miim_write(portid,15, 21, 0xC178);
        gMacDrv->drv_miim_write(portid,15, 31, 0x0008);
        break;

    case MEDIATYPE_FIBER_AUTO:
        gMacDrv->drv_miim_read(portid, 8, 30, &val);
        if ((((val >> 13) & 0x1) == 0) && (((val >> 5) & 0x1) == 0))
        {
            /* no change media */
            return;
        }
        /* turn-on copper */
        val &= ~(1<<13); /* enable auto-sensing */
        val &= ~(1<<5); /* preferred fiber */
        gMacDrv->drv_miim_write(portid, 8, 30, val);
        /* Default setting */
        gMacDrv->drv_miim_write(portid, 8, 31, 0x000F);
        gMacDrv->drv_miim_write(portid,15, 30, 0x001C);
        gMacDrv->drv_miim_write(portid,15, 21, 0x88D0);
        gMacDrv->drv_miim_write(portid,15, 31, 0x0008);
        /* Default setting */
        gMacDrv->drv_miim_write(portid, 8, 31, 0x000F);
        gMacDrv->drv_miim_write(portid,15, 30, 0x001E);
        gMacDrv->drv_miim_write(portid,15, 21, 0xC178);
        gMacDrv->drv_miim_write(portid,15, 31, 0x0008);
        break;

    default:
        return;
    }
} /* end of rtl8214fb_media_set */

/* Function Name:
 *      rtl8214fb_media_get
 * Description:
 *      Get meida of port.
 * Input:
 *      portid - Port number (0~28)
 *      media  - Media
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8214fb_media_get(int portid, int *media)
{
    int phy_idx, portIdx;
    unsigned int val;

    if (gSwitchModel == NULL)
        return;

    if (gMacDrv == NULL)
        return;

    if (NULL == media)
        return;

    if (rtk_portIdxFromMacId(portid, &portIdx) < 0)
        return;

    phy_idx = gSwitchModel->port.list[portIdx].phy_idx;
    if (gSwitchModel->phy.list[phy_idx].chip != RTK_CHIP_RTL8214FB &&
        gSwitchModel->phy.list[phy_idx].chip != RTK_CHIP_RTL8214B &&
        gSwitchModel->phy.list[phy_idx].chip != RTK_CHIP_RTL8212B)
    {
        *media = MEDIATYPE_COPPER;
        return;
    }

    gMacDrv->drv_miim_read(portid, 8, 30, &val);
    if (0x2000 == (val & 0x2000))
    {
        if (0x1000 == (val & 0x1000))
            *media = MEDIATYPE_FIBER;
        else
            *media = MEDIATYPE_COPPER;
    }
    else
    {
        if (0x0020 == (val & 0x0020))
            *media = MEDIATYPE_COPPER_AUTO;
        else
            *media = MEDIATYPE_FIBER_AUTO;
    }

    return;
} /* end of rtl8214fb_media_get */

/* Function Name:
 *      rtl8214f_phyPowerOn
 * Description:
 *      Get meida of port.
 * Input:
 *      portid - Port number (0~28)
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8214f_phyPowerOn(int portid)
{
    unsigned int _RTL8214F_Phy0Id;
    int phy_idx;
    unsigned int val;

    if (gSwitchModel == NULL)
        return;

    if (gMacDrv == NULL)
        return;

    if (portid >= gSwitchModel->port.count)
        return;

    phy_idx = gSwitchModel->port.list[portid].phy_idx;
    if (gSwitchModel->phy.list[phy_idx].chip != RTK_CHIP_RTL8214F)
        return;

    _RTL8214F_Phy0Id = portid - gSwitchModel->port.list[portid].phy;

    gMacDrv->drv_miim_read(_RTL8214F_Phy0Id + 1, 8, 16, &val);
    if ((val & (0x1000 << (portid - _RTL8214F_Phy0Id))) == 0x0000) {
        /* copper mode */
        gMacDrv->drv_miim_read(portid, 0, 0, &val);
        val &= ~(0x1 << 11);
        gMacDrv->drv_miim_write(portid, 0, 0, val);
    } else {
        /* fiber mode */
        gMacDrv->drv_miim_read(portid, 9, 24, &val);
        val |= (0x1 << 3);
        val &= ~(0x1 << 2);
        gMacDrv->drv_miim_write(portid, 9, 24, val);
    }

    return;
} /* end of rtl8214f_phyPowerOn */


/* Function Name:
 *      rtl8214f_phyPowerOff
 * Description:
 *      Get meida of port.
 * Input:
 *      portid - Port number (0~28)
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8214f_phyPowerOff(int portid)
{
    unsigned int _RTL8214F_Phy0Id;
    int phy_idx;
    unsigned int forceReg;
    unsigned int val;

    if (gSwitchModel == NULL)
        return;

    if (gMacDrv == NULL)
        return;

    if (portid >= gSwitchModel->port.count)
        return;

    phy_idx = gSwitchModel->port.list[portid].phy_idx;
    if (gSwitchModel->phy.list[phy_idx].chip != RTK_CHIP_RTL8214F)
        return;

    _RTL8214F_Phy0Id = portid - gSwitchModel->port.list[portid].phy;
    gMacDrv->drv_miim_read(_RTL8214F_Phy0Id + 1, 8, 16, &forceReg);      /* store the original register value */
    gMacDrv->drv_miim_write(_RTL8214F_Phy0Id + 1, 8, 16, (forceReg | 0x0f00) & ~(0xf000));
    gMacDrv->drv_miim_read(_RTL8214F_Phy0Id + 1, 8, 16, &val);
    gMacDrv->drv_miim_write(_RTL8214F_Phy0Id + 1, 8, 16, (val & ~(0x1000 << (gSwitchModel->port.list[portid].phy))));
    gMacDrv->drv_miim_read(portid + gSwitchModel->port.offset, 0, 0, &val);
    gMacDrv->drv_miim_write(portid + gSwitchModel->port.offset, 0, 0, (val | (0x1 << 11)));
    gMacDrv->drv_miim_read(_RTL8214F_Phy0Id + 1, 8, 16, &val);
    gMacDrv->drv_miim_write(_RTL8214F_Phy0Id + 1, 8, 16, (val | (0x1000 << (gSwitchModel->port.list[portid].phy))));
    gMacDrv->drv_miim_read(portid + gSwitchModel->port.offset, 0, 0, &val);
    gMacDrv->drv_miim_write(portid + gSwitchModel->port.offset, 0, 0, (val | (0x1 << 12) | (0x1 << 9)));
    gMacDrv->drv_miim_write(portid + gSwitchModel->port.offset, 0, 0, (val | (0x1 << 11)));
    gMacDrv->drv_miim_read(portid + gSwitchModel->port.offset, 9, 24, &val);
    gMacDrv->drv_miim_write(portid + gSwitchModel->port.offset, 9, 24, (val | (0x1 << 2)));
    gMacDrv->drv_miim_write(portid + gSwitchModel->port.offset, 9, 24, (val & ~(0x1 << 3)));
    gMacDrv->drv_miim_write(_RTL8214F_Phy0Id + 1, 8, 16, forceReg);       /* restore the original register value */
    return;
} /* end of rtl8214f_phyPowerOff */

/* Function Name:
 *      rtl8214f_config
 * Description:
 *      Configuration code for RTL8214F.
 * Input:
 *      phyId - the phyid of PHY0 of the RTL8218
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8214f_config(Tuint8 phyid)
{
    int rtl8214f_phy0_macid = phyid;
    unsigned int val;
    Tuint32 i = 0;

    /* RTL8214F Whole-Chip */
    DBG_PRINT(1, "### Patch RTL8214F:PHY0 (portid = %d) ###\n", rtl8214f_phy0_macid);

    /* VCO High Gain */
    gMacDrv->drv_miim_read(rtl8214f_phy0_macid + 2, 8, 22, &val);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 2, 8, 22, (val | (0xff << 8)));

    /* Serdes CDR CP from 10uA set to 7.5uA
     * CPADJEN from disable set to enable
     */
    gMacDrv->drv_miim_read(rtl8214f_phy0_macid + 0, 9, 19, &val);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 0, 9, 19, ((val & (~(0x9 << 4))) | (0x6 << 4)));
    gMacDrv->drv_miim_read(rtl8214f_phy0_macid + 1, 9, 19, &val);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 1, 9, 19, ((val & (~(0x9 << 4))) | (0x6 << 4)));

    /* Serdes Forcerun reset */
    gMacDrv->drv_miim_read(rtl8214f_phy0_macid + 0, 9, 16, &val);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 0, 9, 16, (val | (0x1 << 8)));
    OSAL_MDELAY(100);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 0, 9, 16, (val & ~(0x1 << 8)));

    /* Disable power saving mode */
    gMacDrv->drv_miim_read(rtl8214f_phy0_macid + 2, 8, 29, &val);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 2, 8, 29, (val | (0x1 << 0)));

    /* Config link/speed LED mode */
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 1, 8, 21, 0x0408);

    /* Patch output amplitude */
    gMacDrv->drv_miim_read(rtl8214f_phy0_macid + 1, 2, 21, &val);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 1, 2, 21, (val | (0xff << 8)));

    /* Fixed the Register Set (RTL8214F only) */
    gMacDrv->drv_miim_read(rtl8214f_phy0_macid + 1, 8, 16, &val);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 1, 8, 16, (val | 0x0f00) & ~0xf000);

    for (i=0; i<4; i++)
    {
        int rtl8214f_macid = rtl8214f_phy0_macid + i;
        unsigned int val;

        /* Echo group for long cable */
        gMacDrv->drv_miim_read(rtl8214f_macid, 1, 14, &val);
        gMacDrv->drv_miim_write(rtl8214f_macid, 1, 14, (val | (0x1 << 3)));

        /* Disable RTL8214F two pair speed down function */
        gMacDrv->drv_miim_read(rtl8214f_macid, 0, 20, &val);
        gMacDrv->drv_miim_write(rtl8214f_macid, 0, 20, (val & ~(0x1 << 15)));

        /* Enable Green ethernet and adjust its threshold
         * Tx = 10m(default), Rx = 0m(Rx has some issue<10m)
         */
        gMacDrv->drv_miim_read(rtl8214f_macid, 3, 23, &val);
        gMacDrv->drv_miim_write(rtl8214f_macid, 3, 23, (val & ~(0x1f << 6)));

        /* Fixed the Power Status */
        gMacDrv->drv_miim_read(rtl8214f_phy0_macid + 1, 8, 16, &val);
        gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 1, 8, 16, (val | (0x1000 << (rtl8214f_macid - rtl8214f_phy0_macid))));
        gMacDrv->drv_miim_read(rtl8214f_macid, 9, 19, &val);
        gMacDrv->drv_miim_write(rtl8214f_macid, 9, 19, (val & ~(0x1 << 4)));
        gMacDrv->drv_miim_read(rtl8214f_macid, 9, 16, &val);
        gMacDrv->drv_miim_write(rtl8214f_macid, 9, 16, (val | (0x1 << 5)));
        gMacDrv->drv_miim_read(rtl8214f_macid, 9, 24, &val);
        gMacDrv->drv_miim_write(rtl8214f_macid, 9, 24, (val | (0x1 << 5)));
        gMacDrv->drv_miim_write(rtl8214f_macid, 9, 24, (val | (0x1 << 2)));
        gMacDrv->drv_miim_write(rtl8214f_macid, 9, 24, (val & ~(0x1 << 3)));
        gMacDrv->drv_miim_read(rtl8214f_macid, 0, 0, &val);
        gMacDrv->drv_miim_write(rtl8214f_macid, 0, 0, (val | (0x1 << 12) | (0x1 << 9)));
        gMacDrv->drv_miim_write(rtl8214f_macid, 0, 0, (val | (0x1 << 11)));
        gMacDrv->drv_miim_read(rtl8214f_phy0_macid + 1, 8, 16, &val);
        gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 1, 8, 16, (val & ~(0x1000 << (rtl8214f_macid - rtl8214f_phy0_macid))));
        gMacDrv->drv_miim_read(rtl8214f_macid, 0, 0, &val);
        gMacDrv->drv_miim_write(rtl8214f_macid, 0, 0, (val | (0x1 << 11)));

        /* Poweroff the RSGMII(2,3) serdes */
        if (i>=2 && i<4)
        {
            gMacDrv->drv_miim_read(rtl8214f_macid, 9, 16, &val);
            gMacDrv->drv_miim_write(rtl8214f_macid, 9, 16, (val | ((0x1 << 5) | (0x1 << 2))));
            gMacDrv->drv_miim_write(rtl8214f_macid, 9, 16, (val & (~((0x1 << 4)) | (0x1 << 3))));
        }
    }

    /* Set the default media - copper */
    gMacDrv->drv_miim_read(rtl8214f_phy0_macid + 1, 8, 16, &val);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 1, 8, 16, (val & (~(0xf000))));

    /* Fixed power force mode (RTL8214F only) */
    gMacDrv->drv_miim_read(rtl8214f_phy0_macid, 9, 16, &val);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid, 9, 16, (val | (0x1 << 0)));

    /* Fixed state-machine (RTL8214F only) */
    gMacDrv->drv_miim_read(rtl8214f_phy0_macid, 8, 21, &val);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid, 8, 21, (val | (0x1 << 1)));

    return;
} /* end of rtl8214f_config */

/* Function Name:
 *      rtl8214fb_phyPowerOn
 * Description:
 *      Get meida of port.
 * Input:
 *      portid - Port number (0~28)
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8214fb_phyPowerOn(int portid)
{
#if 0
    int phy_idx;
#endif
    unsigned int forceReg;
    unsigned int val;

    if (gSwitchModel == NULL)
        return;

    if (gMacDrv == NULL)
        return;

#if 0
    if (portid >= gSwitchModel->port.count)
        return;

    phy_idx = gSwitchModel->port.list[portid].phy_idx;

    if (gSwitchModel->phy.list[phy_idx].chip != RTK_CHIP_RTL8214FB && \
        gSwitchModel->phy.list[phy_idx].chip != RTK_CHIP_RTL8214B &&  \
        gSwitchModel->phy.list[phy_idx].chip != RTK_CHIP_RTL8212B)
        return;
#endif

    gMacDrv->drv_miim_read(portid + gSwitchModel->port.offset, 8, 30, &forceReg);      /* store the original register value */
    gMacDrv->drv_miim_write(portid + gSwitchModel->port.offset, 8, 30, (forceReg & ~(0x3<<12)) | (0x3<<12));
    gMacDrv->drv_miim_read(portid + gSwitchModel->port.offset, 0, 0, &val);
    gMacDrv->drv_miim_write(portid + gSwitchModel->port.offset, 0, 0, (val & ~(0x1 << 11)));

    gMacDrv->drv_miim_write(portid + gSwitchModel->port.offset, 8, 30, (forceReg & ~(0x3<<12)) | (0x2<<12));
    gMacDrv->drv_miim_read(portid + gSwitchModel->port.offset, 0, 0, &val);
    gMacDrv->drv_miim_write(portid + gSwitchModel->port.offset, 0, 0, (val & ~(0x1 << 11)));

    gMacDrv->drv_miim_write(portid + gSwitchModel->port.offset, 8, 30, forceReg);       /* restore the original register value */

    OSAL_MDELAY(1000);
    return;
} /* end of rtl8214fb_phyPowerOn */


/* Function Name:
 *      rtl8214fb_phyPowerOff
 * Description:
 *      Get meida of port.
 * Input:
 *      portid - Port number (0~28)
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8214fb_phyPowerOff(int portid)
{
#if 0
    int phy_idx;
#endif
    unsigned int forceReg;
    unsigned int val;

    if (gSwitchModel == NULL)
        return;

    if (gMacDrv == NULL)
        return;

#if 0
    if (portid >= gSwitchModel->port.count)
        return;

    phy_idx = gSwitchModel->port.list[portid].phy_idx;
    if (gSwitchModel->phy.list[phy_idx].chip != RTK_CHIP_RTL8214FB && \
        gSwitchModel->phy.list[phy_idx].chip != RTK_CHIP_RTL8214B && \
        gSwitchModel->phy.list[phy_idx].chip != RTK_CHIP_RTL8212B)
        return;
#endif

    gMacDrv->drv_miim_read(portid + gSwitchModel->port.offset, 8, 30, &forceReg);      /* store the original register value */
    gMacDrv->drv_miim_write(portid + gSwitchModel->port.offset, 8, 30, (forceReg & ~(0x3<<12)) | (0x3<<12));
    gMacDrv->drv_miim_read(portid + gSwitchModel->port.offset, 0, 0, &val);
    gMacDrv->drv_miim_write(portid + gSwitchModel->port.offset, 0, 0, (val | (0x1 << 11)));

    gMacDrv->drv_miim_write(portid + gSwitchModel->port.offset, 8, 30, (forceReg & ~(0x3<<12)) | (0x2<<12));
    gMacDrv->drv_miim_read(portid + gSwitchModel->port.offset, 0, 0, &val);
    gMacDrv->drv_miim_write(portid + gSwitchModel->port.offset, 0, 0, (val | (0x1 << 11)));

    gMacDrv->drv_miim_write(portid + gSwitchModel->port.offset, 8, 30, forceReg);       /* restore the original register value */
    return;
} /* end of rtl8214fb_phyPowerOff */

/* Function Name:
 *      rtl8214fb_config
 * Description:
 *      Configuration code for RTL8214FB.
 * Input:
 *      phyId - the phyid of PHY0 of the RTL8214FB
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8214fb_config(Tuint8 phyid)
{
    int rtl8214fb_phy0_macid = phyid;
    int i;
    unsigned int val, forceReg, restore_val;
    unsigned int version, data;
    int is_fb, is_12b = 0;

    if((gSwitchModel->chip==RTK_CHIP_RTL8328M) ||\
        (gSwitchModel->chip==RTK_CHIP_RTL8328S) || \
        (gSwitchModel->chip==RTK_CHIP_RTL8328L))
    {
        /* Patch some 8328 MAC auto config phy register back to chip default
         * PHY 0~3, page 0, reg 4, value 0x01A0 (for fiber-1000)
         * PHY 0~3, page 0, reg 9, value 0x0000 (for fiber-1000)
         */
        for (i=0; i<4; i++)
        {
            gMacDrv->drv_miim_read(rtl8214fb_phy0_macid+i, 8, 30, &restore_val);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid+i, 8, 30, 0x3000);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid+i, 8, 4, 0x01A0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid+i, 8, 9, 0x0000);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid+i, 8, 30, restore_val);
        }

        gMacDrv->drv_miim_read(rtl8214fb_phy0_macid,  10, 18, &version);

        is_fb = version & 0xF;
        version = (version >> 8) & 0xF;

        if (is_fb == 0)
        {
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid+3, 8, 18, 0x93f0);
            gMacDrv->drv_miim_read(rtl8214fb_phy0_macid+3, 8, 19, &data);
            if (((data & 0xF) == 0xE) || ((data & 0xF) == 0x8))
                is_12b = 1;
            else
                is_12b = 0;
        }

        if(version == 0)
        {
            /* Configuration code, 2010-12-22 version for version B */
            if (is_fb)
                OSAL_PRINTF("### RTL8214FB Version B config - Phy0Id = %d ###\n", rtl8214fb_phy0_macid);
            else if (is_12b)
                OSAL_PRINTF("### RTL8212B Version B config - Phy0Id = %d ###\n", rtl8214fb_phy0_macid);
            else
                OSAL_PRINTF("### RTL8214B Version B config - Phy0Id = %d ###\n", rtl8214fb_phy0_macid);

            /* Reset Ser-Des0~5 and interface selection to RSGMII with 4 ports 1000Base-T/(1000Base-X or 100Base-FX) and change LED polarity */
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,  8, 0x1c, 0xff00);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid,  8, 0x11, 0x3197);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid, 15, 0x1e, 0x0013);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid, 15, 0x1a, 0xE46A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 1, 15, 0x1e, 0x0013);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 1, 15, 0x1a, 0xE46A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 2, 15, 0x1e, 0x0013);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 2, 15, 0x1a, 0xE46A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 15, 0x1e, 0x0013);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 15, 0x1a, 0xE46A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,  8, 0x1c, 0x9000);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,  8, 0x1c, 0x0000);
            //gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,  8, 0x11, 0x3117);
            /* Serial LED Active High to Low */
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid, 8, 0x1b, 0x000A);
            /* FIFO Setting */
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 1, 8, 0x1b, 0x00D0);
            /* Force Select Copper Standard Register */
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 1, 8, 0x10, 0x0F00);
            /* Enable Parallel Write */
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 8, 0x18, 0x0001);
            /* Modify Ser-Des0,1 DC Offset Calibration Tuning Range */
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 15, 0x1e, 0x000C);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 15, 0x1b, 0xBE03);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 15, 0x1c, 0x8C42);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 15, 0x15, 0x4343);
            /* Modify Ser-Des0,1 Analog Parameter */
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 15, 0x1e, 0x000D);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 15, 0x16, 0x0900);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 15, 0x15, 0x856A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 15, 0x18, 0x80C8);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 15, 0x17, 0x5B93);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 15, 0x17, 0x5393);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 15, 0x17, 0x5B93);
            /* Modify Ser-Des2~5 Register MSB/LSB Swap */
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 15, 0x1e, 0x001D);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 15, 0x16, 0x0C20);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 15, 0x17, 0xACE5);
            /* Power Down PHY */
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,  8, 0x00, 0x1940);
            /* Lock Naro-C */
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,  7, 0x1e, 0x0023);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,  7, 0x17, 0x1111);
            OSAL_MDELAY(100);

            /* Naro-C Patch */
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,  7, 0x1e, 0x0023);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,  7, 0x16, 0x0306);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,  7, 0x16, 0x0307);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,  7, 0x15, 0x0181);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,  7, 0x19, 0x6C09);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,  7, 0x15, 0x0285);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,  7, 0x19, 0x4400);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,  7, 0x15, 0x0286);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,  7, 0x19, 0x4020);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,  7, 0x15, 0x0287);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,  7, 0x19, 0x4480);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,  7, 0x15, 0x0288);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,  7, 0x19, 0x9E00);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,  7, 0x15, 0x0289);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,  7, 0x19, 0x4891);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,  7, 0x15, 0x028A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,  7, 0x19, 0x4CC0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,  7, 0x15, 0x028B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,  7, 0x19, 0x4801);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,  7, 0x15, 0x03E5);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,  7, 0x19, 0x4478);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,  7, 0x15, 0x0000);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,  7, 0x16, 0x0306);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,  7, 0x16, 0x0300);
            /* Micro-C Patch */
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x05, 0xFFF6);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0080);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x05, 0x8B5C);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0000);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x0F, 0x0100);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x05, 0x8000);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0280);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x48F7);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x00E0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFFF7);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xA080);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x02AE);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xF602);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x00F7);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0201);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0402);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0111);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0201);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2102);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8072);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0201);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x4702);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8085);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE08B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x88E1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8B89);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x1E01);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE18B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8A1E);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x01E1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8B8B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x1E01);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE18B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8C1E);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x01E1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8B8D);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x1E01);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE18B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8E1E);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x01A0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x00C7);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xAEBB);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xEE8B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x5702);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0281);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xC8BF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8B88);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xEC00);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x19A9);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8B90);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xF9EE);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFFF6);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x00EE);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFFF7);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFCD1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x00BF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x85D9);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x022A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFAD1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x01BF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x85DC);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x022A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFA04);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xF8E0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8B8C);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xAD20);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0AEE);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8B8C);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0002);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x1678);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0282);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE3FC);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x04F8);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE08B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8EAD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2020);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xF620);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE48B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8E02);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x81DF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0281);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0D02);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8251);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0202);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xB802);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x228F);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0226);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xC702);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0372);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x021E);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x9C02);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x82FD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE08B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8EAD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2108);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xF621);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE48B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8E02);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0338);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE08B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8EAD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2208);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xF622);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE48B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8E02);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x1DF9);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE08B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8EAD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2308);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xF623);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE48B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8E02);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x282D);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE08B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8EAD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2405);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xF624);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE48B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8EE0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8B8E);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xAD25);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x05F6);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x25E4);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8B8E);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE08B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8EAD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x260B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xF626);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE48B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8E02);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x84F8);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0283);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x90E0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8B8E);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xAD27);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x05F6);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x27E4);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8B8E);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0202);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x64FC);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x04F8);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFAEF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x69E0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8B85);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xAD21);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x47E0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE022);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE1E0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2358);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xC059);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x021E);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x01E1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8B60);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x1F10);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x9E34);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE48B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x60AD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x212B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE18B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x84F7);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x29E5);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8B84);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xAC27);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x18AC);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2605);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0281);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xA8AE);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x1BD1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x02BF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2535);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x022A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFA02);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x858B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0224);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xCEAE);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0B02);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8564);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0224);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE7AE);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0302);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8163);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xEF96);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFEFC);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0402);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x81B7);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0285);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xB2D1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x03BF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2535);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x022A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFAD1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x00BF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2538);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x022A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFAD1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x00BF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x253B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x022A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFAD1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x08BF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x252C);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x022A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFAD1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x01BF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x252F);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x022A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFAD1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x01BF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2532);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x022A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFAE1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8B86);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xAD2A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x08D1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x00BF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x253E);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x022A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFA04);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE18B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x86AD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2A08);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xD101);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xBF25);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x3E02);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2AFA);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x04F8);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE0E0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x38E1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE039);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xAC2E);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x05D0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0302);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x20E7);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFC04);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xF8FA);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xEF69);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE08B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x87AD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2008);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xD101);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xBF27);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE302);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2AFA);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xEF96);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFEFC);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x04F8);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xF9FA);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xEF69);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE08B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x87AD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2061);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xD200);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xBF27);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xDA02);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2ACD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x1E21);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xBF22);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x5602);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2ACD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0C11);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x1E21);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xBF85);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xDF02);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2ACD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0C12);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x1E21);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xBF85);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE202);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2ACD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0C13);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x1E21);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xBF85);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE502);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2ACD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0C14);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x1E21);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE08A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE21F);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x029E);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x28E6);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8AE2);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xAD30);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x1CAC);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x3107);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x5A30);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xA200);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x14AE);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0AEF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0258);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0C9E);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x045A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x309F);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x08BF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x85F1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x022A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x5CAE);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x06BF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x85FA);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x022A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x5CEF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x96FE);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFDFC);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x04F8);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE08B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x85AD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2629);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE0E0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x36E1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE037);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE18B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x631F);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x109E);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x1CE4);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8B63);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xAC20);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x08AC);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x210B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xAC27);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0DAE);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0EEE);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8B56);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x00AE);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0802);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8283);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xAE03);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0225);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x6AFC);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x04F8);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFAEF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x6902);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x82A0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE0E0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x14E1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE015);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xAD26);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x08D1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x1EBF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x25CD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x022A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFAEF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x96FE);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFC04);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xF8F9);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE08B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x85AD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2738);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xD00B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0228);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFF58);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8278);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x829F);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2DE0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8B56);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE18B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x571F);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x109E);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2310);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE48B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x56E0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE000);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE1E0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x01F7);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x27E4);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE000);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE5E0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x01E2);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE020);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE3E0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x21AD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x30F7);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xF627);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE4E0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x00E5);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE001);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFDFC);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x04F8);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE0E5);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x14E1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE515);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xAD27);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0EE0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8B01);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xAC27);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x08F7);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x27E4);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8B01);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0284);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xBAFC);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x04F8);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE08B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x01AD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2717);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xAC23);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x14BF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x1DE7);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x022A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xCDAD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x280B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE08B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x01F7);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x23E4);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8B01);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0283);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x1DFC);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x04F8);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xF9E0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE514);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE1E5);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x15E1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8B01);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xAC29);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x62AC);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2604);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xF62A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xAE02);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xF72A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE58B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x01AD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2713);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xF728);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE58B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x01E2);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE518);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE3E5);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x19F6);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x36E6);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE518);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE7E5);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x19AD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x283D);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xF8BF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x1CE7);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x022A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xCDE5);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8B02);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xD101);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xBF1C);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE702);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2AFA);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFCE2);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE0C0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE3E0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xC1AD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x3708);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x5A7F);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE6E0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xC0E7);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE0C1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xEF01);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x5830);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x5BCC);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x1E30);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0D04);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x1E30);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x6A80);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE6E0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xC0E7);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE0C1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xF729);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE58B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x01FD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFC04);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xF8F9);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFAFB);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE0E0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFAE1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE0FB);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xAD24);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x18E2);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8B01);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xAD31);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x12F8);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE18B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x02BF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x1CE7);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x022A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFAFC);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x580F);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xAD34);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x15AE);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x03AF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x84B5);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xEF30);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x5B03);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xAD23);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x026B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x04AD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2202);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x6B08);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xEF03);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xD300);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xEF10);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x590E);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xA108);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x04D3);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x02AE);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2DEF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x1059);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0EA1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0404);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xD302);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xAE22);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xEF10);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x590E);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xA106);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x04D3);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x04AE);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x17A0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0A04);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xD308);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xAE10);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xA00B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x04D3);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x01AE);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x09EF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x1059);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0CA1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0C02);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xD308);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xEF02);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0D04);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x5803);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xD600);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x00D7);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x1111);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xAD3A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0516);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0C6C);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xAE14);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xAD3B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0516);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0C68);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xAE0C);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xAD39);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0516);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0C64);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xAE04);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xAD38);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0116);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xA000);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x02AE);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x060C);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x610C);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x71B0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFAE0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE516);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE1E5);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x177F);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFFFF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x1D47);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x1E46);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE4E5);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x16E5);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE517);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xEF02);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0D04);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x5803);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xBFE5);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x180C);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x011A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x90E0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE0F8);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE1E0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xF9EF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x645E);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x1FFF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xD819);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xD989);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x5CE0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x001E);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x46DC);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x19DD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2A10);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xF631);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xEF02);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x5824);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x3824);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x9E03);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xAD36);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x34AD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x3210);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE0E5);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x16E1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE517);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x5833);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x5933);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE4E5);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x16E5);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE517);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xD200);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE0E5);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x14E1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE515);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xF627);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE4E5);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x14E5);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE515);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE0E5);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x18E1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE519);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xF726);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE4E5);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x18E5);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE519);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0284);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE1E6);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8B01);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0283);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x1DFF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFEFD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFC04);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xF8E0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8B64);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xF723);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE48B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x64D1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x01BF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x36CB);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x022A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFAE0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE2F4);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE1E2);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xF5E4);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8B58);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE58B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x59EE);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE2F4);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xD8EE);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE2F5);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x20FC);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x04F8);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE08B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x58E1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8B59);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE4E2);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xF4E5);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE2F5);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE08B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x64F6);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x23E4);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8B64);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFC04);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xF8F9);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFAEF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x69AC);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x1B5E);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE0E0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x12E1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE013);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xBF26);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xBC02);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2ACD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xEF01);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE28A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xCAE4);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8ACA);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x1F12);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x9E46);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xEF12);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x5907);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x9F1A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xF8E0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8B64);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xF721);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE48B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x64D0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0302);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x20E7);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0226);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x96D1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x00BF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x27E3);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x022A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFAFC);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xA000);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x1402);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2645);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0226);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xADEE);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8AE2);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFFE0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8B64);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xF621);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE48B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x64AE);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0FBF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x36C5);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x022A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x5CBF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x36C2);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x022A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x5C02);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x264E);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xEF96);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFEFD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFC04);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xF8FA);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xEF69);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE08B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x87AD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2118);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xD000);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0229);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x78AD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2F10);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xD101);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xBF85);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xEE02);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2AFA);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xD101);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xBF85);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE802);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2AFA);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xEF96);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFEFC);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x04F8);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFAEF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x69E0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8B87);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xAD21);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x18D0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0002);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2978);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xAD2E);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x10D1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x01BF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x85EB);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x022A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFAD1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x01BF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x85E8);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x022A);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFAEF);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x96FE);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFC04);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xF8FA);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xEF69);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE08B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x87AD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2118);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xD100);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xBF85);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE802);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2AFA);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xD100);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xBF85);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xEB02);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2AFA);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xD100);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xBF85);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xEE02);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x2AFA);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xEF96);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xFEFC);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0400);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE140);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x77E1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x4099);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE036);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x88E0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x36FE);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE022);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x99E2);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x00DD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE200);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xEEE2);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x00E0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x4B10);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE04B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x23E1);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x006C);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE100);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x1CE0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x4B13);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xE04B);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x6000);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x05, 0xE142);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0201);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x05, 0xE140);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x0005);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x0f, 0x0000);
            /* Unlock Naro-C */
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 7, 0x1e, 0x0023);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 7, 0x17, 0x1910);
            OSAL_MDELAY(100);

            /* Micro-C Modify Radom Seed Cause Not Link */
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x05, 0x8B85);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0xC286);
            /* Micro-C Disable EEE 10M */
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x05, 0x8B86);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x8600);
            /* Enable EEE 100/1000M */
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 7, 0x1e, 0x0020);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 7, 0x15, 0x0000);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 7, 0x1b, 0xA0BA);
            /* Enable Down Speed */
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 7, 0x1e, 0x002D);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 7, 0x18, 0xF010);
            /* Disable the Solution of Force 10M to 100M Issue */
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 7, 0x1e, 0x002D);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 7, 0x16, 0x0024);
            /* Modify Analog PHY Parameter */
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 2, 0x08, 0x3602);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 2, 0x12, 0x00DD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 2, 0x0c, 0x5C25);
            /* Modify EEE Unformatted Page2 the Same as Page1 */
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0, 0x0d, 0x0003);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0, 0x0e, 0x0015);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0, 0x0d, 0x4003);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0, 0x0e, 0x0006);
            /* Micro-C Select Calibration Mode */
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x05, 0x8B82);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 5, 0x06, 0x05EB);
            /* Restart Auto-Negotiation */
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 8, 0x00, 0x1340);
            /* Disable Parallel Write */
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 8, 0x18, 0x0000);
            /* Auto Mode Standard Register */
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 1, 8, 0x10, 0x0000);
            /* Reset Ser-Des */
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 8, 0x1c, 0xFF00);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 8, 0x1c, 0x9000);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 8, 0x1c, 0x0000);
        }/*end Version B*/
        else if (version == 2)
        {
        /* Configuration code, 2013-02-05 version for version C */
            if (is_fb)
                OSAL_PRINTF("### RTL8214FB Version C config - Phy0Id = %d ###\n", rtl8214fb_phy0_macid);
            else if (is_12b)
                OSAL_PRINTF("### RTL8212B Version C config - Phy0Id = %d ###\n", rtl8214fb_phy0_macid);
            else
                OSAL_PRINTF("### RTL8214B Version C config - Phy0Id = %d ###\n", rtl8214fb_phy0_macid);

            /*FIFO Setting */
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 1, 0x8, 27, 0x01D1 ); // FIFO setting

            /*Force Select Copper Standard Register*/
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 1, 0x8, 16, 0x0F00 ); // force select copper standard register

            /*Enable Parallel Write*/
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x8, 24, 0x0001 ); // enable parallel write

            /* Enable or Disable SGMII/QSGMII LPI */
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0xf, 30, 0x0009 ); // change to ext. page 9
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0xf, 23, 0x0FA4 ); // enable or disable QSGMII LPI (enable: 0x0FA4, disable: 0x0CA4)
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0xf, 30, 0x0006 ); // change to ext. page 6
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0xf, 21, 0x0616 ); // enable or disable QSGMII LPI (enable: 0x0616, disable: 0x0416)
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0xf, 30, 0x0016 ); // change to ext. page 22
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0xf, 21, 0x0616 ); // enable or disable SGMII LPI (enable: 0x0616, disable: 0x0416)

            /*Modify Ser-Des0,1 EQ Parameter*/
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0xF, 30, 0x000C ); // change to ext. page 12
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0xF, 27, 0xBE03 ); // Ser-Des0,1 DC EQ parameter

            /* Modify Ser-Des2~5 Common Mode Voltage of Charge Pump */
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0xF, 30, 0x001C ); // change to ext. page 28
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0xF, 28, 0x154D ); // Modify Ser-Des2~5 Common Mode Voltage of Charge Pump

            /* Modify Ser-Des2~5 SGMII Output Amplitude */
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0xF, 30, 0x001D ); // change to ext. page 29
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0xF, 21, 0x7180 ); // modify output amplitude

            /*Power Down PHY*/
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x8, 0, 0x1940 ); // power down PHY

            /*Lock Nano-C*/
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x7, 30, 0x0023 ); // change to ext. page 35
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x7, 23, 0x1111 ); // lock Nano-C
            OSAL_MDELAY(100);

            /*Micro-C set abd State Reference Counter in sub_eee_off as a Variable*/
            /*Lower Amplitude Only When Link at 10M*/
            /*Micro-C Set max_wake_time in Different Speed*/
        /*Micro-C Modify sub_slpck_gen to Avoid cntrdy_ps Loss ...*/
        /*Micro-C Add sub_pd_abi_adv_chk ...*/
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 5, 0x8B5C );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x3602 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 15, 0x0100 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 5, 0xFFF6 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0080 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 5, 0x8000 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0280 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x48F7 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x00E0 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xFFF7 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xA080 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x02AE );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xF602 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x00FD );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0201 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0A02 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x807F );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0201 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2702 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x013A );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0201 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x4D02 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x808F );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE08B );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x88E1 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8B89 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x1E01 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE18B );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8A1E );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x01E1 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8B8B );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x1E01 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE18B );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8C1E );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x01E1 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8B8D );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x1E01 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE18B );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8E1E );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x01A0 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x00C7 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xAEBB );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xEE84 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8000 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xEE84 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8100 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xEE8A );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE407 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xEE8A );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE573 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xEE8B );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8781 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xBF8B );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x88EC );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0019 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xA98B );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x90F9 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xEEFF );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xF600 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xEEFF );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xF7FC );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xD100 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xBF83 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x9502 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2CF8 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xD101 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xBF83 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x9802 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2CF8 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x04F8 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE08B );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8AAD );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2007 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xEE8B );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8A00 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0282 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xA4FC );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x04F8 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE08B );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8EAD );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2023 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xF620 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE48B );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8E02 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x819D );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0283 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2302 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x02C9 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0281 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x6D02 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2355 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0227 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xBC02 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x811A );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0203 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8302 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x1F62 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x021B );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xB2E0 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8B8E );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xAD21 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x08F6 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x21E4 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8B8E );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0203 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x49E0 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8B8E );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xAD22 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x08F6 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x22E4 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8B8E );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x021E );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xBFE0 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8B8E );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xAD23 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x08F6 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x23E4 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8B8E );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x022A );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2BE0 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8B8E );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xAD24 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x05F6 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x24E4 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8B8E );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE08B );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8EAD );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2505 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xF625 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE48B );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8EE0 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8B8E );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xAD26 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0BF6 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x26E4 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8B8E );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0226 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xCE02 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x1C45 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE08B );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8EAD );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2705 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xF627 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE48B );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8E02 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0275 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xFC04 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xF8F9 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE08B );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x85AD );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2548 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE08A );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE0E1 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8AE1 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x7C00 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x009E );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x35EE );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8AE0 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x00EE );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8AE1 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x00E0 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8AE4 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE18A );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE5E2 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8480 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE384 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8102 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2BEF );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xAD20 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x12EE );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8AE0 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x03EE );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8AE1 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xB7EE );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8480 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x00EE );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8481 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x00AE );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x1115 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE684 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x80E7 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8481 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xAE08 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xEE84 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8000 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xEE84 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8100 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xFDFC );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x04F8 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xF9E0 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8B87 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xAD20 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x25E0 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE200 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE1E2 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x01AD );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2011 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE2E0 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x22E3 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE023 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xAD39 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x085A );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xC09F );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x04D1 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x01AE );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x02D1 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x00BF );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x29C9 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x022C );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xF802 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2906 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xFDFC );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x04F8 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xFAEF );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x69E0 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8B85 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xAD21 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x39E0 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE022 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE1E0 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2358 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xC059 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x021E );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x01E1 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8B60 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x1F10 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x9E26 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE48B );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x60AD );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x211D );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE18B );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x5EF7 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x29E5 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8B5E );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xAC27 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0DAC );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2605 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0282 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2AAE );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0D02 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8241 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xAE08 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0282 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x65AE );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0302 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x81E5 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xEF96 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xFEFC );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0402 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x25ED );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0229 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8DD1 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x03BF );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2607 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x022C );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xF8D1 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x00BF );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x260A );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x022C );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xF8D1 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x00BF );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x260D );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x022C );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xF8D1 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x00BF );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x25FE );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x022C );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xF8D1 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x01BF );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2601 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x022C );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xF8D1 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x01BF );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2604 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x022C );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xF8E1 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8B86 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xAD2A );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x08D1 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x00BF );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2610 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x022C );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xF804 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xD100 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xBF25 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xFE02 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2CF8 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE18B );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x86AD );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2A08 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xD101 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xBF26 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x1002 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2CF8 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x04D1 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x02BF );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2607 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x022C );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xF802 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2966 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xD102 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xBF25 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xFE02 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2CF8 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xD107 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xBF26 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0102 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2CF8 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xD107 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xBF26 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0402 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2CF8 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x04D1 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x00BF );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2607 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x022C );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xF8D1 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x08BF );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x25FE );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x022C );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xF802 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x293F );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xD011 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x022A );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xFD59 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x03EF );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x01D1 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x00A0 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0002 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xD101 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xBF26 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0A02 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2CF8 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xD111 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xAD20 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x020C );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x11AD );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2102 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0C12 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xBF26 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0D02 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2CF8 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0228 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x4204 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xF8FA );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xEF69 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xBF1E );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xAA02 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2CCB );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x5903 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE08A );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xC41F );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x019F );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x07E0 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8AC5 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x10E4 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8AC5 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE58A );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xC4A1 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0205 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x021E );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2FAE );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x07A1 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0304 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0107 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xAE00 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xEF96 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xFEFC );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x04F8 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xFAEF );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x69F6 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0302 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x82F5 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xF703 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xF706 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xBF1E );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x9402 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2C5A );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xD100 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xBF1E );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xAD02 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2CF8 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xEF96 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xFEFC );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x04F8 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xF91F );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x44D3 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x10F7 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x05AC );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0406 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xB3FB );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xF605 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xAEF3 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE0FF );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xF40D );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0211 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0D01 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x58F0 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x9FF9 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x81E0 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xFFF4 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE4E1 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x4EE5 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE14F );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xF605 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xF604 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xFDFC );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x04F8 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE08B );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x85AD );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2631 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE0E0 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x36E1 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE037 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE18B );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x631F );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x109E );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x24E4 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8B63 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xAC20 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0BAC );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x210E );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xAC25 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x10AC );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2712 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xAE13 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xEE8B );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x5600 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xAE0D );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0226 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x45AE );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0802 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x835D );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xAE03 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0226 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x62FC );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0401 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0104 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xF8FA );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xEF69 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE08B );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x87AD );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2726 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xD00B );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x022A );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xFDAC );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2A05 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xBF83 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x9BAE );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x03BF );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x839E );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x022C );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xCBAC );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2810 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xD101 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xBF83 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xA102 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2CF8 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xD100 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xBF83 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xA102 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x2CF8 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xEF96 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xFEFC );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0400 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE140 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x77E1 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x4065 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE008 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x87E0 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0844 );
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xE028 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 5, 0xE142 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0201 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 5, 0xE140 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x0005 );
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 15, 0x0000 );

            /*Unlock Nano-C*/
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x7, 30, 0x0023 );// change to ext. page 35
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x7, 23, 0x1910 );// unlock Nano-C
            OSAL_MDELAY(100);

            /*Micro-C Enable or Disable Auto Turn off EEE*/
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 5, 0x8B85 );// set Micro-C memory address (modify radom seed cause not link)
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0xC286);// set Micro-C memory data (enable or disable auto turn off EEE) (enable: 0xE286, disable: 0xC286)

            /*Micro-C Control 10M EEE*/
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 5, 0x8B86 );// set Micro-C memory address (control 10M EEE)
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x5, 6, 0x8680 );// set Micro-C memory data (control 10M EEE) (enable: 0x8681, disable: 0x8680)

            /*Enable or Disable EEE*/
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x7, 30, 0x0020 );// change to ext. page 32
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x7, 21, 0x0000 );// enable or disable EEE (enable: 0x1100, disable: 0x0000)
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x7, 27, 0xA0BA );// force EEE PHY mode (PHY mode: 0xA0BA, MAC mode: 0xA03A)

            /*Viterbi Bypass Resume*/
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x7, 30, 0x0020 );// change to ext. page 32
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x7, 24, 0x61FF );// viterbi bypass resume

#if defined(CONFIG_EEE_COMPATIBLE)
            /* Min. IPG Select */
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x7, 30, 0x002C );// change to ext. page 44
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x7, 25, 0x0130 );// min. IPG select (64 bit time: 0x0030, 32 bit time: 0x0130)
#endif

            /* Enable the Solution of Link Partner Force MDI Mode Long Link Issue and Link Partner Force 10M Change to Force 100M Issue */
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x7, 30, 0x002D );// change to ext. page 45
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x7, 22, 0x0084 );// set dis_frc_mode_A-timer22.7 to 1 and dis_rx10_edet22.5 to 0

            /*100/1000M EEE Capability*/
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x0, 13, 0x0007 );// MMD register 7.60
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x0, 14, 0x003C );// MMD register 7.60
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x0, 13, 0x4007 );// MMD register 7.60
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x0, 14, 0x0000 );// disable 100/1000M EEE capability

            /*10M EEE Amplitude*/
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x2, 11, 0x17A7 );// 10M EEE amplitude
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x2, 31, 0x0008 );// change to page 8

            /* 100M Amplitude */
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x2, 19, 0x0F0F );// 100M amplitude
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x2, 31, 0x0008 );// change to page 8

            /*Restart Auto-Negotiation*/
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x8,  0, 0x1340 );// restart auto-negotiation

            /*Disable Parallel Write*/
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x8, 24, 0x0000 );// disable parallel write

            /*Auto Mode Standard Register*/
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 1, 0x8, 16, 0x0000 );// auto mode standard register

        /* Reset Ser-Des */
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x8, 28, 0xFF00);
        OSAL_MDELAY(100);
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x8, 28, 0x9000);
        OSAL_MDELAY(100);
        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3, 0x8, 28, 0x0000);
        }/*end Version C*/

        /* Write the PHY 0-3, Page 8, Register 30 bit[7:6] = 0b00 */
        for (i=0; i<4; i++)
        {
            gMacDrv->drv_miim_read(rtl8214fb_phy0_macid+i, 8, 30, &val);
            val &= ~(1<<7);
            val &= ~(1<<6);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid+i, 8, 30, val);
        }

        /* power down PHY in copper & fiber media */
        for (i=0; i<4; i++)
        {
            gMacDrv->drv_miim_read(rtl8214fb_phy0_macid+i, 8, 30, &forceReg);    /* store the original register value */

            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid+i, 8, 30, (forceReg & ~(0x3<<12)) | (0x3<<12));
            gMacDrv->drv_miim_read(rtl8214fb_phy0_macid+i, 0, 0, &val);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid+i, 0, 0, (val | (0x1 << 11)));

            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid+i, 8, 30, (forceReg & ~(0x3<<12)) | (0x2<<12));
            gMacDrv->drv_miim_read(rtl8214fb_phy0_macid+i, 0, 0, &val);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid+i, 0, 0, (val | (0x1 << 11)));

            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid+i, 8, 30, forceReg);   /* restore the original register value */
        }

        /* power down the serdes 1 for RTL8212B */
        if (is_fb == 0 && is_12b == 1)
        {
            gMacDrv->drv_miim_read(rtl8214fb_phy0_macid+3, 8, 28, &val);
            val |= 0xc000;
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid+3, 8, 28, val);
        }

        /* Configure option when 1000-X Nway is failure, try force mode */
        for (i=0; i<4; i++)
        {
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid+i, 8, 31, 0x000F);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid+i, 15, 30, 0x0018);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid+i, 15, 25, 0x074D);
        }

        return;
    }
    else
    {
        gMacDrv->drv_miim_read(rtl8214fb_phy0_macid,  10, 18, &version);

        is_fb = version & 0xF;
        version = (version >> 8) & 0xF;

        if (is_fb == 0)
        {
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid+3, 8, 18, 0x93f0);
            gMacDrv->drv_miim_read(rtl8214fb_phy0_macid+3, 8, 19, &data);
            if (((data & 0xF) == 0xE) || ((data & 0xF) == 0x8))
                is_12b = 1;
            else
                is_12b = 0;
        }

        if(version == 0)
        {
            /* Configuration code, 2010-12-22 version for version B */
            if (is_fb)
                OSAL_PRINTF("### RTL8214FB Version B config - Phy0Id = %d ###\n", rtl8214fb_phy0_macid);
            else if (is_12b)
                OSAL_PRINTF("### RTL8212B Version B config - Phy0Id = %d ###\n", rtl8214fb_phy0_macid);
            else
                OSAL_PRINTF("### RTL8214B Version B config - Phy0Id = %d ###\n", rtl8214fb_phy0_macid);

            /* RTL8214FB Revision B Per-Chip */
            for (i=0; i<(sizeof(rtl8214fb_perchip_revB)/sizeof(confcode_prv_t)); i++)
            {
                gMacDrv->drv_miim_portmask_write(0x1 << (rtl8214fb_phy0_macid + rtl8214fb_perchip_revB[i].phy), gMacDrv->miim_max_page, \
                    (int)rtl8214fb_perchip_revB[i].reg, (int)rtl8214fb_perchip_revB[i].val);
            }
        }/*end Version B*/
        else//for forward compatiblility use, so delete:::::::::: if (version == 2)
        {
            /* Configuration code, 2011-04-21 version for version C */
            if (is_fb)
                OSAL_PRINTF("### RTL8214FB Version C config - Phy0Id = %d ###\n", rtl8214fb_phy0_macid);
            else if (is_12b)
                OSAL_PRINTF("### RTL8212B Version C config - Phy0Id = %d ###\n", rtl8214fb_phy0_macid);
            else
                OSAL_PRINTF("### RTL8214B Version C config - Phy0Id = %d ###\n", rtl8214fb_phy0_macid);

            /* RTL8214FB Revision C Per-Chip */
#if defined(CONFIG_RTL8390)
            for (i=0; i<(sizeof(rtl8214fb_rtl8390_perchip_revC_1)/sizeof(confcode_prv_t)); i++)
            {
                gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + rtl8214fb_rtl8390_perchip_revC_1[i].phy, \
                        gMacDrv->miim_max_page,
                        (int)rtl8214fb_rtl8390_perchip_revC_1[i].reg,
                        (int)rtl8214fb_rtl8390_perchip_revC_1[i].val);
            }

            OSAL_MDELAY(100);

            for (i=0; i<(sizeof(rtl8214fb_rtl8390_perchip_revC_2)/sizeof(confcode_prv_t)); i++)
            {
                gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + rtl8214fb_rtl8390_perchip_revC_2[i].phy, \
                        gMacDrv->miim_max_page,
                        (int)rtl8214fb_rtl8390_perchip_revC_2[i].reg,
                        (int)rtl8214fb_rtl8390_perchip_revC_2[i].val);
            }

            OSAL_MDELAY(100);

            for (i=0; i<(sizeof(rtl8214fb_rtl8390_perchip_revC_3)/sizeof(confcode_prv_t)); i++)
            {
                gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + rtl8214fb_rtl8390_perchip_revC_3[i].phy, \
                        gMacDrv->miim_max_page,
                        (int)rtl8214fb_rtl8390_perchip_revC_3[i].reg,
                        (int)rtl8214fb_rtl8390_perchip_revC_3[i].val);
            }

            /* The configuration must be keep for 839x/5x, which isn't in the patch */
            /* Disable serdes pre-emphasis */
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid,
                    gMacDrv->miim_max_page, 0x1f, 0xf);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid,
                    gMacDrv->miim_max_page, 0x1e, 0xD);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid,
                    gMacDrv->miim_max_page, 0x18, 0x8088);

            //"Auto Mode Standard Register ..."
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 1,
                    gMacDrv->miim_max_page, 31, 8);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 1,
                    gMacDrv->miim_max_page, 16, 0x0000);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 1,
                    gMacDrv->miim_max_page, 31, 8);
            OSAL_MDELAY(1000);

            //"Reset Ser-Des ..."
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,
                    gMacDrv->miim_max_page, 31, 8);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,
                    gMacDrv->miim_max_page, 28, 0xFF00);
            OSAL_MDELAY(100);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,
                    gMacDrv->miim_max_page, 28, 0x0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,
                    gMacDrv->miim_max_page, 31, 8);

            OSAL_MDELAY(1000);
#elif defined(CONFIG_RTL8380)
            for (i=0; i<(sizeof(rtl8214fb_rtl8380_perchip_revC_1)/sizeof(confcode_prv_t)); i++)
            {
                gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + rtl8214fb_rtl8380_perchip_revC_1[i].phy, \
                        gMacDrv->miim_max_page,
                        (int)rtl8214fb_rtl8380_perchip_revC_1[i].reg,
                        (int)rtl8214fb_rtl8380_perchip_revC_1[i].val);
            }

            OSAL_MDELAY(100);

            for (i=0; i<(sizeof(rtl8214fb_rtl8380_perchip_revC_2)/sizeof(confcode_prv_t)); i++)
            {
                gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + rtl8214fb_rtl8380_perchip_revC_2[i].phy, \
                        gMacDrv->miim_max_page,
                        (int)rtl8214fb_rtl8380_perchip_revC_2[i].reg,
                        (int)rtl8214fb_rtl8380_perchip_revC_2[i].val);
            }

            OSAL_MDELAY(100);

            for (i=0; i<(sizeof(rtl8214fb_rtl8380_perchip_revC_3)/sizeof(confcode_prv_t)); i++)
            {
                gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + rtl8214fb_rtl8380_perchip_revC_3[i].phy, \
                        gMacDrv->miim_max_page,
                        (int)rtl8214fb_rtl8380_perchip_revC_3[i].reg,
                        (int)rtl8214fb_rtl8380_perchip_revC_3[i].val);
            }

            //"Auto Mode Standard Register ..."
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 1,
                    gMacDrv->miim_max_page, 31, 8);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 1,
                    gMacDrv->miim_max_page, 16, 0x0000);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 1,
                    gMacDrv->miim_max_page, 31, 8);
            OSAL_MDELAY(1000);

            //"Reset Ser-Des ..."
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,
                    gMacDrv->miim_max_page, 31, 8);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,
                    gMacDrv->miim_max_page, 28, 0xFF00);
            OSAL_MDELAY(100);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,
                    gMacDrv->miim_max_page, 28, 0x0);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 3,
                    gMacDrv->miim_max_page, 31, 8);

            OSAL_MDELAY(1000);
#else
            for (i=0; i<(sizeof(rtl8214fb_perchip_revC)/sizeof(confcode_prv_t)); i++)
            {
                gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + rtl8214fb_perchip_revC[i].phy, \
                    gMacDrv->miim_max_page, (int)rtl8214fb_perchip_revC[i].reg, (int)rtl8214fb_perchip_revC[i].val);
                //printf("rtk phyreg set %u 0x%04x 0x%04x 0x%04x\n", rtl8214fb_phy0_macid + rtl8214fb_perchip_revC[i].phy, gMacDrv->miim_max_page, (int)rtl8214fb_perchip_revC[i].reg, (int)rtl8214fb_perchip_revC[i].val);
                if (rtl8214fb_perchip_revC[i].reg == 0x17)
                {
                    switch (rtl8214fb_perchip_revC[i].val)
                    {
                        case 0x1111:
                            //OSAL_PRINTF("### Macro-C lock: delay 100ms ###\n");
                            OSAL_MDELAY(100);
                            break;
                        case 0x1910:
                            //OSAL_PRINTF("### Macro-C unlock: delay 100ms ###\n");
                            OSAL_MDELAY(100);
                            break;
                        default:
                            break;
                    }
                }
            }
#endif
        }/*end Version C*/

        /* Write the PHY 0-3, Page 8, Register 30 bit[7:6] = 0b00 */
        for (i=0; i<4; i++)
        {
            gMacDrv->drv_miim_read(rtl8214fb_phy0_macid+i, 8, 30, &val);
            val &= ~(1<<7);
            val &= ~(1<<6);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid+i, 8, 30, val);
        }

        /* power down PHY in copper & fiber media */
        for (i=0; i<4; i++)
        {
            gMacDrv->drv_miim_read(rtl8214fb_phy0_macid+i, 8, 30, &forceReg);    /* store the original register value */

            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid+i, 8, 30, (forceReg & ~(0x3<<12)) | (0x3<<12));
            gMacDrv->drv_miim_read(rtl8214fb_phy0_macid+i, 0, 0, &val);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid+i, 0, 0, (val | (0x1 << 11)));

            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid+i, 8, 30, (forceReg & ~(0x3<<12)) | (0x2<<12));
            gMacDrv->drv_miim_read(rtl8214fb_phy0_macid+i, 0, 0, &val);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid+i, 0, 0, (val | (0x1 << 11)));

            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid+i, 8, 30, forceReg);   /* restore the original register value */
        }

        /* power down the serdes 1 for RTL8212B */
        if (is_fb == 0 && is_12b == 1)
        {
            gMacDrv->drv_miim_read(rtl8214fb_phy0_macid+3, 8, 28, &val);
            val |= 0xc000;
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid+3, 8, 28, val);
        }

        /* Configure option when 1000-X Nway is failure, try force mode */
        for (i=0; i<4; i++)
        {
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid+i, 8, 31, 0x000F);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid+i, 15, 30, 0x0018);
            gMacDrv->drv_miim_write(rtl8214fb_phy0_macid+i, 15, 25, 0x074D);
        }
    }
    return;
} /* end of rtl8214fb_config */

/* Function Name:
 *      rtl8328_rtl8214f_config
 * Description:
 *      Configuration code for RTL8214F for RTL8328M.
 * Input:
 *      phyId - the phyid of PHY0 of the RTL8214F
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8328_rtl8214f_config(Tuint8 phyid)
{
    int rtl8214f_phy0_macid = phyid;
    int i;
    unsigned int restore_val;

    /* RTL8214F Whole-Chip */
    OSAL_PRINTF("### RTL8214F config - Phy0Id = %d ###\n", rtl8214f_phy0_macid);

    /* Patch some 8328 MAC auto config phy register back to chip default
     * PHY 0~3, page 0, reg 4, value 0x01A0 (for fiber-1000)
     * PHY 0~3, page 0, reg 9, value 0x0000 (for fiber-1000)
     */
    gMacDrv->drv_miim_read(rtl8214f_phy0_macid+1, 8, 16, &restore_val);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid+1, 8, 16, 0xFF24);
    for (i=0; i<4; i++)
    {
        gMacDrv->drv_miim_write(rtl8214f_phy0_macid+i, 8, 4, 0x01A0);
        gMacDrv->drv_miim_write(rtl8214f_phy0_macid+i, 8, 9, 0x0000);
    }
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid+1, 8, 16, restore_val);


   /* config 8214F */
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid, 8, 21, 0x1643);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid, 9, 21, 0xDD0A);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid, 9, 19, 0x5865);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 1, 9, 21, 0xDD0A);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 1, 9, 19, 0x5865);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 2, 8, 28, 0x0000);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 2, 8, 22, 0xFF00);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 2, 8, 16, 0x20E4);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 2, 8, 17, 0x026B);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 2, 8, 18, 0x0293);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 1, 8, 21, 0x0408);

    gMacDrv->drv_miim_write(rtl8214f_phy0_macid, 8, 19, 0xFF57);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid, 0, 20, 0x0040);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 1, 0, 20, 0x0040);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 2, 0, 20, 0x0040);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 3, 0, 20, 0x0040);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid, 1, 14, 0x7BAF);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 1, 1, 14, 0x7BAF);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 2, 1, 14, 0x7BAF);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 3, 1, 14, 0x7BAF);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid, 3, 23, 0x2800);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid, 3, 24, 0x3C3C);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid, 3, 25, 0x3C3C);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 1, 3, 23, 0x2800);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 1, 3, 24, 0x3C3C);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 1, 3, 25, 0x3C3C);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 2, 3, 23, 0x2800);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 2, 3, 24, 0x3C3C);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 2, 3, 25, 0x3C3C);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 3, 3, 23, 0x2800);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 3, 3, 24, 0x3C3C);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 3, 3, 25, 0x3C3C);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid, 9, 16, 0x8B18);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid, 9, 24, 0x8B18);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 1, 9, 16, 0x8B18);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 1, 9, 24, 0x8B18);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 2, 9, 16, 0x8B18);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 2, 9, 24, 0x8B18);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 3, 9, 16, 0x8B18);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 3, 9, 24, 0x8B18);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid, 9, 16, 0x8A18);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid, 9, 24, 0x8A18);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 1, 9, 16, 0x8A18);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 1, 9, 24, 0x8A18);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 2, 9, 16, 0x8A18);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 2, 9, 24, 0x8A18);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 3, 9, 16, 0x8A18);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 3, 9, 24, 0x8A18);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid, 8, 0, 0x1340);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 1, 8, 0, 0x1340);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 2, 8, 0, 0x1340);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 3, 8, 0, 0x1340);
    gMacDrv->drv_miim_write(rtl8214f_phy0_macid + 1, 8, 16, 0x0F24);

    return;
} /* end of rtl8328_rtl8214f_config */


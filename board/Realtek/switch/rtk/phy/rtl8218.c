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
#include <rtk/phy/conf/conftypes.h>
#include <rtk/phy/conf/conf_rtl8218.c>
#include <rtk/phy/conf/conf_rtl8218a.c>

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

/*
 * Function Declaration
 */
/* Function Name:
 *      rtl8218_config
 * Description:
 *      Configuration code for RTL8218.
 * Input:
 *      macId - the macId of PHY0 of the RTL8218
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8218_config(Tuint8 macId)
{
    unsigned int rtl8218_macid_pm = (0xFF << macId);
    int rtl8218_phy0_macid = macId;
    unsigned int val;
    int i;

    OSAL_PRINTF("### RTL8218 config - PhyIdMask = 0x%08x ###\n", rtl8218_macid_pm);

    /*
     * RTL8218 formal version
     */

    gMacDrv->drv_miim_write(rtl8218_phy0_macid, gMacDrv->miim_max_page, 31, 0x000e);
    gMacDrv->drv_miim_write(rtl8218_phy0_macid, gMacDrv->miim_max_page, 23, 0x130f);
    gMacDrv->drv_miim_write(rtl8218_phy0_macid, gMacDrv->miim_max_page, 24, 0x0840 | (rtl8218_phy0_macid << 0));
    gMacDrv->drv_miim_write(rtl8218_phy0_macid, gMacDrv->miim_max_page, 21, 0x0003);

    /* RTL8218 (Serdes) Per-Chip */
    for (i=0; i<(sizeof(rtl8218_serdes_perchip)/sizeof(confcode_pprmv_t)); i++)
    {
        gMacDrv->drv_miim_read(rtl8218_phy0_macid + rtl8218_serdes_perchip[i].phy, \
            rtl8218_serdes_perchip[i].page, (int)rtl8218_serdes_perchip[i].reg, &val);

        VALUE_CHG(val, rtl8218_serdes_perchip[i].mask, rtl8218_serdes_perchip[i].val);

        gMacDrv->drv_miim_write(rtl8218_phy0_macid + rtl8218_serdes_perchip[i].phy, \
            rtl8218_serdes_perchip[i].page, (int)rtl8218_serdes_perchip[i].reg, val);

#ifdef DBGMSG
        OSAL_PRINTF("WrPhy[%d], page %d, reg %d, val = 0x%04x\n", \
            rtl8218_phy0_macid + rtl8218_serdes_perchip[i].phy, \
            rtl8218_serdes_perchip[i].page, \
            (int)rtl8218_serdes_perchip[i].reg, \
            val);
#endif
    }

    /* check RTL8218 */
    gMacDrv->drv_miim_write(rtl8218_phy0_macid, 5, 5, 0x3ffe);
    gMacDrv->drv_miim_read(rtl8218_phy0_macid, 5, 6, &val);
    if (val == 0x94eb)
    {
        /* per-port */
        for (i=0; i<(sizeof(rtl8218a_perport)/sizeof(confcode_rv_t)); i++)
        {
            //printf("REG: 0x%02x, VAL: 0x%04X\n", rtl8218_perport[i].reg, rtl8218_perport[i].val);
            gMacDrv->drv_miim_portmask_write(rtl8218_macid_pm, gMacDrv->miim_max_page, (int)rtl8218a_perport[i].reg, \
                (int)rtl8218a_perport[i].val);
        }

        /* per-chip */
        for (i=0; i<(sizeof(rtl8218a_perchip)/sizeof(confcode_prv_t)); i++)
        {
            gMacDrv->drv_miim_portmask_write(0x1 << (rtl8218_phy0_macid + rtl8218a_perchip[i].phy), gMacDrv->miim_max_page, \
                (int)rtl8218a_perchip[i].reg, (int)rtl8218a_perchip[i].val);
        }
    }
    else if (val == 0x2104)
    {
        /* per-port */
        for (i=0; i<(sizeof(rtl8218_perport)/sizeof(confcode_rv_t)); i++)
        {
            //printf("REG: 0x%02x, VAL: 0x%04X\n", rtl8218_perport[i].reg, rtl8218_perport[i].val);
            gMacDrv->drv_miim_portmask_write(rtl8218_macid_pm, gMacDrv->miim_max_page, (int)rtl8218_perport[i].reg, \
                (int)rtl8218_perport[i].val);
        }

        /* per-chip */
        for (i=0; i<(sizeof(rtl8218_perchip)/sizeof(confcode_prv_t)); i++)
        {
            gMacDrv->drv_miim_portmask_write(0x1 << (rtl8218_phy0_macid + rtl8218_perchip[i].phy), gMacDrv->miim_max_page, \
                (int)rtl8218_perchip[i].reg, (int)rtl8218_perchip[i].val);
        }
    }

    /* Update RTCT threshold */
    gMacDrv->drv_miim_portmask_write(rtl8218_macid_pm, 3, 13, 0x0206);

    /* check the PHY register status and recovery, if needed */
    {
        Tuint32 p, e, r;
        
        for (i=0; i<8; i++)
        {
            p = i + macId;

            e = 0;
            gMacDrv->drv_miim_read(p, 0, 21, &val);
            if (val != 0x1006)
            {
                //DBG_PRINT(0, "RTL8218 Phy Reg 21 Err: port %d, val = 0x%04x\n", p, val);
                r = 0;
                do {
                    //DBG_PRINT(0, "RTL8218 Phy Reg 21 Recovery: retry = %d\n", r);
                    gMacDrv->drv_miim_write(p, 0, 21, 0x1006);
                    gMacDrv->drv_miim_read(p, 0, 21, &val);
                    r++;
                } while ((val != 0x1006) && (r <= 10));
                e |= (1 << 0);
            }
            gMacDrv->drv_miim_read(p, 0, 22, &val);
            if (val != 0x4100)
            {
                //DBG_PRINT(0, "RTL8218 Phy Reg 22 Err: port %d, val = 0x%04x\n", p, val);
                r = 0;
                do {
                    //DBG_PRINT(0, "RTL8218 Phy Reg 22 Recovery: retry = %d\n", r);
                    gMacDrv->drv_miim_write(p, 0, 22, 0x4100);
                    gMacDrv->drv_miim_read(p, 0, 22, &val);
                    r++;
                } while ((val != 0x4100) && (r <= 10));
                e |= (1 << 1);
            }
            gMacDrv->drv_miim_read(p, 0, 23, &val);
            if (val != 0x2100)
            {
                //DBG_PRINT(0, "RTL8218 Phy Reg 23 Err: port %d, val = 0x%04x\n", p, val);
                r = 0;
                do {
                    //DBG_PRINT(0, "RTL8218 Phy Reg 23 Recovery: retry = %d\n", r);
                    gMacDrv->drv_miim_write(p, 0, 23, 0x2100);
                    gMacDrv->drv_miim_read(p, 0, 23, &val);
                    r++;
                } while ((val != 0x2100) && (r <= 10));
                e |= (1 << 2);
            }
            gMacDrv->drv_miim_read(p, 0, 24, &val);
            if (val != 0x0000)
            {
                //DBG_PRINT(0, "RTL8218 Phy Reg 24 Err: port %d, val = 0x%04x\n", p, val);
                r = 0;
                do {
                    //DBG_PRINT(0, "RTL8218 Phy Reg 24 Recovery: retry = %d\n", r);
                    gMacDrv->drv_miim_write(p, 0, 24, 0x0000);
                    gMacDrv->drv_miim_read(p, 0, 24, &val);
                    r++;
                } while ((val != 0x0000) && (r <= 10));
                e |= (1 << 3);
            }
            gMacDrv->drv_miim_read(p, 0, 25, &val);
            if (val != 0x8C00)
            {
                //DBG_PRINT(0, "RTL8218 Phy Reg 25 Err: port %d, val = 0x%04x\n", p, val);
                r = 0;
                do {
                    //DBG_PRINT(0, "RTL8218 Phy Reg 25 Recovery: retry = %d\n", r);
                    gMacDrv->drv_miim_write(p, 0, 25, 0x8C00);
                    gMacDrv->drv_miim_read(p, 0, 25, &val);
                    r++;
                } while ((val != 0x8C00) && (r <= 10));

                e |= (1 << 4);
            }
            gMacDrv->drv_miim_read(p, 0, 9, &val);
            if (val != 0x0E00)
            {
                //DBG_PRINT(0, "RTL8218 Phy Reg 9 Err: port %d, val = 0x%04x\n", p, val);
                r = 0;
                do {
                    //DBG_PRINT(0, "RTL8218 Phy Reg 9 Recovery: retry = %d\n", r);
                    gMacDrv->drv_miim_write(p, 0, 9, 0x0E00);
                    gMacDrv->drv_miim_read(p, 0, 9, &val);
                    r++;
                } while ((val != 0x0E00) && (r <= 10));
                e |= (1 << 5);
            }

            if (e != 0)
            {
                //DBG_PRINT(0, "RTL8218 port %d, page0 reg0 write 0x1940\n", p);
                gMacDrv->drv_miim_write(p, 0, 0, 0x1940);
            }
        }
    }

    /* finally, park in reg 31, page 0 */
    gMacDrv->drv_miim_portmask_write(rtl8218_macid_pm, gMacDrv->miim_max_page, 31, 0x0000);

    return;
} /* end of rtl8218_config */

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
#include <rtk_osal.h>
#include <rtk_debug.h>
#include <rtk_switch.h>
#include <common/util.h>
#include <rtk/phy/conf/conftypes.h>
#include <rtk/phy/conf/conf_rtl8218b.c>
#include <rtk/mac/rtl8390/rtl8390_drv.h>
#include <config.h>
#if defined(CONFIG_RTL8380)
#include <rtk_reg.h>
#endif


/*
 * Symbol Definition
 */
#define MEDIATYPE_COPPER        (0)
#define MEDIATYPE_FIBER         (1)
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

#define RTL8218B_CHIP_ID            0x6276

#define RTL8218B_TCA_CHIP_ID        0x0455
#define RTL8218B_INT_TCA_CHIP_ID    0x0477
#define RTL8218B_INT_MP_A_CHIP_ID   0x6276
#define RTL8218B_INT_MP_B_CHIP_ID   0x6275
#define RTL8218B_TCA_CUT_A          0x0
#define RTL8218B_TCA_CUT_B          0x1
/*PHY: 8218FB*/
#define RTL8218FB_TCA_CHIP_ID       0x0455
#define RTL8218FB_TCA_CUT_A         0x0
#define RTL8218FB_TCA_CUT_B         0x1

/*
 * Function Declaration
 */

/* Function Name:
 *      rtl8214fc_media_set
 * Description:
 *      Set meida of port.
 * Input:
 *      portid - Port number
 *      media  - Media
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8214fc_media_set(int portid, int media)
{
    int basePort;
    int phy_idx, portIdx;
    unsigned int val;
    int reg;
    unsigned int power;

    if (gSwitchModel == NULL)
        return;

    if (gMacDrv == NULL)
        return;

    if ((media != MEDIATYPE_COPPER) && (media != MEDIATYPE_FIBER))
        return;

    if (rtk_portIdxFromMacId(portid, &portIdx) < 0)
        return;

    phy_idx = gSwitchModel->port.list[portIdx].phy_idx;

    switch (gSwitchModel->phy.list[phy_idx].chip)
    {
        case RTK_CHIP_RTL8214FC:
            basePort = portid - (portid % 4);
            break;
        case RTK_CHIP_RTL8218FB:
            if (0 == gSwitchModel->port.list[portid].phy / 4)
                return;
            basePort = portid - (portid % 8);
            break;
        default:
            return;
    }

    switch (portid % 4)
    {
        case 0:
            reg = 16;
            break;
        case 1:
            reg = 19;
            break;
        case 2:
            reg = 20;
            break;
        case 3:
            reg = 21;
            break;
        default:
            return ;
    }

    gMacDrv->drv_miim_write(basePort, gMacDrv->miim_max_page, 29, 8);
    gMacDrv->drv_miim_read(basePort, 0x266, reg, &val);

    switch (media)
    {
        case MEDIATYPE_COPPER:
            val |= (1<<10);
            val |= (1<<11);
            /* power off fiber */
            gMacDrv->drv_miim_write(portid, gMacDrv->miim_max_page, 29, 3);
            gMacDrv->drv_miim_read(portid, 0, 16, &power);
            if (!(power & (1 << 11)))
            {
                power |= (1 << 11);
                gMacDrv->drv_miim_write(portid, 0, 16, power);
            }
            break;
        case MEDIATYPE_FIBER:
            val |= (1<<10);
            val &= ~(1<<11);
            /* power off copper */
            gMacDrv->drv_miim_write(portid, gMacDrv->miim_max_page, 29, 1);
            gMacDrv->drv_miim_read(portid, 0xa40, 16, &power);
            if (!(power & (1 << 11)))
            {
                power |= (1 << 11);
                gMacDrv->drv_miim_write(portid, 0xa40, 16, power);
            }
            break;
        case MEDIATYPE_COPPER_AUTO:
            val &= ~(1<<10);
            val |= (1<<11);

            break;
        case MEDIATYPE_FIBER_AUTO:
            val &= ~(1<<10);
            val &= ~(1<<11);
            break;
        default:
            return;
    }

    gMacDrv->drv_miim_write(basePort, gMacDrv->miim_max_page, 29, 8);
    gMacDrv->drv_miim_write(basePort, 0x266, reg, val);
    gMacDrv->drv_miim_write(basePort, gMacDrv->miim_max_page, 29, 0);

    /* power on fiber */
    if (media != MEDIATYPE_COPPER)
    {
        gMacDrv->drv_miim_write(portid, gMacDrv->miim_max_page, 29, 3);
        gMacDrv->drv_miim_read(portid, 0, 16, &power);
        if (power & (1 << 11))
        {
            power &= ~(1 << 11);
            gMacDrv->drv_miim_write(portid, 0, 16, power);
        }
    }

    /* power on copper */
    if (media != MEDIATYPE_FIBER)
    {
        gMacDrv->drv_miim_write(portid, gMacDrv->miim_max_page, 29, 1);
        gMacDrv->drv_miim_read(portid, 0xa40, 16, &power);
        if (power & (1 << 11))
        {
            power &= ~(1 << 11);
            gMacDrv->drv_miim_write(portid, 0xa40, 16, power);
        }
    }

    gMacDrv->drv_miim_write(portid, gMacDrv->miim_max_page, 29, 0);

    return;
} /* end of rtl8214fc_media_set */

/* Function Name:
 *      rtl8214fc_media_get
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
void rtl8214fc_media_get(int portid, int *media)
{
    int basePort;
    int phy_idx, portIdx;
    unsigned int val;
    unsigned int reg;

    if (gSwitchModel == NULL)
        return;

    if (gMacDrv == NULL)
        return;

    if (NULL == media)
        return;

    if (rtk_portIdxFromMacId(portid, &portIdx) < 0)
        return;

    phy_idx = gSwitchModel->port.list[portIdx].phy_idx;

    switch (gSwitchModel->phy.list[phy_idx].chip)
    {
        case RTK_CHIP_RTL8214FC:
            basePort = portid - (portid % 4);
            break;
        case RTK_CHIP_RTL8218FB:
            if (0 == gSwitchModel->port.list[portid].phy / 4)
                return;
            basePort = portid - (portid % 8);
            break;
        default:
            return;
    }

    switch (portid % 4)
    {
        case 0:
            reg = 16;
            break;
        case 1:
            reg = 19;
            break;
        case 2:
            reg = 20;
            break;
        case 3:
            reg = 21;
            break;
        default:
            return ;
    }

    gMacDrv->drv_miim_write(basePort, gMacDrv->miim_max_page, 29, 8);
    gMacDrv->drv_miim_read(basePort, 0x266, reg, &val);
    gMacDrv->drv_miim_write(basePort, gMacDrv->miim_max_page, 29, 0);

    if (val & (1 << 10))
    {
        if (val & (1 << 11))
            *media = MEDIATYPE_COPPER;
        else
            *media = MEDIATYPE_FIBER;
    }
    else
    {
        if (val & (1 << 11))
            *media = MEDIATYPE_COPPER_AUTO;
        else
            *media = MEDIATYPE_FIBER_AUTO;
    }

    return;
} /* end of rtl8214fc_media_get */

void rtl8214fc_fiber_watchdog(int port)
{
    int media;
    unsigned int val, backup_reg10_data;
    int chk_loop, chk_error = 0;

    rtl8214fc_media_get(port, &media);
    /* fiber media */
    if (MEDIATYPE_FIBER == media)
    {
        /* only work for giga */
        gMacDrv->drv_miim_read(port, 0, 0, &val);
        if (0 != ((val >> 13) & 0x1) || 1 != ((val >> 6) & 0x1))
            return;

        gMacDrv->drv_miim_read(port, 0, 1, &val);
        gMacDrv->drv_miim_read(port, 0, 1, &val);

        if((val & 0x4) == 0) /*Checking Port is Link Down*/
        {
            gMacDrv->drv_miim_write(port, 0, 30, 0x3); /*Write page 0 reg 30 = 0x3*/
            gMacDrv->drv_miim_write(port, 0xf, 0x10, 0x10); /*Write page 0xf reg 0x10 = 0x10*/

            /*Check Error counter three times*/
            for(chk_loop = 0; chk_loop < 3; chk_loop++)
            {
                gMacDrv->drv_miim_read(port, 0xf, 0x11, &val); /*Read Error counter*/
                if(val == 0xffff)
                    chk_error++;
            }

            if(chk_error > 2)
            { /*Error happen*/
                /*Reset RX*/
                gMacDrv->drv_miim_write(port, 0, 30, 0x3); /*Write page 0 reg 30 = 0x3*/
                gMacDrv->drv_miim_read(port, 0x8, 0x10, &backup_reg10_data);
                val = (backup_reg10_data & (0xffffffd));
                gMacDrv->drv_miim_write(port, 0x8, 0x10, val); /*Set bit1 to 0 to reset RX*/
                val = (backup_reg10_data | 0x2);
                gMacDrv->drv_miim_write(port, 0x8, 0x10, val); /*Set bit1 to 1 to reset RX*/
                //gMacDrv->drv_miim_write(unit, port_index, 0, 30, 0x0); /*Write page 0 reg 30 = 0x0*/
            }
        }
    }

    return;
}


#if defined(CONFIG_RTL8390)
#define CHECK_TIMES     (150000)

/* Function Name:
 *      rtl8218b_rtl8390_config
 * Description:
 *      Configuration code for RTL8218b.
 * Input:
 *      macId - the macId of PHY0 of the RTL8218b
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8218b_rtl8390_config(Tuint8 macId)
{
    unsigned long long rtl8218_macid_pm = (0xFFULL << macId);
    int rtl8218_phy0_macid = macId - (macId % 8);
    unsigned int val;
    int i, port_id, base_id;
    unsigned int rl_no = 0, ver_no = 0, cut_id, romId;

    base_id = rtl8218_phy0_macid;

    gMacDrv->drv_miim_read(base_id, 0, 3, &val);

    if ((val & 0x3FF) == 0x180)
    {
        //OSAL_PRINTF("### RTL8218B config - MAC ID = %d ###\n", base_id);
        //OSAL_PRINTF("DBG: rtl8218_macid_pm = 0x%08X %08X\n", *(((unsigned int *)&rtl8218_macid_pm) + 0), *(((unsigned int *)&rtl8218_macid_pm) + 1));

        /* Enable PHY */
        gMacDrv->drv_miim_write(rtl8218_phy0_macid, gMacDrv->miim_max_page, 31, 0x000e);
        gMacDrv->drv_miim_write(rtl8218_phy0_macid, gMacDrv->miim_max_page, 23, 0x130f);
        gMacDrv->drv_miim_write(rtl8218_phy0_macid, gMacDrv->miim_max_page, 24, 0x0840 | ((rtl8218_phy0_macid % 24) << 0));
        gMacDrv->drv_miim_write(rtl8218_phy0_macid, gMacDrv->miim_max_page, 21, 0x0003);

        /* Serdes (Per-Chip) */
        for (i=0; i<(sizeof(rtl8218b_rtl8390_serdes_perchip)/sizeof(confcode_prv_t)); i++)
        {
#if 0
            int readback_val;
#endif
            gMacDrv->drv_miim_write(rtl8218_phy0_macid + rtl8218b_rtl8390_serdes_perchip[i].phy, \
                gMacDrv->miim_max_page, \
                (int)rtl8218b_rtl8390_serdes_perchip[i].reg, \
                (int)rtl8218b_rtl8390_serdes_perchip[i].val);

            OSAL_MDELAY(1);

#if 0
            /* read back and check the value */
            gMacDrv->drv_miim_read(rtl8218_phy0_macid + rtl8218b_rtl8390_serdes_perchip[i].phy, \
                gMacDrv->miim_max_page, \
                (int)rtl8218b_rtl8390_serdes_perchip[i].reg, \
                &readback_val);
            if (readback_val != (int)rtl8218b_rtl8390_serdes_perchip[i].val)
            {
                printf("WARN: rtl8218b_rtl8390_serdes_perchip: mac %u reg 0x%04X != 0x%04X (real: 0x%04X)\n", \
                    rtl8218_phy0_macid + rtl8218b_rtl8390_serdes_perchip[i].phy,
                    (int)rtl8218b_rtl8390_serdes_perchip[i].reg, \
                    (int)rtl8218b_rtl8390_serdes_perchip[i].val, \
                    readback_val);
            }
#endif
        }

        /* check RTL8218B */
        gMacDrv->drv_miim_write(rtl8218_phy0_macid, 5, 5, 0x3ffe);
        gMacDrv->drv_miim_read(rtl8218_phy0_macid, 5, 6, &val);

        if (val == 0x2104)
        {
            /* per-port */
            for (i=0; i<(sizeof(rtl8218b_rtl8390_perport)/sizeof(confcode_rv_t)); i++)
            {
#if 0
                int readback_val;
#endif

                gMacDrv->drv_miim_portmask_write(rtl8218_macid_pm, \
                    gMacDrv->miim_max_page,
                    (int)rtl8218b_rtl8390_perport[i].reg,
                    (int)rtl8218b_rtl8390_perport[i].val);
                OSAL_MDELAY(1);

#if 0
                /* read back and check the value */
                gMacDrv->drv_miim_read(rtl8218_phy0_macid, \
                    gMacDrv->miim_max_page, \
                    (int)rtl8218b_rtl8390_perport[i].reg, \
                    &readback_val);
                if (readback_val != (int)rtl8218b_rtl8390_perport[i].val)
                {
                    printf("WARN: rtl8218b_rtl8390_perport: mac %u reg 0x%04X != 0x%04X (real: 0x%04X)\n", \
                        rtl8218_phy0_macid, \
                        (int)rtl8218b_rtl8390_perport[i].reg, \
                        (int)rtl8218b_rtl8390_perport[i].val, \
                        readback_val);
                }
#endif
            }

            /* per-chip */
            for (i=0; i<(sizeof(rtl8218b_rtl8390_perchip)/sizeof(confcode_prv_t)); i++)
            {
#if 0
                int readback_val;
#endif

                gMacDrv->drv_miim_write(rtl8218_phy0_macid + rtl8218b_rtl8390_perchip[i].phy, \
                    gMacDrv->miim_max_page,
                    (int)rtl8218b_rtl8390_perchip[i].reg,
                    (int)rtl8218b_rtl8390_perchip[i].val);
                OSAL_MDELAY(1);

#if 0
                /* read back and check the value */
                gMacDrv->drv_miim_read(rtl8218_phy0_macid + rtl8218b_rtl8390_perchip[i].phy, \
                    gMacDrv->miim_max_page, \
                    (int)rtl8218b_rtl8390_perchip[i].reg, \
                    &readback_val);
                if (readback_val != (int)rtl8218b_rtl8390_perchip[i].val)
                {
                    printf("WARN: rtl8218b_rtl8390_perchip: mac %u reg 0x%04X != 0x%04X (real: 0x%04X)\n", \
                        rtl8218_phy0_macid + rtl8218b_rtl8390_perchip[i].phy, \
                        (int)rtl8218b_rtl8390_perchip[i].reg, \
                        (int)rtl8218b_rtl8390_perchip[i].val, \
                        readback_val);
                }
#endif
            }
        }
    }
    else
    {
        gMacDrv->drv_miim_write(rtl8218_phy0_macid, gMacDrv->miim_max_page, 29, 0x0001);
        gMacDrv->drv_miim_write(rtl8218_phy0_macid, gMacDrv->miim_max_page, 31, 0x0a43);
        gMacDrv->drv_miim_write(rtl8218_phy0_macid, gMacDrv->miim_max_page, 19, 0x0002);
        gMacDrv->drv_miim_read(rtl8218_phy0_macid, gMacDrv->miim_max_page, 20, &rl_no);

        if (RTL8218B_CHIP_ID == rl_no)
        {
            /* Version */
            gMacDrv->drv_miim_write(rtl8218_phy0_macid, gMacDrv->miim_max_page, 19, 0x0004);
            gMacDrv->drv_miim_read(rtl8218_phy0_macid, gMacDrv->miim_max_page, 20, &ver_no);

            gMacDrv->drv_miim_write(base_id, gMacDrv->miim_max_page, 27, 0x0004);
            gMacDrv->drv_miim_read(base_id, gMacDrv->miim_max_page, 28, &romId);

            //OSAL_PRINTF("### RTL8218B config - MAC ID = %d ###\n", base_id);

            /* Per-Chip */
            for (i = 0; i < (sizeof(rtl8218b_6276A_rtl8390_perchip)/sizeof(confcode_prv_t)); ++i)
            {
                gMacDrv->drv_miim_write(rtl8218_phy0_macid + rtl8218b_6276A_rtl8390_perchip[i].phy, \
                        gMacDrv->miim_max_page, \
                        (int)rtl8218b_6276A_rtl8390_perchip[i].reg,  \
                        rtl8218b_6276A_rtl8390_perchip[i].val);
            }

            for (i = 0; i < (sizeof(rtl8218b_6276A_rtl8390_perchip2)/sizeof(confcode_phy_patch_t)); ++i)
            {
                PHY_PATCH_SET(rtl8218_phy0_macid, gMacDrv->miim_max_page, (unsigned int)rtl8218b_6276A_rtl8390_perchip2[i]);
            }

            for (i = 0; i < (sizeof(rtl8218b_6276A_rtl8390_perchip3)/sizeof(confcode_prv_t)); ++i)
            {
                gMacDrv->drv_miim_write(rtl8218_phy0_macid + rtl8218b_6276A_rtl8390_perchip3[i].phy, \
                        gMacDrv->miim_max_page, \
                        (int)rtl8218b_6276A_rtl8390_perchip3[i].reg,  \
                        rtl8218b_6276A_rtl8390_perchip3[i].val);
            }

            for (port_id = 0; port_id < 8; ++port_id)
            {
                int pid = rtl8218_phy0_macid + port_id;

                for (i = 0; i < (sizeof(rtl8218b_6276A_rtl8390_perport)/sizeof(confcode_rv_t)); ++i)
                    gMacDrv->drv_miim_write(pid, gMacDrv->miim_max_page, \
                            (int)rtl8218b_6276A_rtl8390_perport[i].reg, \
                            (int)rtl8218b_6276A_rtl8390_perport[i].val);
            }

            /* system ESD only for ROM ID 0 ~ 2 */
            if (romId <= 2)
            {
                for (port_id = 0; port_id < 8; ++port_id)
                {
                    rtl8390_phyPortPowerOn(rtl8218_phy0_macid + port_id);
                }

                for (port_id = 0; port_id < 8; ++port_id)
                {
                    int pid = rtl8218_phy0_macid + port_id;
                    int flag;

                    /* check PHY status = lan on */
                    flag = 0;
                    for (i = 0; i < CHECK_TIMES; ++i)
                    {
                        gMacDrv->drv_miim_read(pid, 0xa42, 16, &val);
                        if (3 == (val & 0x7))
                        {
                            flag = 1;
                            break;
                        }
                    }

                    if (0 == flag)
                    {
                        printf("%d lan on fail\n", pid);
                    }

                    /* patch request */
                    gMacDrv->drv_miim_read(pid, 0xb82, 16, &val);
                    val |= (1 << 4);
                    gMacDrv->drv_miim_write(pid, 0xb82, 16, val);

                    /* polling patch ready */
                    flag = 0;
                    for (i = 0; i < CHECK_TIMES; ++i)
                    {
                        gMacDrv->drv_miim_read(pid, 0xb80, 16, &val);
                        if ((val & 0x40) != 0)
                        {
                            flag = 1;
                            break;
                        }
                    }

                    if (0 == flag)
                    {
                        printf("%d ready fail\n", pid);
                    }

                    /* input key */
                    gMacDrv->drv_miim_write(pid, gMacDrv->miim_max_page, 27, 0x8146);
                    val = 0x7600 | romId;
                    gMacDrv->drv_miim_write(pid, gMacDrv->miim_max_page, 28, val);

                    /* Nctrl reset PCS RX */
                    for (i = 0; i < (sizeof(rtl8218b_6276_rtl8390_patch)/sizeof(confcode_phy_patch_t)); ++i)
                        PHY_PATCH_SET(pid, gMacDrv->miim_max_page, rtl8218b_6276_rtl8390_patch[i]);
                }

                for (port_id = 0; port_id < 8; ++port_id)
                {
                    rtl8390_phyPortPowerOff(rtl8218_phy0_macid + port_id);
                }
            }   /* end of if (romId <= 2) */
        }
        else
        {
            /*Switch to right TOP address*/
            gMacDrv->drv_miim_write(base_id, 0, 29, 0x0000);
            gMacDrv->drv_miim_write(base_id, 0, 31, 0x0a42);

            /*Check the 8218B internal chip ID*/
            gMacDrv->drv_miim_write(base_id, 31, 27, 0x0002);
            gMacDrv->drv_miim_read(base_id, 31, 28, &val);

            /*Check the 8218B internal cut ID*/
            gMacDrv->drv_miim_write(base_id, 31, 27, 0x0004);
            gMacDrv->drv_miim_read(base_id, 31, 28, &cut_id);

            /*Check the 8218B status*/
            gMacDrv->drv_miim_write(base_id, 31, 31, 0xa42);
            gMacDrv->drv_miim_read(base_id, 31, 16, &val);

            //OSAL_PRINTF("### RTL8218B config - MAC ID = %d, Mode = 0x%08x ###\n", base_id, val);
            //OSAL_PRINTF("### RTL8218B config - MAC ID = %d ###\n", base_id);

            if(cut_id == RTL8218B_TCA_CUT_A)
            {
                if(val == 2) /* ext init mode*/
                {
                    for (i=0; i<(sizeof(rtl8218b_0455A_mode2_perchip)/sizeof(confcode_prv_t)); i++)
                    {
                        gMacDrv->drv_miim_write(rtl8218_phy0_macid + rtl8218b_0455A_mode2_perchip[i].phy, \
                            gMacDrv->miim_max_page, (int)rtl8218b_0455A_mode2_perchip[i].reg, rtl8218b_0455A_mode2_perchip[i].val);
                    }

                    for(port_id = 0; port_id < 8; port_id++)
                    {
                        for (i=0; i<(sizeof(rtl8218b_0455A_mode2_perport)/sizeof(confcode_rv_t)); i++)
                        {
                            if((i == 2))
                                udelay(500*1000);  /*Wait for 8018B leave the command state*/
                            gMacDrv->drv_miim_write(rtl8218_phy0_macid + port_id, \
                            gMacDrv->miim_max_page, (int)rtl8218b_0455A_mode2_perport[i].reg, rtl8218b_0455A_mode2_perport[i].val);
                        }
                    }
                }
                else if((val == 3) || (val == 5)) /* lan mode*/
                {
                    for (i=0; i<(sizeof(rtl8218b_0455A_mode3_perchip)/sizeof(confcode_prv_t)); i++)
                    {
                        gMacDrv->drv_miim_write(rtl8218_phy0_macid + rtl8218b_0455A_mode3_perchip[i].phy, \
                            gMacDrv->miim_max_page, (int)rtl8218b_0455A_mode3_perchip[i].reg, rtl8218b_0455A_mode3_perchip[i].val);
                    }

                    for(port_id = 0; port_id < 8; port_id++)
                    {
                        for (i=0; i<(sizeof(rtl8218b_0455A_mode3_perport)/sizeof(confcode_rv_t)); i++)
                        {
                            if((i == 2))
                                udelay(500*1000);  /*Wait for 8018B leave the command state*/
                            gMacDrv->drv_miim_write(rtl8218_phy0_macid + port_id, \
                            gMacDrv->miim_max_page, (int)rtl8218b_0455A_mode3_perport[i].reg, rtl8218b_0455A_mode3_perport[i].val);
                        }
                    }

                    /*Check the 8218B verA Patch Result*/
                    gMacDrv->drv_miim_write(0, 31, 27, 0x8012); /*Reg 27 is PHY internal SRAM address*/
                    gMacDrv->drv_miim_read(0, 31, 28, &val); /*Reg 28 is PHY internal SRAM address value*/
                    if(val != 0x3f0f)
                        DBG_PRINT(0, "RTL8218B TCA Patch Failed 1!!!\n");
                    gMacDrv->drv_miim_write(0, 31, 27, 0x80d1);
                    gMacDrv->drv_miim_read(0, 31, 28, &val);
                    if(val != 0xa5aa)
                        DBG_PRINT(0, "RTL8218B TCA Patch Failed 2!!!\n");
                } else {
                    OSAL_PRINTF("### RTL8218B TCA in ERROR status %d\n", val);
                    return;
                }
            }
            else if(cut_id == RTL8218B_TCA_CUT_B)
            {
                /* Per-Chip */
                for (i=0; i<(sizeof(rtl8218b_0455B_rtl8390_perchip)/sizeof(confcode_prv_t)); i++)
                {
    #if 0
                    int readback_val;
    #endif

                    gMacDrv->drv_miim_write(rtl8218_phy0_macid + rtl8218b_0455B_rtl8390_perchip[i].phy, \
                        gMacDrv->miim_max_page, \
                        (int)rtl8218b_0455B_rtl8390_perchip[i].reg, \
                        (int)rtl8218b_0455B_rtl8390_perchip[i].val);

    #if 0
                    /* read back and check the value */
                    gMacDrv->drv_miim_read(rtl8218_phy0_macid + rtl8218b_0455B_rtl8390_perchip[i].phy, \
                        gMacDrv->miim_max_page, \
                        (int)rtl8218b_0455B_rtl8390_perchip[i].reg, \
                        &readback_val);
                    if (readback_val != (int)rtl8218b_0455B_rtl8390_perchip[i].val)
                    {
                        printf("WARN: rtl8218b_0455B_rtl8390_perchip: mac %u reg 0x%04X != 0x%04X (real: 0x%04X)\n", \
                            rtl8218_phy0_macid + rtl8218b_0455B_rtl8390_perchip[i].phy, \
                            (int)rtl8218b_0455B_rtl8390_perchip[i].reg, \
                            (int)rtl8218b_0455B_rtl8390_perchip[i].val, \
                            readback_val);
                    }
    #endif
                }

                /* Per-Port */
                for(port_id = 0; port_id < 8; port_id++)
                {
                    for (i=0; i<(sizeof(rtl8218b_0455B_rtl8390_perport)/sizeof(confcode_rv_t)); i++)
                    {
    #if 0
                        int readback_val;
    #endif

                        gMacDrv->drv_miim_write(rtl8218_phy0_macid + port_id, \
                        gMacDrv->miim_max_page, \
                        (int)rtl8218b_0455B_rtl8390_perport[i].reg, \
                        (int)rtl8218b_0455B_rtl8390_perport[i].val);

    #if 0
                        /* read back and check the value */
                        gMacDrv->drv_miim_read(rtl8218_phy0_macid + port_id, \
                            gMacDrv->miim_max_page, \
                            (int)rtl8218b_0455B_rtl8390_perport[i].reg, \
                            &readback_val);
                        if (readback_val != (int)rtl8218b_0455B_rtl8390_perport[i].val)
                        {
                            printf("WARN: rtl8218b_0455B_rtl8390_perport: mac %u reg 0x%04X != 0x%04X (real: 0x%04X)\n", \
                                rtl8218_phy0_macid + port_id, \
                                (int)rtl8218b_0455B_rtl8390_perport[i].reg, \
                                (int)rtl8218b_0455B_rtl8390_perport[i].val, \
                                readback_val);
                        }
    #endif
                    }
                }
            }
        }
    }

    return;
} /* end of rtl8218b_rtl8390_config */

#if defined(CONFIG_RTL8218FB)
/* Function Name:
 *      rtl8218fb_rtl8390_config
 * Description:
 *      Configuration code for RTL8218b.
 * Input:
 *      macId - the macId of PHY0 of the RTL8218b
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8218fb_rtl8390_config(Tuint8 macId)
{
    int rtl8218_phy0_macid = macId - (macId % 8);
    unsigned int val;
    int i, port_id, base_id;
    unsigned int rl_no = 0, cut_id, romId;

    base_id = rtl8218_phy0_macid;

    gMacDrv->drv_miim_write(rtl8218_phy0_macid, gMacDrv->miim_max_page, 29, 0x0001);
    gMacDrv->drv_miim_write(rtl8218_phy0_macid, gMacDrv->miim_max_page, 31, 0x0a43);
    gMacDrv->drv_miim_write(rtl8218_phy0_macid, gMacDrv->miim_max_page, 19, 0x0002);
    gMacDrv->drv_miim_read(rtl8218_phy0_macid, gMacDrv->miim_max_page, 20, &rl_no);

    if (RTL8218B_CHIP_ID == rl_no)
    {
        OSAL_PRINTF("### RTL8218FB config - MAC ID = %d ###\n", base_id);

        gMacDrv->drv_miim_write(base_id, gMacDrv->miim_max_page, 27, 0x0004);
        gMacDrv->drv_miim_read(base_id, gMacDrv->miim_max_page, 28, &romId);

        for (i = 0; i < (sizeof(rtl8218fb_6276A_rtl8390_perchip_serdes)/sizeof(confcode_prv_t)); ++i)
        {
            gMacDrv->drv_miim_write(rtl8218_phy0_macid + rtl8218fb_6276A_rtl8390_perchip_serdes[i].phy, \
                    gMacDrv->miim_max_page, \
                    (int)rtl8218fb_6276A_rtl8390_perchip_serdes[i].reg, \
                    (int)rtl8218fb_6276A_rtl8390_perchip_serdes[i].val);
        }

        for (i = 0; i < (sizeof(rtl8218b_6276A_rtl8390_perchip2)/sizeof(confcode_phy_patch_t)); ++i)
        {
            PHY_PATCH_SET(rtl8218_phy0_macid, gMacDrv->miim_max_page, rtl8218b_6276A_rtl8390_perchip2[i]);
        }

        for (i = 0; i < (sizeof(rtl8218fb_6276A_rtl8390_perchip_serdes3)/sizeof(confcode_prv_t)); ++i)
        {
            gMacDrv->drv_miim_write(rtl8218_phy0_macid + rtl8218fb_6276A_rtl8390_perchip_serdes3[i].phy, \
                    gMacDrv->miim_max_page, \
                    (int)rtl8218fb_6276A_rtl8390_perchip_serdes3[i].reg, \
                    (int)rtl8218fb_6276A_rtl8390_perchip_serdes3[i].val);
        }

        for (port_id = 0; port_id < 8; ++port_id)
        {
            int pid = rtl8218_phy0_macid + port_id;

            for (i = 0; i < (sizeof(rtl8218fb_6276A_rtl8390_perport)/sizeof(confcode_rv_t)); ++i)
                gMacDrv->drv_miim_write(pid, gMacDrv->miim_max_page, \
                        (int)rtl8218fb_6276A_rtl8390_perport[i].reg, \
                        (int)rtl8218fb_6276A_rtl8390_perport[i].val);
        }

        gMacDrv->drv_miim_write(rtl8218_phy0_macid, gMacDrv->miim_max_page, \
                0x1e, 0x8);
        gMacDrv->drv_miim_read(rtl8218_phy0_macid, 0x260, 0x13, &val);
        val &= (0x1F << 8);
        val |= 0x4020;
        gMacDrv->drv_miim_write(rtl8218_phy0_macid, 0x260, 0x13, val);
        gMacDrv->drv_miim_write(rtl8218_phy0_macid, gMacDrv->miim_max_page, \
                0x1e, 0x0);

        for (i = 0; i < (sizeof(rtl8218fb_6276A_rtl8390_fiber_perchip)/sizeof(confcode_prv_t)); ++i)
        {
            gMacDrv->drv_miim_write(rtl8218_phy0_macid + rtl8218fb_6276A_rtl8390_fiber_perchip[i].phy, \
                    gMacDrv->miim_max_page, \
                    (int)rtl8218fb_6276A_rtl8390_fiber_perchip[i].reg, \
                    (int)rtl8218fb_6276A_rtl8390_fiber_perchip[i].val);
        }

        if (romId <= 2)
        {
            for (port_id = 0; port_id < 8; ++port_id)
            {
                rtl8390_phyPortPowerOn(rtl8218_phy0_macid + port_id);
            }

            for (port_id = 0; port_id < 8; ++port_id)
            {
                int pid = rtl8218_phy0_macid + port_id;
                int flag;

                /* check PHY status = lan on */
                flag = 0;
                for (i = 0; i < CHECK_TIMES; ++i)
                {
                    gMacDrv->drv_miim_read(pid, 0xa42, 16, &val);
                    if (3 == (val & 0x7))
                    {
                        flag = 1;
                        break;
                    }
                }

                if (0 == flag)
                {
                    printf("%d lan on fail\n", pid);
                }

                /* patch request */
                gMacDrv->drv_miim_read(pid, 0xb82, 16, &val);
                val |= (1 << 4);
                gMacDrv->drv_miim_write(pid, 0xb82, 16, val);

                /* polling patch ready */
                flag = 0;
                for (i = 0; i < CHECK_TIMES; ++i)
                {
                    gMacDrv->drv_miim_read(pid, 0xb80, 16, &val);
                    if ((val & 0x40) != 0)
                    {
                        flag = 1;
                        break;
                    }
                }

                if (0 == flag)
                {
                    printf("%d ready fail\n", pid);
                }

                gMacDrv->drv_miim_write(pid, gMacDrv->miim_max_page, 27, 0x8146);
                val = 0x7600 | romId;
                gMacDrv->drv_miim_write(pid, gMacDrv->miim_max_page, 28, val);

                for (i = 0; i < (sizeof(rtl8218b_6276_rtl8390_patch)/sizeof(confcode_phy_patch_t)); ++i)
                    PHY_PATCH_SET(pid, gMacDrv->miim_max_page, rtl8218b_6276_rtl8390_patch[i]);
            }

            for (port_id = 0; port_id < 8; ++port_id)
            {
                rtl8390_phyPortPowerOff(rtl8218_phy0_macid + port_id);
            }
        }
    }
    else
    {
        /*Switch to right TOP address*/
        gMacDrv->drv_miim_write(base_id, 0, 29, 0x0000);
        gMacDrv->drv_miim_write(base_id, 0, 31, 0x0a42);

        /*Check the 8218B internal chip ID*/
        gMacDrv->drv_miim_write(base_id, 31, 27, 0x0002);
        gMacDrv->drv_miim_read(base_id, 31, 28, &val);

        /*Check the 8218B internal cut ID*/
        gMacDrv->drv_miim_write(base_id, 31, 27, 0x0004);
        gMacDrv->drv_miim_read(base_id, 31, 28, &cut_id);

        /*Check the 8218B status*/
        gMacDrv->drv_miim_write(base_id, 31, 31, 0xa42);
        gMacDrv->drv_miim_read(base_id, 31, 16, &val);

        //OSAL_PRINTF("### RTL8218B config - MAC ID = %d, Mode = 0x%08x ###\n", base_id, val);
        //OSAL_PRINTF("### RTL8218FB config - MAC ID = %d ###\n", base_id);

        if(cut_id == RTL8218FB_TCA_CUT_A)
        {
            OSAL_PRINTF("### RTL8218FB (A-cut) NOT USE RIGHT NOW!\n");
            return;
        }
        else if(cut_id == RTL8218FB_TCA_CUT_B)
        {
            /* Per-Chip */
            for (i=0; i<(sizeof(rtl8218fb_0455B_rtl8390_perchip)/sizeof(confcode_prv_t)); i++)
            {
                gMacDrv->drv_miim_write(rtl8218_phy0_macid + rtl8218fb_0455B_rtl8390_perchip[i].phy, \
                    gMacDrv->miim_max_page, \
                    (int)rtl8218fb_0455B_rtl8390_perchip[i].reg, \
                    (int)rtl8218fb_0455B_rtl8390_perchip[i].val);
            }

            /* Per-Port */
            for(port_id = 0; port_id < 8; port_id++)
            {
                for (i=0; i<(sizeof(rtl8218fb_0455B_rtl8390_perport)/sizeof(confcode_rv_t)); i++)
                {
                    gMacDrv->drv_miim_write(rtl8218_phy0_macid + port_id, \
                    gMacDrv->miim_max_page, \
                    (int)rtl8218fb_0455B_rtl8390_perport[i].reg, \
                    (int)rtl8218fb_0455B_rtl8390_perport[i].val);
                }
            }
        }
    }

    return;
} /* end of rtl8218fb_rtl8390_config */
#endif  /* defined(CONFIG_RTL8218FB) */
#endif


/*
 * Function Declaration
 */
/* Function Name:
 *      rtl8218b_rtl8380_config
 * Description:
 *      Configuration code for RTL8218b.
 * Input:
 *      macId - the macId of PHY0 of the RTL8218b
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
#if defined(CONFIG_RTL8380)
void rtl8218b_rtl8380_config(Tuint8 phyid)
{
    unsigned int rtl8218_macid_pm = (0xFF << phyid);
    int rtl8218_phy0_macid = phyid - (phyid % 8);
    unsigned int val, cut_id;
    int i, port_id, base_id;
    int loop;

    base_id = rtl8218_phy0_macid;

    //OSAL_PRINTF("**************************************************\n");

    gMacDrv->drv_miim_read(rtl8218_phy0_macid, 0, 3, &val);
    if ((val & 0x3FF) == 0x180)
    {
        //OSAL_PRINTF("**** RTL8218B config - MAC ID = %d ****\n", base_id);

        /* Enable PHY */
        gMacDrv->drv_miim_write(rtl8218_phy0_macid, gMacDrv->miim_max_page, 31, 0x000e);
        gMacDrv->drv_miim_write(rtl8218_phy0_macid, gMacDrv->miim_max_page, 23, 0x130f);
        gMacDrv->drv_miim_write(rtl8218_phy0_macid, gMacDrv->miim_max_page, 24, 0x0840 | ((rtl8218_phy0_macid % 24) << 0));
        gMacDrv->drv_miim_write(rtl8218_phy0_macid, gMacDrv->miim_max_page, 21, 0x0003);

        /* Serdes (Per-Chip) */
        for (i=0; i<(sizeof(rtl8218B_rtl8380_serdes_perchip)/sizeof(confcode_prv_t)); i++)
        {
            gMacDrv->drv_miim_write(rtl8218_phy0_macid + rtl8218B_rtl8380_serdes_perchip[i].phy, \
                gMacDrv->miim_max_page, (int)rtl8218B_rtl8380_serdes_perchip[i].reg, rtl8218B_rtl8380_serdes_perchip[i].val);
            OSAL_MDELAY(1);
        }

        /* check RTL8218B */
        gMacDrv->drv_miim_write(rtl8218_phy0_macid, 5, 5, 0x3ffe);
        gMacDrv->drv_miim_read(rtl8218_phy0_macid, 5, 6, &val);

        if (val == 0x2104)
        {
            /* per-port */
            for (i=0; i<(sizeof(rtl8218B_rtl8380_perport)/sizeof(confcode_rv_t)); i++)
            {
                gMacDrv->drv_miim_portmask_write(rtl8218_macid_pm, \
                    gMacDrv->miim_max_page, (int)rtl8218B_rtl8380_perport[i].reg, rtl8218B_rtl8380_perport[i].val);
                OSAL_MDELAY(1);
            }

            /* per-chip */
            for (i=0; i<(sizeof(rtl8218B_rtl8380_perchip)/sizeof(confcode_prv_t)); i++)
            {
                gMacDrv->drv_miim_write(rtl8218_phy0_macid + rtl8218B_rtl8380_perchip[i].phy, \
                    gMacDrv->miim_max_page, (int)rtl8218B_rtl8380_perchip[i].reg, (int)rtl8218B_rtl8380_perchip[i].val);
                OSAL_MDELAY(1);
            }
        }
    }
    else
    {
    /*Check the 8218B internal chip ID*/
    gMacDrv->drv_miim_write(phyid, 31, 27, 0x0002);
    gMacDrv->drv_miim_read(phyid, 31, 28, &val);

    if((val != RTL8218B_TCA_CHIP_ID) && (val != RTL8218B_INT_TCA_CHIP_ID) \
            && (val != RTL8218B_INT_MP_A_CHIP_ID) && (val != RTL8218B_CHIP_ID) && (val != RTL8218B_INT_MP_B_CHIP_ID)){
        OSAL_PRINTF("### Error chip ID = 0x%08x ###\n", val);
        return;
    }

       //OSAL_PRINTF("#### RTL8218B config - MAC ID = %d ####\n", base_id);

       if(RTL8218B_TCA_CHIP_ID == val)
       {
            /*Check the 8218B internal cut ID*/
            gMacDrv->drv_miim_write(base_id, 31, 27, 0x0004);
            gMacDrv->drv_miim_read(base_id, 31, 28, &cut_id);


            /*Check the 8218B status*/
            gMacDrv->drv_miim_write(base_id, 31, 31, 0xa42);
            gMacDrv->drv_miim_read(base_id, 31, 16, &val);

            if(RTL8218B_TCA_CUT_A == cut_id)
            {
                /*Not maintain right now*/
            }
              else if(cut_id == RTL8218B_TCA_CUT_B)
            {
                //OSAL_PRINTF("Now External 8218B B-CUT\n");
                /* Per-Chip */
                for (i=0; i<(sizeof(rtl8218B_0455B_rtl8380_perchip)/sizeof(confcode_prv_t)); i++)
                {
                    gMacDrv->drv_miim_write(rtl8218_phy0_macid + rtl8218B_0455B_rtl8380_perchip[i].phy, \
                        gMacDrv->miim_max_page, (int)rtl8218B_0455B_rtl8380_perchip[i].reg, rtl8218B_0455B_rtl8380_perchip[i].val);
                }

                /*Enable Phy*/
                for(port_id = 0; port_id < 8; port_id++)
                {
                   gMacDrv->drv_miim_write(rtl8218_phy0_macid+port_id, 0xfff, 0x1f, 0x0000);
                   gMacDrv->drv_miim_write(rtl8218_phy0_macid+port_id, 0xfff, 0x00, 0x1140);
               }
               OSAL_MDELAY(100);

                /*Patch request*/
               for(port_id = 0; port_id < 8; port_id++)
                    {
                   gMacDrv->drv_miim_write(rtl8218_phy0_macid+port_id, 0xfff, 0x1f, 0x0b82);
                   gMacDrv->drv_miim_write(rtl8218_phy0_macid+port_id, 0xfff, 0x10, 0x0010);
                    }
                OSAL_MDELAY(300);

               /*Patch ready check*/
               for(port_id = 0; port_id < 8; port_id++)
               {
                    for(loop = 0; loop < 100; loop++)
                    {
                        gMacDrv->drv_miim_read(rtl8218_phy0_macid + port_id, 0xb80, 0x10, &val);
                        if(val & 0x40)break;
                    }

                    if(loop >= 100)
                    {
                        OSAL_PRINTF("Internal phy port %d not ready!\n",port_id+rtl8218_phy0_macid);
                        return;
                }
            }

               /*Use broadcast ID method to pathc internal phy*/
               gMacDrv->drv_miim_write(rtl8218_phy0_macid, 0xfff, 0x1f, 0x0000);
               gMacDrv->drv_miim_write(rtl8218_phy0_macid, 0xfff, 0x001d, 0x0008);
               gMacDrv->drv_miim_write(rtl8218_phy0_macid, 0xfff, 0x001f, 0x266);
               gMacDrv->drv_miim_write(rtl8218_phy0_macid, 0xfff, 0x16, 0xff00+rtl8218_phy0_macid);
               gMacDrv->drv_miim_write(rtl8218_phy0_macid, 0xfff, 0x1f, 0x0000);
               gMacDrv->drv_miim_write(rtl8218_phy0_macid, 0xfff, 0x1d, 0x0000);
               OSAL_MDELAY(1);

                /* Per-Port */
                    for (i=0; i<(sizeof(rtl8218B_0455B_rtl8380_perport)/sizeof(confcode_rv_t)); i++)
            {
                    gMacDrv->drv_miim_write(rtl8218_phy0_macid, \
                        gMacDrv->miim_max_page, (int)rtl8218B_0455B_rtl8380_perport[i].reg, rtl8218B_0455B_rtl8380_perport[i].val);
                    }

               /*Disable broadcast ID*/
               gMacDrv->drv_miim_write(rtl8218_phy0_macid, 0xfff, 0x1f, 0x0000);
               gMacDrv->drv_miim_write(rtl8218_phy0_macid, 0xfff, 0x001d, 0x0008);
               gMacDrv->drv_miim_write(rtl8218_phy0_macid, 0xfff, 0x001f, 0x266);
               gMacDrv->drv_miim_write(rtl8218_phy0_macid, 0xfff, 0x16, 0+rtl8218_phy0_macid);
               gMacDrv->drv_miim_write(rtl8218_phy0_macid, 0xfff, 0x1f, 0x0000);
               gMacDrv->drv_miim_write(rtl8218_phy0_macid, 0xfff, 0x1d, 0x0000);
               OSAL_MDELAY(1);
            }
               else
            {
            /*right now nothing to do*/
            }

       }
       else if(val == RTL8218B_INT_TCA_CHIP_ID)
       {    /*INTERNAL PHY*/
            OSAL_PRINTF("Now 8218B Internal phy!\n");

            /*Enable Phy*/
            MEM32_WRITE(0xbb00a100, 0x000080a8);
            OSAL_MDELAY(100);

            /*Patch request*/
            for(port_id = 0; port_id < 8; port_id++)
            {
                gMacDrv->drv_miim_write(rtl8218_phy0_macid+port_id, 0xfff, 0x1f, 0x0b82);
                gMacDrv->drv_miim_write(rtl8218_phy0_macid+port_id, 0xfff, 0x10, 0x0010);
            }
            OSAL_MDELAY(300);

           /*Patch ready check*/
           for(port_id = 0; port_id < 8; port_id++)
           {
                for(loop = 0; loop < 100; loop++)
                {
                    gMacDrv->drv_miim_read(rtl8218_phy0_macid + port_id, 0xb80, 0x10, &val);
                    if(val & 0x40)break;
                }

                if(loop >= 100)
                     {
                    OSAL_PRINTF("Internal phy port %d not ready!\n",port_id+rtl8218_phy0_macid);
                    return;
                }
            }
           /*Use broadcast ID method to pathc internal phy*/
           MEM32_WRITE(0xbb000120, 0xff2810);
           OSAL_MDELAY(1);

            /* Per-Port */
            for (i=0; i<(sizeof(rtl838x_intPhy_0477B_perport)/sizeof(confcode_rv_t)); i++)
            {
            gMacDrv->drv_miim_write(rtl8218_phy0_macid, \
                gMacDrv->miim_max_page, (int)rtl838x_intPhy_0477B_perport[i].reg, rtl838x_intPhy_0477B_perport[i].val);
            }

           /*Disable broadcast ID*/
           MEM32_WRITE(0xbb000120, 0xff0010);
           OSAL_MDELAY(1);
        }
       else if((val == RTL8218B_INT_MP_A_CHIP_ID) || (val == RTL8218B_CHIP_ID) || (val == RTL8218B_INT_MP_B_CHIP_ID))
       {
                /*MP Chip internal PHY AND external 8218b MP have the same RLNO: 6276,but at last internal phy will be 6275*/
                /*note that the definition of RTL8218B_INT_MP_CHIP_ID should be 0x6275 in B-cut, mantis#0012977*/
                if((rtl8218_phy0_macid == 0) || (rtl8218_phy0_macid == 16))
                {
                        /*External PHY*/
                        //OSAL_PRINTF("Now External 8218B\n");

              /*CHIP Version*/
                    gMacDrv->drv_miim_write(rtl8218_phy0_macid, 0xfff, 0x1f, 0x0);
                    gMacDrv->drv_miim_write(rtl8218_phy0_macid, 0xfff, 0x1b, 0x4);
              gMacDrv->drv_miim_read(rtl8218_phy0_macid, 0xfff, 0x1c, &cut_id);

                        /* Per-Chip */
              if(0 == cut_id)
             {
                            for (i=0; i<(sizeof(rtl8218B_6276A_rtl8380_perchip)/sizeof(confcode_prv_t)); i++)
                            {
                                gMacDrv->drv_miim_write(rtl8218_phy0_macid + rtl8218B_6276A_rtl8380_perchip[i].phy, \
                                    gMacDrv->miim_max_page, (int)rtl8218B_6276A_rtl8380_perchip[i].reg, rtl8218B_6276A_rtl8380_perchip[i].val);
                            }
             }
             else
             {
                            for (i=0; i<(sizeof(rtl8218B_6276B_rtl8380_perchip)/sizeof(confcode_prv_t)); i++)
                            {
                                gMacDrv->drv_miim_write(rtl8218_phy0_macid + rtl8218B_6276B_rtl8380_perchip[i].phy, \
                                    gMacDrv->miim_max_page, (int)rtl8218B_6276B_rtl8380_perchip[i].reg, rtl8218B_6276B_rtl8380_perchip[i].val);
                            }
             }

                        /*Enable Phy*/
                        for(port_id = 0; port_id < 8; port_id++)
                        {
                           gMacDrv->drv_miim_write(rtl8218_phy0_macid+port_id, 0xfff, 0x1f, 0x0000);
                           gMacDrv->drv_miim_write(rtl8218_phy0_macid+port_id, 0xfff, 0x00, 0x1140);
                       }
                       OSAL_MDELAY(100);

                        /*Patch request*/
                       for(port_id = 0; port_id < 8; port_id++)
                       {
                           gMacDrv->drv_miim_write(rtl8218_phy0_macid+port_id, 0xfff, 0x1f, 0x0b82);
                           gMacDrv->drv_miim_write(rtl8218_phy0_macid+port_id, 0xfff, 0x10, 0x0010);
                       }
                        OSAL_MDELAY(300);

                       /*Patch ready check*/
                       for(port_id = 0; port_id < 8; port_id++)
                       {
                            for(loop = 0; loop < 100; loop++)
                            {
                                gMacDrv->drv_miim_read(rtl8218_phy0_macid + port_id, 0xb80, 0x10, &val);
                                if(val & 0x40)break;
                            }

                            if(loop >= 100)
                            {
                                OSAL_PRINTF("Internal phy port %d not ready!\n",port_id+rtl8218_phy0_macid);
                                return;
                            }
                       }

                       /*Use broadcast ID method to pathc internal phy*/
                       gMacDrv->drv_miim_write(rtl8218_phy0_macid, 0xfff, 0x1f, 0x0000);
                       gMacDrv->drv_miim_write(rtl8218_phy0_macid, 0xfff, 0x001d, 0x0008);
                       gMacDrv->drv_miim_write(rtl8218_phy0_macid, 0xfff, 0x001f, 0x266);
                       gMacDrv->drv_miim_write(rtl8218_phy0_macid, 0xfff, 0x16, 0xff00+rtl8218_phy0_macid);
                       gMacDrv->drv_miim_write(rtl8218_phy0_macid, 0xfff, 0x1f, 0x0000);
                       gMacDrv->drv_miim_write(rtl8218_phy0_macid, 0xfff, 0x1d, 0x0000);
                       OSAL_MDELAY(1);


                        /* Per-Port */
              if(0 == cut_id)
              {
                            for (i=0; i<(sizeof(rtl8218B_6276A_rtl8380_perport)/sizeof(confcode_rv_t)); i++)
                           {
                                gMacDrv->drv_miim_write(rtl8218_phy0_macid, \
                                gMacDrv->miim_max_page, (int)rtl8218B_6276A_rtl8380_perport[i].reg, rtl8218B_6276A_rtl8380_perport[i].val);
                            }
              }
              else
              {
                            for (i=0; i<(sizeof(rtl8218B_6276B_rtl8380_perport)/sizeof(confcode_rv_t)); i++)
                           {
                                gMacDrv->drv_miim_write(rtl8218_phy0_macid, \
                                gMacDrv->miim_max_page, (int)rtl8218B_6276B_rtl8380_perport[i].reg, rtl8218B_6276B_rtl8380_perport[i].val);
                            }
              }

                       /*Disable broadcast ID*/
                       gMacDrv->drv_miim_write(rtl8218_phy0_macid, 0xfff, 0x1f, 0x0000);
                       gMacDrv->drv_miim_write(rtl8218_phy0_macid, 0xfff, 0x001d, 0x0008);
                       gMacDrv->drv_miim_write(rtl8218_phy0_macid, 0xfff, 0x001f, 0x266);
                       gMacDrv->drv_miim_write(rtl8218_phy0_macid, 0xfff, 0x16, 0+rtl8218_phy0_macid);
                       gMacDrv->drv_miim_write(rtl8218_phy0_macid, 0xfff, 0x1f, 0x0000);
                       gMacDrv->drv_miim_write(rtl8218_phy0_macid, 0xfff, 0x1d, 0x0000);
                       OSAL_MDELAY(1);
                }
                else if(rtl8218_phy0_macid == 8)
                {
                        if(val == RTL8218B_INT_MP_A_CHIP_ID)
                        {
                            /*Internal PHY*/
                             //OSAL_PRINTF("Now Internal PHY-A\n");

                            /*Enable Phy*/
                            val = MEM32_READ(0xbb00a100);
                            val |= 1 << 15;
                            MEM32_WRITE(0xbb00a100, val);
                            OSAL_MDELAY(100);

                            /*Patch request*/
                           for(port_id = 0; port_id < 8; port_id++)
                           {
                               gMacDrv->drv_miim_write(rtl8218_phy0_macid+port_id, 0xfff, 0x1f, 0x0b82);
                               gMacDrv->drv_miim_write(rtl8218_phy0_macid+port_id, 0xfff, 0x10, 0x0010);
                           }
                            OSAL_MDELAY(300);

                           /*Patch ready check*/
                           for(port_id = 0; port_id < 8; port_id++)
                           {
                                for(loop = 0; loop < 100; loop++)
                                {
                                    gMacDrv->drv_miim_read(rtl8218_phy0_macid + port_id, 0xb80, 0x10, &val);
                                    if(val & 0x40)break;
                                }

                                if(loop >= 100)
                                {
                                    OSAL_PRINTF("Internal phy port %d not ready!\n",port_id+rtl8218_phy0_macid);
                                    return;
                                }
                           }

                            /*phy per-port patch*/
                           for(port_id = 0; port_id < 8; port_id++)
                           {
                                /* Per-Port */
                                for (i=0; i<(sizeof(rtl838x_6275A_intPhy_perport)/sizeof(confcode_rv_t)); i++)
                                {
                                gMacDrv->drv_miim_write(rtl8218_phy0_macid+port_id, \
                                        gMacDrv->miim_max_page, (int)rtl838x_6275A_intPhy_perport[i].reg, rtl838x_6275A_intPhy_perport[i].val);
                                }
                           }
                        }
                        else if(val == RTL8218B_INT_MP_B_CHIP_ID)
                        {
                            /*Internal PHY*/
                             //OSAL_PRINTF("Now Internal PHY\n");

                            /*Enable Phy*/
                            val = MEM32_READ(0xbb00a100);
                            val |= 1 << 15;
                            MEM32_WRITE(0xbb00a100, val);
                            OSAL_MDELAY(100);

                            /*Patch request*/
                           for(port_id = 0; port_id < 8; port_id++)
                           {
                               gMacDrv->drv_miim_write(rtl8218_phy0_macid+port_id, 0xfff, 0x1f, 0x0b82);
                               gMacDrv->drv_miim_write(rtl8218_phy0_macid+port_id, 0xfff, 0x10, 0x0010);
                           }
                            OSAL_MDELAY(300);

                           /*Patch ready check*/
                           for(port_id = 0; port_id < 8; port_id++)
                           {
                                for(loop = 0; loop < 100; loop++)
                                {
                                    gMacDrv->drv_miim_read(rtl8218_phy0_macid + port_id, 0xb80, 0x10, &val);
                                    if(val & 0x40)break;
                                }

                                if(loop >= 100)
                                {
                                    OSAL_PRINTF("Internal phy port %d not ready!\n",port_id+rtl8218_phy0_macid);
                                    return;
                                }
                           }

                            /*phy per-port patch*/
                           for(port_id = 0; port_id < 8; port_id++)
                           {
                                /* Per-Port */
                                for (i=0; i<(sizeof(rtl838x_6275B_intPhy_perport)/sizeof(confcode_rv_t)); i++)
                                {
                                gMacDrv->drv_miim_write(rtl8218_phy0_macid+port_id, \
                                        gMacDrv->miim_max_page, (int)rtl838x_6275B_intPhy_perport[i].reg, rtl838x_6275B_intPhy_perport[i].val);
                                }
                           }
                        }
                        else
                        {
                            /*Do nothing*/
                        }
             }
             else
             {
                        /*right now nothing to do*/
             }
        }
      }

    return;
} /* end of rtl8218b_rtl8380_config */


void rtl8218fb_rtl8380_config(Tuint8 phyid)
{
    unsigned int rtl8218_macid_pm = (0xFF << phyid);
    int rtl8218_phy0_macid = phyid - (phyid % 8);
    unsigned int val, cut_id;
    int i, port_id, base_id;

    base_id = rtl8218_phy0_macid;

    OSAL_PRINTF("///////////////////////////////////////////////////////////\n");

    /*Check the 8218FB internal chip ID*/
    gMacDrv->drv_miim_write(phyid, 31, 27, 0x0002);
    gMacDrv->drv_miim_read(phyid, 31, 28, &val);

    if(val != RTL8218B_TCA_CHIP_ID)
       {
        OSAL_PRINTF("### Error chip ID = 0x%08x ###\n", val);
        return;
    }

      //OSAL_PRINTF("### RTL8218B config - PhyIdMask = 0x%08x ###\n", rtl8218_macid_pm);

       if(RTL8218FB_TCA_CHIP_ID == val)
       {
            /*Check the 8218FB internal cut ID*/
            gMacDrv->drv_miim_write(base_id, 31, 27, 0x0004);
            gMacDrv->drv_miim_read(base_id, 31, 28, &cut_id);


            /*Check the 8218FB status*/
            gMacDrv->drv_miim_write(base_id, 31, 31, 0xa42);
            gMacDrv->drv_miim_read(base_id, 31, 16, &val);

            if(RTL8218FB_TCA_CUT_A == cut_id)
            {
                        OSAL_PRINTF("### RTL8218FB  NOT USE RIGHT NOW!\n");
                return;
            }
              else if(cut_id == RTL8218FB_TCA_CUT_B)
            {
                    //OSAL_PRINTF("Now External 8218FB B-CUT\n");
                    /* Per-Chip */
                    for (i=0; i<(sizeof(rtl8218fB_revB_rtl8380_perchip)/sizeof(confcode_prv_t)); i++)
                    {
                        gMacDrv->drv_miim_write(rtl8218_phy0_macid + rtl8218fB_revB_rtl8380_perchip[i].phy, \
                            gMacDrv->miim_max_page, (int)rtl8218fB_revB_rtl8380_perchip[i].reg, rtl8218fB_revB_rtl8380_perchip[i].val);
                    }

            /* Per-Port */
            for(port_id = 0; port_id < 8; port_id++)
            {
                        for (i=0; i<(sizeof(rtl8218fB_revB_rtl8380_perport)/sizeof(confcode_rv_t)); i++)
                {
                    gMacDrv->drv_miim_write(rtl8218_phy0_macid + port_id, \
                            gMacDrv->miim_max_page, (int)rtl8218fB_revB_rtl8380_perport[i].reg, rtl8218fB_revB_rtl8380_perport[i].val);
                        }
                    }
                }
               else
               {
                    /*right now nothing to do*/
            }

        }
        else
        {
            /*Error, wrong CHIP ID*/
            OSAL_PRINTF("### CHIP ID ERROR!\n");
            return;
        }

    return;
} /* end of rtl8218b_rtl8380_config */

#endif



/*
 * Function Declaration
 */
/* Function Name:
 *      rtl8218b_phyPowerOff
 * Description:
 *      Configuration code for RTL8218b.
 * Input:
 *      macId - the macId of PHY0 of the RTL8218b
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8218b_phyPowerOff(int macId)
{
    int i, base_id;
    unsigned int val, cut_id;

    /*Check the 8218B internal cut ID*/
    base_id = macId - (macId % 8);
    gMacDrv->drv_miim_write(base_id, 31, 27, 0x0004);
    gMacDrv->drv_miim_read(base_id, 31, 28, &cut_id);

       i = macId;
    if(cut_id == RTL8218B_TCA_CUT_A)
    {
        if((i%8) == 0) /*For Base ID only*/
        {
            /* Disable the 8218B TCA ESD Polliing*/
            val = 0x0008;
            gMacDrv->drv_miim_write(i, gMacDrv->miim_max_page, 29, val);
            val = 0x1330;
            gMacDrv->drv_miim_write(i, gMacDrv->miim_max_page, 31, val);
            val = 0x0003;
            gMacDrv->drv_miim_write(i, gMacDrv->miim_max_page, 18, val);
            val = 0x0000;
            gMacDrv->drv_miim_write(i, gMacDrv->miim_max_page, 29, val);
            /* End of Disable the 8218B ESD Polliing*/
        }
    }
}

void rtl8218fb_media_set(int portid, int media)
{
    unsigned int _RTL8218FB_Phy0Id, reg;
    int phy_idx;
    unsigned int val;

    if (gSwitchModel == NULL)
        return;

    if (gMacDrv == NULL)
        return;

    if ((media != MEDIATYPE_COPPER) && (media != MEDIATYPE_FIBER))
        return;

    phy_idx = gSwitchModel->port.list[portid].phy_idx;
    if (gSwitchModel->phy.list[phy_idx].chip != RTK_CHIP_RTL8218FB)
        return;

    _RTL8218FB_Phy0Id = portid - gSwitchModel->port.list[portid].phy;

    switch (portid % 8)
    {
        case 4:
            reg = 16;
            break;
        case 5:
            reg = 19;
            break;
        case 6:
            reg = 20;
            break;
        case 7:
            reg = 21;
            break;
        default:
            return;
    }

    gMacDrv->drv_miim_write(_RTL8218FB_Phy0Id, 0xa42, 29, 0x0008);
    gMacDrv->drv_miim_read(_RTL8218FB_Phy0Id, 0x266, reg, &val);

    switch (media)
    {
        case MEDIATYPE_COPPER:
            val |= (1<<10);
            val |= (1<<11);
            break;
        case MEDIATYPE_FIBER:
            val |= (1<<10);
            val &= ~(1<<11);
            break;
        default:
            break;
    }

    gMacDrv->drv_miim_write(_RTL8218FB_Phy0Id, 0x266, reg, val);
    gMacDrv->drv_miim_write(_RTL8218FB_Phy0Id, 0xa42, 29, 0x0000);

    return;
} /* end of rtl8218fb_media_set */

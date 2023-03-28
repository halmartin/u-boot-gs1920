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
#include <rtk/phy/conf/conf_rtl8214fc.c>
#include <config.h>

#if defined(CONFIG_RTL8390)
#include <rtk/mac/rtl8390/rtl8390_drv.h>
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
#define RTL8214FC_CHIP_ID           0x6276

#define RTL8214FC_TCA_CHIP_ID       0x0455
#define RTL8214FC_INT_TCA_CHIP_ID   0x0477
#define RTL8214FC_TCA_CUT_A         0x0
#define RTL8214FC_TCA_CUT_B         0x1

#define RTL8214FC_MP_CHIP_ID        0x6276
#define RTL8214FC_MP_CUT_A          0x0
/*
 * Function Declaration
 */
/* Function Name:
 *      rtl8214fc_rtl8390_config
 * Description:
 *      Configuration code for RTL8214FC.
 * Input:
 *      macId - the macId of PHY0 of the RTL8214FC
 *      portNum - the number of port
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
#if defined(CONFIG_RTL8390)
#define CHECK_TIMES     (150000)

void rtl8214fc_rtl8390_config(Tuint8 macId, Tuint8 portNum)
{
    unsigned long long rtl8214_macid_pm = (0xFFULL << macId);
    int rtl8214_phy0_macid = macId - (macId % 4);
    unsigned int val;
    int i, base_id;
    unsigned int rl_no, cut_id;

    base_id = rtl8214_phy0_macid;

    gMacDrv->drv_miim_write(rtl8214_phy0_macid, gMacDrv->miim_max_page, 29, 0x0001);
    gMacDrv->drv_miim_write(rtl8214_phy0_macid, gMacDrv->miim_max_page, 31, 0x0a43);
    gMacDrv->drv_miim_write(rtl8214_phy0_macid, gMacDrv->miim_max_page, 19, 0x0002);
    gMacDrv->drv_miim_read(rtl8214_phy0_macid, gMacDrv->miim_max_page, 20, &rl_no);

    gMacDrv->drv_miim_write(base_id, gMacDrv->miim_max_page, 27, 0x0004);
    gMacDrv->drv_miim_read(base_id, gMacDrv->miim_max_page, 28, &cut_id);

    if (RTL8214FC_CHIP_ID == rl_no)
    {
        int port_id;

        OSAL_PRINTF("### RTL8214FC config - MAC ID = %d ###\n", rtl8214_phy0_macid);

        /* Serdes */
        for (i = 0; i < (sizeof(rtl8214fc_6276A_rtl8390_perchip_serdes)/sizeof(confcode_prv_t)); ++i)
        {
            gMacDrv->drv_miim_write(rtl8214_phy0_macid + rtl8214fc_6276A_rtl8390_perchip_serdes[i].phy, \
                    gMacDrv->miim_max_page, \
                    (int)rtl8214fc_6276A_rtl8390_perchip_serdes[i].reg, \
                    (int)rtl8214fc_6276A_rtl8390_perchip_serdes[i].val);
            OSAL_MDELAY(5);
        }

        for (i = 0; i < (sizeof(rtl8214fc_6276A_rtl8390_perchip_serdes2)/sizeof(confcode_phy_patch_t)); ++i)
        {
            PHY_PATCH_SET(rtl8214_phy0_macid, gMacDrv->miim_max_page, rtl8214fc_6276A_rtl8390_perchip_serdes2[i]);
        }

        for (i = 0; i < (sizeof(rtl8214fc_6276A_rtl8390_perchip_serdes3)/sizeof(confcode_prv_t)); ++i)
        {
            gMacDrv->drv_miim_write(rtl8214_phy0_macid + rtl8214fc_6276A_rtl8390_perchip_serdes3[i].phy, \
                    gMacDrv->miim_max_page, \
                    (int)rtl8214fc_6276A_rtl8390_perchip_serdes3[i].reg, \
                    (int)rtl8214fc_6276A_rtl8390_perchip_serdes3[i].val);
            OSAL_MDELAY(5);
        }

        for (port_id = 0; port_id < 4; ++port_id)
        {
            int pid = rtl8214_phy0_macid + port_id;

            for (i = 0; i < (sizeof(rtl8214fc_6276A_rtl8390_perport)/sizeof(confcode_rv_t)); ++i)
            {
                //printf("p%d %d) reg %x val %x\n", pid, i, rtl8214fc_6276A_rtl8390_perport[i].reg, rtl8214fc_6276A_rtl8390_perport[i].val);
                gMacDrv->drv_miim_write(pid, gMacDrv->miim_max_page, \
                        (int)rtl8214fc_6276A_rtl8390_perport[i].reg, \
                        (int)rtl8214fc_6276A_rtl8390_perport[i].val);
            }
        }

        gMacDrv->drv_miim_write(rtl8214_phy0_macid, gMacDrv->miim_max_page, \
                0x1e, 0x8);
        gMacDrv->drv_miim_read(rtl8214_phy0_macid, 0x260, 0x13, &val);
        val &= (0x1F << 8);
        val |= 0x4020;
        gMacDrv->drv_miim_write(rtl8214_phy0_macid, 0x260, 0x13, val);
        gMacDrv->drv_miim_write(rtl8214_phy0_macid, gMacDrv->miim_max_page, \
                0x1e, 0x0);

        for (i = 0; i < (sizeof(rtl8214fc_6276A_rtl8390_fiber_perchip)/sizeof(confcode_prv_t)); ++i)
        {
            //printf("%d) port %d reg %x val %x\n", i, rtl8214fc_6276A_rtl8390_fiber_perchip[i].phy, rtl8214fc_6276A_rtl8390_fiber_perchip[i].reg, rtl8214fc_6276A_rtl8390_fiber_perchip[i].val);
            gMacDrv->drv_miim_write(rtl8214_phy0_macid + rtl8214fc_6276A_rtl8390_fiber_perchip[i].phy, \
                    gMacDrv->miim_max_page, \
                    (int)rtl8214fc_6276A_rtl8390_fiber_perchip[i].reg, \
                    (int)rtl8214fc_6276A_rtl8390_fiber_perchip[i].val);
        }

        if (cut_id <= 2)
        {
            for (port_id = 0; port_id < portNum; ++port_id)
            {
                rtl8390_phyPortPowerOn(macId + port_id);
            }

            for (port_id = 0; port_id < portNum; ++port_id)
            {
                int pid = macId + port_id;

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
                val = 0x7600 | cut_id;
                gMacDrv->drv_miim_write(pid, gMacDrv->miim_max_page, 28, val);

                for (i = 0; i < (sizeof(rtl8214fc_6276_rtl8390_patch)/sizeof(confcode_phy_patch_t)); ++i)
                    PHY_PATCH_SET(pid, gMacDrv->miim_max_page, rtl8214fc_6276_rtl8390_patch[i]);
            }

            for (port_id = 0; port_id < portNum; ++port_id)
            {
                rtl8390_phyPortPowerOff(macId + port_id);
            }
        }
    }
    else
    {
        /*Switch to right TOP address*/
        gMacDrv->drv_miim_write(base_id, 0, 29, 0x0000);
        gMacDrv->drv_miim_write(base_id, 0, 31, 0x0a42);

        /*Check the 8214B internal chip ID*/
        gMacDrv->drv_miim_write(base_id, gMacDrv->miim_max_page, 27, 0x0002);
        gMacDrv->drv_miim_read(base_id, gMacDrv->miim_max_page, 28, &val);
        if(val != RTL8214FC_TCA_CHIP_ID){
            OSAL_PRINTF("### Error chip ID = 0x%08x ###\n", val);
            return;
        }

        /*Check the 8214FC internal cut ID*/
        gMacDrv->drv_miim_write(base_id, gMacDrv->miim_max_page, 27, 0x0004);
        gMacDrv->drv_miim_read(base_id, gMacDrv->miim_max_page, 28, &cut_id);

        /*Check the 8214FC status*/
        gMacDrv->drv_miim_write(base_id, gMacDrv->miim_max_page, 31, 0xa42);
        gMacDrv->drv_miim_read(base_id, gMacDrv->miim_max_page, 16, &val);

        OSAL_PRINTF("### RTL8214FC config - MAC ID = %d, Mode = 0x%08x ###\n", rtl8214_phy0_macid, val);

        if (cut_id == RTL8214FC_TCA_CUT_B)
        {
            /* Per-Chip Serdes */
            //OSAL_PRINTF("### RTL8214FC config - MAC ID = %d, per-chip_serdes Patch ###\n", rtl8214_phy0_macid);
            for (i=0; i<(sizeof(rtl8214fc_0455B_rtl8390_perchip_serdes)/sizeof(confcode_prv_t)); i++)
            {
                gMacDrv->drv_miim_write(rtl8214_phy0_macid + rtl8214fc_0455B_rtl8390_perchip_serdes[i].phy, \
                    gMacDrv->miim_max_page, \
                    (int)rtl8214fc_0455B_rtl8390_perchip_serdes[i].reg, \
                    (int)rtl8214fc_0455B_rtl8390_perchip_serdes[i].val);
                OSAL_MDELAY(5);
            }

            /* Per-Port */
            //OSAL_PRINTF("### RTL8214FC config - MAC ID = %d, per-port Patch ###\n", rtl8214_phy0_macid);
            for (i=0; i<(sizeof(rtl8214fc_0455B_rtl8390_perport)/sizeof(confcode_rv_t)); i++)
            {
                gMacDrv->drv_miim_portmask_write(rtl8214_macid_pm, \
                    gMacDrv->miim_max_page, \
                    (int)rtl8214fc_0455B_rtl8390_perport[i].reg, \
                    (int)rtl8214fc_0455B_rtl8390_perport[i].val);
                OSAL_MDELAY(5);
            }

            /* Per-Chip */
            //OSAL_PRINTF("### RTL8214FC config - MAC ID = %d, per-chip Patch ###\n", rtl8214_phy0_macid);
            for (i=0; i<(sizeof(rtl8214fc_0455B_rtl8390_perchip)/sizeof(confcode_prv_t)); i++)
            {
                gMacDrv->drv_miim_write(rtl8214_phy0_macid + rtl8214fc_0455B_rtl8390_perchip[i].phy, \
                    gMacDrv->miim_max_page, \
                    (int)rtl8214fc_0455B_rtl8390_perchip[i].reg, \
                    (int)rtl8214fc_0455B_rtl8390_perchip[i].val);
                OSAL_MDELAY(5);
            }
        }
    }

    return;
} /* end of rtl8214fc_rtl8390_config */
#endif

 #if defined(CONFIG_RTL8380)
void rtl8214fc_rtl8380_config(Tuint8 phyid)
{
    int rtl8214_phy0_macid = phyid;
    unsigned int val, cut_id;
    int i, port_id, base_id;
    int loop;


    OSAL_PRINTF("**************************************************\n");

    base_id = rtl8214_phy0_macid;

    /*Switch to Copper address*/
    gMacDrv->drv_miim_write(base_id, 0, 30, 0x0001);

    gMacDrv->drv_miim_write(base_id, 0, 31, 0x0a42);

    /*Check the 8214B internal chip ID*/
    gMacDrv->drv_miim_write(base_id, 31, 27, 0x0002);
    gMacDrv->drv_miim_read(base_id, 31, 28, &val);
    if((val != RTL8214FC_MP_CHIP_ID))
       {
        OSAL_PRINTF("### Error chip ID = 0x%08x ###\n", val);
        return;
    }

    /*Check the 8214FC internal cut ID*/
        gMacDrv->drv_miim_write(base_id, gMacDrv->miim_max_page, 27, 0x0004);
        gMacDrv->drv_miim_read(base_id, gMacDrv->miim_max_page, 28, &cut_id);

    /*Back to Auto address*/
    gMacDrv->drv_miim_write(base_id, 0, 30, 0x0000);

        if(val == RTL8214FC_MP_CHIP_ID)
        {
            OSAL_PRINTF("**** RTL8214FC config - MAC ID = %d ****\n", base_id);

            /*External PHY*/
            OSAL_PRINTF("Now External 8214FC\n");
            if(cut_id == RTL8214FC_TCA_CUT_A)
            {
                /* Per-Chip */
                for (i=0; i<(sizeof(rtl8214fc_6276A_rtl8380_perchip)/sizeof(confcode_prv_t)); i++)
                {
                    gMacDrv->drv_miim_write(rtl8214_phy0_macid + rtl8214fc_6276A_rtl8380_perchip[i].phy, \
                        gMacDrv->miim_max_page, (int)rtl8214fc_6276A_rtl8380_perchip[i].reg, rtl8214fc_6276A_rtl8380_perchip[i].val);
                }
            }
            else
            {
                   uint32 page_temp;
                   uint32 reg_temp;
                   uint32 val_temp;

		     page_temp = 0;
		     reg_temp = 0;

                    /* Per-Chip */
                    for (i=0; i<(sizeof(rtl8214fc_6276B_rtl8380_perchip)/sizeof(confcode_prv_t)); i++)
                    {
			  /*Fix 14FC PHYID Fixed bug*/
			  if(rtl8214fc_6276B_rtl8380_perchip[i].reg == 0x1f)
			  	page_temp = rtl8214fc_6276B_rtl8380_perchip[i].val;

			  if((rtl8214fc_6276B_rtl8380_perchip[i].reg == 0x13) && (page_temp == 0x260))
		  	 {
		  	    gMacDrv->drv_miim_read(rtl8214_phy0_macid + rtl8214fc_6276B_rtl8380_perchip[i].phy, 0x260, 0x13, &val_temp);
			    val_temp = (val_temp & 0x1f00) | (rtl8214fc_6276B_rtl8380_perchip[i].val & 0xe0ff);
	                    gMacDrv->drv_miim_write(rtl8214_phy0_macid + rtl8214fc_6276B_rtl8380_perchip[i].phy, \
	                        gMacDrv->miim_max_page, (int)rtl8214fc_6276B_rtl8380_perchip[i].reg, val_temp);
			  }
			  else
		  	 {
	                    gMacDrv->drv_miim_write(rtl8214_phy0_macid + rtl8214fc_6276B_rtl8380_perchip[i].phy, \
	                        gMacDrv->miim_max_page, (int)rtl8214fc_6276B_rtl8380_perchip[i].reg, rtl8214fc_6276B_rtl8380_perchip[i].val);
			 }
                    }


                    /*Force Copper*/
                   for(port_id = 0; port_id < 4; port_id++)
                    {
                       gMacDrv->drv_miim_write(rtl8214_phy0_macid+port_id, 0xfff, 0x1f, 0x0000);
                       gMacDrv->drv_miim_write(rtl8214_phy0_macid+port_id, 0xfff, 0x1e, 0x0001);
                   }

                    /*Enable Phy*/
                    for(port_id = 0; port_id < 4; port_id++)
                    {
                       gMacDrv->drv_miim_write(rtl8214_phy0_macid+port_id, 0xfff, 0x1f, 0x0000);
                       gMacDrv->drv_miim_write(rtl8214_phy0_macid+port_id, 0xfff, 0x00, 0x1140);
                   }
                   OSAL_MDELAY(100);

                   /*Disable AutoSending Featrue*/
                   for(port_id = 0; port_id < 4; port_id++)
                   {
                        for(loop = 0; loop < 100; loop++)
                        {
                            gMacDrv->drv_miim_read(rtl8214_phy0_macid + port_id, 0x0a42, 0x10, &val);
                            if((val & 0x7)>=3)break;
                        }

                        if(loop >= 100)
                        {
                            OSAL_PRINTF("Disable Port %d AutoSending Featrue Not OK!\n",port_id+rtl8214_phy0_macid);
                            return;
                        }
                   }


                    /*Patch request*/
                   for(port_id = 0; port_id < 4; port_id++)
                   {
                       gMacDrv->drv_miim_write(rtl8214_phy0_macid+port_id, 0xfff, 0x1f, 0x0b82);
                       gMacDrv->drv_miim_write(rtl8214_phy0_macid+port_id, 0xfff, 0x10, 0x0010);
                   }
                    OSAL_MDELAY(300);

                   /*Patch ready check*/
                   for(port_id = 0; port_id < 4; port_id++)
                   {
                        for(loop = 0; loop < 100; loop++)
                        {
                            gMacDrv->drv_miim_read(rtl8214_phy0_macid + port_id, 0xb80, 0x10, &val);
                            if(val & 0x40)break;
                        }

                        if(loop >= 100)
                        {
                            OSAL_PRINTF("Internal phy port %d not ready!\n",port_id+rtl8214_phy0_macid);
                            return;
                        }
                   }

                   /*Use broadcast ID method to pathc internal phy*/
                   gMacDrv->drv_miim_write(rtl8214_phy0_macid, 0xfff, 0x1f, 0x0000);
                   gMacDrv->drv_miim_write(rtl8214_phy0_macid, 0xfff, 0x001d, 0x0008);
                   gMacDrv->drv_miim_write(rtl8214_phy0_macid, 0xfff, 0x001f, 0x266);
                   gMacDrv->drv_miim_write(rtl8214_phy0_macid, 0xfff, 0x16, 0xff00+rtl8214_phy0_macid);
                   gMacDrv->drv_miim_write(rtl8214_phy0_macid, 0xfff, 0x1f, 0x0000);
                   gMacDrv->drv_miim_write(rtl8214_phy0_macid, 0xfff, 0x1d, 0x0000);
                   OSAL_MDELAY(1);

                    /* Per-Port */
                    for (i=0; i<(sizeof(rtl8214fc_6276B_rtl8380_perport)/sizeof(confcode_rv_t)); i++)
                    {
                    gMacDrv->drv_miim_write(rtl8214_phy0_macid, \
                        gMacDrv->miim_max_page, (int)rtl8214fc_6276B_rtl8380_perport[i].reg, rtl8214fc_6276B_rtl8380_perport[i].val);
                    }

                   /*Disable broadcast ID*/
                   gMacDrv->drv_miim_write(rtl8214_phy0_macid, 0xfff, 0x1f, 0x0000);
                   gMacDrv->drv_miim_write(rtl8214_phy0_macid, 0xfff, 0x001d, 0x0008);
                   gMacDrv->drv_miim_write(rtl8214_phy0_macid, 0xfff, 0x001f, 0x266);
                   gMacDrv->drv_miim_write(rtl8214_phy0_macid, 0xfff, 0x16, 0+rtl8214_phy0_macid);
                   gMacDrv->drv_miim_write(rtl8214_phy0_macid, 0xfff, 0x1f, 0x0000);
                   gMacDrv->drv_miim_write(rtl8214_phy0_macid, 0xfff, 0x1d, 0x0000);
                   OSAL_MDELAY(1);

                    /*Enable Auto*/
                   for(port_id = 0; port_id < 4; port_id++)
                    {
                       gMacDrv->drv_miim_write(rtl8214_phy0_macid+port_id, 0xfff, 0x1f, 0x0000);
                       gMacDrv->drv_miim_write(rtl8214_phy0_macid+port_id, 0xfff, 0x1e, 0x0000);
                   }

#if 0
                    /*Force 14FC Fiber*/
                   gMacDrv->drv_miim_write(rtl8214_phy0_macid, 0, 29, 8);
                   gMacDrv->drv_miim_write(rtl8214_phy0_macid, 0x266, 16, 0x795);
                   gMacDrv->drv_miim_write(rtl8214_phy0_macid, 0x266, 19, 0x795);
                   gMacDrv->drv_miim_write(rtl8214_phy0_macid, 0x266, 20, 0x795);
                   gMacDrv->drv_miim_write(rtl8214_phy0_macid, 0x266, 21, 0x795);
                   gMacDrv->drv_miim_write(rtl8214_phy0_macid, 0, 29, 0);
#endif

            }
        }

    return;
} /* end of rtl8214fc_rtl8380_config */
#endif




/*
 * Function Declaration
 */
/* Function Name:
 *      rtl8214fc_phyPowerOff
 * Description:
 *      Configuration code for RTL8214b.
 * Input:
 *      macId - the macId of PHY0 of the RTL8214b
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8214fc_phyPowerOff(int macId)
{
    unsigned int  cut_id;
    int base_id;

    /*Check the 8214FC internal cut ID*/
    base_id = macId - (macId % 8);
    gMacDrv->drv_miim_write(base_id, 31, 27, 0x0004);
    gMacDrv->drv_miim_read(base_id, 31, 28, &cut_id);
}

void rtk_8214fc_dumpTop(Tuint8 macId)
{
    int rtl8214_phy0_macid = macId - (macId % 8);
    int page, reg;
    unsigned int val;
    int base_id;

    base_id = rtl8214_phy0_macid;
    OSAL_PRINTF("macId = %d\n", macId);

    /*Switch to right TOP address*/
    gMacDrv->drv_miim_write(base_id, 0, 29, 0x0008);

    OSAL_PRINTF("########################################################################\n");
    OSAL_PRINTF("  Page  | Reg-16  Reg-17  Reg-18  Reg-19  Reg-20  Reg-21  Reg-22  Reg-23\n");
    OSAL_PRINTF("------------------------------------------------------------------------\n");
    for (page = 0x260; page <= 0x278; page++)
    {
        gMacDrv->drv_miim_write(base_id, 0, 31, page);
        OSAL_PRINTF(" 0x%04X | ", page);
        for (reg = 16; reg <= 23; reg++)
        {
            gMacDrv->drv_miim_read(base_id, page, reg, &val);
            if (reg == 23)
                OSAL_PRINTF("0x%04X\n", val);
            else
                OSAL_PRINTF("0x%04X  ", val);
        }
    }
    OSAL_PRINTF("########################################################################\n");
    OSAL_PRINTF("  Page  | Reg-16  Reg-17  Reg-18  Reg-19  Reg-20  Reg-21  Reg-22  Reg-23\n");
    OSAL_PRINTF("------------------------------------------------------------------------\n");
    for (page = 0x460; page <= 0x469; page++)
    {
        gMacDrv->drv_miim_write(base_id, 0, 31, page);
        OSAL_PRINTF(" 0x%04X | ", page);
        for (reg = 16; reg <= 23; reg++)
        {
            gMacDrv->drv_miim_read(base_id, page, reg, &val);
            //OSAL_PRINTF("### Page = 0x%x, Reg=0x%x, Val=0x%04x ###\n", page, reg, val);
            if (reg == 23)
                OSAL_PRINTF("0x%04X\n", val);
            else
                OSAL_PRINTF("0x%04X  ", val);
        }
    }
    OSAL_PRINTF("########################################################################\n");
    OSAL_PRINTF("  Page  | Reg-16  Reg-17  Reg-18  Reg-19  Reg-20  Reg-21  Reg-22  Reg-23\n");
    OSAL_PRINTF("------------------------------------------------------------------------\n");
    for (page = 0x404; page <= 0x40f; page++)
    {
        gMacDrv->drv_miim_write(base_id, 0, 31, page);
        OSAL_PRINTF(" 0x%04X | ", page);
        for (reg = 16; reg <= 23; reg++)
        {
            gMacDrv->drv_miim_read(base_id, page, reg, &val);
            //OSAL_PRINTF("### Page = 0x%x, Reg=0x%x, Val=0x%04x ###\n", page, reg, val);
            if (reg == 23)
                OSAL_PRINTF("0x%04X\n", val);
            else
                OSAL_PRINTF("0x%04X  ", val);
        }
    }
    OSAL_PRINTF("########################################################################\n");
    OSAL_PRINTF("  Page  | Reg-16  Reg-17  Reg-18  Reg-19  Reg-20  Reg-21  Reg-22  Reg-23\n");
    OSAL_PRINTF("------------------------------------------------------------------------\n");
    for (page = 0x420; page <= 0x42f; page++)
    {
        gMacDrv->drv_miim_write(base_id, 0, 31, page);
        OSAL_PRINTF(" 0x%04X | ", page);
        for (reg = 16; reg <= 23; reg++)
        {
            gMacDrv->drv_miim_read(base_id, page, reg, &val);
            //OSAL_PRINTF("### Page = 0x%x, Reg=0x%x, Val=0x%04x ###\n", page, reg, val);
            if (reg == 23)
                OSAL_PRINTF("0x%04X\n", val);
            else
                OSAL_PRINTF("0x%04X  ", val);
        }
    }
    OSAL_PRINTF("########################################################################\n");

    gMacDrv->drv_miim_write(base_id, 0, 29, 0x0000);
    return;
}

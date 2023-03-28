/*
 * Copyright(c) Realtek Semiconductor Corporation, 2012
 * All rights reserved.
 *
 * Purpose : RTL8380 SOC commands for U-Boot.
 *
 * Feature :
 *
 */


/*
 * Include Files
 */
#include <common.h>
#include <command.h>
#include <rtk_osal.h>
#include <rtk/mac/rtl8380/rtl8380_rtk.h>
#include <rtk/mac/rtl8380/rtl8380_drv.h>
#include <rtk/drv/rtl8231/rtl8231_drv.h>
#include <rtk/mac/rtl8380/rtl8380_mdc_mdio.h>
#include <rtk/mac/rtl8380/rtl8380_swcore_reg.h>
#include <rtk_reg.h>

#if (defined(CONFIG_RTL8214FC) || defined(CONFIG_RTL8218FB))
#include <rtk/phy/rtl8218b.h>
#endif

#if (defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B) || defined(CONFIG_RTL8212B))
#include <rtk/phy/conf/conftypes.h>
#endif


/*
 * Symbol Definition
 */
#define FLASH_LAYOUT_JFFS2_START    (0xBD040000)
#define FLASH_LAYOUT_JFFS2_END      (0xBD0FFFFF)

#define MEDIATYPE_COPPER    (0)
#define MEDIATYPE_FIBER     (1)

/*
 * Data Declaration
 */
const char jffs2_pattern_default[] = {  \
    0x19, 0x85, 0x20, 0x03, 0x00, 0x00, 0x00, 0x0C, 0xF0, 0x60, 0xDC, 0x98, 0x19, 0x85, 0xE0, 0x01, \
    0x00, 0x00, 0x00, 0x2B, 0x3E, 0x42, 0x24, 0x27, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x02, 0x49, 0xC2, 0x63, 0x49, 0x03, 0x04, 0x00, 0x00, 0x0D, 0x75, 0xC6, 0xA9, \
    0x70, 0x7E, 0xB1, 0xD7, 0x6C, 0x6F, 0x67, 0xFF, 0x19, 0x85, 0xE0, 0x02, 0x00, 0x00, 0x00, 0x44, \
    0xA4, 0xEF, 0x22, 0x3E, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x41, 0xC9, \
    0x2E, 0xC4, 0x19, 0x06, 0x00, 0x00, 0x00, 0x00, 0x49, 0xC2, 0x63, 0x49, 0x49, 0xC2, 0x63, 0x49, \
    0x49, 0xC2, 0x63, 0x49, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBA, 0x0A, 0xC7, 0xCE };

static unsigned int gComboPortMode = -1;               /* 0: Copper, 1: Fiber */


#ifdef CONFIG_EEE
#if (defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B) || defined(CONFIG_RTL8212B))
confcode_prv_t rtl8380_rtl8214fb_eee_enable[] = {
	//puts "=============================================================================="
	//puts "Start at: [clock format [clock seconds] -format %c]"
	//puts "=============================================================================="
	//set PHYID 0
	//####################################################################################################
	//#    Force Select Copper Standard Register
	//####################################################################################################
	//puts "Force Select Copper Standard Register ..."
	{1, 31, 0x0008}, // change to page 8
	{1, 16, 0x0F00}, // force select copper standard register
	{1, 31, 0x0008}, // change to page 8
	//####################################################################################################
	//#    Enable Parallel Write
	//####################################################################################################
	//puts "Enable Parallel Write ..."
	{3, 31, 0x0008}, // change to page 8
	{3, 24, 0x0001}, // enable parallel write
	{3, 31, 0x0008}, // change to page 8
	//####################################################################################################
	//#    Power Down PHY
	//####################################################################################################
	//puts "Power Down PHY ..."
	{3, 31, 0x0008}, // change to page 8
	{3,  0, 0x1940}, // power down PHY
	{3, 31, 0x0008}, // change to page 8
	//####################################################################################################
	//#    Micro-C Enable or Disable Auto Turn off EEE
	//####################################################################################################
	//puts "Micro-C Enable or Disable Auto Turn off EEE ..."
	{3, 31, 0x0005}, // change to page 5
	{3,  5, 0x8B85}, // set Micro-C memory address (enable or disable auto turn off EEE)
	{3,  6, 0xE286}, // set Micro-C memory data (enable or disable auto turn off EEE) (enable: 0xE286, disable: 0xC286)
	{3, 31, 0x0008}, // change to page 8
	//####################################################################################################
	//#    Micro-C Control 10M EEE
	//####################################################################################################
	//puts "Micro-C Control 10M EEE ..."
	{3, 31, 0x0005}, // change to page 5
	{3,  5, 0x8B86}, // set Micro-C memory address (control 10M EEE)
	{3,  6, 0x8600}, // set Micro-C memory data (control 10M EEE) (enable: 0x8601, disable: 0x8600)
	{3, 31, 0x0008}, // change to page 8
	//####################################################################################################
	//#    Enable or Disable EEE
	//####################################################################################################
	//puts "Enable or Disable EEE ..."
	{3, 31, 0x0007}, // change to page 7
	{3, 30, 0x0020}, // change to ext. page 32
	{3, 21, 0x0100}, // enable or disable EEE (enable: 0x0100, disable: 0x0000)
	{3, 27, 0xA03A}, // force EEE PHY mode (PHY mode: 0xA0BA, MAC mode: 0xA03A)
	{3, 31, 0x0008}, // change to page 8
	//####################################################################################################
	//#    100/1000M EEE Capability
	//####################################################################################################
	//puts "100/1000M EEE Capability ..."
	{3, 31, 0x0000}, // change to page 0
	{3, 13, 0x0007}, // MMD register 7.60
	{3, 14, 0x003C}, // MMD register 7.60
	{3, 13, 0x4007}, // MMD register 7.60
	{3, 14, 0x0006}, // enable 100/1000M EEE capability
	{3, 13, 0x0000}, // MMD register
	{3, 31, 0x0008}, // change to page 8
	//####################################################################################################
	//#    10M EEE Amplitude
	//####################################################################################################
	//puts "10M EEE Amplitude ..."
	{3, 31, 0x0002}, // change to page 2
	{3, 11, 0x17A7}, // 10M EEE amplitude
	{3, 31, 0x0008}, // change to page 8
	//####################################################################################################
	//#    Disable Parallel Write
	//####################################################################################################
	//puts "Disable Parallel Write ..."
	{3, 31, 0x0008}, // change to page 8
	{3, 24, 0x0000}, // disable parallel write
	{3, 31, 0x0008}, // change to page 8
};

confcode_prv_t rtl8380_rrtl8214fb_eee_disable[] = {
	//puts "=============================================================================="
	//puts "Start at: [clock format [clock seconds] -format %c]"
	//puts "=============================================================================="
	//set PHYID 0
	//####################################################################################################
	//#    Force Select Copper Standard Register
	//####################################################################################################
	//puts "Force Select Copper Standard Register ..."
	{1, 31, 0x0008}, // change to page 8
	{1, 16, 0x0F00}, // force select copper standard register
	{1, 31, 0x0008}, // change to page 8
	//####################################################################################################
	//#    Enable Parallel Write
	//####################################################################################################
	//puts "Enable Parallel Write ..."
	{3, 31, 0x0008}, // change to page 8
	{3, 24, 0x0001}, // enable parallel write
	{3, 31, 0x0008}, // change to page 8
	//####################################################################################################
	//#    Power Down PHY
	//####################################################################################################
	//puts "Power Down PHY ..."
	{3, 31, 0x0008}, // change to page 8
	{3,  0, 0x1940}, // power down PHY
	{3, 31, 0x0008}, // change to page 8
	//####################################################################################################
	//#    Micro-C Enable or Disable Auto Turn off EEE
	//####################################################################################################
	//puts "Micro-C Enable or Disable Auto Turn off EEE ..."
	{3, 31, 0x0005}, // change to page 5
	{3,  5, 0x8B85}, // set Micro-C memory address (enable or disable auto turn off EEE)
	{3,  6, 0xC286}, // set Micro-C memory data (enable or disable auto turn off EEE) (enable: 0xE286, disable: 0xC286)
	{3, 31, 0x0008}, // change to page 8
	//####################################################################################################
	//#    Micro-C Control 10M EEE
	//####################################################################################################
	//puts "Micro-C Control 10M EEE ..."
	{3, 31, 0x0005}, // change to page 5
	{3,  5, 0x8B86}, // set Micro-C memory address (control 10M EEE)
	{3,  6, 0x8600}, // set Micro-C memory data (control 10M EEE) (enable: 0x8601, disable: 0x8600)
	{3, 31, 0x0008}, // change to page 8
	//####################################################################################################
	//#    Enable or Disable EEE
	//####################################################################################################
	//puts "Enable or Disable EEE ..."
	{3, 31, 0x0007}, // change to page 7
	{3, 30, 0x0020}, // change to ext. page 32
	{3, 21, 0x0000}, // enable or disable EEE (enable: 0x0100, disable: 0x0000)
	{3, 27, 0xA03A}, // force EEE PHY mode (PHY mode: 0xA0BA, MAC mode: 0xA03A)
	{3, 31, 0x0008}, // change to page 8
	//####################################################################################################
	//#    100/1000M EEE Capability
	//####################################################################################################
	//puts "100/1000M EEE Capability ..."
	{3, 31, 0x0000}, // change to page 0
	{3, 13, 0x0007}, // MMD register 7.60
	{3, 14, 0x003C}, // MMD register 7.60
	{3, 13, 0x4007}, // MMD register 7.60
	{3, 14, 0x0000}, // disable 100/1000M EEE capability
	{3, 13, 0x0000}, // MMD register
	{3, 31, 0x0008}, // change to page 8
	//####################################################################################################
	//#    10M EEE Amplitude
	//####################################################################################################
	//puts "10M EEE Amplitude ..."
	{3, 31, 0x0002}, // change to page 2
	{3, 11, 0x17A7}, // 10M EEE amplitude
	{3, 31, 0x0008}, // change to page 8
	//####################################################################################################
	//#    Disable Parallel Write
	//####################################################################################################
	//puts "Disable Parallel Write ..."
	{3, 31, 0x0008}, // change to page 8
	{3, 24, 0x0000}, // disable parallel write
	{3, 31, 0x0008}, // change to page 8
};
#endif /* (defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B) || defined(CONFIG_RTL8212B)) */
#endif




/*
 * Macro Definition
 */


/*
 * Function Declaration
 */

/* Function Name:
 *      rtk_network_on
 * Description:
 *      Enable network function.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtk_network_on(void)
{
    /*port 28 force link up*/
    REG32(0xbb00a174) = 0x6192f;

    rtl8380_phyPowerOn();

    /* delay 2.0 sec for link-status stable */
    printf("Please wait for PHY init-time ...\n\n");
    udelay(4000000);

    return;
} /* end of rtk_network_on */

/* Function Name:
 *      rtk_phyPortPowerOn
 * Description:
 *      Power-On PHY by port.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtk_phyPortPowerOn(int mac_idx)
{
    if (gSwitchModel == NULL)
        return;

    rtl8380_phyPortPowerOn(gSwitchModel->port.list[mac_idx].mac_id);
}

/* Function Name:
 *      rtk_portlink_get
 * Description:
 *      Get port link status.
 * Input:
 *      int unit
 *      int port
 * Output:
 *      int *link
 * Return:
 *      always 0
 * Note:
 *      None
 */
int rtk_portlink_get(int unit,int port,int *link)
{
    unsigned int val;
    val = MEM32_READ(SWCORE_BASE_ADDR | RTL8380_MAC_LINK_STS_ADDR);
    val = MEM32_READ(SWCORE_BASE_ADDR | RTL8380_MAC_LINK_STS_ADDR);

    if((val>>port)&&0x1 == 0x1)
    {
        *link=1;
    }
    else
        *link=0;
    return 0;
}
/* Function Name:
 *      rtk_network_off
 * Description:
 *      Disable network function.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtk_network_off(void)
{

    /*port 28 force link down*/
    REG32(0xbb00a174) = 0x6192d;

    rtl8380_phyPowerOff();

    return;
}/* end of rtk_network_off */


void rtk_default(void)
{
    cmd_tbl_t *run;
    int rc;
    char *argu[8];
    char addr_str1[12];
    char addr_str2[12];
    char len_str[4];

    /* erase 0xbd040000 0xbd0fffff; */
    run = find_cmd("erase");
    if (NULL == run)
    {
        printf("erase command is not supported!\n");
        return;
    }

    sprintf(addr_str1, "0x%08X", FLASH_LAYOUT_JFFS2_START);
    sprintf(addr_str2, "0x%08X", FLASH_LAYOUT_JFFS2_END);

    argu[0] = "erase";
    argu[1] = addr_str1;
    argu[2] = addr_str2;
    argu[3] = NULL;

    rc = run->cmd(run, 0, 3, argu);
    if (rc != 0)
    {
        printf("[DBG] %s %s %s\n", argu[0], argu[1], argu[2]);
        printf("Error: erase flash failed!\n");
        return;
    }

    /* cp.b 0x81000000 0xbd040000 7c */
    run = find_cmd("cp.b");
    if (NULL == run)
    {
        printf("erase command is not supported!\n");
        return;
    }

    sprintf(addr_str1, "0x%08X", (uint32)&jffs2_pattern_default);
    sprintf(addr_str2, "0x%08X", FLASH_LAYOUT_JFFS2_START);
    sprintf(len_str, "%X", sizeof(jffs2_pattern_default));

    argu[0] = "cp.b";
    argu[1] = addr_str1;
    argu[2] = addr_str2;
    argu[3] = len_str;
    argu[4] = NULL;

    rc = run->cmd(run, 0, 4, argu);
    if (rc != 0)
    {
        printf("[DBG] %s %s %s %s\n", argu[0], argu[1], argu[2], argu[3]);
        printf("Error: initial configuration failed!\n");
        return;
    }

    printf("Success.\n");

    return;
} /* end of rtk_default */

#ifdef CONFIG_EEE
void rtk_eee_on(const rtk_switch_model_t *pModel)
{
    int i, j;
    unsigned int polling_mask, val;

    /*Backup polling mask*/
    polling_mask = REG32(0xbb00a17c);

    /*Disable MAC polling PHY*/
    REG32(0xbb00a17c) = 0;

    //printf("REG32(0xbb00a17c) is %8x\n",REG32(0xbb00a17c));

    for (i=0; i<pModel->phy.count; i++)
    {
        switch (pModel->phy.list[i].chip)
        {
            #if (defined(CONFIG_RTL8208))
            case RTK_CHIP_RTL8208D:
            case RTK_CHIP_RTL8208L:
                {
                    unsigned int phyData;
                    unsigned int rtl8208d_phy0_id = pModel->phy.list[i].mac_id;
                    //unsigned int rtl8208d_macid_pm = (0xFF << pModel->phy.list[i].mac_id);

                    //printf("PHY[%d]: enable EEE on port [0x%08x]\n", i, rtl8208d_macid_pm);

                    for (j=(rtl8208d_phy0_id); j<(rtl8208d_phy0_id+8); j++)
                    {
                        /* enable EEE function */
                        gMacDrv->drv_miim_read((int)j, 4, 16, &phyData);
                        phyData |= (3 << 12);
                        gMacDrv->drv_miim_write((int)j, 4, 16, (int)phyData);

                        gMacDrv->drv_miim_write((int)j, gMacDrv->miim_max_page, 31, 0x0);
                        gMacDrv->drv_miim_write((int)j, gMacDrv->miim_max_page, 13, 0x7);
                        gMacDrv->drv_miim_write((int)j, gMacDrv->miim_max_page, 14, 0x3c);
                        gMacDrv->drv_miim_write((int)j, gMacDrv->miim_max_page, 13, 0x4007);
                        gMacDrv->drv_miim_write((int)j, gMacDrv->miim_max_page, 14, 0x2);
                        gMacDrv->drv_miim_write((int)j, gMacDrv->miim_max_page, 13, 0x0);

                        /* Force re-autonegotiation if AN is on */
                        gMacDrv->drv_miim_read((int)j, 0, 0, &phyData);
                        if (phyData & 0x1000)
                        {
                            phyData |= (1 << 9);
                        }
                        gMacDrv->drv_miim_write(j, 0, 0, phyData);
                    }
                }
                break;
            #endif
            #if (defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B) || defined(CONFIG_RTL8212B))
            case RTK_CHIP_RTL8214FB:
            case RTK_CHIP_RTL8214B:
            case RTK_CHIP_RTL8212B:
                {
                    unsigned int rtl8214fb_phy0_macid = pModel->phy.list[i].mac_id;
                    unsigned int          forceReg, phyData;
                    int          idx;

                    /* store the original register value */
                    gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 1,
                            gMacDrv->miim_max_page, 31, 8);
                    gMacDrv->drv_miim_read(rtl8214fb_phy0_macid + 1,
                            gMacDrv->miim_max_page, 16, &forceReg);
                    gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 1,
                            gMacDrv->miim_max_page, 31, 8);

                    for (idx = 0; idx < (sizeof(rtl8380_rtl8214fb_eee_enable)/sizeof(confcode_prv_t)); ++idx)
                    {
                        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + rtl8380_rtl8214fb_eee_enable[idx].phy, \
        				        gMacDrv->miim_max_page,
        				        (int)rtl8380_rtl8214fb_eee_enable[idx].reg,
        				        (int)rtl8380_rtl8214fb_eee_enable[idx].val);
                    }

                    /* Restart Auto-Negotiation */
                    for (idx = 0; idx < 4; ++idx)
                    {
                        gMacDrv->drv_miim_read(rtl8214fb_phy0_macid + idx,
                                gMacDrv->miim_max_page, 0, &phyData);

                        phyData |= (1 << 9);
                        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + idx,
                                gMacDrv->miim_max_page, 0, phyData);
                    }

                    /* restore the original register value */
                    gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 1,
                            gMacDrv->miim_max_page, 31, 8);
        			gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 1,
        			        gMacDrv->miim_max_page, 16, forceReg);
                    gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 1,
        			        gMacDrv->miim_max_page, 31, 8);
        			OSAL_MDELAY(1000);
                }
                break;
            #endif
            #if (defined(CONFIG_RTL8218))
            case RTK_CHIP_RTL8218:
                {
                    unsigned int rtl8218_macid_pm = (0xFF << pModel->phy.list[i].mac_id);
                    //int val;

                    //printf("PHY[%d]: enable EEE on port [0x%08x]\n", i, rtl8218_macid_pm);

                    /* enable main function */
                    {
                        //## ext page 32
                        gMacDrv->drv_miim_portmask_write(rtl8218_macid_pm,7,0x1e,0x0020);
                        gMacDrv->drv_miim_portmask_write(rtl8218_macid_pm,7,0x17,0x000a);
                        gMacDrv->drv_miim_portmask_write(rtl8218_macid_pm,7,0x1b,0x2fca);
                        gMacDrv->drv_miim_portmask_write(rtl8218_macid_pm,7,0x15,0x0100);

                        //###### negear EEE Nway ability autooff
                        gMacDrv->drv_miim_portmask_write(rtl8218_macid_pm,31,0x1f,0x0005);
                        gMacDrv->drv_miim_portmask_write(rtl8218_macid_pm,31,0x05,0x8b84);
                        //gMacDrv->drv_miim_portmask_write(rtl8218_macid_pm,31,0x06,0x0026);
                        gMacDrv->drv_miim_portmask_write(rtl8218_macid_pm,31,0x06,0x0062);
                        gMacDrv->drv_miim_portmask_write(rtl8218_macid_pm,31,0x1f,0x0000);
                    }

                    gMacDrv->drv_miim_portmask_write(rtl8218_macid_pm,0,0x1f,0x0000);
                }

                /* Force re-autonegotiation if AN is on */
                {
                    for (j=(pModel->phy.list[i].mac_id+0); j<(pModel->phy.list[i].mac_id+8); j++)
                    {
                        gMacDrv->drv_miim_read(j, 0, 0, &val);
                        if (val & (1 << 12))
                        {
                            gMacDrv->drv_miim_write(j, 0, 0, (val | (1 << 9)));
                        }
                    }
                }
                break;
            #endif
            #if (defined(CONFIG_RTL8218B) || defined(CONFIG_RTL8218FB) || defined(CONFIG_RTL8214FC))
            case RTK_CHIP_RTL8218B:
            case RTK_CHIP_RTL8218FB:
            case RTK_CHIP_RTL8214FC:
                {
                     //int rtl8218b_macid_pm = (0xFF << pModel->phy.list[i].mac_id);
                     int phy_idx;
                     unsigned int value;

                    //printf("PHY[%d]: enable EEE on port [0x%08x]\n", i, rtl8218b_macid_pm);


                    /* 10M EEE Amplitude */
                    for(phy_idx=0; phy_idx<pModel->phy.list[i].phy_max; phy_idx++)
                    {
                         /* Force Copper */
			    gMacDrv->drv_miim_write(pModel->phy.list[i].mac_id+phy_idx,0,30,0x0001);

                         /* 100/1000M EEE Capability */
                        gMacDrv->drv_miim_write(pModel->phy.list[i].mac_id+phy_idx,0,13,0x0007);
                        gMacDrv->drv_miim_write(pModel->phy.list[i].mac_id+phy_idx,0,14,0x003C);
                        gMacDrv->drv_miim_write(pModel->phy.list[i].mac_id+phy_idx,0,13,0x4007);
                        gMacDrv->drv_miim_write(pModel->phy.list[i].mac_id+phy_idx,0,14,0x0006);


                        gMacDrv->drv_miim_read(pModel->phy.list[i].mac_id+phy_idx,0x0A43,25,&value);
                        value |= 1UL<<4;
                        gMacDrv->drv_miim_write(pModel->phy.list[i].mac_id+phy_idx,0x0A43,25,value);

	                /* Force re-autonegotiation if AN is on */
			  gMacDrv->drv_miim_read(pModel->phy.list[i].mac_id+phy_idx, 0, 0, &val);
                        if (val & (1 << 12))
                        {
                            gMacDrv->drv_miim_write(pModel->phy.list[i].mac_id+phy_idx, 0, 0, (val | (1 << 9)));
                        }

                         /* Back to  AUTO */
                        gMacDrv->drv_miim_write(pModel->phy.list[i].mac_id+phy_idx,0,30,0x0000);
                    }
                }

                break;
            #endif
            default:
                //printf("PHY[%d]: not supported in EEE\n", i);
                break;
        }
    }


     /*Restore polling mask*/
     REG32(0xbb00a17c) = polling_mask;

    return;
} /* end of rtk_eee_on */

void rtk_eee_off(const rtk_switch_model_t *pModel)
{
    int i, j;
    unsigned int polling_mask, val;

    /*Backup polling mask*/
    polling_mask = REG32(0xbb00a17c);

    /*Disable MAC polling PHY*/
    REG32(0xbb00a17c) = 0;

    //printf("REG32(0xbb00a17c) is %8x\n",REG32(0xbb00a17c));

    for (i=0; i<pModel->phy.count; i++)
    {
        switch (pModel->phy.list[i].chip)
        {
            #if (defined(CONFIG_RTL8208))
            case RTK_CHIP_RTL8208D:
            case RTK_CHIP_RTL8208L:
                {
                    unsigned int rtl8208d_phy0_id = pModel->phy.list[i].mac_id;
                    unsigned int phyData;

                    for (j=(rtl8208d_phy0_id); j<(rtl8208d_phy0_id+8); j++)
                    {
                        /* enable EEE function */
                        gMacDrv->drv_miim_read(j, 4, 16, &phyData);
                        phyData &= ~(3 << 12);
                        gMacDrv->drv_miim_write(j, 4, 16, phyData);

                        gMacDrv->drv_miim_write(j, gMacDrv->miim_max_page, 31, 0x0);
                        gMacDrv->drv_miim_write(j, gMacDrv->miim_max_page, 13, 0x7);
                        gMacDrv->drv_miim_write(j, gMacDrv->miim_max_page, 14, 0x3c);
                        gMacDrv->drv_miim_write(j, gMacDrv->miim_max_page, 13, 0x4007);
                        gMacDrv->drv_miim_write(j, gMacDrv->miim_max_page, 14, 0x0);
                        gMacDrv->drv_miim_write(j, gMacDrv->miim_max_page, 13, 0x0);

                        /* Force re-autonegotiation if AN is on */
                        gMacDrv->drv_miim_read(j, 0, 0, &phyData);
                        if (phyData & 0x1000)
                        {
                            phyData |= (1 << 9);
                        }
                        gMacDrv->drv_miim_write(j, 0, 0, phyData);
                    }
                }
                break;
            #endif
            #if (defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B) || defined(CONFIG_RTL8212B))
            case RTK_CHIP_RTL8214FB:
            case RTK_CHIP_RTL8214B:
            case RTK_CHIP_RTL8212B:
                {
                    unsigned int    rtl8214fb_phy0_macid = pModel->phy.list[i].mac_id;
                    unsigned int             forceReg, phyData;
                    int             idx;

                    /* store the original register value */
                    gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 1,
                            gMacDrv->miim_max_page, 31, 8);
                    gMacDrv->drv_miim_read(rtl8214fb_phy0_macid + 1,
                            gMacDrv->miim_max_page, 16, &forceReg);
                    gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 1,
                            gMacDrv->miim_max_page, 31, 8);

                    for (idx = 0; idx < (sizeof(rtl8380_rrtl8214fb_eee_disable)/sizeof(confcode_prv_t)); ++idx)
                    {
                        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + rtl8380_rrtl8214fb_eee_disable[idx].phy,
                                gMacDrv->miim_max_page,
                                (int)rtl8380_rrtl8214fb_eee_disable[idx].reg,
                                (int)rtl8380_rrtl8214fb_eee_disable[idx].val);
                    }

                    /* Restart Auto-Negotiation */
                    for (idx = 0; idx < 4; ++idx)
                    {
                        gMacDrv->drv_miim_read(rtl8214fb_phy0_macid + idx,
                                gMacDrv->miim_max_page, 0, &phyData);

                        phyData |= (1 << 9);
                        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + idx,
                                gMacDrv->miim_max_page, 0, phyData);
                    }

                    /* restore the original register value */
                    gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 1,
                            gMacDrv->miim_max_page, 31, 8);
                    gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 1,
        			        gMacDrv->miim_max_page, 16, forceReg);
                    gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 1,
                            gMacDrv->miim_max_page, 31, 8);
                    OSAL_MDELAY(1000);
                }
                break;
            #endif

            #if (defined(CONFIG_RTL8218))
            case RTK_CHIP_RTL8218:
                {
                    unsigned int rtl8218_macid_pm = (0xFF << pModel->phy.list[i].mac_id);

                    //printf("PHY[%d]: disable EEE on port [0x%08x]\n", i, rtl8218_macid_pm);

                    //###### negear EEE Nway ability autooff
                    gMacDrv->drv_miim_portmask_write(rtl8218_macid_pm,127,0x1f,0x0005);
                    gMacDrv->drv_miim_portmask_write(rtl8218_macid_pm,127,0x05,0x8b84);
                    gMacDrv->drv_miim_portmask_write(rtl8218_macid_pm,127,0x06,0x0042);
                    gMacDrv->drv_miim_portmask_write(rtl8218_macid_pm,127,0x1f,0x0000);

                    //## ext page 32
                    gMacDrv->drv_miim_portmask_write(rtl8218_macid_pm,7,0x1e,0x0020);
                    gMacDrv->drv_miim_portmask_write(rtl8218_macid_pm,7,0x17,0x000a);
                    gMacDrv->drv_miim_portmask_write(rtl8218_macid_pm,7,0x1b,0x2f4a);
                    gMacDrv->drv_miim_portmask_write(rtl8218_macid_pm,7,0x15,0x0000);
                }

                /* Force re-autonegotiation if AN is on */
                {
                    for (j=(pModel->phy.list[i].mac_id+0); j<(pModel->phy.list[i].mac_id+8); j++)
                    {
                        gMacDrv->drv_miim_read(j, 0, 0, &val);
                        if (val & (1 << 12))
                        {
                            gMacDrv->drv_miim_write(j, 0, 0, (val | (1 << 9)));
                        }
                    }
                }
                break;
            #endif
            #if (defined(CONFIG_RTL8218B) || defined(CONFIG_RTL8214FC) || defined(CONFIG_RTL8218FB))
            case RTK_CHIP_RTL8218B:
            case RTK_CHIP_RTL8214FC:
            case RTK_CHIP_RTL8218FB:
                {
                     //int rtl8218b_macid_pm = (0xFF << pModel->phy.list[i].mac_id);
                     int phy_idx;
                     unsigned int value;

                    //printf("PHY[%d]: disable EEE on port [0x%08x]\n", i, rtl8218b_macid_pm);

                    /* 10M EEE Amplitude */
                    for(phy_idx=0; phy_idx<pModel->phy.list[i].phy_max; phy_idx++)
                    {
                         /* Force Copper */
        			    gMacDrv->drv_miim_write(pModel->phy.list[i].mac_id+phy_idx,0,30,0x0001);

                         /* 100/1000M EEE Capability */
                        gMacDrv->drv_miim_write(pModel->phy.list[i].mac_id+phy_idx,0,13,0x0007);
                        gMacDrv->drv_miim_write(pModel->phy.list[i].mac_id+phy_idx,0,14,0x003C);
                        gMacDrv->drv_miim_write(pModel->phy.list[i].mac_id+phy_idx,0,13,0x4007);
                        gMacDrv->drv_miim_write(pModel->phy.list[i].mac_id+phy_idx,0,14,0x0000);

                        gMacDrv->drv_miim_read(pModel->phy.list[i].mac_id+phy_idx,0x0A43,25,&value);
                        value &= ~(1UL<<4);
                        gMacDrv->drv_miim_write(pModel->phy.list[i].mac_id+phy_idx,0x0A43,25,value);

	                /* Force re-autonegotiation if AN is on */
			  gMacDrv->drv_miim_read(pModel->phy.list[i].mac_id+phy_idx, 0, 0, &val);
                        if (val & (1 << 12))
                        {
                            gMacDrv->drv_miim_write(pModel->phy.list[i].mac_id+phy_idx, 0, 0, (val | (1 << 9)));
                        }

                         /* Back to  AUTO */
                        gMacDrv->drv_miim_write(pModel->phy.list[i].mac_id+phy_idx,0,30,0x0000);
                    }
                }
                break;
            #endif
            default:
                //printf("PHY[%d]: not supported in EEE\n", i);
                break;
        }
    }

     /*Restore polling mask*/
     REG32(0xbb00a17c) = polling_mask;

    return;
} /* end of rtk_eee_off */
#endif

/* Function Name:
 *      rtk_comboport_copper
 * Description:
 *      Force comboport into copper mode.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
 #if (defined(CONFIG_RTL8214F))
 void rtl8214f_media_set(int portid, int media);
#endif

 #if (defined(CONFIG_RTL8214FB))
void rtl8218fb_media_set(int portid, int media);
#endif

void rtk_comboport_copper(void)
{
    unsigned int _RTL8214F_PhyId, _RTL8218FB_PhyId;
    int i;
    int portid;

    if (gSwitchModel == NULL)
        return;

    if (MEDIATYPE_COPPER == gComboPortMode)
        return;

    gComboPortMode = MEDIATYPE_COPPER;

    for (i=0; i<gSwitchModel->phy.count; i++)
    {

        #if (defined(CONFIG_RTL8214F))
        if (gSwitchModel->phy.list[i].chip == RTK_CHIP_RTL8214F)
        {
            _RTL8214F_PhyId = gSwitchModel->phy.list[i].mac_id - gSwitchModel->phy.list[0].mac_id;

            //printf("RTL8214F PHY ID: %d\n", _RTL8214F_PhyId);

            for (portid=_RTL8214F_PhyId; portid<(_RTL8214F_PhyId+gSwitchModel->phy.list[i].phy_max); portid++)
            {
                rtl8214f_media_set(portid, MEDIATYPE_COPPER);
            }

            break;
        }
        #endif
        #if (defined(CONFIG_RTL8214FC))
        if (gSwitchModel->phy.list[i].chip ==  RTK_CHIP_RTL8214FC)
        {
            _RTL8218FB_PhyId = gSwitchModel->phy.list[i].mac_id - gSwitchModel->phy.list[0].mac_id;

            for (portid=_RTL8218FB_PhyId; portid<(_RTL8218FB_PhyId+gSwitchModel->phy.list[i].phy_max); portid++)
                rtl8214fc_media_set(portid, MEDIATYPE_COPPER);
            break;
        }
        #endif
        #if (defined(CONFIG_RTL8218FB))
        if (gSwitchModel->phy.list[i].chip == RTK_CHIP_RTL8218FB)
        {
            _RTL8218FB_PhyId = gSwitchModel->phy.list[i].mac_id - gSwitchModel->phy.list[0].mac_id;

            //printf("RTL8218FB PHY ID: %d\n", _RTL8218FB_PhyId);

            for (portid=_RTL8218FB_PhyId+4; portid<(_RTL8218FB_PhyId+gSwitchModel->phy.list[i].phy_max); portid++)
            {
                rtl8218fb_media_set(portid, MEDIATYPE_COPPER);
            }

            break;
        }
        #endif
    }

    return;
} /* end of rtk_comboport_copper */


/* Function Name:
 *      rtk_comboport_fiber
 * Description:
 *      Force comboport into fiber mode.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtk_comboport_fiber(void)
{
    unsigned int _RTL8214F_PhyId ,_RTL8214FC_PhyId, _RTL8218FB_PhyId;
    int i;
    int portid;

    if (gSwitchModel == NULL)
        return;

    if (MEDIATYPE_FIBER == gComboPortMode)
        return;

    gComboPortMode = MEDIATYPE_FIBER;

    for (i=0; i<gSwitchModel->phy.count; i++)
    {
        #if (defined(CONFIG_RTL8214F))
        if (gSwitchModel->phy.list[i].chip == RTK_CHIP_RTL8214F)
        {
            _RTL8214F_PhyId = gSwitchModel->phy.list[i].mac_id - gSwitchModel->phy.list[0].mac_id;

            //printf("RTL8214F PHY ID: %d\n", _RTL8214F_PhyId);

            for (portid=_RTL8214F_PhyId; portid<(_RTL8214F_PhyId+gSwitchModel->phy.list[i].phy_max); portid++)
            {
                rtl8214f_media_set(portid, MEDIATYPE_FIBER);
            }

            break;
        }
        #endif
        #if (defined(CONFIG_RTL8214FC))
        if (gSwitchModel->phy.list[i].chip == RTK_CHIP_RTL8214FC)
        {
            _RTL8214FC_PhyId = gSwitchModel->phy.list[i].mac_id - gSwitchModel->phy.list[0].mac_id;

            //printf("RTL8214FC PHY ID: %d\n", _RTL8214FC_PhyId);

            for (portid=_RTL8214FC_PhyId; portid<(_RTL8214FC_PhyId+gSwitchModel->phy.list[i].phy_max); portid++)
            {
                rtl8214fc_media_set(portid, MEDIATYPE_FIBER);
            }

            break;
        }
        #endif
        #if (defined(CONFIG_RTL8218FB))
        if (gSwitchModel->phy.list[i].chip == RTK_CHIP_RTL8218FB)
        {
            _RTL8218FB_PhyId = gSwitchModel->phy.list[i].mac_id - gSwitchModel->phy.list[0].mac_id;

            //printf("RTL8218FB PHY ID: %d\n", _RTL8218FB_PhyId);

            for (portid=_RTL8218FB_PhyId+4; portid<(_RTL8218FB_PhyId+gSwitchModel->phy.list[i].phy_max); portid++)
            {
                rtl8218fb_media_set(portid, MEDIATYPE_FIBER);
            }

            break;
        }
        #endif
    }

    return;
} /* end of rtk_comboport_fiber */

void rtk_linkdown_powersaving_patch(void)
{
    unsigned char port, tryCount;
    unsigned int value, value1;
    unsigned int flag = FALSE;

    while(1)
    {
        for(port= 0; port < 16; port++)
        {
            gMacDrv->drv_miim_read((int)port, 0, 1, &value);
            gMacDrv->drv_miim_read((int)port, 0, 1, &value);

            if ((value & 0x4) == 0)
            {
                flag = TRUE;
                for (tryCount = 0; tryCount < 20; tryCount++)
                {
                    gMacDrv->drv_miim_read((int)port, 0, 19, &value1);
                    if (value1 & 0x8000)
                    {
                        gMacDrv->drv_miim_write((int)port, 0, 24, 0x0310);
                        break;
                    }
                }
            }
        }

        if(TRUE)
        {
            udelay(800);
            gMacDrv->drv_miim_write(0xff, 0x0, 24, 0x8310);
        }
    }

} /* end of rtk_linkdown_powersaving_patch */

void rtk_l2testmode_on(void)
{
    /*Enable all port phy  power up*/
    rtk_network_on();

    printf("Force port28 link down\n\n");
    /*port 28 force link down*/
    REG32(0xbb00a174) = 0x6192d;     /*port 28*/
}

void rtk_l2testmode_off(void)
{
    /*Enable all port phy  power down*/
    rtk_network_off();
}


void rtk_port_isolation_on(void)
{
    unsigned char i;
    /*Port Isolation settings: port 0<-->1, 2<-->3, 4<-->5.......*/
    //printf("Port0<-->Port1, Port2<-->Port3, ........\n\n");
    for(i = 0; i < 28; i = i+2)
    {
        REG32(0xbb004100+i*4) = 0x3<<i;
        REG32(0xbb004100+i*4+4) = 0x3<<i;
    }

    /* For 838X, port 24 and 26 are fiber ports */
    REG32(0xbb004100 + (24 * 4)) = 0x3 << 24;
    REG32(0xbb004100 + (26 * 4)) = 0x3 << 24;

    /*Disable Port0-Port28 SA learning*/
    //printf("Disable Port0-Port28 SA learning\n\n");
    REG32(0xbb00328C) = 0xAAAAAAAA;
    REG32(0xbb003290) = 0xAAAAAAAA;
}

void rtk_port_isolation_off(void)
{
    unsigned char i;

    //printf("All ports can communicate with each other.\n\n");
    for(i = 0; i < 28; i++)
    {
        REG32(0xbb004100+i*4) = 0x1fffffff;
    }

    /*Enable Port0-Port28 SA learning*/
    //printf("Enable Port0-Port28 SA learning\n\n");
    REG32(0xbb00328C) = 0;
    REG32(0xbb003290) = 0;
}

void rtk_portIsolation(int srcPort,int destPort)
{
    /*Port Isolation settings: srcPort-->destPort */
    //printf("port%d-->Port%d\n",srcPort,destPort);
    REG32(0xbb004100+srcPort*4) = 0x1<<destPort;
}

void rtk_portIsolationCPUgoto(int port)
{
    /*Port Isolation settings: CPU-->port */
    //printf("CPU-->port%2d\n",port);
    REG32(0xbb004100+28*4) = 0x1<<port;
}


void rtk_portIsolationToCPU(int port)
{
    /*Port Isolation settings: port-->CPU */
    //printf("port%2d-->CPU.\n",port);
    REG32(0xbb004100+port*4) = 0x1<<28;
    REG32(0xbb004100+port*4) |= 0x1<<port;
}



void rtk_saLearning(int state)
{

    if(state==0){
        /*Disable Port0-Port28 SA learning*/
        printf("Disable Port0-Port28 SA learning\n\n");
        REG32(0xbb00328C) = 0xAAAAAAAA;
        REG32(0xbb003290) = 0xAAAAAAAA;
    }else{
    /*Enable Port0-Port28 SA learning*/
    printf("Enable Port0-Port28 SA learning\n\n");
    REG32(0xbb00328C) = 0;
    REG32(0xbb003290) = 0;
}
}


void rtk_phy_selfLoop_on(int portId)
{
    unsigned int val;
    rtl8380_getPhyReg(portId,0,0,&val);
    val |= 0x4000;
    rtl8380_setPhyReg(portId,0,0,val);

    return;
}

void rtk_phy_selfLoop_off(int portId)
{
    unsigned int val;
    rtl8380_getPhyReg(portId,0,0,&val);
    val &= 0xffffBfff;
    rtl8380_setPhyReg(portId,0,0,val);

    return;
}

int32 rtk_portLinkStatus_get(int portId)
{
    return rtl8380_getPortLinkStatus(portId);
}

void rtk_smiRead(uint32 phyad, uint32 regad, uint32* pData)
{
#if defined(CONFIG_MDC_MDIO_EXT_SUPPORT)
    rtl8380_smiRead(phyad, regad, pData);
#endif
}

void rtk_smiWrite(uint32 phyad, uint32 regad, uint32 data)
{
#if defined(CONFIG_MDC_MDIO_EXT_SUPPORT)
    rtl8380_smiWrite(phyad, regad, data);
#endif
}


int32 rtk_rstDeftGpio_init(void)
{
    return 0;
}   /* end of rtk_rstDeftGpio_init */


#define RTL8231_GPIO_ACCESS_DELAY (0xFFF)
#define RTL8231_GPIO_READ_TIMES (10)
#define RTL8231_GPIO_VALUE_THRD (3)
int32 rtk_htp_detect(void)
{
    uint32 i;
    uint32 htp_gpio_val;
    uint32 round,read_delay;
    uint32 htp_gpio_val_sum=0;

    extGpio_pin_dir_set(0, RTL8231_PIN_7, rtl8231_PIN_DIR_IN);

    for (i = 0; i < 0xF; ++i)
    {
        if (1 == extGpio_pin_data_get(0, RTL8231_PIN_7, &htp_gpio_val))
            break;
    }

    for(round=0;round<RTL8231_GPIO_READ_TIMES;round++)
    {
        extGpio_pin_data_get(0, RTL8231_PIN_7, &htp_gpio_val);
        htp_gpio_val_sum += htp_gpio_val;
        for(read_delay=0;read_delay<RTL8231_GPIO_ACCESS_DELAY;read_delay++);
    }

    if(htp_gpio_val_sum < RTL8231_GPIO_VALUE_THRD)
        return 1;
    else
        return 0;
}

int32 rtk_rstDeftGpio_detect(void)
{
    uint32 rst_gpio_val=0;

    extGpio_pin_dir_set(0, RTL8231_PIN_3, rtl8231_PIN_DIR_IN);
    extGpio_pin_data_get(0, RTL8231_PIN_3, &rst_gpio_val);

    if(0 == rst_gpio_val)
        return 1;

    return 0;
}   /* end of rtk_rstDeftGpio_detect */

void rtk_sys_led_on(void)
{
    uint32 val;

    /*RTL838xM internal GPIO_A0 is defined as sys_led*/
    val = REG32(0xBB00A000) & (~(0x7<<15));
    REG32(0xBB00A000) = val | (0x7<<15);
}

void rtk_sys_led_off(void)
{
    uint32 val;

    /*RTL838xM internal GPIO_A0 is defined as sys_led*/
    val = REG32(0xBB00A000);
    REG32(0xBB00A000) = val & (~(1<<15));
}

void rtk_sys_led_blink(void)
{
    uint32 val;

    /*RTL838xM internal GPIO_A0 is defined as sys_led*/
    val = REG32(0xBB00A000) & (~(0x7<<15));
    REG32(0xBB00A000) = val | (0x3<<15);
}


void rtk_mac_polling_enable(int port)
{
    uint32  val;

    val = MEM32_READ(0xbb00a17c);
    val |= (1 << port);
    MEM32_WRITE(0xbb00a17c, val);

    return;
}

void rtk_mac_polling_disable(int port)
{
    uint32  val;

    val = MEM32_READ(0xbb00a17c);
    val &= ~(1 << port);
    MEM32_WRITE(0xbb00a17c, val);

    return;
}

void rtk_serdes_fiber_watchdog(int port)
{
    uint32  reg, val;
    int     chk_loop, chk_error = 0;

    val = 0;
    if (24 == port)
    {
        MEM32_WRITE(0xbb00f7e0, val);
        val = MEM32_READ(0xbb00f800);

        reg = 0xbb00f7e4;
    }
    else if (26 == port)
    {
        MEM32_WRITE(0xbb00f8e0, val);
        val = MEM32_READ(0xbb00f900);

        reg = 0xbb00f8e4;
    }
    else
        return;

    /* only work for giga */
    if (0 != ((val >> 13) & 0x1) || 1 != ((val >> 6) & 0x1))
        return;

    /* read & clear */
    val = MEM32_READ(reg);

    for(chk_loop = 0; chk_loop < 3; ++chk_loop)
    {
        val = MEM32_READ(reg);
        if (0xFF == (val & 0xFF))
        {
            chk_error++;
        }
        OSAL_MDELAY(200);
    }

    if (chk_error >= 2)
    { /*Error happen*/
        if (24 == port)
        {
            reg = 0xbb00ef80;
        }
        else if (26 == port)
        {
            reg = 0xbb00f180;
        }

        val = MEM32_READ(reg);
        val &= ~(1 << 1);
        MEM32_WRITE(reg, val);

        OSAL_MDELAY(200);

        val |= (1 << 1);
        MEM32_WRITE(reg, val);
    }

    return;
}

extern void rtl8214fc_fiber_watchdog(int port);
/* Function Name:
 *      rtk_fiber_watchdog
 * Description:
 *      None
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtk_fiberRxWatchdog(void)
{
    int i, phyIdx, port;

    if (gSwitchModel == NULL)
        return;

    for (i = 0; i < gSwitchModel->port.count; ++i)
    {
        port = gSwitchModel->port.list[i].mac_id;

        phyIdx = gSwitchModel->port.list[i].phy_idx;
        switch(gSwitchModel->phy.list[phyIdx].chip)
        {
            #if (defined(CONFIG_RTL8214FC) || defined(CONFIG_RTL8218FB))
            case RTK_CHIP_RTL8218FB:
                if (4 > gSwitchModel->port.list[i].phy)
                    break;
            case RTK_CHIP_RTL8214FC:
        	  rtk_mac_polling_disable(port);
                rtl8214fc_fiber_watchdog(port);
       	  rtk_mac_polling_enable(port);
                break;
            #endif
            case RTK_CHIP_NONE:
        	  rtk_mac_polling_disable(port);
                rtk_serdes_fiber_watchdog(port);
       	  rtk_mac_polling_enable(port);
                break;
            default:
                break;
        }

    }

    return;
}




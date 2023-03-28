/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * Purpose : RTL8328 SOC commands for U-Boot.
 *
 * Feature : 
 *
 */


/*  
 * Include Files 
 */
#include <common.h>
#include <command.h>
#include <rtk/mac/rtl8328/rtl8328_rtk.h>
#include <rtk/mac/rtl8328/rtl8328_drv.h>
#include "rtl8328_asicregs.h"

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

static unsigned int gComboPortMode;               /* 0: Copper, 1: Fiber */

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
    unsigned int port, val;
    
    /* Enable Mac TxRx */
    for(port = 0; port < 28; port++)
    {
        REG32(0xbb900000 + port*0x100) |= 4;        
    } 

    rtl8328_phyPowerOn();
    
    /* Enable serial mode per-port LED */
    if (gSwitchModel)
    {
        if (gSwitchModel->led.count == 32)
            val = 0xFFFFFFFF;
        else
            val = ((1 << (gSwitchModel->led.count)) - 1) << gSwitchModel->led.offset;
        MEM32_WRITE(0xbb670064, val);
    }
    else
    {
        MEM32_WRITE(0xbb670064, 0xFFFFFFFF);
    }

    /* delay 2.0 sec for link-status stable */
    printf("Please wait for PHY init-time ...\n\n");
    udelay(2000000);

    return;
} /* end of rtk_network_on */


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
    unsigned int port;
    
    /* Disable Mac TxRx and Power PHY off */
    for(port = 0; port < 28; port++)
    {
        REG32(0xbb900000 + port*0x100) &= ~4;     
    } 

    rtl8328_phyPowerOff();

    /* Disable serial mode per-port LED */
    MEM32_WRITE(0xbb670064, 0x00000000);

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

    sprintf(addr_str1, "0x%08X", &jffs2_pattern_default);
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
    int i, j, val;

    for (i=0; i<pModel->phy.count; i++)
    {
        switch (pModel->phy.list[i].chip)
        {
        case RTK_CHIP_RTL8208D:
            {
                unsigned int phyData;
                unsigned int rtl8208d_phy0_id = pModel->phy.list[i].phyid;
                unsigned int rtl8208d_macid_pm = (0xFF << pModel->phy.list[i].phyid);

                printf("PHY[%d]: enable EEE on port [0x%08x]\n", i, rtl8208d_macid_pm);

                for (j=(rtl8208d_phy0_id); j<(rtl8208d_phy0_id+8); j++)
                {
                    /* enable EEE function */
                    gMacDrv->drv_miim_read(j, 4, 16, &phyData);
                    phyData |= (3 << 12);
                    gMacDrv->drv_miim_write(j, 4, 16, phyData);
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

        case RTK_CHIP_RTL8214FB:
        case RTK_CHIP_RTL8214B:
        case RTK_CHIP_RTL8212B:
            {
                unsigned int restore_phyData, phyData, is_phyDown;
                unsigned int rtl8214fb_phy0_id = pModel->phy.list[i].phyid;
                unsigned int rtl8214fb_macid_pm = (0xF << pModel->phy.list[i].phyid);

                printf("PHY[%d]: enable EEE on port [0x%08x]\n", i, rtl8214fb_macid_pm);

                for (j=(rtl8214fb_phy0_id); j<(rtl8214fb_phy0_id+4); j++)
                {
                    gMacDrv->drv_miim_read(rtl8214fb_phy0_id+1, 8, 16, &restore_phyData);
                    phyData = restore_phyData & ~(1<<(12+j-rtl8214fb_phy0_id));
                    phyData |= (1<<(8+j-rtl8214fb_phy0_id));
                    gMacDrv->drv_miim_write(rtl8214fb_phy0_id+1, 8, 16, phyData);
                    /* Power Down PHY */
                    gMacDrv->drv_miim_read(j, 0, 0, &phyData);
                    is_phyDown = (phyData >> 11) & 0x1;
                    phyData |= (1 << 11);
                    gMacDrv->drv_miim_write(j, 0, 0, phyData);
                    /* Micro-C Enable or Disable Auto Turn off EEE */
                    gMacDrv->drv_miim_write(j, 5, 5, 0x8B85);
                    gMacDrv->drv_miim_write(j, 5, 6, 0xE286);
                    gMacDrv->drv_miim_write(j, 5, 31, 0x0008);
                    /* Micro-C Control 10M EEE */
                    gMacDrv->drv_miim_write(j, 5, 5, 0x8B86);
                    gMacDrv->drv_miim_write(j, 5, 6, 0x8601);
                    gMacDrv->drv_miim_write(j, 5, 31, 0x0008);
                    /* Enable or Disable EEE */
                    gMacDrv->drv_miim_write(j, 7, 30, 0x0020);
                    gMacDrv->drv_miim_write(j, 7, 21, 0x1100);
                    gMacDrv->drv_miim_write(j, 7, 27, 0xA0BA);
                    gMacDrv->drv_miim_write(j, 7, 31, 0x0008);
                    /* 100/1000M EEE Capability */
                    gMacDrv->drv_miim_write(j, 0, 13, 0x0007);
                    gMacDrv->drv_miim_write(j, 0, 14, 0x003C);
                    gMacDrv->drv_miim_write(j, 0, 13, 0x4007);
                    gMacDrv->drv_miim_write(j, 0, 14, 0x0006);
                    gMacDrv->drv_miim_write(j, 0, 31, 0x0008);
                    /* 10M EEE Amplitude */
                    gMacDrv->drv_miim_write(j, 2, 11, 0x17A7);
                    gMacDrv->drv_miim_write(j, 2, 31, 0x0008);
                    /* Force re-autonegotiation if AN is on */
                    gMacDrv->drv_miim_read(j, 0, 0, &phyData);
                    if (phyData & 0x1000)
                    {
                        phyData |= (1 << 9);
                    }
                    if (!is_phyDown)
                        phyData &= ~(1<<11);
                    gMacDrv->drv_miim_write(j, 0, 0, phyData);

                    /* restore PHY 1, page 8, register 16 */
                    gMacDrv->drv_miim_write(rtl8214fb_phy0_id+1, 8, 16, restore_phyData);
                }
            }
            break;

        case RTK_CHIP_RTL8218:
            {
                unsigned int rtl8218_macid_pm = (0xFF << pModel->phy.list[i].phyid);
                //int val;
                
                printf("PHY[%d]: enable EEE on port [0x%08x]\n", i, rtl8218_macid_pm);

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
                for (j=(pModel->phy.list[i].phyid+0); j<(pModel->phy.list[i].phyid+8); j++)
                {
                    gMacDrv->drv_miim_read(j, 0, 0, &val);
                    if (val & (1 << 12))
                    {
                        gMacDrv->drv_miim_write(j, 0, 0, (val | (1 << 9)));
                    }
                }
            }
            break;

        default:
            printf("PHY[%d]: not supported in EEE\n", i);
            break;
        }
    }

    return;
} /* end of rtk_eee_on */

void rtk_eee_off(const rtk_switch_model_t *pModel)
{
    int i, j, val;

    for (i=0; i<pModel->phy.count; i++)
    {
        switch (pModel->phy.list[i].chip)
        {
        case RTK_CHIP_RTL8208D:
            {
                unsigned int phyData;
                unsigned int rtl8208d_phy0_id = pModel->phy.list[i].phyid;
                unsigned int rtl8208d_macid_pm = (0xFF << pModel->phy.list[i].phyid);

                printf("PHY[%d]: disable EEE on port [0x%08x]\n", i, rtl8208d_macid_pm);

                for (j=(rtl8208d_phy0_id); j<(rtl8208d_phy0_id+8); j++)
                {
                    /* enable EEE function */
                    gMacDrv->drv_miim_read(j, 4, 16, &phyData);
                    phyData &= ~(3 << 12);
                    gMacDrv->drv_miim_write(j, 4, 16, phyData);
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

        case RTK_CHIP_RTL8214FB:
        case RTK_CHIP_RTL8214B:
        case RTK_CHIP_RTL8212B:
            {
                unsigned int restore_phyData, phyData, is_phyDown;
                unsigned int rtl8214fb_phy0_id = pModel->phy.list[i].phyid;
                unsigned int rtl8214fb_macid_pm = (0xF << pModel->phy.list[i].phyid);

                printf("PHY[%d]: disable EEE on port [0x%08x]\n", i, rtl8214fb_macid_pm);

                for (j=(rtl8214fb_phy0_id); j<(rtl8214fb_phy0_id+4); j++)
                {
                    gMacDrv->drv_miim_read(rtl8214fb_phy0_id+1, 8, 16, &restore_phyData);
                    phyData = restore_phyData & ~(1<<(12+j-rtl8214fb_phy0_id));
                    phyData |= (1<<(8+j-rtl8214fb_phy0_id));
                    gMacDrv->drv_miim_write(rtl8214fb_phy0_id+1, 8, 16, phyData);
                    /* Power Down PHY */
                    gMacDrv->drv_miim_read(j, 0, 0, &phyData);
                    is_phyDown = (phyData >> 11) & 0x1;
                    phyData |= (1 << 11);
                    gMacDrv->drv_miim_write(j, 0, 0, phyData);
                    /* Micro-C Enable or Disable Auto Turn off EEE */
                    gMacDrv->drv_miim_write(j, 5, 5, 0x8B85);
                    gMacDrv->drv_miim_write(j, 5, 6, 0xC286);
                    gMacDrv->drv_miim_write(j, 5, 31, 0x0008);
                    /* Micro-C Control 10M EEE */
                    gMacDrv->drv_miim_write(j, 5, 5, 0x8B86);
                    gMacDrv->drv_miim_write(j, 5, 6, 0x8600);
                    gMacDrv->drv_miim_write(j, 5, 31, 0x0008);
                    /* Enable or Disable EEE */
                    gMacDrv->drv_miim_write(j, 7, 30, 0x0020);
                    gMacDrv->drv_miim_write(j, 7, 21, 0x0000);
                    gMacDrv->drv_miim_write(j, 7, 27, 0xA0BA);
                    gMacDrv->drv_miim_write(j, 7, 31, 0x0008);
                    /* 100/1000M EEE Capability */
                    gMacDrv->drv_miim_write(j, 0, 13, 0x0007);
                    gMacDrv->drv_miim_write(j, 0, 14, 0x003C);
                    gMacDrv->drv_miim_write(j, 0, 13, 0x4007);
                    gMacDrv->drv_miim_write(j, 0, 14, 0x0000);
                    gMacDrv->drv_miim_write(j, 0, 31, 0x0008);
                    /* 10M EEE Amplitude */
                    gMacDrv->drv_miim_write(j, 2, 11, 0x17A7);
                    gMacDrv->drv_miim_write(j, 2, 31, 0x0008);
                    /* Force re-autonegotiation if AN is on */
                    gMacDrv->drv_miim_read(j, 0, 0, &phyData);
                    if (phyData & 0x1000)
                    {
                        phyData |= (1 << 9);
                    }
                    if (!is_phyDown)
                        phyData &= ~(1<<11);
                    gMacDrv->drv_miim_write(j, 0, 0, phyData);

                    /* restore PHY 1, page 8, register 16 */
                    gMacDrv->drv_miim_write(rtl8214fb_phy0_id+1, 8, 16, restore_phyData);
                }
            }
            break;

        case RTK_CHIP_RTL8218:
            {
                unsigned int rtl8218_macid_pm = (0xFF << pModel->phy.list[i].phyid);
                
                printf("PHY[%d]: disable EEE on port [0x%08x]\n", i, rtl8218_macid_pm);

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
                for (j=(pModel->phy.list[i].phyid+0); j<(pModel->phy.list[i].phyid+8); j++)
                {
                    gMacDrv->drv_miim_read(j, 0, 0, &val);
                    if (val & (1 << 12))
                    {
                        gMacDrv->drv_miim_write(j, 0, 0, (val | (1 << 9)));
                    }
                }
            }
            break;

        default:
            printf("PHY[%d]: not supported in EEE\n", i);
            break;
        }
    }

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
void rtk_comboport_copper(void)
{
    unsigned int _RTL8214F_PhyId;
    int i;
    int portid;

    if (gSwitchModel == NULL)
        return;

    if (MEDIATYPE_COPPER == gComboPortMode)
        return;

    gComboPortMode = MEDIATYPE_COPPER;

    for (i=0; i<gSwitchModel->phy.count; i++)
    {
        if (gSwitchModel->phy.list[i].chip == RTK_CHIP_RTL8214F)
        {
            _RTL8214F_PhyId = gSwitchModel->phy.list[i].phyid - gSwitchModel->phy.list[0].phyid;

            printf("RTL8214F PHY ID: %d\n", _RTL8214F_PhyId);

            for (portid=_RTL8214F_PhyId; portid<(_RTL8214F_PhyId+gSwitchModel->phy.list[i].phy_max); portid++)
            {
                rtl8214f_media_set(portid, MEDIATYPE_COPPER);
            }

            break;
        }
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
    unsigned int _RTL8214F_PhyId;
    int i;
    int portid;

    if (gSwitchModel == NULL)
        return;

    if (MEDIATYPE_FIBER == gComboPortMode)
        return;

    gComboPortMode = MEDIATYPE_FIBER;

    for (i=0; i<gSwitchModel->phy.count; i++)
    {
        if (gSwitchModel->phy.list[i].chip == RTK_CHIP_RTL8214F)
        {
            _RTL8214F_PhyId = gSwitchModel->phy.list[i].phyid - gSwitchModel->phy.list[0].phyid;

            printf("RTL8214F PHY ID: %d\n", _RTL8214F_PhyId);

            for (portid=_RTL8214F_PhyId; portid<(_RTL8214F_PhyId+gSwitchModel->phy.list[i].phy_max); portid++)
            {
                rtl8214f_media_set(portid, MEDIATYPE_FIBER);
            }

            break;
        }
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
            gMacDrv->drv_miim_read(port, 0, 1, &value);
            gMacDrv->drv_miim_read(port, 0, 1, &value);

            if ((value & 0x4) == 0)
            {
                flag = TRUE;
                for (tryCount = 0; tryCount < 20; tryCount++)
                {
                    gMacDrv->drv_miim_read(port, 0, 19, &value1);
                    if (value1 & 0x8000)
                    {
                        gMacDrv->drv_miim_write(port, 0, 24, 0x0310);
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

void rtk_l2testmode_on()
{
    uint32 port;
    for(port = 0; port < gSwitchModel->port.count; port++)
    {        
        /*RTL8328M, patch rrcp drop*/
        REG32(0xBBD00000+port*0x100)&= 0xFFFFFF0F;
        REG32(0xBBD00008+port*0x100)&= 0xFFFFFFF9;
        REG32(0xBBD00014+port*0x100)&= 0xFFFFFFFC;
        REG32(0xBBD00004+port*0x100) = 0xFFFF;
    }

    /*RTL8328M, patch rrcp drop*/
    REG32(0xBB600000) &= 0xFFFEFFFF;

    /*RTL8328M, Disable Tx CRC*/
    REG32(0xBB550000) = 0;

    /*RTL8328M, Force linkdown port28*/
    REG32(0xBB040088) = 2;

    /*RTL8328M, disable port28 L3, L4 checksum error drop*/
    REG32(0xBBD01C08) = 1;
}

void rtk_l2testmode_off()
{
    /*enable cpu port*/
    REG32(0xbb040088) = 0x706;

    /*RTL8328M, enable Tx CRC*/
    REG32(0xBB550000) = 0x1fffffff;

    REG32(0xBBD01C08) = 0;
}


/*
 * Copyright(c) Realtek Semiconductor Corporation, 2012
 * All rights reserved.
 *
 * Purpose :
 *
 * Feature :
 *
 */

/*
 * Include Files
 */
#include <rtk_type.h>
#include <rtk_osal.h>
#include <rtk_debug.h>
#include <rtk_switch.h>
#include <rtk_reg.h>
#include <init.h>
#include <interrupt.h>
#include <rtk/mac/rtl8380/rtl8380_swcore_reg.h>
#include <rtk/mac/rtl8380/rtl8380_drv.h>
#include <rtk/drv/rtl8231/rtl8231_drv.h>


#include <rtk/phy/conf/conftypes.h>



#if (defined(CONFIG_RTL8214) || defined(CONFIG_RTL8214F) || defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B) || defined(CONFIG_RTL8212B))
#include <rtk/phy/rtl8214f.h>
#endif

#if (defined(CONFIG_RTL8218) || defined(CONFIG_RTL8218B))
#include <rtk/phy/rtl8218.h>
#endif

#if (defined(CONFIG_RTL8218B) || defined(CONFIG_RTL8218FB))
#include <rtk/phy/rtl8218b.h>
#endif

#if (defined(CONFIG_RTL8214FC))
#include <rtk/phy/rtl8214fc.h>
#endif

#ifdef CONFIG_RTL8208
#include <rtk/phy/rtl8208.h>
#endif

#include <customer/customer_diag.h>
#include <rtk/mac/rtl8380/rtl8380_mdc_mdio.h>


/*
 * Symbol Definition
 */
/* per-port: {reg, val} */
typedef struct {
    uint32  reg;
    uint32  val;
} rtl8380_reg_value_t;

/*
 * Macro Definition
 */

/*
 * Data Declaration
 */
#define RTL8380_VERSION_NONE  0x0
#define RTL8380_VERSION_A         0x1
#define RTL8380_VERSION_B         0x2

uint32 rtl8380_version;

/*
 * Function Declaration
 */

/* Function Name:
 *      rtl8380_platform_config_init
 * Description:
 *      Platform Configuration code in RTL8380
 * Input:
 *      pModel - pointer to switch model of platform
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
static void rtl8380_platform_config_init(const rtk_switch_model_t *pModel)
{
    char *s_mac = NULL;

    /* write MAC addr to register */
    s_mac = getenv("ethaddr");
    if (s_mac != NULL)
    {
        int i;
        char enetaddr[6], *e;

        for (i=0; i<6; i++)
        {
            enetaddr[i] = (unsigned char)(s_mac ? simple_strtoul(s_mac, &e, 16) : 0);
            if (s_mac)
                s_mac = (*e) ? e+1 : e;
        }

        /* Store the MAC address info specify registers */
        //REG32(0xBB00A9EC) = (*(unsigned int *)&enetaddr[0] & 0x0000FFFFU);
        //REG32(0xBB00A9F0) = (*(unsigned int *)&enetaddr[2] & 0xFFFFFFFFU);
        REG32(SWCORE_BASE_ADDR | RTL8380_MAC_ADDR_CTRL_ADDR) = (((uint8)enetaddr[0]) << 8) | ((uint8)enetaddr[1]);
        REG32(SWCORE_BASE_ADDR | (RTL8380_MAC_ADDR_CTRL_ADDR + 4)) = (((uint8)enetaddr[2]) << 24) | (((uint8)enetaddr[3]) << 16) | (((uint8)enetaddr[4]) << 8) | ((uint8)enetaddr[5]);
        /* Write to another two duplicate registers, ALE/MAC block */
        REG32(SWCORE_BASE_ADDR | RTL8380_MAC_ADDR_CTRL_ALE_ADDR) = (((uint8)enetaddr[0]) << 8) | ((uint8)enetaddr[1]);
        REG32(SWCORE_BASE_ADDR | (RTL8380_MAC_ADDR_CTRL_ALE_ADDR + 4)) = (((uint8)enetaddr[2]) << 24) | (((uint8)enetaddr[3]) << 16) | (((uint8)enetaddr[4]) << 8) | ((uint8)enetaddr[5]);
        REG32(SWCORE_BASE_ADDR | RTL8380_MAC_ADDR_CTRL_MAC_ADDR) = (((uint8)enetaddr[0]) << 8) | ((uint8)enetaddr[1]);
        REG32(SWCORE_BASE_ADDR | (RTL8380_MAC_ADDR_CTRL_MAC_ADDR + 4)) = (((uint8)enetaddr[2]) << 24) | (((uint8)enetaddr[3]) << 16) | (((uint8)enetaddr[4]) << 8) | ((uint8)enetaddr[5]);
    }


#if 0
    /*led initial*/
    uint32 val;
    /* (BY CUSTOMER APPLICATION) LED System Control Register */
    val = ((pModel->led.sel_p24_p27_led_mod & 0x7FFF) << 15) | ((pModel->led.sel_p0_p23_led_mod & 0x7FFF) << 0);
    MEM32_WRITE(0xbb00a004, val);

    /* Set N-port serial LED output port mask */
    if (pModel->led.count == 28)
        val = 0xFFFFFFF;
    else
        val = ((1 << (pModel->led.count)) - 1) << pModel->led.offset;
    MEM32_WRITE(0xbb00a008, val);
#endif

} /* end of rtl8380_platform_config_init */


/* Function Name:
 *      rtl8380_mac_config_init
 * Description:
 *      Mac Configuration code in RTL8380
 * Input:
 *      pModel - pointer to switch model of platform
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
 static rtl8380_reg_value_t rtl8380_mac_patch[] = {\
    /*Patch for EEE, a timer cannot be zero*/
    {0xbb00AA04, 0x5001411},\
    {0xbb00AA08, 0x5001417},\
};

static void rtl8380_mac_config_init(const rtk_switch_model_t *pModel)
{
    uint32 i, val;
    uint32 portId;

    /*Set Port28-CPU Port Egress Drop always enable*/
    val = MEM32_READ(0xbb006B1C+28*4);
    val |= 1UL<<11;
    MEM32_WRITE(0xbb006B1C+28*4, val);

    /* Serdes: change tx & rx polarity */
    for (i=(pModel->serdes.offset); i<(pModel->serdes.offset + pModel->serdes.count); i++)
    {
        Tuint8 polarity_conf = 0;

        val = MEM32_READ(0xbb00e780 + i * 0x200);
        val = (val & ~(7 << 8));
        polarity_conf |= (pModel->serdes.list[(i - pModel->serdes.offset)].tx_polarity == SERDES_POLARITY_NORMAL)? 0x0 : 0x1;
        polarity_conf |= (pModel->serdes.list[(i - pModel->serdes.offset)].rx_polarity == SERDES_POLARITY_NORMAL)? 0x0 : 0x2;
        polarity_conf |= 0x4;
        val |= (polarity_conf << 8);
        MEM32_WRITE(0xbb00e780 + i * 0x200, val);
    }

    /******************************LED Settings***************************/
    /*LED NUMBER*/
    /*Set Port0-Port23, Port24-Port27 LED Number to be the same*/
    val = MEM32_READ(0xbb00a000);
    val &= ~(0x3f);

    switch (pModel->led.p0_p23_led_num)
    {
        /* bit2:0 : port0~port23 */
        case 0:
            val |= (0x0);
            break;
        case 1:
            val |= (0x1);
            break;
        case 2:
            val |= (0x3);
            break;
        case 3:
            val |= (0x7);
            break;
        default:
            val |= (0x7);
            break;
    }

    switch (pModel->led.p24_p27_led_num)
    {
        /* bit5:3 : port24~port27 */
        case 0:
            val |= ((0x0) << 3);
            break;
        case 1:
            val |= ((0x1) << 3);
            break;
        case 2:
            val |= ((0x3) << 3);
            break;
        case 3:
            val |= ((0x7) << 3);
            break;
        default:
            val |= ((0x7) << 3);
            break;
    }
    MEM32_WRITE(0xbb00a000, val);


    /*Combo Port LED mode*/
    if((pModel->chip == RTK_CHIP_RTL8382M) || (pModel->chip == RTK_CHIP_RTL8332M) || (pModel->chip == RTK_CHIP_RTL8330M))
    {
        val = MEM32_READ(0xbb00a000);
        val &= ~(0x3<<7);
        val |= (0x2 << 7);
        MEM32_WRITE(0xbb00a000, val);
    }


    /*LED MODE CONTROL*/
    val = pModel->led.sel_p0_p23_led_mod | (pModel->led.sel_p24_p27_led_mod<<15);
    MEM32_WRITE(0xbb00a004, val);

    /*LED MASK TO LIGHT LEDS*/
    val = 0;
    {
        /*The first 24 ports(Port0-Port23) settings*/
        if((pModel->led.count / 8) == 1)
        {
            val = 0x00FF00;
        }
        else  if((pModel->led.count / 8) == 2)
        {
            val = 0xFFFF00;
        }
        else  if((pModel->led.count / 8) == 3)
        {
            val = 0xFFFFFF;
        }
        else
        {
            val = 0;
        }

        /*The next four(Port24-Port27) ports settings*/
        if((pModel->led.count % 8) == 0)
        {
            val |= 0x0000000;
        }
        else  if((pModel->led.count % 8) == 1)
        {
            val |= 0x1000000;
        }
        else  if((pModel->led.count % 8) == 2)
        {
            /*Check Serdes4 Mode*/
            if(pModel->serdes.list[4].mii == RTK_MII_RSGMII)
            {
                val |= 0x3000000; /*RSGMII: 2.5G*/
            }
            else
            {
                val |= 0x5000000; /*SGMII: 1.25G*/
            }
        }
        else  if((pModel->led.count % 8) == 3)
        {
            val |= 0x7000000;
        }
        else  if((pModel->led.count % 8) == 4)
        {
            val |= 0xF000000;
        }
        else
        {
            val |= 0x0000000;
        }
    }
    MEM32_WRITE(0xbb00a008, val);


    /*Power on blinking*/
    val = MEM32_READ(0xbb001004);
    val &= ~(0x3<<2);
    val |= (0x2 << 2);
    MEM32_WRITE(0xbb001004, val);


    /******************************Giga Ability & PortID***************************/
    /*Disable Giga Ability & remapping PORTID*/
    switch(pModel->chip)
    {
        case RTK_CHIP_RTL8330M:
                 /* Port addr remapping:
                            Portid:0-5     -->Phyid:24--29
                            Portid:6-7     -->Phyid:14-15
                            Portid:8-15   -->Phyid:0--7
                            Portid:24-27 -->Phyid:8-11  */
                 MEM32_WRITE(0xbb00a1c8, 0x3BCDEB38);
                 MEM32_WRITE(0xbb00a1cc, 0x62081EE);
                 MEM32_WRITE(0xbb00a1d0, 0x230398a4);
                 MEM32_WRITE(0xbb00a1d8, 0x0005a928);
             break;
         default:
            /*Do nothing*/
            break;
    }

    /*MAC Patch*/
    for (i=0; i<(sizeof(rtl8380_mac_patch)/sizeof(rtl8380_reg_value_t)); i++)
    {
        MEM32_WRITE(rtl8380_mac_patch[i].reg,  rtl8380_mac_patch[i].val);
        OSAL_MDELAY(1);
    }

    /*Patch for disable Special Drain Out*/
    if((pModel->chip == RTK_CHIP_RTL8332M) || (pModel->chip == RTK_CHIP_RTL8382M))
    {
        for(i = 0; i <= 28; i++)
            MEM32_WRITE(0xBB00d57c+i*0x80,  0);
    }
    else if((pModel->chip == RTK_CHIP_RTL8330M) || (pModel->chip == RTK_CHIP_RTL8380M))
    {
        for(i = 8; i <= 28; i++)
            MEM32_WRITE(0xBB00d57c+i*0x80,  0);
    }
    else
    {
        /*Do nothing*/
    }

    /*begin patch mantis#0013049*/
    /************************disable two-pair down speed feature*************/
    /************************Added on 20130207 ***************************/
    for(portId = 0; portId < 29; portId++)
    {
        val = MEM32_READ(0xbb00a104+4*portId);
        val &= ~((1UL<<21) | (1UL<<24) | (1UL<<27));
        MEM32_WRITE(0xbb00a104+4*portId, val);
    }
    /*end patch mantis#0013049*/

    return;
} /* end of rtl8380_mac_config_init */

/* Function Name:
 *      rtl8380_phy_config_init
 * Description:
 *      PHY Configuration code that connect with RTL8380
 * Input:
 *      pModel - pointer to switch model of platform
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
static void rtl8380_phy_config_init(const rtk_switch_model_t *pModel)
{
    unsigned int phy_idx;

    for (phy_idx=0; phy_idx<pModel->phy.count; phy_idx++)
    {
        switch (pModel->phy.list[phy_idx].chip)
        {
            #if (defined(CONFIG_RTL8214FB))
            case RTK_CHIP_RTL8214FB:
                {
                    Tuint8 rtl8214fb_phy0_macid = pModel->phy.list[phy_idx].mac_id - pModel->phy.baseid;
                    rtl8214fb_config(rtl8214fb_phy0_macid);
                }
                break;
            #endif
            #if (defined(CONFIG_RTL8218B))
            case RTK_CHIP_RTL8218B:
                {
                    Tuint8 rtl8218b_phy_macid = pModel->phy.list[phy_idx].mac_id - pModel->phy.baseid;
                    rtl8218b_rtl8380_config(rtl8218b_phy_macid);
                }
                break;
            #endif
            #if (defined(CONFIG_RTL8218FB))
            case RTK_CHIP_RTL8218FB:
                {
                    Tuint8 rtl8218fb_phy_macid = pModel->phy.list[phy_idx].mac_id - pModel->phy.baseid;
                    rtl8218fb_rtl8380_config(rtl8218fb_phy_macid);
                }
                break;
            #endif
            #if (defined(CONFIG_RTL8214FC))
            case RTK_CHIP_RTL8214FC:
                {
                    Tuint8 rtl8214fc_phy_macid = pModel->phy.list[phy_idx].mac_id - pModel->phy.baseid;
                    rtl8214fc_rtl8380_config(rtl8214fc_phy_macid);
                }
                break;
            #endif
            #if (defined(CONFIG_RTL8208))
            case RTK_CHIP_RTL8208L:
                {
                    Tuint8 rtl8208_phy_macid = pModel->phy.list[phy_idx].mac_id - pModel->phy.baseid;
                    rtl8208_config(rtl8208_phy_macid);
                }
                break;
            #endif
            default:
                break;
        }
    }

    /*For LED on reason, not power on serdes-direct-fiber*/
    rtl8380_phyPowerOn_except_serdes_fiber();

    OSAL_MDELAY(500);

    rtl8380_phyPowerOff();


    return;
} /* end of rtl8380_phy_config_init */

/* Function Name:
 *      rtl8380_intraSerdes_config_init
 * Description:
 *      Intra Serdes Configuration code that connect with RTL8380
 * Input:
 *      pModel - pointer to switch model of platform
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */

static rtl8380_reg_value_t rtl8380_serdes_soft_reset_take[] = {\
  {0xbb000034, 0x0000003f},{0xbb00003c, 0x00000010},\
  {0xbb00e78c, 0x00007146},{0xbb00e98c, 0x00007146},{0xbb00eb8c, 0x00007146},{0xbb00ed8c, 0x00007146},\
  {0xbb00ef8c, 0x00007146},{0xbb00f18c, 0x00007146},\
};

static rtl8380_reg_value_t rtl8380_serdes_common_patch[] = {\
  {0xbb00f878, 0x0000071e},{0xbb00f978, 0x0000071e},{0xbb00e784, 0x00000F00},{0xbb00e984, 0x00000F00},\
  {0xbb00eb84, 0x00000F00},{0xbb00ed84, 0x00000F00},{0xbb00ef84, 0x00000F00},{0xbb00f184, 0x00000F00},\
  {0xbb00e788, 0x00007060},{0xbb00e988, 0x00007060},{0xbb00eb88, 0x00007060},{0xbb00ed88, 0x00007060},\
  {0xbb00ef88, 0x00007060},{0xbb00f188, 0x00007060},{0xbb00ef90, 0x0000074d},{0xbb00f190, 0x0000074d},\
};


/*******************************RTL8380 Version-A*********************************/
/*Serdes0,1*/
static rtl8380_reg_value_t rtl8380_serdes01_rsgmiip_6275a[] = {\
  {0xbb000ff8, 0xaaaaaaaf},{0xbb00f388, 0x000085fa},{0xbb00f488, 0x000085fa},{0xbb00f398, 0x000020d8},\
  {0xbb00f498, 0x000020d8},{0xbb00f3c4, 0x0000B7C9},{0xbb00f4ac, 0x00001482},{0xbb00f4a8, 0x000080c7},\
  {0xbb00f3c8, 0x0000ab8e},{0xbb00f3ac, 0x00001482},{0xbb00f380, 0x00004040},{0xbb00f380, 0x00004000},\
  {0xbb00f480, 0x00004040},{0xbb00f480, 0x00004000},{0xbb00f3a4, 0x00008e64},{0xbb00f3a4, 0x00008c64},\
  {0xbb00f4a4, 0x00008e64},{0xbb00f4a4, 0x00008c64},
 };
static rtl8380_reg_value_t rtl8380_serdes01_qsgmii_6275a[] = {\
  {0xbb000ff8, 0xaaaaaaaf},{0xbb00f388, 0x000085fa},{0xbb00f488, 0x000085fa},{0xbb00f398, 0x000020d8},\
  {0xbb00f498, 0x000020d8},{0xbb00f3c4, 0x0000B7C9},{0xbb00f4ac, 0x00001482},{0xbb00f4a8, 0x000080c7},\
  {0xbb00f3c8, 0x0000ab8e},{0xbb00f3ac, 0x00001482},{0xbb00f380, 0x00004040},{0xbb00f380, 0x00004000},\
  {0xbb00f480, 0x00004040},{0xbb00f480, 0x00004000},{0xbb00f3a4, 0x00008e64},{0xbb00f3a4, 0x00008c64},\
  {0xbb00f4a4, 0x00008e64},{0xbb00f4a4, 0x00008c64},
 };
static rtl8380_reg_value_t rtl8380_serdes01_xsmii_6275a[] = {\
  {0xbb000058, 0x00000003},{0xbb000ff8, 0xaaaaaaab},{0xbb00f3c4, 0x0000b7c9},{0xbb00f3c8, 0x0000838e},\
  {0xbb00f3cc, 0x00000a4b},{0xbb00f3d0, 0x00007211},{0xbb00f388, 0x000085fa},{0xbb00f38c, 0x00008c6f},\
  {0xbb00f390, 0x0000dccc},{0xbb00f394, 0x00000000},{0xbb00f398, 0x000020d8},{0xbb00f39c, 0x00000003},\
  {0xbb00f3a0, 0x000079aa},{0xbb00f3a4, 0x00008c64},{0xbb00f3a8, 0x000000c3},{0xbb00f3ac, 0x00001482},\
  {0xbb00f460, 0x000014aa},{0xbb00f464, 0x00004800},{0xbb00f3b8, 0x0000f002},{0xbb00f46c, 0x000004bf},\
  {0xbb00f4c4, 0x00004209},{0xbb00f4c8, 0x0000c1f5},{0xbb00f4cc, 0x00000000},{0xbb00f4d0, 0x00000000},\
  {0xbb00f488, 0x000085fa},{0xbb00f48c, 0x00000000},{0xbb00f490, 0x0000dccc},{0xbb00f494, 0x00000000},\
  {0xbb00f498, 0x000020d8},{0xbb00f49c, 0x00000003},{0xbb00f4a0, 0x000079aa},{0xbb00f4a4, 0x00008c64},\
  {0xbb00f4a8, 0x000000c3},{0xbb00f4ac, 0x00001482},{0xbb00f560, 0x000014aa},{0xbb00f564, 0x00004800},\
  {0xbb00f4b8, 0x0000f002},{0xbb00f56c, 0x000004bf},{0xbb00f3a4, 0x00008e64},{0xbb00f3a4, 0x00008c64},\
  {0xbb00f4a4, 0x00008e64},{0xbb00f4a4, 0x00008c64},{0xbb000058, 0x00000000},\
 };


/*Serdes2,3*/
static rtl8380_reg_value_t rtl8380_serdes23_rsgmiip_6275a[] = {\
  {0xbb00f588, 0x000085fa},{0xbb00f688, 0x000085fa},{0xbb00f788, 0x000085fa},{0xbb00f598, 0x000020d8},\
  {0xbb00f698, 0x000020d8},{0xbb00f5c4, 0x0000B7C9},{0xbb00f5c8, 0x0000ab8e},{0xbb00f5ac, 0x00001482},\
  {0xbb00f6ac, 0x00001482},{0xbb00f580, 0x00004040},{0xbb00f580, 0x00004000},{0xbb00f680, 0x00004040},\
  {0xbb00f680, 0x00004000},{0xbb00f5a4, 0x00008e64},{0xbb00f5a4, 0x00008c64},{0xbb00f6a4, 0x00008e64},\
  {0xbb00f6a4, 0x00008c64},
 };
static rtl8380_reg_value_t rtl8380_serdes23_qsgmii_6275a[] = {\
  {0xbb00f588, 0x000085fa},{0xbb00f688, 0x000085fa},{0xbb00f788, 0x000085fa},{0xbb00f598, 0x000020d8},\
  {0xbb00f698, 0x000020d8},{0xbb00f5c4, 0x0000B7C9},{0xbb00f5c8, 0x0000ab8e},{0xbb00f5ac, 0x00001482},\
  {0xbb00f6ac, 0x00001482},{0xbb00f580, 0x00004040},{0xbb00f580, 0x00004000},{0xbb00f680, 0x00004040},\
  {0xbb00f680, 0x00004000},{0xbb00f5a4, 0x00008e64},{0xbb00f5a4, 0x00008c64},{0xbb00f6a4, 0x00008e64},\
  {0xbb00f6a4, 0x00008c64},
 };
static rtl8380_reg_value_t rtl8380_serdes23_xsmii_6275a[] = {\
  {0xbb00f5c4, 0x0000B7C9},{0xbb00f5c8, 0x0000838e},{0xbb00f5cc, 0x00000a4b},{0xbb00f5d0, 0x00007211},\
  {0xbb00f588, 0x000085fa},{0xbb00f58c, 0x00008c6f},{0xbb00f590, 0x0000dccc},{0xbb00f594, 0x00000000},\
  {0xbb00f598, 0x000020d8},{0xbb00f59c, 0x00000003},{0xbb00f5a0, 0x000079aa},{0xbb00f5a4, 0x00008c64},\
  {0xbb00f5a8, 0x000000c3},{0xbb00f5ac, 0x00001482},{0xbb00f660, 0x000014aa},{0xbb00f664, 0x00004800},\
  {0xbb00f5b8, 0x0000f002},{0xbb00f66c, 0x000004bf},{0xbb00f6c4, 0x00004209},{0xbb00f6c8, 0x0000c1f5},\
  {0xbb00f6cc, 0x00000000},{0xbb00f6d0, 0x00000000},{0xbb00f688, 0x000085fa},{0xbb00f68c, 0x00000000},\
  {0xbb00f690, 0x0000dccc},{0xbb00f694, 0x00000000},{0xbb00f698, 0x000020d8},{0xbb00f69c, 0x00000003},\
  {0xbb00f6a0, 0x000079aa},{0xbb00f6a4, 0x00008c64},{0xbb00f6a8, 0x000000c3},{0xbb00f6ac, 0x00001482},\
  {0xbb00f760, 0x000014aa},{0xbb00f764, 0x00004800},{0xbb00f6b8, 0x0000f002},{0xbb00f76c, 0x000004bf},\
  {0xbb00f5a4, 0x00008e64},{0xbb00f5a4, 0x00008c64},{0xbb00f6a4, 0x00008e64},{0xbb00f6a4, 0x00008c64},\
 };


/*serdes4*/
static rtl8380_reg_value_t rtl8380_serdes4_rsgmii_6275a[] = {\
  {0xbb00f7c4, 0x0000B7C9},{0xbb00f7c8, 0x00001e5e},{0xbb00f7cc, 0x00000a4b},{0xbb00f7d0, 0x00007211},\
  {0xbb00f788, 0x000085fa},{0xbb00f78c, 0x00008c6f},{0xbb00f790, 0x0000dccc},{0xbb00f794, 0x00000000},\
  {0xbb00f798, 0x000020d8},{0xbb00f79c, 0x00000003},{0xbb00f7a0, 0x000079aa},{0xbb00f7a4, 0x00008c64},\
  {0xbb00f7a8, 0x000000c3},{0xbb00f7ac, 0x00001482},{0xbb00f860, 0x000014aa},{0xbb00f864, 0x00004800},\
  {0xbb00f7b8, 0x0000f002},{0xbb00f86c, 0x000004bf},{0xbb00f7a4, 0x00008e64},{0xbb00f7a4, 0x00008c64},\
 };
static rtl8380_reg_value_t rtl8380_serdes4_qsgmii_6275a[] = {\
    {0xbb00f798, 0x000020d8},{0xbb00f7a8, 0x000058c7},{0xbb00f7c4, 0x0000B7C9},{0xbb00f7c8, 0x0000ab8e},\
    {0xbb00f7ac, 0x00001482},{0xbb00f780, 0x00004040},{0xbb00f780, 0x00004000},{0xbb00f7a4, 0x00008e64},\
    {0xbb00f7a4, 0x00008c64},
};
static rtl8380_reg_value_t rtl8380_serdes4_rsgmiip_6275a[] = {\
    {0xbb00f798, 0x000020d8},{0xbb00f7a8, 0x000058c7},{0xbb00f7c4, 0x0000B7C9},{0xbb00f7c8, 0x0000ab8e},\
    {0xbb00f7ac, 0x00001482},{0xbb00f780, 0x00004040},{0xbb00f780, 0x00004000},{0xbb00f7a4, 0x00008e64},\
    {0xbb00f7a4, 0x00008c64},
};
static rtl8380_reg_value_t rtl8380_serdes4_fiber_6275a[] = {\
    {0xbb00f798, 0x000020d8},{0xbb00f7a8, 0x000058c7},{0xbb00f7c4, 0x0000B7C9},{0xbb00f7c8, 0x0000ab8e},\
    {0xbb00f7ac, 0x00001482},{0xbb00f780, 0x00004040},{0xbb00f780, 0x00004000},{0xbb00f7a4, 0x00008e64},\
    {0xbb00f7a4, 0x00008c64},{0xbb00f878, 0x0000071e},{0xbb00ef90, 0x0000074d},
};
static rtl8380_reg_value_t rtl8380_serdes4_nophy_6275a[] = {\
    {0xbb00f798, 0x000020d8},{0xbb00f7a8, 0x000058c7},{0xbb00f7c4, 0x0000B7C9},{0xbb00f7c8, 0x0000ab8e},\
    {0xbb00f7ac, 0x00001482},{0xbb00f780, 0x00004040},{0xbb00f780, 0x00004000},{0xbb00f7a4, 0x00008e64},\
    {0xbb00f7a4, 0x00008c64},{0xbb00f878, 0x0000071e},{0xbb00ef90, 0x0000074d},
};


/*serdes5*/
static rtl8380_reg_value_t rtl8380_serdes5_rsgmii_6275a[] = {\
  {0xbb00f8c4, 0x00004209},{0xbb00f8c8, 0x0000c1f5},{0xbb00f8cc, 0x00000000},{0xbb00f8d0, 0x00000000},\
  {0xbb00f888, 0x000085fa},{0xbb00f88c, 0x00000000},{0xbb00f890, 0x0000dccc},{0xbb00f894, 0x00000000},\
  {0xbb00f898, 0x000020d8},{0xbb00f89c, 0x00000003},{0xbb00f8a0, 0x000079aa},{0xbb00f8a4, 0x00008c64},\
  {0xbb00f8a8, 0x000000c3},{0xbb00f8ac, 0x00001482},{0xbb00f960, 0x000014aa},{0xbb00f964, 0x00004800},\
  {0xbb00f8b8, 0x0000f002},{0xbb00f96c, 0x000004bf},{0xbb00f8a4, 0x00008e64},{0xbb00f8a4, 0x00008c64},\
};
static rtl8380_reg_value_t rtl8380_serdes5_fiber_6275a[] = {\
  {0xbb00f8c4, 0x00004209},{0xbb00f8c8, 0x0000c1f5},{0xbb00f8cc, 0x00000000},{0xbb00f8d0, 0x00000000},\
  {0xbb00f888, 0x000088ff},{0xbb00f88c, 0x00000000},{0xbb00f890, 0x0000dccc},{0xbb00f894, 0x00000000},\
  {0xbb00f898, 0x0000861b},{0xbb00f89c, 0x00000003},{0xbb00f8a0, 0x000079aa},{0xbb00f8a4, 0x00008822},\
  {0xbb00f8a8, 0x000000c3},{0xbb00f8ac, 0x00001482},{0xbb00f960, 0x000014aa},{0xbb00f964, 0x00000300},\
  {0xbb00f8b8, 0x0000f002},{0xbb00f96c, 0x000004bf},{0xbb00f8a4, 0x00008a22},{0xbb00f8a4, 0x00008822},\
  {0xbb00f978, 0x0000071e},{0xbb00f190, 0x0000074d},
};
static rtl8380_reg_value_t rtl8380_serdes5_nophy_6275a[] = {\
  {0xbb00f8c4, 0x00004209},{0xbb00f8c8, 0x0000c1f5},{0xbb00f8cc, 0x00000000},{0xbb00f8d0, 0x00000000},\
  {0xbb00f888, 0x000088ff},{0xbb00f88c, 0x00000000},{0xbb00f890, 0x0000dccc},{0xbb00f894, 0x00000000},\
  {0xbb00f898, 0x0000861b},{0xbb00f89c, 0x00000003},{0xbb00f8a0, 0x000079aa},{0xbb00f8a4, 0x00008822},\
  {0xbb00f8a8, 0x000000c3},{0xbb00f8ac, 0x00001482},{0xbb00f960, 0x000014aa},{0xbb00f964, 0x00000300},\
  {0xbb00f8b8, 0x0000f002},{0xbb00f96c, 0x000004bf},{0xbb00f8a4, 0x00008a22},{0xbb00f8a4, 0x00008822},\
  {0xbb00f978, 0x0000071e},{0xbb00f190, 0x0000074d},
};



/*******************************RTL8380 Version-B*********************************/
/*Serdes0,1*/
static rtl8380_reg_value_t rtl8380_serdes01_rsgmiip_6275b[] = {\
  {0xbb000ff8, 0xaaaaaaaf},{0xbb00f38c, 0x0000f46f},\
  {0xbb00f388, 0x000085fa},{0xbb00f488, 0x000085fa},{0xbb00f398, 0x000020d8},{0xbb00f498, 0x000020d8},\
  {0xbb00f3c4, 0x0000B7C9},{0xbb00f4ac, 0x00000482},{0xbb00f4a8, 0x000080c7},{0xbb00f3c8, 0x0000ab8e},\
  {0xbb00f3ac, 0x00000482},{0xbb00f3cc, 0x000024ab},{0xbb00f4c4, 0x00004208},{0xbb00f4c8, 0x0000c208},\
  {0xbb00f464, 0x00000303},{0xbb00f564, 0x00000303},{0xbb00f3b8, 0x0000FCC2},{0xbb00f4b8, 0x0000FCC2},\
  {0xbb00f3a4, 0x00008e64},{0xbb00f3a4, 0x00008c64},{0xbb00f4a4, 0x00008e64},{0xbb00f4a4, 0x00008c64},\
 };

static rtl8380_reg_value_t rtl8380_serdes01_qsgmii_6275b[] = {\
  {0xbb000ff8, 0xaaaaaaaf},{0xbb00f38c, 0x0000f46f},\
  {0xbb00f388, 0x000085fa},{0xbb00f488, 0x000085fa},{0xbb00f398, 0x000020d8},{0xbb00f498, 0x000020d8},\
  {0xbb00f3c4, 0x0000B7C9},{0xbb00f4ac, 0x00000482},{0xbb00f4a8, 0x000080c7},{0xbb00f3c8, 0x0000ab8e},\
  {0xbb00f3ac, 0x00000482},{0xbb00f3cc, 0x000024ab},{0xbb00f4c4, 0x00004208},{0xbb00f4c8, 0x0000c208},\
  {0xbb00f464, 0x00000303},{0xbb00f564, 0x00000303},{0xbb00f3b8, 0x0000FCC2},{0xbb00f4b8, 0x0000FCC2},\
  {0xbb00f3a4, 0x00008e64},{0xbb00f3a4, 0x00008c64},{0xbb00f4a4, 0x00008e64},{0xbb00f4a4, 0x00008c64},\
  };

static rtl8380_reg_value_t rtl8380_serdes01_xsmii_6275b[] = {\
  {0xbb000ff8, 0xaaaaaaab},{0xbb00f3c4, 0x000003c7},{0xbb00f388, 0x000085fa},{0xbb00f38c, 0x00008c6f},\
  {0xbb00f390, 0x0000dccc},{0xbb00f394, 0x00000000},{0xbb00f398, 0x00003600},{0xbb00f39c, 0x00000003},\
  {0xbb00f3a0, 0x000079aa},{0xbb00f3a4, 0x00008c64},{0xbb00f3a8, 0x000000c3},{0xbb00f3ac, 0x00001482},\
  {0xbb00f460, 0x000014aa},{0xbb00f464, 0x00004800},{0xbb00f3b8, 0x0000f002},{0xbb00f46c, 0x000004bf},\
  {0xbb00f488, 0x000085fa},{0xbb00f48c, 0x00000000},{0xbb00f490, 0x0000dccc},{0xbb00f494, 0x00000000},\
  {0xbb00f498, 0x00003600},{0xbb00f49c, 0x00000003},{0xbb00f4a0, 0x000079aa},{0xbb00f4a4, 0x00008c64},\
  {0xbb00f4a8, 0x000000c3},{0xbb00f4ac, 0x00001482},{0xbb00f560, 0x000014aa},{0xbb00f564, 0x00004800},\
  {0xbb00f4b8, 0x0000f002},{0xbb00f56c, 0x000004bf},{0xbb00f3a4, 0x00008e64},{0xbb00f3a4, 0x00008c64},\
  {0xbb00f4a4, 0x00008e64},{0xbb00f4a4, 0x00008c64},
 };


/*Serdes2,3*/
static rtl8380_reg_value_t rtl8380_serdes23_rsgmiip_6275b[] = {\
  {0xbb00f58c, 0x0000f46d},\
  {0xbb00f588, 0x000085fa},{0xbb00f688, 0x000085fa},{0xbb00f788, 0x000085fa},{0xbb00f598, 0x000020d8},\
  {0xbb00f698, 0x000020d8},{0xbb00f5c4, 0x0000B7C9},{0xbb00f5c8, 0x0000ab8e},{0xbb00f5ac, 0x00000482},\
  {0xbb00f6ac, 0x00000482},{0xbb00f5cc, 0x000024ab},{0xbb00f6c4, 0x00004208},{0xbb00f6c8, 0x0000c208},\
  {0xbb00f664, 0x00000303},{0xbb00f764, 0x00000303},{0xbb00f5b8, 0x0000FCC2},{0xbb00f6b8, 0x0000FCC2},\
  {0xbb00f5a4, 0x00008e64},{0xbb00f5a4, 0x00008c64},{0xbb00f6a4, 0x00008e64},{0xbb00f6a4, 0x00008c64},\
 };

static rtl8380_reg_value_t rtl8380_serdes23_qsgmii_6275b[] = {\
  {0xbb00f58c, 0x0000f46d},\
  {0xbb00f588, 0x000085fa},{0xbb00f688, 0x000085fa},{0xbb00f788, 0x000085fa},{0xbb00f598, 0x000020d8},\
  {0xbb00f698, 0x000020d8},{0xbb00f5c4, 0x0000B7C9},{0xbb00f5c8, 0x0000ab8e},{0xbb00f5ac, 0x00000482},\
  {0xbb00f6ac, 0x00000482},{0xbb00f5cc, 0x000024ab},{0xbb00f6c4, 0x00004208},{0xbb00f6c8, 0x0000c208},\
  {0xbb00f664, 0x00000303},{0xbb00f764, 0x00000303},{0xbb00f5b8, 0x0000FCC2},{0xbb00f6b8, 0x0000FCC2},\
  {0xbb00f5a4, 0x00008e64},{0xbb00f5a4, 0x00008c64},{0xbb00f6a4, 0x00008e64},{0xbb00f6a4, 0x00008c64},\
 };

static rtl8380_reg_value_t rtl8380_serdes23_xsmii_6275b[] = {\
  {0xbb00f5c4, 0x000003c7},{0xbb00f588, 0x000085fa},{0xbb00f58c, 0x00008c6f},{0xbb00f590, 0x0000dccc},\
  {0xbb00f594, 0x00000000},{0xbb00f598, 0x00003600},{0xbb00f59c, 0x00000003},{0xbb00f5a0, 0x000079aa},\
  {0xbb00f5a4, 0x00008c64},{0xbb00f5a8, 0x000000c3},{0xbb00f5ac, 0x00001482},{0xbb00f660, 0x000014aa},\
  {0xbb00f664, 0x00004800},{0xbb00f5b8, 0x0000f002},{0xbb00f66c, 0x000004bf},{0xbb00f688, 0x000085fa},\
  {0xbb00f68c, 0x00000000},{0xbb00f690, 0x0000dccc},{0xbb00f694, 0x00000000},{0xbb00f698, 0x00003600},\
  {0xbb00f69c, 0x00000003},{0xbb00f6a0, 0x000079aa},{0xbb00f6a4, 0x00008c64},{0xbb00f6a8, 0x000000c3},\
  {0xbb00f6ac, 0x00001482},{0xbb00f760, 0x000014aa},{0xbb00f764, 0x00004800},{0xbb00f6b8, 0x0000f002},\
  {0xbb00f76c, 0x000004bf},{0xbb00f5a4, 0x00008e64},{0xbb00f5a4, 0x00008c64},{0xbb00f6a4, 0x00008e64},\
  {0xbb00f6a4, 0x00008c64},
 };

/*serdes4*/
/*2.5G*/
static rtl8380_reg_value_t rtl8380_serdes4_rsgmii_6275b[] = {\
  {0xbb00f7c4, 0x0000086f},{0xbb00f788, 0x000085fa},{0xbb00f78c, 0x00008c6f},{0xbb00f790, 0x0000dccc},\
  {0xbb00f794, 0x00000000},{0xbb00f798, 0x00003600},{0xbb00f79c, 0x00000003},{0xbb00f7a0, 0x000079aa},\
  {0xbb00f7a4, 0x00008c64},{0xbb00f7a8, 0x000000c3},{0xbb00f7ac, 0x00001482},{0xbb00f860, 0x000014aa},\
  {0xbb00f864, 0x00004800},{0xbb00f7b8, 0x0000f002},{0xbb00f86c, 0x000004bf},{0xbb00f7a4, 0x00008e64},\
  {0xbb00f7a4, 0x00008c64},
 };

/*1.25G*/
static rtl8380_reg_value_t rtl8380_serdes4_sgmii_6275b[] = {\
  {0xbb00f7c4, 0x0000086f},{0xbb00f788, 0x000085fa},{0xbb00f78c, 0x00008c6f},{0xbb00f790, 0x0000dccc},\
  {0xbb00f794, 0x00000000},{0xbb00f798, 0x000020d8},{0xbb00f79c, 0x00000003},{0xbb00f7a0, 0x000079aa},\
  {0xbb00f7a4, 0x00008c64},{0xbb00f7a8, 0x000000c3},{0xbb00f7ac, 0x00001482},{0xbb00f860, 0x000014aa},\
  {0xbb00f864, 0x00004800},{0xbb00f7b8, 0x0000f002},{0xbb00f86c, 0x000004bf},{0xbb00f7a4, 0x00008e64},\
  {0xbb00f7a4, 0x00008c64},
 };


/*5G*/
static rtl8380_reg_value_t rtl8380_serdes4_qsgmii_6275b[] = {\
  {0xbb00f78c, 0x0000f46d},{0xbb00f788, 0x000085fa},{0xbb00f7ac, 0x00000482},{0xbb00f798, 0x000020d8},\
  {0xbb00f7a8, 0x000058c7},{0xbb00f7c4, 0x0000B7C9},{0xbb00f7c8, 0x0000ab8e},{0xbb00f864, 0x00000303},\
  {0xbb00f7b8, 0x0000FCC2},{0xbb00f7a4, 0x00008e64},{0xbb00f7a4, 0x00008c64},\
};

/*5G*/
static rtl8380_reg_value_t rtl8380_serdes4_rsgmiip_6275b[] = {\
  {0xbb00f78c, 0x0000f46d},{0xbb00f788, 0x000085fa},{0xbb00f7ac, 0x00000482},{0xbb00f798, 0x000020d8},\
  {0xbb00f7a8, 0x000058c7},{0xbb00f7c4, 0x0000B7C9},{0xbb00f7c8, 0x0000ab8e},{0xbb00f864, 0x00000303},\
  {0xbb00f7b8, 0x0000FCC2},{0xbb00f7a4, 0x00008e64},{0xbb00f7a4, 0x00008c64},\
};


/*1.25G*/
static rtl8380_reg_value_t rtl8380_serdes4_fiber_6275b[] = {\
  {0xbb00f788, 0x000085fa},{0xbb00f7ac, 0x00001482},{0xbb00f798, 0x000020d8},{0xbb00f7a8, 0x000000c3},\
  {0xbb00f7c4, 0x0000B7C9},{0xbb00f7c8, 0x0000ab8e},{0xbb00f864, 0x00000303},{0xbb00f7b8, 0x0000FCC2},\
  {0xbb00f7a4, 0x00008e64},{0xbb00f7a4, 0x00008c64},\
};

static rtl8380_reg_value_t rtl8380_serdes4_nophy_6275b[] = {\
  {0xbb00f788, 0x000085fa},{0xbb00f7ac, 0x00001482},{0xbb00f798, 0x000020d8},{0xbb00f7a8, 0x000000c3},\
  {0xbb00f7c4, 0x0000B7C9},{0xbb00f7c8, 0x0000ab8e},{0xbb00f864, 0x00000303},{0xbb00f7b8, 0x0000FCC2},\
  {0xbb00f7a4, 0x00008e64},{0xbb00f7a4, 0x00008c64},\
};

/*serdes5*/
/*2.5G*/
static rtl8380_reg_value_t rtl8380_serdes5_rsgmii_6275b[] = {\
  {0xbb00f888, 0x000085fa},{0xbb00f88c, 0x00000000},{0xbb00f890, 0x0000dccc},{0xbb00f894, 0x00000000},\
  {0xbb00f898, 0x00003600},{0xbb00f89c, 0x00000003},{0xbb00f8a0, 0x000079aa},{0xbb00f8a4, 0x00008c64},\
  {0xbb00f8a8, 0x000000c3},{0xbb00f8ac, 0x00001482},{0xbb00f960, 0x000014aa},{0xbb00f964, 0x00004800},\
  {0xbb00f8b8, 0x0000f002},{0xbb00f96c, 0x000004bf},{0xbb00f8a4, 0x00008e64},{0xbb00f8a4, 0x00008c64},\
};

/*1.25G*/
static rtl8380_reg_value_t rtl8380_serdes5_sgmii_6275b[] = {\
  {0xbb00f888, 0x000085fa},{0xbb00f88c, 0x00000000},{0xbb00f890, 0x0000dccc},{0xbb00f894, 0x00000000},\
  {0xbb00f898, 0x000020d8},{0xbb00f89c, 0x00000003},{0xbb00f8a0, 0x000079aa},{0xbb00f8a4, 0x00008c64},\
  {0xbb00f8a8, 0x000000c3},{0xbb00f8ac, 0x00001482},{0xbb00f960, 0x000014aa},{0xbb00f964, 0x00004800},\
  {0xbb00f8b8, 0x0000f002},{0xbb00f96c, 0x000004bf},{0xbb00f8a4, 0x00008e64},{0xbb00f8a4, 0x00008c64},\
};

/*1.25G*/
static rtl8380_reg_value_t rtl8380_serdes5_fiber_6275b[] = {\
  {0xbb00f888, 0x000085fa},{0xbb00f88c, 0x00000000},{0xbb00f890, 0x0000dccc},{0xbb00f894, 0x00000000},\
  {0xbb00f898, 0x00003600},{0xbb00f89c, 0x00000003},{0xbb00f8a0, 0x000079aa},{0xbb00f8a4, 0x00008c64},\
  {0xbb00f8a8, 0x000000c3},{0xbb00f8ac, 0x00001482},{0xbb00f960, 0x000014aa},{0xbb00f964, 0x00000303},\
  {0xbb00f8b8, 0x0000f002},{0xbb00f96c, 0x000004bf},{0xbb00f8a4, 0x00008e64},{0xbb00f8a4, 0x00008c64},\
};

/*1.25G*/
static rtl8380_reg_value_t rtl8380_serdes5_nophy_6275b[] = {\
  {0xbb00f888, 0x000085fa},{0xbb00f88c, 0x00000000},{0xbb00f890, 0x0000dccc},{0xbb00f894, 0x00000000},\
  {0xbb00f898, 0x00003600},{0xbb00f89c, 0x00000003},{0xbb00f8a0, 0x000079aa},{0xbb00f8a4, 0x00008c64},\
  {0xbb00f8a8, 0x000000c3},{0xbb00f8ac, 0x00001482},{0xbb00f960, 0x000014aa},{0xbb00f964, 0x00000303},\
  {0xbb00f8b8, 0x0000f002},{0xbb00f96c, 0x000004bf},{0xbb00f8a4, 0x00008e64},{0xbb00f8a4, 0x00008c64},\
};



static rtl8380_reg_value_t rtl8380_serdes_reset[] = {\
  {0xbb00e780, 0x00000c00},{0xbb00e980, 0x00000c00},{0xbb00eb80, 0x00000c00},{0xbb00ed80, 0x00000c00},\
  {0xbb00ef80, 0x00000c00},{0xbb00f180, 0x00000c00},{0xbb00e780, 0x00000c03},{0xbb00e980, 0x00000c03},\
  {0xbb00eb80, 0x00000c03},{0xbb00ed80, 0x00000c03},{0xbb00ef80, 0x00000c03},{0xbb00f180, 0x00000c03},\
};

static rtl8380_reg_value_t rtl8380_serdes_soft_reset_release[] = {\
  {0xbb00e78c, 0x00007106},{0xbb00e98c, 0x00007106},{0xbb00eb8c, 0x00007106},{0xbb00ed8c, 0x00007106},\
  {0xbb00ef8c, 0x00007106},{0xbb00f18c, 0x00007106},\
};


static void rtl8380_intraSerdes_config_init(const rtk_switch_model_t *pModel)
{
    uint32 i;
    uint32 value;
    uint32 value_temp;

    uint32 sds_power_down_value;


    /* Back up serdes power down value */
     sds_power_down_value = MEM32_READ(0xbb000034);

    /*serdes software reset take own ship*/
    for (i=0; i<(sizeof(rtl8380_serdes_soft_reset_take)/sizeof(rtl8380_reg_value_t)); i++)
    {
        MEM32_WRITE(rtl8380_serdes_soft_reset_take[i].reg,  rtl8380_serdes_soft_reset_take[i].val);
        OSAL_MDELAY(1);
    }

    /*Serdes Common Patch*/
    for (i=0; i<(sizeof(rtl8380_serdes_common_patch)/sizeof(rtl8380_reg_value_t)); i++)
    {
        MEM32_WRITE(rtl8380_serdes_common_patch[i].reg,  rtl8380_serdes_common_patch[i].val);
        OSAL_MDELAY(1);
    }

     /* Enable Internal Read/Write */
     MEM32_WRITE(0xbb000058, 0x00000003);

    /******************************MAC Serdes Interface Settings***************************/
    /*Set Serdes4 MAC Interface register*/
     value = MEM32_READ(0xbb00005c);
     value &= (~0x7);
    if(pModel->serdes.list[4].mii == RTK_MII_1000BX_FIBER)
    {
         value |= 0x1;
    }
    else if(pModel->serdes.list[4].mii == RTK_MII_100BX_FIBER)
    {
         value |= 0x1;
    }
    else if(pModel->serdes.list[4].mii == RTK_MII_SGMII)
    {
         value |= 0x2;
    }
    else if(pModel->serdes.list[4].mii == RTK_MII_RSGMII)
    {
         value |= 0x3;
    }
    else if(pModel->serdes.list[4].mii == RTK_MII_HISGMII)
    {
         value |= 0x4;
    }
    else if(pModel->serdes.list[4].mii == RTK_MII_RSGMII_PLUS)
    {
         value |= 0x5;
    }
    else if(pModel->serdes.list[4].mii == RTK_MII_QSGMII)
    {
         value |= 0x5;
    }
    else
    {
        /*Do nothing, means disable Serdes4*/
    }
    MEM32_WRITE(0xbb00005c, value);


    /*Set Serdes5 MAC Interface register*/
     value = MEM32_READ(0xbb00005c);
     value &= (~0x38);
    if(pModel->serdes.list[5].mii == RTK_MII_1000BX_FIBER)
    {
         value |= 0x1<<3;
    }
    else if(pModel->serdes.list[5].mii == RTK_MII_100BX_FIBER)
    {
         value |= 0x1<<3;
    }
    else if(pModel->serdes.list[5].mii == RTK_MII_SGMII)
    {
         value |= 0x2<<3;
    }
    else if(pModel->serdes.list[5].mii == RTK_MII_RSGMII)
    {
         value |= 0x3<<3;
    }
    else if(pModel->serdes.list[5].mii == RTK_MII_HISGMII)
    {
         value |= 0x4<<3;
    }
    else
    {
        /*Do nothing, means disable Serdes5*/
    }
    MEM32_WRITE(0xbb00005c, value);

    /****************************** Serdes Module Settings***************************/
    /*Set Serdes0-Serdes5 Module Mode*/
    /*Serdes0-Serdes3*/
    value  = 0;
    for(i = 0; i < 4; i++)
    {
        if(pModel->serdes.list[i].mii == RTK_MII_RSGMII_PLUS)
        {
             value |= 0x0<<(25-5*i);
        }
        else if(pModel->serdes.list[i].mii == RTK_MII_QSGMII)
        {
             value |= 0x6<<(25-5*i);
        }
        else if(pModel->serdes.list[i].mii == RTK_MII_XSMII)
        {
             value |= 0x9<<(25-5*i);
        }
        else
        {
            /*Do nothing, means disable this Serdes*/
        }
    }

    /*Serdes4*/
    if(pModel->serdes.list[4].mii == RTK_MII_RSGMII_PLUS)
    {
         value |= 0x0<<5;
    }
    else if(pModel->serdes.list[4].mii == RTK_MII_RSGMII)
    {
         value |= 0x1<<5;
    }
    else if(pModel->serdes.list[4].mii == RTK_MII_SGMII)
    {
         value |= 0x2<<5;
    }
    else if(pModel->serdes.list[4].mii == RTK_MII_1000BX_FIBER)
    {
         value |= 0x4<<5;
    }
    else if(pModel->serdes.list[4].mii == RTK_MII_100BX_FIBER)
    {
         value |= 0x5<<5;
    }
    else if(pModel->serdes.list[4].mii == RTK_MII_QSGMII)
    {
         value |= 0x6<<5;
    }
    else if(pModel->serdes.list[4].mii == RTK_MII_HISGMII)
    {
         value |= 0x12<<5;
    }
    else
    {
        /*Do nothing, means disable this Serdes*/
    }

    /*Serdes5*/
    if(pModel->serdes.list[5].mii == RTK_MII_RSGMII)
    {
         value |= 0x1<<0;
    }
    else if(pModel->serdes.list[5].mii == RTK_MII_SGMII)
    {
         value |= 0x2<<0;
    }
    else if(pModel->serdes.list[5].mii == RTK_MII_1000BX_FIBER)
    {
         value |= 0x4<<0;
    }
    else if(pModel->serdes.list[5].mii == RTK_MII_100BX_FIBER)
    {
         value |= 0x5<<0;
    }
    else if(pModel->serdes.list[5].mii == RTK_MII_HISGMII)
    {
         value |= 0x12<<0;
    }
    else
    {
        /*Do nothing, means disable this Serdes*/
    }
    MEM32_WRITE(0xbb000028, value);


    if(RTL8380_VERSION_A == rtl8380_version)
    {
        /* Serdes0, 1*/
        if(pModel->serdes.list[0].mii == RTK_MII_RSGMII_PLUS)
        {
            /*Save 0xbb000FF8 bit4-bit32 from SOC*/
            value = MEM32_READ(0xbb000FF8);
            value &= 0xFFFFFFF0;

            /*Save 0xbb000FF8 bit0-bit3 from Array*/
            value_temp = rtl8380_serdes01_rsgmiip_6275a[0].val;
            value_temp &= 0x0000000F;

            /*Reconstruct data and write back into array*/
            value |= value_temp;
            rtl8380_serdes01_rsgmiip_6275a[0].val = value;

            for (i=0; i<(sizeof(rtl8380_serdes01_rsgmiip_6275a)/sizeof(rtl8380_reg_value_t)); i++)
            {
                MEM32_WRITE(rtl8380_serdes01_rsgmiip_6275a[i].reg,  rtl8380_serdes01_rsgmiip_6275a[i].val);
                OSAL_MDELAY(1);
            }
        }
        else if(pModel->serdes.list[0].mii == RTK_MII_QSGMII)
        {

            /*Save 0xbb000FF8 bit4-bit32 from SOC*/
            value = MEM32_READ(0xbb000FF8);
            value &= 0xFFFFFFF0;

            /*Save 0xbb000FF8 bit0-bit3 from Array*/
            value_temp = rtl8380_serdes01_qsgmii_6275a[0].val;
            value_temp &= 0x0000000F;

            /*Reconstruct data and write back into array*/
            value |= value_temp;
            rtl8380_serdes01_qsgmii_6275a[0].val = value;

            for (i=0; i<(sizeof(rtl8380_serdes01_qsgmii_6275a)/sizeof(rtl8380_reg_value_t)); i++)
            {
                MEM32_WRITE(rtl8380_serdes01_qsgmii_6275a[i].reg,  rtl8380_serdes01_qsgmii_6275a[i].val);
                OSAL_MDELAY(1);
            }
        }
        else if(pModel->serdes.list[0].mii == RTK_MII_XSMII)
        {

            /*Save 0xbb000FF8 bit4-bit32 from SOC*/
            value = MEM32_READ(0xbb000FF8);
            value &= 0xFFFFFFF0;

            /*Save 0xbb000FF8 bit0-bit3 from Array*/
            value_temp = rtl8380_serdes01_xsmii_6275a[1].val;
            value_temp &= 0x0000000F;

            /*Reconstruct data and write back into array*/
            value |= value_temp;
            rtl8380_serdes01_xsmii_6275a[1].val = value;


            for (i=0; i<(sizeof(rtl8380_serdes01_xsmii_6275a)/sizeof(rtl8380_reg_value_t)); i++)
            {
                MEM32_WRITE(rtl8380_serdes01_xsmii_6275a[i].reg,  rtl8380_serdes01_xsmii_6275a[i].val);
                OSAL_MDELAY(1);
            }
        }
        else
        {
            /*Do nothing*/
        }

        /* Serdes2,3*/
        if(pModel->serdes.list[2].mii == RTK_MII_RSGMII_PLUS)
        {
            for (i=0; i<(sizeof(rtl8380_serdes23_rsgmiip_6275a)/sizeof(rtl8380_reg_value_t)); i++)
            {
                MEM32_WRITE(rtl8380_serdes23_rsgmiip_6275a[i].reg,  rtl8380_serdes23_rsgmiip_6275a[i].val);
                OSAL_MDELAY(1);
            }
        }
        else if(pModel->serdes.list[2].mii == RTK_MII_QSGMII)
        {
            for (i=0; i<(sizeof(rtl8380_serdes23_qsgmii_6275a)/sizeof(rtl8380_reg_value_t)); i++)
            {
                MEM32_WRITE(rtl8380_serdes23_qsgmii_6275a[i].reg,  rtl8380_serdes23_qsgmii_6275a[i].val);
                OSAL_MDELAY(1);
            }
        }
        else if(pModel->serdes.list[2].mii == RTK_MII_XSMII)
        {
            for (i=0; i<(sizeof(rtl8380_serdes23_xsmii_6275a)/sizeof(rtl8380_reg_value_t)); i++)
            {
                MEM32_WRITE(rtl8380_serdes23_xsmii_6275a[i].reg,  rtl8380_serdes23_xsmii_6275a[i].val);
                OSAL_MDELAY(1);
            }
        }
        else
        {
            /*Do nothing*/
        }

        /* Serdes4*/
        if(pModel->serdes.list[4].mii == RTK_MII_RSGMII_PLUS)
        {
            for (i=0; i<(sizeof(rtl8380_serdes4_rsgmiip_6275a)/sizeof(rtl8380_reg_value_t)); i++)
            {
                MEM32_WRITE(rtl8380_serdes4_rsgmiip_6275a[i].reg,  rtl8380_serdes4_rsgmiip_6275a[i].val);
                OSAL_MDELAY(1);
            }
        }
        else if(pModel->serdes.list[4].mii == RTK_MII_RSGMII)
        {
            for (i=0; i<(sizeof(rtl8380_serdes4_rsgmii_6275a)/sizeof(rtl8380_reg_value_t)); i++)
            {
                MEM32_WRITE(rtl8380_serdes4_rsgmii_6275a[i].reg,  rtl8380_serdes4_rsgmii_6275a[i].val);
                OSAL_MDELAY(1);
            }
        }
        else if(pModel->serdes.list[4].mii == RTK_MII_SGMII)
        {

        }
        else if(pModel->serdes.list[4].mii == RTK_MII_1000BX_FIBER)
        {
            for (i=0; i<(sizeof(rtl8380_serdes4_fiber_6275a)/sizeof(rtl8380_reg_value_t)); i++)
            {
                MEM32_WRITE(rtl8380_serdes4_fiber_6275a[i].reg,  rtl8380_serdes4_fiber_6275a[i].val);
                OSAL_MDELAY(1);
            }
        }
        else if(pModel->serdes.list[4].mii == RTK_MII_100BX_FIBER)
        {
            for (i=0; i<(sizeof(rtl8380_serdes4_fiber_6275a)/sizeof(rtl8380_reg_value_t)); i++)
            {
                MEM32_WRITE(rtl8380_serdes4_fiber_6275a[i].reg,  rtl8380_serdes4_fiber_6275a[i].val);
                OSAL_MDELAY(1);
            }
        }
        else if(pModel->serdes.list[4].mii == RTK_MII_QSGMII)
        {
            for (i=0; i<(sizeof(rtl8380_serdes4_qsgmii_6275a)/sizeof(rtl8380_reg_value_t)); i++)
            {
                MEM32_WRITE(rtl8380_serdes4_qsgmii_6275a[i].reg,  rtl8380_serdes4_qsgmii_6275a[i].val);
                OSAL_MDELAY(1);
            }
        }
        else if(pModel->serdes.list[4].mii == RTK_MII_HISGMII)
        {
        }
        else
        {
            for (i=0; i<(sizeof(rtl8380_serdes4_nophy_6275a)/sizeof(rtl8380_reg_value_t)); i++)
            {
                MEM32_WRITE(rtl8380_serdes4_nophy_6275a[i].reg,  rtl8380_serdes4_nophy_6275a[i].val);
                OSAL_MDELAY(1);
            }
        }

        /*Serdes5*/
        if(pModel->serdes.list[5].mii == RTK_MII_RSGMII)
        {
            for (i=0; i<(sizeof(rtl8380_serdes5_rsgmii_6275a)/sizeof(rtl8380_reg_value_t)); i++)
            {
                MEM32_WRITE(rtl8380_serdes5_rsgmii_6275a[i].reg,  rtl8380_serdes5_rsgmii_6275a[i].val);
                OSAL_MDELAY(1);
            }
        }
        else if(pModel->serdes.list[5].mii == RTK_MII_SGMII)
        {
        }
        else if(pModel->serdes.list[5].mii == RTK_MII_1000BX_FIBER)
        {
            for (i=0; i<(sizeof(rtl8380_serdes5_fiber_6275a)/sizeof(rtl8380_reg_value_t)); i++)
            {
                MEM32_WRITE(rtl8380_serdes5_fiber_6275a[i].reg,  rtl8380_serdes5_fiber_6275a[i].val);
                OSAL_MDELAY(1);
            }
        }
        else if(pModel->serdes.list[5].mii == RTK_MII_100BX_FIBER)
        {
            for (i=0; i<(sizeof(rtl8380_serdes5_fiber_6275a)/sizeof(rtl8380_reg_value_t)); i++)
            {
                MEM32_WRITE(rtl8380_serdes5_fiber_6275a[i].reg,  rtl8380_serdes5_fiber_6275a[i].val);
                OSAL_MDELAY(1);
            }
        }
        else if(pModel->serdes.list[5].mii == RTK_MII_HISGMII)
        {
        }
        else
        {
            for (i=0; i<(sizeof(rtl8380_serdes5_nophy_6275a)/sizeof(rtl8380_reg_value_t)); i++)
            {
                MEM32_WRITE(rtl8380_serdes5_nophy_6275a[i].reg,  rtl8380_serdes5_nophy_6275a[i].val);
                OSAL_MDELAY(1);
            }
        }
    }
    else
    {
        /* Serdes0, 1*/
        if(pModel->serdes.list[0].mii == RTK_MII_RSGMII_PLUS)
        {
            /*Save 0xbb000FF8 bit4-bit32 from SOC*/
            value = MEM32_READ(0xbb000FF8);
            value &= 0xFFFFFFF0;

            /*Save 0xbb000FF8 bit0-bit3 from Array*/
            value_temp = rtl8380_serdes01_rsgmiip_6275b[0].val;
            value_temp &= 0x0000000F;

            /*Reconstruct data and write back into array*/
            value |= value_temp;
            rtl8380_serdes01_rsgmiip_6275b[0].val = value;

            for (i=0; i<(sizeof(rtl8380_serdes01_rsgmiip_6275b)/sizeof(rtl8380_reg_value_t)); i++)
            {
                MEM32_WRITE(rtl8380_serdes01_rsgmiip_6275b[i].reg,  rtl8380_serdes01_rsgmiip_6275b[i].val);
                OSAL_MDELAY(1);
            }
        }
        else if(pModel->serdes.list[0].mii == RTK_MII_QSGMII)
        {
            /*Save 0xbb000FF8 bit4-bit32 from SOC*/
            value = MEM32_READ(0xbb000FF8);
            value &= 0xFFFFFFF0;

            /*Save 0xbb000FF8 bit0-bit3 from Array*/
            value_temp = rtl8380_serdes01_qsgmii_6275b[0].val;
            value_temp &= 0x0000000F;

            /*Reconstruct data and write back into array*/
            value |= value_temp;
            rtl8380_serdes01_qsgmii_6275b[0].val = value;

            for (i=0; i<(sizeof(rtl8380_serdes01_qsgmii_6275b)/sizeof(rtl8380_reg_value_t)); i++)
            {
                MEM32_WRITE(rtl8380_serdes01_qsgmii_6275b[i].reg,  rtl8380_serdes01_qsgmii_6275b[i].val);
                OSAL_MDELAY(1);
            }
        }
        else if(pModel->serdes.list[0].mii == RTK_MII_XSMII)
        {

            /*Save 0xbb000FF8 bit4-bit32 from SOC*/
            value = MEM32_READ(0xbb000FF8);
            value &= 0xFFFFFFF0;

            /*Save 0xbb000FF8 bit0-bit3 from Array*/
            value_temp = rtl8380_serdes01_xsmii_6275b[0].val;
            value_temp &= 0x0000000F;

            /*Reconstruct data and write back into array*/
            value |= value_temp;
            rtl8380_serdes01_xsmii_6275b[0].val = value;

            for (i=0; i<(sizeof(rtl8380_serdes01_xsmii_6275b)/sizeof(rtl8380_reg_value_t)); i++)
            {
                MEM32_WRITE(rtl8380_serdes01_xsmii_6275b[i].reg,  rtl8380_serdes01_xsmii_6275b[i].val);
                OSAL_MDELAY(1);
            }
        }
        else
        {
            /*Do nothing*/
        }

        /* Serdes2,3*/
        if(pModel->serdes.list[2].mii == RTK_MII_RSGMII_PLUS)
        {
            for (i=0; i<(sizeof(rtl8380_serdes23_rsgmiip_6275b)/sizeof(rtl8380_reg_value_t)); i++)
            {
                MEM32_WRITE(rtl8380_serdes23_rsgmiip_6275b[i].reg,  rtl8380_serdes23_rsgmiip_6275b[i].val);
                OSAL_MDELAY(1);
            }
        }
        else if(pModel->serdes.list[2].mii == RTK_MII_QSGMII)
        {
            for (i=0; i<(sizeof(rtl8380_serdes23_qsgmii_6275b)/sizeof(rtl8380_reg_value_t)); i++)
            {
                MEM32_WRITE(rtl8380_serdes23_qsgmii_6275b[i].reg,  rtl8380_serdes23_qsgmii_6275b[i].val);
                OSAL_MDELAY(1);
            }
        }
        else if(pModel->serdes.list[2].mii == RTK_MII_XSMII)
        {
            for (i=0; i<(sizeof(rtl8380_serdes23_xsmii_6275b)/sizeof(rtl8380_reg_value_t)); i++)
            {
                MEM32_WRITE(rtl8380_serdes23_xsmii_6275b[i].reg,  rtl8380_serdes23_xsmii_6275b[i].val);
                OSAL_MDELAY(1);
            }
        }
        else
        {
            /*Do nothing*/
        }

        /* Serdes4*/
        if(pModel->serdes.list[4].mii == RTK_MII_RSGMII_PLUS)
        {
            for (i=0; i<(sizeof(rtl8380_serdes4_rsgmiip_6275b)/sizeof(rtl8380_reg_value_t)); i++)
            {
                MEM32_WRITE(rtl8380_serdes4_rsgmiip_6275b[i].reg,  rtl8380_serdes4_rsgmiip_6275b[i].val);
                OSAL_MDELAY(1);
            }
        }
        else if(pModel->serdes.list[4].mii == RTK_MII_RSGMII)
        {
            for (i=0; i<(sizeof(rtl8380_serdes4_rsgmii_6275b)/sizeof(rtl8380_reg_value_t)); i++)
            {
                MEM32_WRITE(rtl8380_serdes4_rsgmii_6275b[i].reg,  rtl8380_serdes4_rsgmii_6275b[i].val);
                OSAL_MDELAY(1);
            }
        }
        else if(pModel->serdes.list[4].mii == RTK_MII_SGMII)
        {
            for (i=0; i<(sizeof(rtl8380_serdes4_sgmii_6275b)/sizeof(rtl8380_reg_value_t)); i++)
            {
                MEM32_WRITE(rtl8380_serdes4_sgmii_6275b[i].reg,  rtl8380_serdes4_sgmii_6275b[i].val);
                OSAL_MDELAY(1);
            }
        }
        else if(pModel->serdes.list[4].mii == RTK_MII_1000BX_FIBER)
        {
            for (i=0; i<(sizeof(rtl8380_serdes4_fiber_6275b)/sizeof(rtl8380_reg_value_t)); i++)
            {
                MEM32_WRITE(rtl8380_serdes4_fiber_6275b[i].reg,  rtl8380_serdes4_fiber_6275b[i].val);
                OSAL_MDELAY(1);
            }
        }
        else if(pModel->serdes.list[4].mii == RTK_MII_100BX_FIBER)
        {
            for (i=0; i<(sizeof(rtl8380_serdes4_fiber_6275b)/sizeof(rtl8380_reg_value_t)); i++)
            {
                MEM32_WRITE(rtl8380_serdes4_fiber_6275b[i].reg,  rtl8380_serdes4_fiber_6275b[i].val);
                OSAL_MDELAY(1);
            }
        }
        else if(pModel->serdes.list[4].mii == RTK_MII_QSGMII)
        {
            for (i=0; i<(sizeof(rtl8380_serdes4_qsgmii_6275b)/sizeof(rtl8380_reg_value_t)); i++)
            {
                MEM32_WRITE(rtl8380_serdes4_qsgmii_6275b[i].reg,  rtl8380_serdes4_qsgmii_6275b[i].val);
                OSAL_MDELAY(1);
            }
        }
        else if(pModel->serdes.list[4].mii == RTK_MII_HISGMII)
        {
        }
        else
        {
            for (i=0; i<(sizeof(rtl8380_serdes4_nophy_6275b)/sizeof(rtl8380_reg_value_t)); i++)
            {
                MEM32_WRITE(rtl8380_serdes4_nophy_6275b[i].reg,  rtl8380_serdes4_nophy_6275b[i].val);
                OSAL_MDELAY(1);
            }
        }

        /*Serdes5*/
        if(pModel->serdes.list[5].mii == RTK_MII_RSGMII)
        {
            for (i=0; i<(sizeof(rtl8380_serdes5_rsgmii_6275b)/sizeof(rtl8380_reg_value_t)); i++)
            {
                MEM32_WRITE(rtl8380_serdes5_rsgmii_6275b[i].reg,  rtl8380_serdes5_rsgmii_6275b[i].val);
                OSAL_MDELAY(1);
            }
        }
        else if(pModel->serdes.list[5].mii == RTK_MII_SGMII)
        {
            for (i=0; i<(sizeof(rtl8380_serdes5_sgmii_6275b)/sizeof(rtl8380_reg_value_t)); i++)
            {
                MEM32_WRITE(rtl8380_serdes5_sgmii_6275b[i].reg,  rtl8380_serdes5_sgmii_6275b[i].val);
                OSAL_MDELAY(1);
            }
        }
        else if(pModel->serdes.list[5].mii == RTK_MII_1000BX_FIBER)
        {
            for (i=0; i<(sizeof(rtl8380_serdes5_fiber_6275b)/sizeof(rtl8380_reg_value_t)); i++)
            {
                MEM32_WRITE(rtl8380_serdes5_fiber_6275b[i].reg,  rtl8380_serdes5_fiber_6275b[i].val);
                OSAL_MDELAY(1);
            }
        }
        else if(pModel->serdes.list[5].mii == RTK_MII_100BX_FIBER)
        {
            for (i=0; i<(sizeof(rtl8380_serdes5_fiber_6275b)/sizeof(rtl8380_reg_value_t)); i++)
            {
                MEM32_WRITE(rtl8380_serdes5_fiber_6275b[i].reg,  rtl8380_serdes5_fiber_6275b[i].val);
                OSAL_MDELAY(1);
            }
        }
        else if(pModel->serdes.list[5].mii == RTK_MII_HISGMII)
        {
        }
        else
        {
            for (i=0; i<(sizeof(rtl8380_serdes5_nophy_6275b)/sizeof(rtl8380_reg_value_t)); i++)
            {
                MEM32_WRITE(rtl8380_serdes5_nophy_6275b[i].reg,  rtl8380_serdes5_nophy_6275b[i].val);
                OSAL_MDELAY(1);
            }
        }
    }


    /* Serdes reset*/
    for (i=0; i<(sizeof(rtl8380_serdes_reset)/sizeof(rtl8380_reg_value_t)); i++)
    {
        MEM32_WRITE(rtl8380_serdes_reset[i].reg,  rtl8380_serdes_reset[i].val);
        OSAL_MDELAY(1);
    }

    /*serdes software reset release own ship*/
    for (i=0; i<(sizeof(rtl8380_serdes_soft_reset_release)/sizeof(rtl8380_reg_value_t)); i++)
    {
        MEM32_WRITE(rtl8380_serdes_soft_reset_release[i].reg,  rtl8380_serdes_soft_reset_release[i].val);
        OSAL_MDELAY(1);
    }

    /*************************MAC Serdes force linkdown Settings**********************/
    MEM32_WRITE(0xbb000034, sds_power_down_value);

    /*Set Serdes4 MAC Interface register*/
     value = MEM32_READ(0xbb000034);
     value &= (~0x10);
    if(pModel->serdes.list[4].mii == RTK_MII_NONE)
    {
         value |= 0x1<<4;
    }
    MEM32_WRITE(0xbb000034, value);

    /*Set Serdes5 MAC Interface register*/
     value = MEM32_READ(0xbb000034);
     value &= (~0x20);
    if(pModel->serdes.list[5].mii == RTK_MII_NONE)
    {
         value |= 0x1<<5;
    }
    MEM32_WRITE(0xbb000034, value);


    /*Power off Fiber to avoid led light*/
    if((pModel->serdes.list[4].mii == RTK_MII_1000BX_FIBER) ||(pModel->serdes.list[4].mii == RTK_MII_100BX_FIBER))
    {
  	   uint32 serdes_reg;
	   uint32 serdes_val;
          serdes_reg = 0xbb00f800;
          serdes_val = MEM32_READ(serdes_reg);
          serdes_val |= (1 << 11);
          MEM32_WRITE(serdes_reg, serdes_val);
    }

    if((pModel->serdes.list[5].mii == RTK_MII_1000BX_FIBER) ||(pModel->serdes.list[5].mii == RTK_MII_100BX_FIBER))
    {
  	   uint32 serdes_reg;
	   uint32 serdes_val;
          serdes_reg = 0xbb00f900;
          serdes_val = MEM32_READ(serdes_reg);
          serdes_val |= (1 << 11);
          MEM32_WRITE(serdes_reg, serdes_val);
    }

      return;
} /* end of rtl8380_intraSerdes_config_init */

/* Function Name:
 *      rtl8380_misc_config_init
 * Description:
 *      Misc Configuration code in the RTL8380
 * Input:
 *      pModel - pointer to switch model of platform
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
#ifdef CONFIG_EEE
extern void rtk_eee_off(const rtk_switch_model_t *pModel);
#endif

static void rtl8380_misc_config_init(const rtk_switch_model_t *pModel)
{
 #ifdef CONFIG_EEE
    rtk_eee_off(pModel);
 #endif

    //rtl8380_phyPowerOff();
    return;
} /* end of rtl8380_misc_config_init */

/* Function Name:
 *      rtl8380_config
 * Description:
 *      Configuration code for RTL8380
 * Input:
 *      pModel - pointer to switch model of platform
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
 static void rtl8380_disable_polling_phy(void)
{
    MEM32_WRITE(0xbb00a17c, 0x00000000);
}

 static  void rtl8380_enable_polling_phy(const rtk_switch_model_t *pModel)
{
    uint32 val;

    OSAL_MDELAY(1000);

    {
        /*The first 24 ports(Port0-Port23) settings*/
        if((pModel->port.count / 8) == 1)
        {
            val = 0x00FF00;
        }
        else  if((pModel->port.count / 8) == 2)
        {
            val = 0xFFFF00;
        }
        else  if((pModel->port.count / 8) == 3)
        {
            val = 0xFFFFFF;
        }
        else
        {
            val = 0;
        }

        /*The next four(Port24-Port27) ports settings*/
        if((pModel->port.count % 8) == 0)
        {
            val |= 0x0000000;
        }
        else  if((pModel->port.count % 8) == 1)
        {
            val |= 0x1000000;
        }
        else  if((pModel->port.count % 8) == 2)
        {
            /*Check Serdes4 Mode*/
            if(pModel->serdes.list[4].mii == RTK_MII_RSGMII)
            {
                val |= 0x3000000; /*RSGMII: 2.5G*/
            }
            else if(pModel->serdes.list[4].mii == RTK_MII_QSGMII)
            {
                val |= 0x3000000; /*RTK_MII_QSGMII: 5G; Only Using 2 ports*/
            }
            else if(pModel->serdes.list[4].mii == RTK_MII_SGMII)
            {
                val |= 0x5000000; /*RTK_MII_SGMII: 1.25G; Only Using 2 ports*/
            }
        }
        else  if((pModel->port.count % 8) == 3)
        {
            val |= 0x7000000;
        }
        else  if((pModel->port.count % 8) == 4)
        {
            val |= 0xF000000;
        }
        else
        {
            val |= 0x0000000;
        }
    }
    MEM32_WRITE(0xbb00a17c, val);

    if(RTL8380_VERSION_A == rtl8380_version)
    {
        /*Bug: Only for Final Chip A-CUT  && 833X Series, mantis#0013048 */
        if((pModel->chip == RTK_CHIP_RTL8330M) || (pModel->chip == RTK_CHIP_RTL8332M))
        {
            val = MEM32_READ(0xbb00a17c);
            val &= 0xffffff;
            MEM32_WRITE(0xbb00a17c,  val);

            val = MEM32_READ(0xbb00a100);
            val &= ~0x80;
            MEM32_WRITE(0xbb00a100, val);
        }
    }

    /*PHY PATCH OK*/
    val = MEM32_READ(0xbb00a100);
    val |= 0x8000;
    MEM32_WRITE(0xbb00a100, val);

}


#if CONFIG_RTL8380_EQC_TEST
static void rtl8380_eqc_test_config(void)
{
      /*This section is only for 8380 EQC test use*/
      uint32 value,port;

      /*phy patch already ok*/
       rtl8380_phyPowerOn();

       /* Enable MAC TX & RX*/
       for(port = 0; port < 28; port++)
       {
            REG32(0xbb00d560 + port*0x80) |= 0x3;
       }

      MEM32_WRITE(0xbb000058, 0x00000003);
      MEM32_WRITE(0xbb00d044, 0x0000aa55);
      OSAL_PRINTF("PHY patch already ok, register 0xbb00d044 should be 0xaa55 \n");

      /*Then hardware can run their own l2 cases, if already done, set 0xd048 to 0xaa55*/
      OSAL_PRINTF("Waiting untill hardware l2 test cases run already..........\n");
      do
      {
          value = MEM32_READ(0xbb00d048);
      }while(value == 0);
      OSAL_PRINTF("hardware test cases already ok \n");
}
#endif


static void rtl8380_version_check(void)
{
    /*Chip Version Control*/
    uint32 original_data_intRd;
    uint32 original_data_chipRd;
    uint32 temp_chip_info;

    original_data_intRd = MEM32_READ(0xBB000058);
    MEM32_WRITE(0xBB000058, original_data_intRd | 0x3);

    original_data_chipRd = MEM32_READ(0xBB0000D8);
    MEM32_WRITE(0xBB0000D8, original_data_chipRd | 0xA0000000);

    temp_chip_info = MEM32_READ(0xBB0000D8);
    MEM32_WRITE(0xBB0000D8, original_data_chipRd);
    MEM32_WRITE(0xBB000058, original_data_intRd);

    if (((temp_chip_info & 0xFFFF) == 0x6275))
    {
        if(((temp_chip_info>>16) & 0x1F) == 0x1)
        {
            rtl8380_version = RTL8380_VERSION_A;
        }
        else if(((temp_chip_info>>16) & 0x1F) == 0x2)
        {
            rtl8380_version = RTL8380_VERSION_B;
        }
	 else
 	{
            rtl8380_version = RTL8380_VERSION_B;
 	}
    }
    else
    {
            rtl8380_version = RTL8380_VERSION_NONE;
    }
}


#if  defined(CONFIG_RTL8380_OLT_TEST)
void rtl8380_olt_loop(void)
{
      uint32 value,port;
      uint32 sds_no;

       /* LED Settings */
	value = MEM32_READ(0xbb00a000);
	value &= ~(0x7<<0);
	value |= 0x1;
	MEM32_WRITE(0xbb00a000, value);


	value = MEM32_READ(0xbb00a000);
	value &= ~(0x7<<3);
	value |= 0x1<<3;
	MEM32_WRITE(0xbb00a000, value);

	value = MEM32_READ(0xbb00a000);
	value &= ~(0x3<<7);
	MEM32_WRITE(0xbb00a000, value);

	MEM32_WRITE(0xbb00a004, 0x0);

       /* Serdes Settings */
	MEM32_WRITE(0xbb000028, 0xC631821);
	MEM32_WRITE(0xbb00005c, 0x1b);
	MEM32_WRITE(0xbb000034, 0x0);

	/*Set GPIO1-GPIO2-GPIO3 to GPI*/
	value = MEM32_READ(0xb8003500);
	value &= ~(0x7<<25);
	MEM32_WRITE(0xb8003500, value);

	value = MEM32_READ(0xb8003508);
	value &= ~(0x7<<25);
	MEM32_WRITE(0xb8003508, value);


	/*Check Mode: EQC/OLT/Idle*/
	value = MEM32_READ(0xb800350c);
	value = (value >> 25) & 0x7;


	if(0x1 == value)	/*EQC mode*/
      	{
	      OSAL_PRINTF("OLT--->EQC..........\n");

	      /*phy patch already ok*/
	       rtl8380_phyPowerOn();

	       /* Enable Mac TxRx*/
	       for(port = 0; port < 28; port++)
	       {
	            REG32(0xbb00d560 + port*0x80) |= 0x3;
	       }

	      /*Set Flag to notify DVT that u-boot patch code has been ready*/
	      MEM32_WRITE(0xbb00d044, 0x0000aa55);

	      OSAL_PRINTF("PHY patch already ok, register 0xbb00d044 should be 0xaa55 \n");

	      /*Then hardware can run their own l2 cases, if already done, set 0xd048 to 0xaa55*/
	      OSAL_PRINTF("Waiting untill hardware l2 test cases run already..........\n");

	      /*Wait untill Hardware-L2 cases ok*/
	      do
	      {
	          value = MEM32_READ(0xbb00d048);
	      }while(value == 0);

	      OSAL_PRINTF("hardware test cases already ok \n");

      	}
	else if(0x2 == value)	/*OLT mode*/
	{
	      OSAL_PRINTF("OLT--->OLT..........\n");

	      /*phy patch already ok*/
	       rtl8380_phyPowerOn();

	       /* Enable Mac TxRx*/
	       for(port = 0; port < 28; port++)
	       {
	            REG32(0xbb00d560 + port*0x80) |= 0x3;
	       }

	      /*Enable Internal PHY Loopback*/
            gMacDrv->drv_miim_write(8,  0xa44, 17, 0x0419);
            gMacDrv->drv_miim_write(9,  0xa44, 17, 0x0419);
            gMacDrv->drv_miim_write(10,  0xa44, 17, 0x0419);
            gMacDrv->drv_miim_write(11,  0xa44, 17, 0x0419);
            gMacDrv->drv_miim_write(12,  0xa44, 17, 0x0419);
            gMacDrv->drv_miim_write(13,  0xa44, 17, 0x0419);
            gMacDrv->drv_miim_write(14,  0xa44, 17, 0x0419);
            gMacDrv->drv_miim_write(15,  0xa44, 17, 0x0419);

	      /*Enable Serdes Loopback by hardware*/
	      /*No need, hardware will cover*/
	     for(sds_no = 0; sds_no<6; sds_no++)
     	    {
		   MEM32_WRITE(0xbb00e780+sds_no*0x200, 0x0503);
	     }

	     /*Force Port Link status*/
	      for(port = 0; port < 28; port++)
            {
		      MEM32_WRITE(0xbb00a104+4*port, 0x000001eb);
            }
	     /*Force Port28 link down*/
	     MEM32_WRITE(0xbb00a174, 0x6192d);

	      OSAL_PRINTF("OLT--->Begin Gen-Pkts..........\n");
	      /*PktGen One Packet to switch core*/
	     MEM32_WRITE(0xbb00ce68, 0xfffffff);	 	 //#clear pktgen tx done
	     MEM32_WRITE(0xbb00ce60, 0x1);	 		 //#stop tx and reset counter and enable spg
	     MEM32_WRITE(0xbb00ce64, 0xfffffff);		  //#txportmask
	     MEM32_WRITE(0xbb00ce6c, 0x0);	 		 //#random datapattern
	     MEM32_WRITE(0xbb00ce70, 100000000);	 		 //#pktcnt
	     MEM32_WRITE(0xbb00ce78, 0x10000);	 	 //#da:0x1
	     MEM32_WRITE(0xbb00ce7c, 0x2);	 	       //#sa:0x2
	     MEM32_WRITE(0xbb00ce88, 0x101005EE);		//#random length


	    for(port = 0; port < 28; port++)
	    {
	        REG32(0xbb004100+port*4) = 1UL<<port;
	    }


	     MEM32_WRITE(0xbb00ce60, 0x5);			//#trigger
	     OSAL_PRINTF("OLT--->End Gen-Pkts..........\n");

  	     /*Delay 60s*/
            OSAL_MDELAY(60000);

            /*Hardware Reset*/
	     MEM32_WRITE(0xbb000040, 0x1);

	}
	else/*Idle mode*/
	{
	      OSAL_PRINTF("OLT--->IDLE..........\n");
	}
}
#endif

#if 0
#if defined(CONFIG_MDC_MDIO_EXT_SUPPORT)
void rtl8231_init(void)
{
    uint32 val;

     /* Add GPIO output driver current */
    val = MEM32_READ(0xbb001010);
    val |= 0x2;
    MEM32_WRITE(0xbb001010, val);


    /*RTL838xM internal GPIO_A1 is defined to reset rtl8231,
        so we need to set internal GPIO_A1 to be 0x1 before rtl8231
        init*/
    /*configure GPIO_A1 direction as output*/
    val = REG32(GPIO_PABC_DIR);
    REG32(GPIO_PABC_DIR) = val | (1<<GPIO_A1);

    /*configure GPIO_A1 data as 0x1*/
    val = REG32(GPIO_PABC_DATA);
    REG32(GPIO_PABC_DATA) = val | (1<<GPIO_A1);

    /*delay 50ms*/
    udelay(50000);

    /*init rtl8231*/
    rtl8380_smiInit(GPIO_A2,GPIO_A3);
    /*gpio 15~0*/
    rtl8380_smiWrite(RTL8231_ADDR, RTL8231_PIN_SEL_REG, 0xffff);
    /*gpio 31~16*/
    rtl8380_smiWrite(RTL8231_ADDR, RTL8231_PIN_SEL_REG+1, 0xffff);
    /*gpio 36~32*/
    rtl8380_smiRead(RTL8231_ADDR, 0x4, &val);
    val |= 0x1f;
    rtl8380_smiWrite(RTL8231_ADDR, 0x4, val);

    /*Configure all 8231 GPIO to GPI*/
    /* reg 0x5(gpio 15~0) */
    val = 0xffff;
    rtl8380_smiWrite(RTL8231_ADDR, 0x5, val);

    /* reg 0x6(gpio 31~16) */
    val = 0xffff;
    rtl8380_smiWrite(RTL8231_ADDR, 0x6, val);

    /* reg 0x4[9:5] (gpio 36~32) */
    rtl8380_smiRead(RTL8231_ADDR, 0x4, &val);
    val |= 0x1f << 5;
    rtl8380_smiWrite(RTL8231_ADDR, 0x4, val);

    /*Set 8231 PIN5/6/8 output*/
    rtl8231_pin_direction_set(RTL8231_PIN_5,rtl8231_PIN_DIR_OUT);
    rtl8231_pin_direction_set(RTL8231_PIN_6,rtl8231_PIN_DIR_OUT);
    rtl8231_pin_direction_set(RTL8231_PIN_8,rtl8231_PIN_DIR_OUT);
    rtl8231_pin_direction_set(RTL8231_PIN_23,rtl8231_PIN_DIR_OUT);
    rtl8231_pin_direction_set(RTL8231_PIN_29,rtl8231_PIN_DIR_OUT);

    /*Set 8231 PIN5/6/8 output high*/
    rtl8231_pin_data_set(RTL8231_PIN_5,0x1);
    rtl8231_pin_data_set(RTL8231_PIN_6,0x1);
    rtl8231_pin_data_set(RTL8231_PIN_8,0x1);
    rtl8231_pin_data_set(RTL8231_PIN_23,0x0);
    rtl8231_pin_data_set(RTL8231_PIN_29,0x0);

    /*Enable 8231*/
    rtl8380_smiRead(RTL8231_ADDR, 0, &val);
    val |= 0x2;
    rtl8380_smiWrite(RTL8231_ADDR, 0, val);
}
#endif
#endif


void rtl8380_config(const rtk_switch_model_t *pModel)
{
    rtl8380_version_check();

    if(rtl8380_version >= RTL8380_VERSION_A)
    {
#if CONFIG_RTL8380_EQC_TEST
        /*For  EQC Use*/
        rtl8380_eqc_test_config();
#else

#if defined(CONFIG_MDC_MDIO_EXT_SUPPORT)
/*
rtl8231_init(): config gpio as software mode, this mode is just for
                        8380 A-cut. others would be MDC/MDIO mode.
*/
	/*rtl8231_init();*/
#endif
        extGpio_init();
        extGpio_dev_enable(0, 1);

        /*Disable MAC Polling PHY*/
        rtl8380_disable_polling_phy();

        DBG_PRINT(1, "### Platform Config ###\n");
        rtl8380_platform_config_init(gSwitchModel);

        /*MAC Configration*/
        DBG_PRINT(1, "### MAC Config ###\n");
        rtl8380_mac_config_init(gSwitchModel);

#if defined(CONFIG_CUSTOMER_BOARD)
        /*Customer MAC Config*/
        DBG_PRINT(1, "### Customer MAC Config ###\n");
        customer_mac_config_init(gSwitchModel);
#endif

        /*Serdes Patch*/
        DBG_PRINT(1, "### Intra-Serdes Config ###\n");
        rtl8380_intraSerdes_config_init(gSwitchModel);

        /*Phy Patch*/
        DBG_PRINT(1, "### PHY Config (RTL82XX) ###\n");
        rtl8380_phy_config_init(gSwitchModel);

#if defined(CONFIG_CUSTOMER_BOARD)
        /*Customer PHY Config*/
        DBG_PRINT(1, "### Customer PHY Config (RTL82XX) ###\n");
        customer_phy_config_init(gSwitchModel);
#endif

        DBG_PRINT(1, "### Misc Config ###\n");
        rtl8380_misc_config_init(gSwitchModel);

#if defined(CONFIG_CUSTOMER_BOARD)
        DBG_PRINT(1, "### Customer Misc Config ###\n");
        customer_mac_misc_config_init(gSwitchModel);
#endif

        /*Enable MAC Polling PHY*/
        rtl8380_enable_polling_phy(gSwitchModel);

        /*Phy power down*/
        rtl8380_phyPowerOff();

#endif
    }

    return;
} /* end of rtl8380_config */



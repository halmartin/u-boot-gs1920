/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
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
#include <init.h>
#include <rtk/mac/rtl8328/rtl8328_drv.h>
#include <rtk/phy/rtl8214f.h>
#include <rtk/phy/rtl8218.h>
#include "rtl8328_asicregs.h"

/*
 * Symbol Definition
 */

/*
 * Macro Definition
 */

/*
 * Data Declaration
 */

/*
 * Function Declaration
 */

/* Function Name:
 *      rtl8328_platform_config_init
 * Description:
 *      Platform Configuration code in RTL8328M
 * Input:
 *      pModel - pointer to switch model of platform
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
static void rtl8328_platform_config_init(const rtk_switch_model_t *pModel)
{
    unsigned int val;
    unsigned int val_2;
    unsigned int val_3;

    /*common init process*/
    do {
        val = MEM32_READ(0xbb050018);
    }while(!(val&(1 << 16)));

    /*enable read*/
    MEM32_WRITE(0xbb06FFF0, 1);
    val = MEM32_READ(0xbb06FFFC);
    if(((val >> 12) & 0xF) == 0x0)
    {   /* OLD: rtl8328m_patch */
        int i=0;

        DBG_PRINT(1, "RTL8328 A basic init\n");
        val_2 = MEM32_READ(0xbb040014);
        /*RTL8328M, Disable polling*/
        MEM32_WRITE(0xbb040014, 0);

        /*RTL8328M, Patch bandgap*/
        gMacDrv->drv_miim_read(0, 1, 23, &val_3);
        gMacDrv->drv_miim_write(0, 1, 23, (val_3 | (0x1 << 14)) & (~(0x1<<15)));

        /*RTL8328M, Patch eee dac cause crc*/
        gMacDrv->drv_miim_read(0, 4, 25, &val_3);
        gMacDrv->drv_miim_portmask_write(0xffff, 4, 25, (val_3 & (~(0x1<<10))));

        /*RTL8328M, Patch eee symbol error bug*/
        for(i=0;i<29;i++) {      
            MEM32_WRITE((0xBB9B0000+i*0x100), 0x6A1800);
        }
    
        /*RTL8328M, calicode force 0x8888 */
        gMacDrv->drv_miim_read(0, 1, 29, &val_3);
        gMacDrv->drv_miim_portmask_write(0xffff, 1, 29, (val_3 | (0x1<<2)));
        gMacDrv->drv_miim_portmask_write(0xffff, 1, 28, 0x8888);
    
        /*RTL8328M, adtune force 0x6 */
        gMacDrv->drv_miim_read(0, 0, 22, &val_3);
        gMacDrv->drv_miim_portmask_write(0xffff, 0, 22, (val_3 | (0x7<<1)) & (~(0x1<<0)));

        /*RTL8328M, agcend set 0x3*/
        gMacDrv->drv_miim_read(0, 0, 16, &val_3);
        gMacDrv->drv_miim_portmask_write(0xffff, 0, 16, (val_3 | (0x3<<8)) & (~(0x3<<10)));

        /*RTL8328M, disable linkdown power saving mode*/
        gMacDrv->drv_miim_read(0, 0, 24, &val_3);
        gMacDrv->drv_miim_portmask_write(0xffff, 0, 24, (val_3  & (~(0x1<<15))));

        /*RTL8328M, disable port28 L3, L4 checksum error drop*/
        MEM32_WRITE(0xBBD01C08, 1);

        /*RTL8328M, jitter_finetuning for serdes*/
        MEM32_WRITE(0xBB07010C, 0x81A8);
        MEM32_WRITE(0xBB070114, 0x049E);
        MEM32_WRITE(0xBB07050C, 0x81A8);
        MEM32_WRITE(0xBB070514, 0x049E);
        MEM32_WRITE(0xBB07088C, 0x81A8);
        MEM32_WRITE(0xBB070894, 0x049E);
    
        MEM32_WRITE(0xBB070100, 0x800C);
        MEM32_WRITE(0xBB070500, 0x800C);
        MEM32_WRITE(0xBB070880, 0x800C);
    
        udelay(1000);
        MEM32_WRITE(0xBB070100, 0x800D);
        MEM32_WRITE(0xBB070500, 0x800D);
        MEM32_WRITE(0xBB070880, 0x800D);
    
        /*RTL8328M, Adjust flow control threshold para*/
        MEM32_WRITE(0xBB290004, 0x4DE);
        MEM32_WRITE(0xBB290008, 0xF80128);
        MEM32_WRITE(0xBB29000C, 0xA000E0);
        MEM32_WRITE(0xBB29002C, 0x1E00210);
        MEM32_WRITE(0xBB290030, 0x19001C0);
        for(i=0;i<24;i++) {      
            MEM32_WRITE((0xBBA90004+i*0x100), 0x400070);
        }
    
        for(i=24;i<29;i++) {    
            MEM32_WRITE((0xBBA90004+i*0x100), 0x8000C0);
        }
    
        for(i=0;i<24;i++) {      
            MEM32_WRITE((0xBBA90008+i*0x100), 0xC0012);
        }
    
        for(i=24;i<29;i++) {    
            MEM32_WRITE((0xBBA90008+i*0x100), 0x2A0036);
        }
    
        /*RTL8328M, Patch EEE bonding to PCS, disable 10M EEE*/
        gMacDrv->drv_miim_read(0, 4, 16, &val_3);
        gMacDrv->drv_miim_portmask_write(0xffff, 4, 16, (val_3  | (0x1<<12)));
        //Restart Nway.
        gMacDrv->drv_miim_portmask_write(0xffff, 0, 0, 0x3300);
    
        /*RTL8328M, Patch EEE RAM Code*/
        gMacDrv->drv_miim_write(15, 4, 28, 0x1BC);
        gMacDrv->drv_miim_write(15, 4, 29, 0xFFC4);
        gMacDrv->drv_miim_write(15, 4, 28, 0x80);
    
        /*RTL8328M, Patch EEE wait idle timer count*/
        gMacDrv->drv_miim_portmask_write(0xffff, 4, 24, 0xC0F3);
    
        /*RTL8328M, Disable 125MHz output*/
        MEM32_WRITE(0xBB030004, 0x1B87);
    
        /*RTL8328M, Disable serdes 1 25MHz output*/
        MEM32_WRITE(0xBB070500, MEM32_READ(0xBB070500) & 0xFFFF7FFF);
    
        /*RTL8328M, #16. EnIOLLatColjamen Default value change*/
        /*Sel6b Default value change to 0b0 */
        MEM32_WRITE(0xBB100000, 0x41602);

        /*RTL8328M, #17. RG2X_P15~0 [9] Default vlaue change*/
        /*RTL8328M, #20. surge help*/
        gMacDrv->drv_miim_read(0, 1, 18, &val_3);
        gMacDrv->drv_miim_portmask_write(0xffff, 1, 18, (val_3  | (0x1<<9)));

        gMacDrv->drv_miim_read(0, 1, 18, &val_3);
        gMacDrv->drv_miim_portmask_write(0xffff, 1, 18, (val_3  & (~(0x1<<8))));

        /*RTL8328M, Idle number select default change*/
        gMacDrv->drv_miim_read(0, 0, 22, &val_3);
        gMacDrv->drv_miim_portmask_write(0xffff, 0, 22, (val_3  & (~(0x1<<14))));
        /*Patch RTL8328M finished*/
    
        /*Patch RTL8208D begin*/
        /*RTL8208D, Patch Serdes*/
        gMacDrv->drv_miim_write(16, 0x40, 23, 0x0B49);
        gMacDrv->drv_miim_write(16, 0x40, 19, 0x7146);
        gMacDrv->drv_miim_write(16, 0x40, 19, 0x7106);
    
        /*RTL8208D, Patch EEE*/
        gMacDrv->drv_miim_portmask_write(0xff0000, 4, 24, 0xC0F3);
        gMacDrv->drv_miim_write(23, 4, 28, 0x01BC);
        gMacDrv->drv_miim_write(23, 4, 29, 0xFFC4);
        gMacDrv->drv_miim_write(23, 4, 28, 0x80);
    
        /*RTL8208D, calicode force 0x8888 */
        gMacDrv->drv_miim_portmask_write(0xff0000, 1, 29, 0x5);
        gMacDrv->drv_miim_portmask_write(0xff0000, 1, 28, 0x8888);
        
        /*RTL8208D, #adtune force 0x6 */
        gMacDrv->drv_miim_portmask_write(0xff0000, 0, 22, 0x5B8E);
        /* RTL8208D, Idle number select default change */
        gMacDrv->drv_miim_read(16, 0, 22, &val_3);
        gMacDrv->drv_miim_portmask_write(0xff0000, 0, 22, (val_3 & (~(0x1<<14))));

        /*RTL8208D, #agcend set 0x3*/
        gMacDrv->drv_miim_portmask_write(0xff0000, 0, 16, 0x31F);

        /*RTL8328M, Enable polling*/
        MEM32_WRITE(0xbb040014, val_2);

        /*SEL_SDS2_ORG_CRS=0, use RXDV*/
        MEM32_WRITE(0xbb060054, MEM32_READ(0xbb060054) & ~(0x1<<28));

        /*INTF_DMY_REG_7_0, select TXEN & RXDV to detect collision*/
        MEM32_WRITE(0xbb060134, MEM32_READ(0xbb060134) & 0xFFFFFF00);
    } /* end of if(((val >> 12) & 0xF) == 0x0) */
    else
    {   /* NEW: rtl8328m_patch */
        int i=0;

        DBG_PRINT(1, "RTL8328 B basic init\n");
        val_2 = MEM32_READ(0xbb040014);
		/*RTL8328M, Disable polling*/
        MEM32_WRITE(0xbb040014, 0);

        /*RTL8328M, disable port28 L3, L4 checksum error drop*/
        MEM32_WRITE(0xBBD01C08, 1);
		
        /*RTL8328M, Adjust flow control threshold para*/
        MEM32_WRITE(0xBB290004, 0x4DE);
        MEM32_WRITE(0xBB290008, 0xF80128);
        MEM32_WRITE(0xBB29000C, 0xA000E0);
        MEM32_WRITE(0xBB29002C, 0x1E00210);
        MEM32_WRITE(0xBB290030, 0x19001C0);
        for(i=0;i<24;i++) {      
            MEM32_WRITE((0xBBA90004+i*0x100), 0x400070);
        }
          
        for(i=24;i<29;i++) {    
            MEM32_WRITE((0xBBA90004+i*0x100), 0x8000C0);
        }

        for(i=0;i<24;i++) {      
            MEM32_WRITE((0xBBA90008+i*0x100), 0xC0012);
        }

        for(i=24;i<29;i++) {    
            MEM32_WRITE((0xBBA90008+i*0x100), 0x2A0036);
        }
          
        /*RTL8328M, Patch EEE bonding to PCS, enable 10M EEE*/
        gMacDrv->drv_miim_read(0, 4, 16, &val_3);
        gMacDrv->drv_miim_portmask_write(0xffff, 4, 16, (val_3 | (0x3<<12)));
        //Restart Nway.
        gMacDrv->drv_miim_portmask_write(0xffff, 0, 0, 0x3300);

        /*RTL8328M, Disable 125MHz output*/
        MEM32_WRITE(0xBB030004, 0x1B87);
          
        /*RTL8328M, Disable serdes 1 25MHz output*/
        MEM32_WRITE(0xBB070500, MEM32_READ(0xBB070500) & 0xFFFF7FFF);

        /*RTL8328M, #16. Sel6b Default value change to 0b0*/
        MEM32_WRITE(0xBB100000, 0x41602);

		/*RTL8328M, Idle number select default change*/
        gMacDrv->drv_miim_read(0, 0, 22, &val_3);
        gMacDrv->drv_miim_portmask_write(0xffff, 0, 22, (val_3 & (~(0x1<<14))));

        /*RTL8328M, #20. surge help*/
        gMacDrv->drv_miim_read(0, 1, 18, &val_3);
        gMacDrv->drv_miim_portmask_write(0xffff, 1, 18, (val_3 & (~(0x1<<8))));

        /*RTL8328M, #21. With IXIA giga port link @ 10M link down issue*/
        gMacDrv->drv_miim_read(0, 5, 16, &val_3);
        gMacDrv->drv_miim_portmask_write(0xffff, 5, 16, (val_3 | (0x1<<6)));

        /*RTL8328M, #22. Low speed cable insert/remove issue*/
        gMacDrv->drv_miim_read(0, 5, 16, &val_3);
        gMacDrv->drv_miim_portmask_write(0xffff, 5, 16, (val_3 & ~(0x1<<5)));

        /*RTL8328M, #23. 10M EEE MAU*/
        gMacDrv->drv_miim_read(0, 5, 16, &val_3);
        gMacDrv->drv_miim_portmask_write(0xffff, 5, 16, (val_3 | (0x3<<8)));

        /*RTL8328M, change page 5, reg 16, bit[15:12] from 0xF to 0xE */
        gMacDrv->drv_miim_read(0, 5, 16, &val_3);
        val_3 &= ~(0xF<<12);
        gMacDrv->drv_miim_portmask_write(0xffff, 5, 16, (val_3 | (0xE<<12)));

#if defined(CONFIG_EEE_COMPATIBLE)
        /*RTL8328M, Patch eee packet lose IOP issue */
        for(i=0;i<29;i++) {      
            val_3 = MEM32_READ(0xBB9B0000+i*0x100);
            val_3 &= ~(0x7 << 16);
            val_3 |= (0x1 << 16);
            MEM32_WRITE((0xBB9B0000+i*0x100), val_3);
            val_3 = MEM32_READ(0xBB9B0008+i*0x100);
            val_3 &= ~(0xFFFF);
            val_3 |= (0x0020);
            MEM32_WRITE((0xBB9B0008+i*0x100), val_3);
        }
        /*RTL8328M, Adjust Tw timer parameter for eee packet lose issue */
        for(i=24;i<29;i++) {      
            val_3 = MEM32_READ(0xBB9B000C+i*0x100);
            val_3 &= ~(0xFF << 24);
            val_3 |= (0x14 << 24);
            MEM32_WRITE((0xBB9B000C+i*0x100), val_3);
        }
#endif
        /*Patch RTL8328M finished*/
          
        /*Patch RTL8208D begin*/
        /*RTL8208D, Patch Serdes*/
        gMacDrv->drv_miim_write(16, 0x40, 23, 0x0B49);
        gMacDrv->drv_miim_write(16, 0x40, 19, 0x7146);
        gMacDrv->drv_miim_write(16, 0x40, 19, 0x7106);
          
        /*RTL8208D, Patch EEE*/
        gMacDrv->drv_miim_portmask_write(0xff0000, 4, 24, 0xC0F3);
        gMacDrv->drv_miim_write(23, 4, 28, 0x01BC);
        gMacDrv->drv_miim_write(23, 4, 29, 0xFFC4);
        gMacDrv->drv_miim_write(23, 4, 28, 0x80);

        /*RTL8208D, calicode force 0x8888 */
        gMacDrv->drv_miim_portmask_write(0xff0000, 1, 29, 0x5);
        gMacDrv->drv_miim_portmask_write(0xff0000, 1, 28, 0x8888);
              
        /*RTL8208D, #adtune force 0x6 */
        gMacDrv->drv_miim_portmask_write(0xff0000, 0, 22, 0x5B8E);
        /* RTL8208D, Idle number select default change */
        gMacDrv->drv_miim_read(16, 0, 22, &val_3);
        gMacDrv->drv_miim_portmask_write(0xff0000, 0, 22, (val_3 & (~(0x1<<14))));

        /*RTL8208D, #agcend set 0x3*/
        gMacDrv->drv_miim_portmask_write(0xff0000, 0, 16, 0x31F);

        /*#ST_CHG_LATCH_EN =1*/
        MEM32_WRITE(0xBB040000, MEM32_READ(0xBB040000) | (1 << 11));        

        /*RTL8328M, Enable polling*/
        MEM32_WRITE(0xbb040014, val_2);

        val = MEM32_READ(0xbb060128);
        if(((val >> 4) & 0x1) == 0x1)
        {
            DBG_PRINT(1, "QFP package\n");
            /*RTL8328M, power down SDS1*/
            MEM32_WRITE(0xBB010008, MEM32_READ(0xBB010008) & 0xFF7FFFFF);
            
            /*RTL8328M, link down p26~27*/
            MEM32_WRITE(0xbb040080, 2);
            MEM32_WRITE(0xbb040084, 2);
            			  
            /*RTL8328M, Disable port26~27 polling*/
            val_2 = MEM32_READ(0xbb040014);
            MEM32_WRITE(0xbb040014, (val_2 & 0x3FFFFFF));
        }

        /*SEL_SDS2_ORG_CRS=0, use RXDV*/
        MEM32_WRITE(0xbb060054, MEM32_READ(0xbb060054) & ~(0x1<<28));

        /*INTF_DMY_REG_7_0, select TXEN & RXDV to detect collision*/
        MEM32_WRITE(0xbb060134, MEM32_READ(0xbb060134) & 0xFFFFFF00);
    }
    /*disable read*/
    MEM32_WRITE(0xbb06FFF0, 0);

    /* enable LED load indication and select led number */
    val = MEM32_READ(0xbb670058);
    val &= ~(1 << 5);  /*Set Port 24~27 LED to copper+fiber*/
    val |= (1 << 6);   /*FE Port LED load indication*/
    val |= (1 << 7);   /*Giga Port LED load indication*/
    val &= ~(3 << 1);  /*Clear SEL_LED_NUM field*/
    if (pModel->led.num != 4)
        val |= ((pModel->led.num & 0x3) << 1);
    MEM32_WRITE(0xbb670058, val);

    /* (BY CUSTOMER APPLICATION) LED System Control Register 2 */
    val = ((pModel->led.sel_p24_p27_led_mod & 0xFFFF) << 16) | ((pModel->led.sel_p0_p23_led_mod & 0xFFFF) << 0);
    MEM32_WRITE(0xbb670060, val);
    
    /* Set N-port serial LED output port mask */
    if (pModel->led.count == 32)
        val = 0xFFFFFFFF;
    else
        val = ((1 << (pModel->led.count)) - 1) << pModel->led.offset;
    MEM32_WRITE(0xbb670064, val);

    return;
} /* end of rtl8328_platform_config_init */

/* Function Name:
 *      rtl8328_mac_config_init
 * Description:
 *      Mac Configuration code in RTL8328M
 * Input:
 *      pModel - pointer to switch model of platform
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
static void rtl8328_mac_config_init(const rtk_switch_model_t *pModel)
{
    unsigned int port, val, i;

    /* Configure MLFID to 0 */
    val = MEM32_READ(0xbb080020);
    val = val & ~(1 << 0) ;
    MEM32_WRITE(0xbb080020, val);

    /* CPU port property configuration */
    MEM32_WRITE(0xbb040088, 0x706);

    /*config 6102 checksum error drop, MAC*/
    for(port = 0; port < 28; port++)
    {
        MEM32_WRITE(0xbbd00008 + port*0x100, 0x1);
    }
    MEM32_WRITE(0xbbd01c08, 0);

    /*config all port CRC recalulate, MAC*/
    MEM32_WRITE(0xbb550000, 0xffffffff);

    /*disable cpu port min-max leaky bucket, MAC*/
    MEM32_WRITE(0xbbB51c78, 0);
    MEM32_WRITE(0xbbB51c74, 0);

    /*disable EEE, MAC*/
    MEM32_WRITE(0xbb1b0000, 1);

    MEM32_WRITE(0xbb6B0094,0xffffffff);

    /*disable CPU port VLAN ingress/egress Filter*/
    MEM32_WRITE(0xbb881c00 + 0x12*4,0);
    MEM32_WRITE(0xbb881c00 + 0x14*4,0);

    /*disable RX/TX cpu tag
      *disable cpu port crc re-cal
      */
    MEM32_WRITE(0xbb580000,0x8899);
    MEM32_WRITE(0xbb580008,0x0);    

    /*disable cpu port pause
      *** disable cpu port RX/TX Pause
      *** Config cpu port force mode & flow control off
    */
    val = MEM32_READ(0xbb901c00);
    val = val & ~(1 << 5) ;
    val = val & ~(1 << 6) ;
    MEM32_WRITE(0xbb901c00, val);
    val = MEM32_READ(0xbba91c00);
    val = (val & ~(1 << 0));
    MEM32_WRITE(0xbba91c00, val);    
    val = MEM32_READ(0xbbb51c90);
    val = (val & ~(1 << 0)) | 1 ;
    MEM32_WRITE(0xbbb51c90, val);       

#if defined(CONFIG_EEE_COMPATIBLE)
    /* configure RX IPG will reduce 4 tx clock cycle for EEE compatible issue */
    for(port = 0; port <= 28; port++)
    {
        val = MEM32_READ(0xbb900000 + port*0x100);
        val = val | (1 << 8) ;
        MEM32_WRITE(0xbb900000 + port*0x100, val);
    }
#endif

    /* Serdes: change tx & rx polarity */
    for (i=(pModel->serdes.offset); i<(pModel->serdes.offset + pModel->serdes.count); i++)
    {
        Tuint8 polarity_conf = 0;

        if (i >= 2) break;
        val = MEM32_READ(0xbb070080 + i * 0x400);
        val = (val & ~(7 << 8));
        polarity_conf |= (pModel->serdes.list[(i - pModel->serdes.offset)].tx_polarity == SERDES_POLARITY_CHANGE)? 0x1 : 0x0;
        polarity_conf |= (pModel->serdes.list[(i - pModel->serdes.offset)].rx_polarity == SERDES_POLARITY_CHANGE)? 0x2 : 0x0;
        polarity_conf |= 0x4;
        val |= (polarity_conf << 8);
        MEM32_WRITE(0xbb070080 + i * 0x400, val);
    }
    if (2 < (pModel->serdes.offset + pModel->serdes.count))
    {
        Tuint8 polarity_conf = 0;

        val = MEM32_READ(0xbb070800);
        val = (val & ~(7 << 8));
        polarity_conf |= (pModel->serdes.list[(2 - pModel->serdes.offset)].tx_polarity == SERDES_POLARITY_CHANGE)? 0x1 : 0x0;
        polarity_conf |= (pModel->serdes.list[(2 - pModel->serdes.offset)].rx_polarity == SERDES_POLARITY_CHANGE)? 0x2 : 0x0;
        polarity_conf |= 0x4;
        val |= (polarity_conf << 8);
        MEM32_WRITE(0xbb070800, val);
    }

    /* Disable serial mode per-port LED to prevent LED instantly blink once PHY power on
     * and will turn on LED after system is initialized (in initd.c)
     */
    REG32(0xbb670064) = 0x00000000;

    return;
} /* end of rtl8328_mac_config_init */

/* Function Name:
 *      rtl8328_phy_config_init
 * Description:
 *      PHY Configuration code that connect with RTL8328M
 * Input:
 *      pModel - pointer to switch model of platform
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
static void rtl8328_phy_config_init(const rtk_switch_model_t *pModel)
{
    unsigned int phy_idx;
    unsigned int val;

    val = MEM32_READ(0xbb040014);

    for (phy_idx=0; phy_idx<pModel->phy.count; phy_idx++)
    {
        switch (pModel->phy.list[phy_idx].chip)
        {
        case RTK_CHIP_RTL8214F:
            {
                Tuint8 rtl8214f_phy0_macid = pModel->phy.list[phy_idx].phyid - pModel->phy.baseid;

                /*RTL8328M, Disable port24~27 polling*/
                MEM32_WRITE(0xbb040014, (val & 0xFFFFFF));	
                rtl8328_rtl8214f_config(rtl8214f_phy0_macid);
                /*RTL8328M, Enable port24~27 polling*/
                MEM32_WRITE(0xbb040014, val);
            }
            break;

        case RTK_CHIP_RTL8214FB:
        case RTK_CHIP_RTL8214B:
        case RTK_CHIP_RTL8212B:
            {
                Tuint8 rtl8214fb_phy0_macid = pModel->phy.list[phy_idx].phyid - pModel->phy.baseid;

                /*RTL8328M, Disable port24~27 polling*/
                MEM32_WRITE(0xbb040014, (val & 0xFFFFFF));
                rtl8214fb_config(rtl8214fb_phy0_macid);
                /*RTL8328M, Enable port24~27 polling*/
                MEM32_WRITE(0xbb040014, val);
            }
            break;

        case RTK_CHIP_RTL8212F:
            rtl8328_rtl8212f_config();
            break;

        case RTK_CHIP_RTL8214:
        case RTK_CHIP_RTL8218:
        default:
            break;
        }
    }

    OSAL_MDELAY(100);    /* delay 100mS */

    return;
} /* end of rtl8328_phy_config_init */

/* Function Name:
 *      rtl8328_serdes_config_init
 * Description:
 *      Intra Serdes Configuration code that connect with RTL8328M
 * Input:
 *      pModel - pointer to switch model of platform
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
static void rtl8328_serdes_config_init(const rtk_switch_model_t *pModel)
{
    return;
} /* end of rtl8328_serdes_config_init */

/* Function Name:
 *      rtl8328_misc_config_init
 * Description:
 *      Misc Configuration code in the RTL8328M
 * Input:
 *      pModel - pointer to switch model of platform
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
static void rtl8328_misc_config_init(const rtk_switch_model_t *pModel)
{
    unsigned int port;

    /* Disable Mac TxRx and Power PHY off */
    for(port = 0; port < 28; port++)
    {
        REG32(0xbb900000 + port*0x100) &= ~4;      
    } 

    rtl8328_phyPowerOff();

    return;
} /* end of rtl8328_misc_config_init */

/* Function Name:
 *      rtl8328_config
 * Description:
 *      Configuration code for RTL8328M.
 * Input:
 *      pModel - pointer to switch model of platform
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8328_config(const rtk_switch_model_t *pModel)
{
    DBG_PRINT(1, "### Platform Config ###\n");
    rtl8328_platform_config_init(gSwitchModel);

    //DBG_PRINT(1, "### change MDC freq. from 96ns to 384ns ###\n");
    //fun_001(gSwitchModel); /* Move the code into platform_config */

    //DBG_PRINT(1, "### Reset PHY ###\n");
    //fun_002(gSwitchModel); /* Move the code into platform_config */
    
    DBG_PRINT(1, "### MAC Config ###\n");
    rtl8328_mac_config_init(gSwitchModel);
    
#if defined(CONFIG_CUSTOMER_BOARD)
    DBG_PRINT(1, "### Customer MAC Config ###\n");
    customer_mac_config_init(gSwitchModel);
#endif

    DBG_PRINT(1, "### Power down PHY (RTL82XX) ###\n");
    rtl8328_phyPowerOff();
    
    DBG_PRINT(1, "### PHY Config (RTL82XX) ###\n");
    rtl8328_phy_config_init(gSwitchModel);
    
#if defined(CONFIG_CUSTOMER_BOARD)
    DBG_PRINT(1, "### Customer PHY Config (RTL82XX) ###\n");
    customer_phy_config_init(gSwitchModel);
#endif

    DBG_PRINT(1, "### Intra-Serdes Config ###\n");
    rtl8328_serdes_config_init(gSwitchModel);

    DBG_PRINT(1, "### Misc Config ###\n");
    rtl8328_misc_config_init(gSwitchModel);

#if defined(CONFIG_CUSTOMER_BOARD)
    DBG_PRINT(1, "### Customer Misc Config ###\n");
    customer_mac_misc_config_init(gSwitchModel);
#endif

    return;
} /* end of rtl8328_config */

/*
 * Copyright(c) Realtek Semiconductor Corporation, 2012
 * All rights reserved.
 *
 * Purpose : Related implementation of the RTL8380 board for U-Boot.
 *
 * Feature : RTL8380 platform
 *
 */


/*
 * Include Files
 */
#include <rtk_type.h>
#include <rtk_reg.h>
#include <init.h>
#include <rtk/mac/rtl8380/rtl8380_swcore_reg.h>
#include <config.h>
#include <rtk_osal.h>
#include <rtk/phy/rtl8214f.h>

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
 *      rtl8380_setPhyReg
 * Description:
 *      Set PHY register.
 * Input:
 *      portid - Port number (0~28)
 *      page   - PHY page (0~4095)
 *      reg    - PHY register (0~31)
 *      val    - data to write
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
int rtl8380_setPhyReg(int portid, int page, int reg, unsigned int val)
{
    unsigned int value;

    /*check portID range*/
    portid += (gSwitchModel->port.offset);
    if(portid>28)
		return 0;
    portid &= 0x1F;

    /*check page*/
    page &= 0xFFF;

    /*check reg*/
    reg &= 0x1F;

    /*check vale*/
    val &= 0xFFFF;

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    while ((MEM32_READ(SWCORE_BASE_ADDR| RTL8380_SMI_ACCESS_PHY_CTRL_1_ADDR) & 0x1) == 0x1);

    /*step0*/
    /* select PHY to access */
    value = 1UL << portid;
    MEM32_WRITE(SWCORE_BASE_ADDR| RTL8380_SMI_ACCESS_PHY_CTRL_0_ADDR, value);

    OSAL_MDELAY(1);

    /*step1*/
    /* RWOP = 1(write), then INDATA[15:0] = DATA[15:0] */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8380_SMI_ACCESS_PHY_CTRL_2_ADDR, \
                        RTL8380_SMI_ACCESS_PHY_CTRL_2_INDATA_15_0_OFFSET, \
                        RTL8380_SMI_ACCESS_PHY_CTRL_2_INDATA_15_0_MASK, val);

    /*step2*/
    /* select register number to access */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8380_SMI_ACCESS_PHY_CTRL_1_ADDR, \
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_REG_ADDR_4_0_OFFSET, \
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_REG_ADDR_4_0_MASK, reg);

    /* select main page number to access */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8380_SMI_ACCESS_PHY_CTRL_1_ADDR, \
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_MAIN_PAGE_11_0_OFFSET,\
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_MAIN_PAGE_11_0_MASK, page);

    /* clear park page */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8380_SMI_ACCESS_PHY_CTRL_1_ADDR, \
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_PARK_PAGE_4_0_OFFSET,\
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_PARK_PAGE_4_0_MASK, 0x1F); /*keep park*/

    /* set PHY register type to normal */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8380_SMI_ACCESS_PHY_CTRL_1_ADDR, \
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_TYPE_OFFSET,\
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_TYPE_MASK, 0);

    /* write operation */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8380_SMI_ACCESS_PHY_CTRL_1_ADDR, \
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_RWOP_OFFSET,\
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_RWOP_MASK, 1);

    /* request MAC to access PHY MII register */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8380_SMI_ACCESS_PHY_CTRL_1_ADDR, \
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_CMD_OFFSET,\
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_CMD_MASK, 1);

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    while ((MEM32_READ(SWCORE_BASE_ADDR| RTL8380_SMI_ACCESS_PHY_CTRL_1_ADDR) & 0x1) == 0x1);

	return 0;

} /* end of rtl8380_setPhyReg */

/* Function Name:
 *      rtl8380_getPhyReg
 * Description:
 *      Get PHY register.
 * Input:
 *      portid - Port number (0~28)
 *      page   - PHY page (0~4095)
 *      reg    - PHY register (0~31)
 *      val    - Read data
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
int rtl8380_getPhyReg(int portid, int page, int reg, unsigned int *val)
{
    unsigned int value;
    unsigned int port_rmk_regaddr;
    unsigned int portid_rmk_phyid;

    /*check portID range*/
    portid += (gSwitchModel->port.offset);
    if(portid>28)
		return 0;
    portid &= 0x1F;

    /*check page*/
    page &= 0xFFF;

    /*check reg*/
    reg &= 0x1F;

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    while ((MEM32_READ(SWCORE_BASE_ADDR| RTL8380_SMI_ACCESS_PHY_CTRL_1_ADDR) & 0x1) == 0x1);

    /*step0*/
    /* RWOP = 0(read), then INDATA[15:0] represents PORTID*/
     /*For 80 8ports mode & 16 ports mode, loader may remark phyID,
        so here get real phyID according to remark table, although 82m no need this, but here also using this mechanism*/
    if((portid >= 0) && (portid <= 5))
    {
        port_rmk_regaddr = SWCORE_BASE_ADDR| RTL8380_SMI_PORT0_5_ADDR_CTRL_ADDR;
    }
    else if((portid >= 6) && (portid <= 11))
    {
        port_rmk_regaddr = SWCORE_BASE_ADDR| RTL8380_SMI_PORT6_11_ADDR_CTRL_ADDR;
    }
    else if((portid >= 12) && (portid <= 17))
    {
        port_rmk_regaddr = SWCORE_BASE_ADDR| RTL8380_SMI_PORT12_17_ADDR_CTRL_ADDR;
    }
    else if((portid >= 18) && (portid <= 23))
    {
        port_rmk_regaddr = SWCORE_BASE_ADDR| RTL8380_SMI_PORT18_23_ADDR_CTRL_ADDR;
    }
    else if((portid >= 24) && (portid <= 27))
    {
        port_rmk_regaddr = SWCORE_BASE_ADDR| RTL8380_SMI_PORT24_27_ADDR_CTRL_ADDR;
    }
    else
    {
    	 return 0;
    }

   portid_rmk_phyid =  MEM32_READ(port_rmk_regaddr);
   portid = (portid_rmk_phyid>>(5*(portid%6))) & 0x1F;


    value = portid;
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8380_SMI_ACCESS_PHY_CTRL_2_ADDR, \
                        RTL8380_SMI_ACCESS_PHY_CTRL_2_INDATA_15_0_OFFSET, \
                        RTL8380_SMI_ACCESS_PHY_CTRL_2_INDATA_15_0_MASK, value);

    /*step1*/
    /* select register number to access */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8380_SMI_ACCESS_PHY_CTRL_1_ADDR, \
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_REG_ADDR_4_0_OFFSET, \
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_REG_ADDR_4_0_MASK, reg);

    /* select main page number to access */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8380_SMI_ACCESS_PHY_CTRL_1_ADDR, \
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_MAIN_PAGE_11_0_OFFSET,\
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_MAIN_PAGE_11_0_MASK, page);

    /* clear park page */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8380_SMI_ACCESS_PHY_CTRL_1_ADDR, \
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_PARK_PAGE_4_0_OFFSET,\
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_PARK_PAGE_4_0_MASK, 0x1F);

    /* set PHY register type to normal */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8380_SMI_ACCESS_PHY_CTRL_1_ADDR, \
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_TYPE_OFFSET,\
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_TYPE_MASK, 0);

    /* read operation */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8380_SMI_ACCESS_PHY_CTRL_1_ADDR, \
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_RWOP_OFFSET,\
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_RWOP_MASK, 0);

    /* request MAC to access PHY MII register */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8380_SMI_ACCESS_PHY_CTRL_1_ADDR, \
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_CMD_OFFSET,\
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_CMD_MASK, 1);


    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    while ((MEM32_READ(SWCORE_BASE_ADDR| RTL8380_SMI_ACCESS_PHY_CTRL_1_ADDR) & 0x1) == 0x1);

    /* get the read result */
    *val = MEM32_READ(SWCORE_BASE_ADDR| RTL8380_SMI_ACCESS_PHY_CTRL_2_ADDR) & \
                        RTL8380_SMI_ACCESS_PHY_CTRL_2_DATA_15_0_MASK;

	return 0;
} /* end of rtl8380_getPhyReg */


int32 rtl8380_getPortLinkStatus(int portId)
{
    uint32 val;

    val = MEM32_READ(0xbb00a188);
    return (val>>portId)&0x1;
}

/* Function Name:
 *      rtl8380_setPhyRegByMask
 * Description:
 *      Set PHY register by portmask.
 * Input:
 *      port_mask - Port mask
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
int rtl8380_setPhyRegByMask(unsigned long long port_mask, int page, int reg, unsigned int val)
{
    unsigned int value;

    /*check page*/
    page &= 0xFFF;

    /*check reg*/
    reg &= 0x1F;

    /*check vale*/
    val &= 0xFFFF;


    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    while ((MEM32_READ(SWCORE_BASE_ADDR| RTL8380_SMI_ACCESS_PHY_CTRL_1_ADDR) & 0x1) == 0x1);

    /*step0*/
    /* select PHY to access */
    value = port_mask;
    MEM32_WRITE(SWCORE_BASE_ADDR| RTL8380_SMI_ACCESS_PHY_CTRL_0_ADDR, value);

    OSAL_MDELAY(1);

    /*step1*/
    /* RWOP = 1(write), then INDATA[15:0] = DATA[15:0] */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8380_SMI_ACCESS_PHY_CTRL_2_ADDR, \
                        RTL8380_SMI_ACCESS_PHY_CTRL_2_INDATA_15_0_OFFSET, \
                        RTL8380_SMI_ACCESS_PHY_CTRL_2_INDATA_15_0_MASK, val);
    /*step2*/
    /* select register number to access */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8380_SMI_ACCESS_PHY_CTRL_1_ADDR, \
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_REG_ADDR_4_0_OFFSET, \
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_REG_ADDR_4_0_MASK, reg);

    /* select main page number to access */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8380_SMI_ACCESS_PHY_CTRL_1_ADDR, \
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_MAIN_PAGE_11_0_OFFSET,\
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_MAIN_PAGE_11_0_MASK, page);

    /* clear park page */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8380_SMI_ACCESS_PHY_CTRL_1_ADDR, \
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_PARK_PAGE_4_0_OFFSET,\
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_PARK_PAGE_4_0_MASK, 0x1F); /*keep park*/

    /* set PHY register type to normal */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8380_SMI_ACCESS_PHY_CTRL_1_ADDR, \
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_TYPE_OFFSET,\
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_TYPE_MASK, 0);

    /* write operation */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8380_SMI_ACCESS_PHY_CTRL_1_ADDR, \
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_RWOP_OFFSET,\
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_RWOP_MASK, 1);

    /* request MAC to access PHY MII register */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8380_SMI_ACCESS_PHY_CTRL_1_ADDR, \
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_CMD_OFFSET,\
                        RTL8380_SMI_ACCESS_PHY_CTRL_1_CMD_MASK, 1);

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    while ((MEM32_READ(SWCORE_BASE_ADDR| RTL8380_SMI_ACCESS_PHY_CTRL_1_ADDR) & 0x1) == 0x1);

	return 0;
} /* end of rtl8380_setPhyRegByMask */


static void rtl8380_phyCmnPowerON(int mac_id)
{
        unsigned int  reg_val;
        gMacDrv->drv_miim_read(mac_id, 0, 0, &reg_val);
        reg_val &= ~(0x1 << 11);
        gMacDrv->drv_miim_write(mac_id, 0, 0, reg_val);
        return;
}

static void rtl8380_phyCmnPowerOFF(int mac_id)
{
        unsigned int  reg_val;
        gMacDrv->drv_miim_read(mac_id, 0, 0, &reg_val);
        reg_val |= (0x1 << 11);
        gMacDrv->drv_miim_write(mac_id, 0, 0, reg_val);
        return;
}

static void rtl8380_phyCmnReset(int mac_id)
{
        unsigned int  reg_val;
        gMacDrv->drv_miim_read(mac_id, 0, 0, &reg_val);
        reg_val |= (0x1 << 15);
        gMacDrv->drv_miim_write(mac_id, 0, 0, reg_val);
        return;
}

/* Function Name:
 *      rtl8380_phyPowerOn_except_serdes_fiber
 * Description:
 *      Power-On PHY except serdes fiber.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8380_phyPowerOn_except_serdes_fiber(void)
{
    int mac_idx, phy_idx, mac_id;
    unsigned int reg_val;


    if (gSwitchModel == NULL)
        return;

    if (gMacDrv == NULL)
        return;

    /* power-on all ports */
    for (mac_idx=0; mac_idx<gSwitchModel->port.count; mac_idx++)
    {
        mac_id = gSwitchModel->port.list[mac_idx].mac_id;

        phy_idx = gSwitchModel->port.list[mac_idx].phy_idx;

        switch (gSwitchModel->phy.list[phy_idx].chip)
        {
            #if (defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B) || defined(CONFIG_RTL8212B))
            case RTK_CHIP_RTL8214FB:
            case RTK_CHIP_RTL8214B:
            case RTK_CHIP_RTL8212B:
                rtl8214fb_phyPowerOn(mac_id);
                break;
            #endif
            #if (defined(CONFIG_RTL8214FC))
            case RTK_CHIP_RTL8214FC:
                /* fiber */
                gMacDrv->drv_miim_write(mac_id, gMacDrv->miim_max_page, 30, 3);
                gMacDrv->drv_miim_read(mac_id, 0, 16, &reg_val);
                gMacDrv->drv_miim_write(mac_id, 0, 16, reg_val & ~(0x1 << 11));

                /* copper */
                gMacDrv->drv_miim_write(mac_id, gMacDrv->miim_max_page, 30, 1);
                gMacDrv->drv_miim_read(mac_id, 0xa40, 16, &reg_val);
                gMacDrv->drv_miim_write(mac_id, 0xa40, 16, reg_val & ~(0x1 << 11));

                gMacDrv->drv_miim_write(mac_id, gMacDrv->miim_max_page, 30, 0);
                break;
            #endif
            default:
                rtl8380_phyCmnPowerON(mac_id);
                break;
        }
        udelay(20*1000);    /* delay 20mS */
    }
    return;
}/* end of rtl8380_phyPowerOn */


void rtl8380_serdesFiber_powerOnOff(int mac_id, int val)
{
       uint32 serdes_reg0;
       uint32 serdes_offset;
	uint32 serdes_mode;
	   
       uint32 serdes_reg1;
       uint32 serdes_reg2;

       uint32 serdes_reg3;
   
       uint32 serdes_val;

       if((24  != mac_id) && (26  != mac_id))
	   	return;

 	 /*Serdes Mode*/
	 serdes_reg0 = 0xbb000028;
        if (24 == mac_id)
            serdes_offset = 5;
        else
            serdes_offset = 0;

        if (0x1 == val)
            serdes_mode = 0x4;
        else
            serdes_mode = 0x9;

        serdes_val = MEM32_READ(serdes_reg0);
        serdes_val &= ~(0x1f << serdes_offset);
        serdes_val |= (serdes_mode << serdes_offset);
        MEM32_WRITE(serdes_reg0, serdes_val);	


	 /*Serdes Reset*/
        if (24 == mac_id)
        {
            serdes_reg1 = 0xbb00ef80;
            serdes_reg2 = 0xbb00ef8c;
	     serdes_reg3 = 0xbb00f800;
        }
        else
        {
            serdes_reg1 = 0xbb00f180;
            serdes_reg2 = 0xbb00f18c;
	     serdes_reg3 = 0xbb00f900;
        }

        /*Always do this no matter whether Power on of Power off*/
        serdes_val = MEM32_READ(serdes_reg3);
        serdes_val &= ~(0x1 << 11);
        MEM32_WRITE(serdes_reg3, serdes_val);
		
	 /*Analog Reset*/
        serdes_val = MEM32_READ(serdes_reg1);
        serdes_val &= ~(0x3 << 0);
        MEM32_WRITE(serdes_reg1, serdes_val);	
		
        serdes_val = MEM32_READ(serdes_reg1);
        serdes_val &= ~(0x3 << 0);
        serdes_val |= (0x3 << 0);
        MEM32_WRITE(serdes_reg1, serdes_val);		
		
         /*Digital Reset*/
        serdes_val = MEM32_READ(serdes_reg2);
        serdes_val |= (0x1 << 6);
        MEM32_WRITE(serdes_reg2, serdes_val);	
		
        serdes_val = MEM32_READ(serdes_reg2);
        serdes_val &= ~(0x1 << 6);
        MEM32_WRITE(serdes_reg2, serdes_val);		

    return;
}

/* Function Name:
 *      rtl8380_portIdxFromMacId
 * Description:
 *      Get PHY index from MAC port id
 * Input:
 *      macId   - MAC port id
 * Output:
 *      portIdx - port index relates MAC port id
 * Return:
 *      None
 * Note:
 *      None
 */
int rtl8380_portIdxFromMacId(int macId, int *portIdx)
{
    int idx;

    for (idx = 0; idx < gSwitchModel->port.count; ++idx)
    {
        if (gSwitchModel->port.list[idx].mac_id == macId)
        {
            *portIdx = idx;
            return 0;
        }
    }

    return -1;
}

/* Function Name:
 *      rtl8380_phyPortPowerOn
 * Description:
 *      Power-On PHY by port.
 * Input:
 *      int mac_id
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8380_phyPortPowerOn(int mac_id)
{
    int phy_idx, portIdx;
    unsigned int reg_val;


    if (gSwitchModel == NULL)
        return;

    if (gMacDrv == NULL)
        return;

    if (rtl8380_portIdxFromMacId(mac_id, &portIdx) < 0)
        return;

    /* power-on all ports */
    phy_idx = gSwitchModel->port.list[portIdx].phy_idx;

        switch (gSwitchModel->phy.list[phy_idx].chip)
        {
            #if (defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B) || defined(CONFIG_RTL8212B))
            case RTK_CHIP_RTL8214FB:
            case RTK_CHIP_RTL8214B:
            case RTK_CHIP_RTL8212B:
                rtl8214fb_phyPowerOn(mac_id);
                break;
            #endif
            #if (defined(CONFIG_RTL8214FC))
            case RTK_CHIP_RTL8214FC:
                /* fiber */
                gMacDrv->drv_miim_write(mac_id, gMacDrv->miim_max_page, 30, 3);
                gMacDrv->drv_miim_read(mac_id, 0, 16, &reg_val);
                gMacDrv->drv_miim_write(mac_id, 0, 16, reg_val & ~(0x1 << 11));

                /* copper */
                gMacDrv->drv_miim_write(mac_id, gMacDrv->miim_max_page, 30, 1);
                gMacDrv->drv_miim_read(mac_id, 0xa40, 16, &reg_val);
                gMacDrv->drv_miim_write(mac_id, 0xa40, 16, reg_val & ~(0x1 << 11));

                gMacDrv->drv_miim_write(mac_id, gMacDrv->miim_max_page, 30, 0);
                break;
            #endif
            case RTK_CHIP_NONE:		
		  /*Power on*/
		 rtl8380_serdesFiber_powerOnOff(mac_id, 0x1);	
                break;
			
            default:
                rtl8380_phyCmnPowerON(mac_id);
                break;
        }

    return;
}/* end of rtl8380_phyPowerOn */

/* Function Name:
 *      rtl8380_phyPowerOn
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
void rtl8380_phyPowerOn(void)
{
    int mac_id;


    if (gSwitchModel == NULL)
        return;

    if (gMacDrv == NULL)
        return;

    /* power-on all ports */
    for (mac_id=0; mac_id<gSwitchModel->port.count; mac_id++)
    {
        rtl8380_phyPortPowerOn((gSwitchModel->port.list[mac_id].mac_id));
        udelay(20*1000);    /* delay 20mS */
    }
    return;
}/* end of rtl8380_phyPowerOn */


/* Function Name:
 *      rtl8380_phyPowerOff
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
void rtl8380_phyPowerOff(void)
{
    int mac_idx, phy_idx, mac_id;
	unsigned int reg_val;

    if (gSwitchModel == NULL)
        return;

    if (gMacDrv == NULL)
        return;

    MEM32_WRITE(0xBB000140, 0xC);

     /* power-off all ports */
    for (mac_idx=0; mac_idx<gSwitchModel->port.count; mac_idx++)
    {
        mac_id = gSwitchModel->port.list[mac_idx].mac_id;

        phy_idx = gSwitchModel->port.list[mac_idx].phy_idx;

        switch (gSwitchModel->phy.list[phy_idx].chip)
        {
            #if (defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B) || defined(CONFIG_RTL8212B))
            case RTK_CHIP_RTL8214FB:
            case RTK_CHIP_RTL8214B:
            case RTK_CHIP_RTL8212B:
                rtl8214fb_phyPowerOff(mac_id);
                break;
            #endif
            #if (defined(CONFIG_RTL8214FC))
            case RTK_CHIP_RTL8214FC:
                /* fiber */
                gMacDrv->drv_miim_write(mac_id, gMacDrv->miim_max_page, 30, 3);
                gMacDrv->drv_miim_read(mac_id, 0, 16, &reg_val);
                gMacDrv->drv_miim_write(mac_id, 0, 16, reg_val | (0x1 << 11));

                /* copper */
                gMacDrv->drv_miim_write(mac_id, gMacDrv->miim_max_page, 30, 1);
                gMacDrv->drv_miim_read(mac_id, 0xa40, 16, &reg_val);
                gMacDrv->drv_miim_write(mac_id, 0xa40, 16, reg_val | (0x1 << 11));

                gMacDrv->drv_miim_write(mac_id, gMacDrv->miim_max_page, 30, 0);
                break;
            #endif
            case RTK_CHIP_NONE:
		  /*Power off*/
		 rtl8380_serdesFiber_powerOnOff(mac_id, 0);	
                break;
            #if (defined(CONFIG_RTL8208))
            case RTK_CHIP_RTL8208:
            case RTK_CHIP_RTL8208D:
            #endif
            #if (defined(CONFIG_RTL8214))
            case RTK_CHIP_RTL8214:
            #endif
            default:
                rtl8380_phyCmnPowerOFF(mac_id);
                break;
        }
        udelay(20*1000);    /* delay 20mS */
    }
    return;
} /* end of rtl8380_phyPowerOff */

/* Function Name:
 *      rtl8380_phyReset
 * Description:
 *      Reset PHY.
 * Input:
 *      pModel - pointer to switch model of platform
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8380_phyReset(const rtk_switch_model_t *pModel)
{
    int mac_idx, mac_id;

    if (gSwitchModel == NULL)
        return;

    if (gMacDrv == NULL)
        return;

    /*reset all ports */
    for (mac_idx=0; mac_idx<gSwitchModel->port.count; mac_idx++)
    {
        mac_id = gSwitchModel->port.list[mac_idx].mac_id;
        rtl8380_phyCmnReset(mac_id);
    }
    return;
} /* end of rtl8380_phyReset */


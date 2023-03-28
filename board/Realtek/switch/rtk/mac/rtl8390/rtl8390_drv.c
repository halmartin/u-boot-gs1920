/*
 * Copyright(c) Realtek Semiconductor Corporation, 2012
 * All rights reserved.
 *
 * Purpose : Related implementation of the RTL8390 board for U-Boot.
 *
 * Feature : RTL8390 platform
 *
 */


/*
 * Include Files
 */
#include <rtk_type.h>
#include <rtk_reg.h>
#include <rtk_osal.h>
#include <init.h>
#include <common/util.h>
#include <../rtl839x/rtl8390_soc_reg.h>
#include <rtk/mac/rtl8390/rtl8390_swcore_reg.h>
#include <rtk/phy/rtl8214f.h>
#include <rtk/phy/rtl8218b.h>
#include <rtk/drv/gpio/rtl8390_gpio_drv.h>
#include <rtk/drv/gpio/gpio.h>
#include <rtk/drv/gpio/ext_gpio.h>

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
 *      rtl8390_setPhyReg
 * Description:
 *      Set PHY register.
 * Input:
 *      portid - Port number (0~51)
 *      page   - PHY page (0~127)
 *      reg    - PHY register (0~31)
 *      val    - data to write
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
int rtl8390_setPhyReg(int portid, int page, int reg, unsigned int val)
{
    portid += (gSwitchModel->port.offset);
    portid &= 0x3F;
    page &= 0x1FFF;
    reg &= 0x1F;
    val &= 0xFFFF;

    /* select PHY to access */
    MEM32_WRITE(SWCORE_BASE_ADDR| (RTL8390_PHYREG_PORT_CTRL_ADDR(0)), 0);
    MEM32_WRITE(SWCORE_BASE_ADDR| (RTL8390_PHYREG_PORT_CTRL_ADDR(0)+4), 0);
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_PHYREG_PORT_CTRL_ADDR(portid), \
        RTL8390_PHYREG_PORT_CTRL_PHYMSK_OFFSET(portid), \
        RTL8390_PHYREG_PORT_CTRL_PHYMSK_MASK(portid), \
        1);

    /* RWOP = 1(write), then INDATA[15:0] = DATA[15:0] */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_PHYREG_DATA_CTRL_ADDR, \
        RTL8390_PHYREG_DATA_CTRL_INDATA_OFFSET, \
        RTL8390_PHYREG_DATA_CTRL_INDATA_MASK, \
        val);

    /* select register number to access */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_PHYREG_ACCESS_CTRL_ADDR, \
        RTL8390_PHYREG_ACCESS_CTRL_REG_OFFSET, \
        RTL8390_PHYREG_ACCESS_CTRL_REG_MASK, \
        reg);

    /* select main page number to access */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_PHYREG_ACCESS_CTRL_ADDR, \
        RTL8390_PHYREG_ACCESS_CTRL_MAIN_PAGE_OFFSET, \
        RTL8390_PHYREG_ACCESS_CTRL_MAIN_PAGE_MASK, \
        page);


    /* park page */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_PHYREG_ACCESS_CTRL_ADDR, \
        RTL8390_PHYREG_ACCESS_CTRL_PARK_PAGE_OFFSET, \
        RTL8390_PHYREG_ACCESS_CTRL_PARK_PAGE_MASK, \
        (page == 0x1FFF)? 0x1F : 0);

    /* don't change extension page */
    MEM32_WRITE(SWCORE_BASE_ADDR| RTL8390_PHYREG_CTRL_ADDR, 0x1FF);

    /* set PHY register type to normal */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_PHYREG_ACCESS_CTRL_ADDR, \
        RTL8390_PHYREG_ACCESS_CTRL_TYPE_OFFSET, \
        RTL8390_PHYREG_ACCESS_CTRL_TYPE_MASK, \
        0);

    /* write operation */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_PHYREG_ACCESS_CTRL_ADDR, \
        RTL8390_PHYREG_ACCESS_CTRL_RWOP_OFFSET, \
        RTL8390_PHYREG_ACCESS_CTRL_RWOP_MASK, \
        1);

    /* disable broadcast operation */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_PHYREG_ACCESS_CTRL_ADDR, \
        RTL8390_PHYREG_ACCESS_CTRL_BROADCAST_OFFSET, \
        RTL8390_PHYREG_ACCESS_CTRL_BROADCAST_MASK, \
        0);

    /* request MAC to access PHY MII register */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_PHYREG_ACCESS_CTRL_ADDR, \
        RTL8390_PHYREG_ACCESS_CTRL_CMD_OFFSET, \
        RTL8390_PHYREG_ACCESS_CTRL_CMD_MASK, \
        1);

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    while ((MEM32_READ(SWCORE_BASE_ADDR| RTL8390_PHYREG_ACCESS_CTRL_ADDR) & 0x1) == 0x1);

    if ((MEM32_READ(SWCORE_BASE_ADDR| RTL8390_PHYREG_ACCESS_CTRL_ADDR) & 0x2) != 0)
    {
        printf("[Err] setPhyReg port %u page %u reg %u val 0x%04x fail\n", portid, page, reg, val);
        /* clear the fail bit */
        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_PHYREG_ACCESS_CTRL_ADDR, \
            RTL8390_PHYREG_ACCESS_CTRL_FAIL_OFFSET, \
            RTL8390_PHYREG_ACCESS_CTRL_FAIL_MASK, \
            1);
    }

    return 0;
} /* end of rtl8390_setPhyReg */

int32 rtl8390_getPortLinkStatus(int portId)
{
    uint32 val;
    uint32 bitOffset;

    val = MEM32_READ(0xbb000390 + (portId>>5)*0x4);
    bitOffset = portId%32;

    return (val>>bitOffset)&0x1;
}

/* Function Name:
 *      rtl8390_getPhyReg
 * Description:
 *      Get PHY register.
 * Input:
 *      portid - Port number (0~51)
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
int rtl8390_getPhyReg(int portid, int page, int reg, unsigned int *val)
{
    portid += (gSwitchModel->port.offset);
    portid &= 0x3F;
    page &= 0x1FFF;
    reg &= 0x1F;

    /* INDATA[5:0] is the PHY address when RWOP = 0b0 */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_PHYREG_DATA_CTRL_ADDR, RTL8390_PHYREG_DATA_CTRL_INDATA_OFFSET, \
                        RTL8390_PHYREG_DATA_CTRL_INDATA_MASK, portid);

    /* select register number to access */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_PHYREG_ACCESS_CTRL_ADDR, RTL8390_PHYREG_ACCESS_CTRL_REG_OFFSET, \
                        RTL8390_PHYREG_ACCESS_CTRL_REG_MASK, reg);

    /* select main page number to access */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_PHYREG_ACCESS_CTRL_ADDR, RTL8390_PHYREG_ACCESS_CTRL_MAIN_PAGE_OFFSET, \
                        RTL8390_PHYREG_ACCESS_CTRL_MAIN_PAGE_MASK, page);


    /* park page */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_PHYREG_ACCESS_CTRL_ADDR, RTL8390_PHYREG_ACCESS_CTRL_PARK_PAGE_OFFSET, \
                        RTL8390_PHYREG_ACCESS_CTRL_PARK_PAGE_MASK, (page == 0x1FFF)? 0x1F : 0);

    /* don't change extension page */
    MEM32_WRITE(SWCORE_BASE_ADDR| RTL8390_PHYREG_CTRL_ADDR, 0x1FF);

    /* set PHY register type to normal */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_PHYREG_ACCESS_CTRL_ADDR, RTL8390_PHYREG_ACCESS_CTRL_TYPE_OFFSET, \
                        RTL8390_PHYREG_ACCESS_CTRL_TYPE_MASK, 0);

    /* read operation */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_PHYREG_ACCESS_CTRL_ADDR, RTL8390_PHYREG_ACCESS_CTRL_RWOP_OFFSET, \
                        RTL8390_PHYREG_ACCESS_CTRL_RWOP_MASK, 0);

    /* disable broadcast operation */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_PHYREG_ACCESS_CTRL_ADDR, RTL8390_PHYREG_ACCESS_CTRL_BROADCAST_OFFSET, \
                        RTL8390_PHYREG_ACCESS_CTRL_BROADCAST_MASK, 0);

    /* request MAC to access PHY MII register */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_PHYREG_ACCESS_CTRL_ADDR, RTL8390_PHYREG_ACCESS_CTRL_CMD_OFFSET, \
                        RTL8390_PHYREG_ACCESS_CTRL_CMD_MASK, 1);

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    while ((MEM32_READ(SWCORE_BASE_ADDR| RTL8390_PHYREG_ACCESS_CTRL_ADDR) & 0x1) == 0x1);

    /* get the read result */
    *val = MEM32_READ(SWCORE_BASE_ADDR| RTL8390_PHYREG_DATA_CTRL_ADDR) & RTL8390_PHYREG_DATA_CTRL_DATA_MASK;

    return 0;
} /* end of rtl8390_getPhyReg */

/* Function Name:
 *      rtl8390_setPhyRegByMask
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
int rtl8390_setPhyRegByMask(unsigned long long port_mask, int page, int reg, unsigned int val)
{
    unsigned int pm0 = (&port_mask)[0];
    unsigned int pm1 = (&port_mask)[1] & 0xFFFFF;

    page &= 0x1FFF;
    reg &= 0x1F;
    val &= 0xFFFF;

    //printf("DBG: page = 0x%04X, reg = 0x%04X, val = 0x%04X\n", page, reg, val);
    //printf("DBG: port_mask = 0x%08X %08X\n", *(((unsigned int *)&port_mask) + 0), *(((unsigned int *)&port_mask) + 1));

    /* RWOP = 1(write), then INDATA[15:0] = DATA[15:0] */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_PHYREG_DATA_CTRL_ADDR, \
        RTL8390_PHYREG_DATA_CTRL_INDATA_OFFSET, \
        RTL8390_PHYREG_DATA_CTRL_INDATA_MASK, \
        val);

    /* select register number to access */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_PHYREG_ACCESS_CTRL_ADDR, \
        RTL8390_PHYREG_ACCESS_CTRL_REG_OFFSET, \
        RTL8390_PHYREG_ACCESS_CTRL_REG_MASK, \
        reg);

    /* select main page number to access */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_PHYREG_ACCESS_CTRL_ADDR, \
        RTL8390_PHYREG_ACCESS_CTRL_MAIN_PAGE_OFFSET, \
        RTL8390_PHYREG_ACCESS_CTRL_MAIN_PAGE_MASK, \
        page);

    /* park page */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_PHYREG_ACCESS_CTRL_ADDR, \
        RTL8390_PHYREG_ACCESS_CTRL_PARK_PAGE_OFFSET, \
        RTL8390_PHYREG_ACCESS_CTRL_PARK_PAGE_MASK, \
        (page == 0x1FFF)? 0x1F : 0);

    /* don't change extension page */
    MEM32_WRITE(SWCORE_BASE_ADDR| RTL8390_PHYREG_CTRL_ADDR, 0x1FF);

    /* set PHY register type to normal */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_PHYREG_ACCESS_CTRL_ADDR, \
        RTL8390_PHYREG_ACCESS_CTRL_TYPE_OFFSET, \
        RTL8390_PHYREG_ACCESS_CTRL_TYPE_MASK, \
        0);

    /* write operation */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_PHYREG_ACCESS_CTRL_ADDR, \
        RTL8390_PHYREG_ACCESS_CTRL_RWOP_OFFSET, \
        RTL8390_PHYREG_ACCESS_CTRL_RWOP_MASK, \
        1);

    /* disable broadcast operation */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_PHYREG_ACCESS_CTRL_ADDR, \
        RTL8390_PHYREG_ACCESS_CTRL_BROADCAST_OFFSET, \
        RTL8390_PHYREG_ACCESS_CTRL_BROADCAST_MASK, \
        0);

    /* select PHY to access */
    MEM32_WRITE(SWCORE_BASE_ADDR| (RTL8390_PHYREG_PORT_CTRL_ADDR(0)), 0);
    MEM32_WRITE(SWCORE_BASE_ADDR| (RTL8390_PHYREG_PORT_CTRL_ADDR(0)+4), 0);

    MEM32_WRITE(SWCORE_BASE_ADDR| (RTL8390_PHYREG_PORT_CTRL_ADDR(0)), pm0);
    MEM32_WRITE(SWCORE_BASE_ADDR| (RTL8390_PHYREG_PORT_CTRL_ADDR(0)+4), pm1);

#if 0
    printf("DBG: RTL8390_PHYREG_PORT_CTRL_ADDR = 0x%08X 0x%08X\n", \
        MEM32_READ(SWCORE_BASE_ADDR| (RTL8390_PHYREG_PORT_CTRL_ADDR(0))), \
        MEM32_READ(SWCORE_BASE_ADDR| (RTL8390_PHYREG_PORT_CTRL_ADDR(0)+4)));
#endif

    /* request MAC to access PHY MII register */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_PHYREG_ACCESS_CTRL_ADDR, \
        RTL8390_PHYREG_ACCESS_CTRL_CMD_OFFSET, \
        RTL8390_PHYREG_ACCESS_CTRL_CMD_MASK, \
        1);

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    while ((MEM32_READ(SWCORE_BASE_ADDR| RTL8390_PHYREG_ACCESS_CTRL_ADDR) & 0x1) == 0x1);

    if ((MEM32_READ(SWCORE_BASE_ADDR| RTL8390_PHYREG_ACCESS_CTRL_ADDR) & 0x2) != 0)
    {
        printf("[Err] setPhyRegByMask * page %u reg %u val 0x%04x fail\n", page, reg, val);
        /* clear the fail bit */
        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_PHYREG_ACCESS_CTRL_ADDR, \
            RTL8390_PHYREG_ACCESS_CTRL_FAIL_OFFSET, \
            RTL8390_PHYREG_ACCESS_CTRL_FAIL_MASK, \
            1);
    }

    return 0;
} /* end of rtl8390_setPhyRegByMask */

/* Function Name:
 *      rtl8390_portIdxFromMacId
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
int rtl8390_portIdxFromMacId(int macId, int *portIdx)
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
 *      rtl8390_phyPowerOn
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
void rtl8390_phyPortPowerOn(int portId)
{
    int macId, portIdx;
    int type = 0;
    int reg;
    unsigned int val;

    if (gSwitchModel == NULL)
        return;

    if (gMacDrv == NULL)
        return;

    macId = portId;

    if (rtk_portIdxFromMacId(macId, &portIdx) < 0)
        return;

    switch (gSwitchModel->phy.list[gSwitchModel->port.list[portIdx].phy_idx].chip)
    {
        #if (defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B) || defined(CONFIG_RTL8212B))
        case RTK_CHIP_RTL8214FB:
        case RTK_CHIP_RTL8214B:
        case RTK_CHIP_RTL8212B:
            rtl8214fb_phyPowerOn(macId);
            return;
        #endif
        #if (defined(CONFIG_RTL8218FB))
        case RTK_CHIP_RTL8218FB:
            if (0 == gSwitchModel->port.list[portIdx].phy / 4)
                type = 0;
            else
                type = 1;
            break;
        #endif
        #if (defined(CONFIG_RTL8214FC))
        case RTK_CHIP_RTL8214FC:
            type = 1;
            break;
        #endif
        case RTK_CHIP_NONE:
            if (49 == macId)
            {
                reg = 0xbb00b880;
            }
            else
            {
                reg = 0xbb00b980;
            }

            val = MEM32_READ(reg);
            val &= ~(1 << 11);
            MEM32_WRITE(reg, val);
            return;
        default:
            type = 0;
    }

    switch (type)
    {
        case 0:
            gMacDrv->drv_miim_read(macId, 0, 0, &val);
            gMacDrv->drv_miim_write(macId, 0, 0, val & ~(0x1 << 11));
            break;
        case 1:
            /* copper */
            gMacDrv->drv_miim_write(macId, gMacDrv->miim_max_page, 29, 1);
            gMacDrv->drv_miim_read(macId, 0xa40, 16, &val);
            gMacDrv->drv_miim_write(macId, 0xa40, 16, val & ~(0x1 << 11));
            /* fiber */
            gMacDrv->drv_miim_write(macId, gMacDrv->miim_max_page, 29, 3);
            gMacDrv->drv_miim_read(macId, 0, 16, &val);
            gMacDrv->drv_miim_write(macId, 0, 16, val & ~(0x1 << 11));

            gMacDrv->drv_miim_write(macId, gMacDrv->miim_max_page, 29, 0);
            break;
    }

    return;
}   /* end of rtl8390_phyPortPowerOn */

/* Function Name:
 *      rtl8390_phyPowerOn
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
void rtl8390_phyPowerOn(void)
{
    int phyIdx, portId, basePort;
    int macId, reg;
    unsigned int val;

    if (gSwitchModel == NULL)
        return;

    if (gMacDrv == NULL)
        return;

    for (phyIdx = 0; phyIdx < gSwitchModel->phy.count; ++phyIdx)
    {   /* power-on all ports */
        basePort = gSwitchModel->phy.list[phyIdx].mac_id;
        switch (gSwitchModel->phy.list[phyIdx].chip)
        {
            #if (defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B))
            case RTK_CHIP_RTL8214FB:
            case RTK_CHIP_RTL8214B:
                basePort = basePort - (basePort % 4);
                for (portId = 0; portId < 4; ++portId)
                    rtl8214fb_phyPowerOn(basePort + portId);
                break;
            #endif
            #if (defined(CONFIG_RTL8212B))
            case RTK_CHIP_RTL8212B:
                basePort = basePort - (basePort % 2);
                for (portId = 0; portId < 2; ++portId)
                    rtl8214fb_phyPowerOn(basePort + portId);
                break;
            #endif
            #if (defined(CONFIG_RTL8218FB))
            case RTK_CHIP_RTL8218FB:
                basePort = basePort - (basePort % 8);
                for (portId = 0; portId < 8; ++portId)
                {
                    macId = basePort + portId;
                    if (portId < 4)
                    {
                        gMacDrv->drv_miim_read(macId, 0, 0, &val);
                        gMacDrv->drv_miim_write(macId, 0, 0, val & ~(0x1 << 11));
                    }
                    else
                    {
                        /* copper */
                        gMacDrv->drv_miim_write(macId, gMacDrv->miim_max_page, 29, 1);
                        gMacDrv->drv_miim_read(macId, 0xa40, 16, &val);
                        gMacDrv->drv_miim_write(macId, 0xa40, 16, val & ~(0x1 << 11));
                        /* fiber */
                        gMacDrv->drv_miim_write(macId, gMacDrv->miim_max_page, 29, 3);
                        gMacDrv->drv_miim_read(macId, 0, 16, &val);
                        gMacDrv->drv_miim_write(macId, 0, 16, val & ~(0x1 << 11));

                        gMacDrv->drv_miim_write(macId, gMacDrv->miim_max_page, 29, 0);
                    }
                }
                break;
            #endif
            #if (defined(CONFIG_RTL8214FC))
            case RTK_CHIP_RTL8214FC:
                basePort = basePort - (basePort % 4);
                for (portId = 0; portId < 4; ++portId)
                {
                    macId = basePort + portId;
                    /* copper */
                    gMacDrv->drv_miim_write(macId, gMacDrv->miim_max_page, 29, 1);
                    gMacDrv->drv_miim_read(macId, 0xa40, 16, &val);
                    gMacDrv->drv_miim_write(macId, 0xa40, 16, val & ~(0x1 << 11));
                    /* fiber */
                    gMacDrv->drv_miim_write(macId, gMacDrv->miim_max_page, 29, 3);
                    gMacDrv->drv_miim_read(macId, 0, 16, &val);
                    gMacDrv->drv_miim_write(macId, 0, 16, val & ~(0x1 << 11));

                    gMacDrv->drv_miim_write(macId, gMacDrv->miim_max_page, 29, 0);
                }
                break;
            #endif
            case RTK_CHIP_NONE:
                for (portId = 0; portId < 2; ++portId)
                {
                    macId = basePort + portId;
                    if (49 == macId)
                        reg = 0xbb00b880;
                    else
                        reg = 0xbb00b980;

                    val = MEM32_READ(reg);
                    val &= ~(1 << 11);
                    MEM32_WRITE(reg, val);
                }
                break;
            default:
                basePort = basePort - (basePort % 8);
                for (portId = 0; portId < 8; ++portId)
                {
                    macId = basePort + portId;
                    gMacDrv->drv_miim_read(macId, 0, 0, &val);
                    gMacDrv->drv_miim_write(macId, 0, 0, val & ~(0x1 << 11));
                }
                break;
        }
    }

    return;
} /* end of rtl8390_phyPowerOn */

/* Function Name:
 *      rtl8390_phyPortPowerOff
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
void rtl8390_phyPortPowerOff(int portId)
{
    int macId, portIdx;
    int type = 0;
    int reg;
    unsigned int val;

    if (gSwitchModel == NULL)
        return;

    if (gMacDrv == NULL)
        return;

    macId = portId;

    if (rtk_portIdxFromMacId(macId, &portIdx) < 0)
        return;

    switch (gSwitchModel->phy.list[gSwitchModel->port.list[portIdx].phy_idx].chip)
    {
        #if (defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B) || defined(CONFIG_RTL8212B))
        case RTK_CHIP_RTL8214FB:
        case RTK_CHIP_RTL8214B:
        case RTK_CHIP_RTL8212B:
            rtl8214fb_phyPowerOff(macId);
            return;
        #endif
        #if (defined(CONFIG_RTL8218FB))
        case RTK_CHIP_RTL8218FB:
            if (0 == gSwitchModel->port.list[portIdx].phy / 4)
                type = 0;
            else
                type = 1;
            break;
        #endif
        #if (defined(CONFIG_RTL8214FC))
        case RTK_CHIP_RTL8214FC:
            type = 1;
            break;
        #endif
        case RTK_CHIP_NONE:
            if (49 == macId)
            {
                reg = 0xbb00b980;
            }
            else
            {
                reg = 0xbb00b880;
            }

            val = MEM32_READ(reg);
            val |= (1 << 11);
            MEM32_WRITE(reg, val);
            return;
        default:
            type = 0;
    }

    switch (type)
    {
        case 0:
            gMacDrv->drv_miim_read(macId, 0, 0, &val);
            gMacDrv->drv_miim_write(macId, 0, 0, val | (0x1 << 11));
            break;
        case 1:
            /* copper */
            gMacDrv->drv_miim_write(macId, gMacDrv->miim_max_page, 29, 1);
            gMacDrv->drv_miim_read(macId, 0xa40, 16, &val);
            gMacDrv->drv_miim_write(macId, 0xa40, 16, val | (0x1 << 11));
            /* fiber */
            gMacDrv->drv_miim_write(macId, gMacDrv->miim_max_page, 29, 3);
            gMacDrv->drv_miim_read(macId, 0, 16, &val);
            gMacDrv->drv_miim_write(macId, 0, 16, val | (0x1 << 11));

            gMacDrv->drv_miim_write(macId, gMacDrv->miim_max_page, 29, 0);
            break;
    }

    return;
}   /* end of rtl8390_phyPortPowerOff */

/* Function Name:
 *      rtl8390_phyPowerOff
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
void rtl8390_phyPowerOff(void)
{
    int phyIdx, portId, basePort;
    int macId, reg;
    unsigned int val;

    if (gSwitchModel == NULL)
        return;

    if (gMacDrv == NULL)
        return;

    for (phyIdx = 0; phyIdx < gSwitchModel->phy.count; ++phyIdx)
    {   /* power-on all ports */
        basePort = gSwitchModel->phy.list[phyIdx].mac_id;
        switch (gSwitchModel->phy.list[phyIdx].chip)
        {
            #if (defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B))
            case RTK_CHIP_RTL8214FB:
            case RTK_CHIP_RTL8214B:
                basePort = basePort - (basePort % 4);
                for (portId = 0; portId < 4; ++portId)
                    rtl8214fb_phyPowerOff(basePort + portId);
                break;
            #endif
            #if (defined(CONFIG_RTL8212B))
            case RTK_CHIP_RTL8212B:
                basePort = basePort - (basePort % 2);
                for (portId = 0; portId < 2; ++portId)
                    rtl8214fb_phyPowerOff(basePort + portId);
                break;
            #endif
            #if (defined(CONFIG_RTL8218FB))
            case RTK_CHIP_RTL8218FB:
                basePort = basePort - (basePort % 8);
                for (portId = 0; portId < 8; ++portId)
                {
                    macId = basePort + portId;
                    if (portId < 4)
                    {
                        gMacDrv->drv_miim_read(macId, 0, 0, &val);
                        gMacDrv->drv_miim_write(macId, 0, 0, val | (0x1 << 11));
                    }
                    else
                    {
                        /* copper */
                        gMacDrv->drv_miim_write(macId, gMacDrv->miim_max_page, 29, 1);
                        gMacDrv->drv_miim_read(macId, 0xa40, 16, &val);
                        gMacDrv->drv_miim_write(macId, 0xa40, 16, val | (0x1 << 11));
                        /* fiber */
                        gMacDrv->drv_miim_write(macId, gMacDrv->miim_max_page, 29, 3);
                        gMacDrv->drv_miim_read(macId, 0, 16, &val);
                        gMacDrv->drv_miim_write(macId, 0, 16, val | (0x1 << 11));

                        gMacDrv->drv_miim_write(macId, gMacDrv->miim_max_page, 29, 0);
                    }
                }
                break;
            #endif
            #if (defined(CONFIG_RTL8214FC))
            case RTK_CHIP_RTL8214FC:
                basePort = basePort - (basePort % 4);
                for (portId = 0; portId < 4; ++portId)
                {
                    macId = basePort + portId;
                    /* copper */
                    gMacDrv->drv_miim_write(macId, gMacDrv->miim_max_page, 29, 1);
                    gMacDrv->drv_miim_read(macId, 0xa40, 16, &val);
                    gMacDrv->drv_miim_write(macId, 0xa40, 16, val | (0x1 << 11));
                    /* fiber */
                    gMacDrv->drv_miim_write(macId, gMacDrv->miim_max_page, 29, 3);
                    gMacDrv->drv_miim_read(macId, 0, 16, &val);
                    gMacDrv->drv_miim_write(macId, 0, 16, val | (0x1 << 11));

                    gMacDrv->drv_miim_write(macId, gMacDrv->miim_max_page, 29, 0);
                }
                break;
            #endif
            case RTK_CHIP_NONE:
                for (portId = 0; portId < 2; ++portId)
                {
                    macId = basePort + portId;
                    if (49 == macId)
                        reg = 0xbb00b880;
                    else
                        reg = 0xbb00b980;

                    val = MEM32_READ(reg);
                    val |= (1 << 11);
                    MEM32_WRITE(reg, val);
                }
                break;
            default:
                basePort = basePort - (basePort % 8);
                for (portId = 0; portId < 8; ++portId)
                {
                    macId = basePort + portId;
                    gMacDrv->drv_miim_read(macId, 0, 0, &val);
                    gMacDrv->drv_miim_write(macId, 0, 0, val | (0x1 << 11));
                }
                break;
        }
    }

    return;
} /* end of rtl8390_phyPowerOff */

/* Function Name:
 *      rtl8390_phyReset
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
void rtl8390_phyReset(const rtk_switch_model_t *pModel)
{
    uint32  rstAddr;
    uint32  resetPin;
    uint32  i, gpioFlag = 0;

    switch (pModel->phy.rstGpioType)
    {
        case PHY_RSTGPIOTYPE_INTERNAL:
            resetPin = pModel->phy.rstGpio.rstIntGpio.pin;

            intGpio_PinSet(resetPin,0);
            OSAL_MDELAY(10 * 2);    /* Min 10mS (from Spec) * 2 = 20mS (double for safety) */
            intGpio_PinSet(resetPin,1);
            OSAL_MDELAY(140 * 2);   /* 50mS (power-on waiting) + 90mS (basic procedure) = 140mS */
            break;
        case PHY_RSTGPIOTYPE_EXTERNAL:
            rstAddr = pModel->phy.rstGpio.rstExtGpio.addr;
            resetPin = pModel->phy.rstGpio.rstExtGpio.pin;

            extGpio_pin_dir_set(rstAddr, resetPin, EXT_GPIO_DIR_OUT);
            for (i = 0; i < 0xF; ++i)
            {
                if (1 == extGpio_pin_data_set(rstAddr, resetPin, 0))
                {
                    gpioFlag = 1;
                    break;
                }
            }

            if (0 == gpioFlag)
            {
                printf("PHY reset fail\n");
                return;
            }

            OSAL_MDELAY(10 * 2);    /* Min 10mS (from Spec) * 2 = 20mS (double for safety) */
            gpioFlag = 0;
            for (i = 0; i < 0xF; ++i)
            {
                if (1 == extGpio_pin_data_set(rstAddr, resetPin, 1))
                {
                    gpioFlag = 1;
                    break;
                }
            }

            if (0 == gpioFlag)
            {
                printf("release PHY reset fail\n");
                return;
            }

            OSAL_MDELAY(140 * 2);   /* 50mS (power-on waiting) + 90mS (basic procedure) = 140mS */
            break;
        default:
            return;
    }

    return;
} /* end of rtl8390_phyReset */

/* Function Name:
 *      rtl8390_drv_macPhyPatch1
 * Description:
 *      Rx forcerun reset
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8390_drv_macPhyPatch1(void)
{
    int i, macId;
    #if (defined(CONFIG_RTL8208))
    unsigned int val;
    #endif

    if (gSwitchModel == NULL)
        return;

    for (i = 0; i < gSwitchModel->phy.count; ++i)
    {
        macId = gSwitchModel->phy.list[i].mac_id;

        switch (gSwitchModel->phy.list[i].chip)
        {
            #if (defined(CONFIG_RTL8208))
            case RTK_CHIP_RTL8208D:
            case RTK_CHIP_RTL8208L:
                if (macId == 0 || macId == 8 || macId == 16 || macId == 24 || macId == 40)
                {
                    gMacDrv->drv_miim_read(macId, 64, 16, &val);
                    val &= ~(0x1 << 13);
                    val |= (0x1 << 13);
                    gMacDrv->drv_miim_write(macId, 64, 16, val);
                }
                break;
            #endif
            #if (defined(CONFIG_RTL8218B) || defined(CONFIG_RTL8214FC) || defined(CONFIG_RTL8218FB))
            case RTK_CHIP_RTL8218B:
            case RTK_CHIP_RTL8214FC:
            case RTK_CHIP_RTL8218FB:
                if (macId == 0 || macId == 8 || macId == 16 || macId == 24 || macId == 40)
                {
                    gMacDrv->drv_miim_write(macId, gMacDrv->miim_max_page, 29, 8);
                    gMacDrv->drv_miim_write(macId, 0x467, 0x14, 0x3c);
                }
                break;
            #endif
        }
    }
    return;
} /* end of rtl8390_drv_macPhyPatch1 */

/* Function Name:
 *      rtl8390_drv_macPhyPatch2
 * Description:
 *      Rx forcerun reset
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8390_drv_macPhyPatch2(void)
{
    int i, macId;
    #if (defined(CONFIG_RTL8208))
    unsigned int val;
    #endif

    if (gSwitchModel == NULL)
        return;

    for (i = 0; i < gSwitchModel->phy.count; ++i)
    {
        macId = gSwitchModel->phy.list[i].mac_id;

        switch (gSwitchModel->phy.list[i].chip)
        {
            #if (defined(CONFIG_RTL8208))
            case RTK_CHIP_RTL8208D:
            case RTK_CHIP_RTL8208L:
                if (macId == 0 || macId == 8 || macId == 16 || macId == 24 || macId == 40)
                {
                    gMacDrv->drv_miim_read(macId, 64, 16, &val);
                    val &= ~(0x1 << 13);
                    gMacDrv->drv_miim_write(macId, 64, 16, val);
                }
                break;
            #endif
            #if (defined(CONFIG_RTL8218B) || defined(CONFIG_RTL8214FC) || defined(CONFIG_RTL8218FB))
            case RTK_CHIP_RTL8218B:
            case RTK_CHIP_RTL8214FC:
            case RTK_CHIP_RTL8218FB:
                if (macId == 0 || macId == 8 || macId == 16 || macId == 24 || macId == 40)
                {
                    gMacDrv->drv_miim_write(macId, 0x467, 0x14, 0);
                    gMacDrv->drv_miim_write(macId, gMacDrv->miim_max_page, 29, 0);
                }
                break;
            #endif
        }
    }
    return;
} /* end of rtl8390_drv_macPhyPatch2 */

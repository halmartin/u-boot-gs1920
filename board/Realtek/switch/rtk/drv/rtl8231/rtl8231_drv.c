/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * Purpose : Related definition of RTL8231 driver for U-Boot.
 *
 * Feature : RTL8231 driver function
 *
 */


/*
 * Include Files
 */
#include <config.h>
#include <rtk_osal.h>
#include <rtk_switch.h>
#include <init.h>
#include <rtk/drv/rtl8231/rtl8231_drv.h>
#include <rtk/mac/rtl8380/rtl8380_rtk.h>

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
#if 0
/* Function Name:
 *      rtl83xx_setPhyRegBits
 * Description:
 *      Set PHY register bits.
 * Input:
 *      macid  - MAC id (0~28)
 *      page   - PHY page (0~31)
 *      reg    - PHY register (0~31)
 *      val    - Read data
 *      bits   - Bits to be configured
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
static void rtl83xx_setPhyRegBits(int macid, int page, int reg, int val, int bits)
{
    int valTmp;
    
    gMacDrv->drv_miim_read(macid, page, reg, &valTmp);
    valTmp &= ~bits;
    valTmp |= val&bits;
    gMacDrv->drv_miim_write(macid, page, reg, valTmp);
    return;
}

/* Function Name:
 *      rtl8231_setStartStatus
 * Description:
 *      Configuration start status for RTL8231.
 * Input:
 *      start - start or stop rtl8231
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8231_setStartStatus(Tuint8 start)
{
    int regData;

    /*set MAC polling PHY reg to ADD port 28*/
    REG32(0xbb001d08) |= 0x1 << 28;

    /*Init 8231 to GPIO mode*/
    gMacDrv->drv_miim_read(28, 31, 1, &regData);
   
    while(regData != 0x0371)
    {   
        gMacDrv->drv_miim_read(28, 31, 1, &regData);
    }
    
    gMacDrv->drv_miim_read(28, 31, 0, &regData);
    regData &= ~ (1 <<  1);
    regData |= (start << 1);
    gMacDrv->drv_miim_write(28, 31, 0, regData);
    return;
}

/* Function Name:
 *      rtl8231_setGPIOPin
 * Description:
 *      Configuration pin status for RTL8231.
 * Input:
 *      pinId : pin ID, range from 0 to 36
 *      state : pin state: normal, gpio in or gpio out
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8231_setPinStatus(Tuint8 pinId, rtl8231_pin_state_t state)
{
    if(RTL8231_PIN_NORMAL == state)
    {
        /*disable SEL_GPIO*/
        if(pinId <= 15)
        {
            rtl83xx_setPhyRegBits(28, 31, 2, 0 << pinId, 1 << pinId);
        }
        else if(pinId <= 31)
        {
            rtl83xx_setPhyRegBits(28, 31, 3, 0 << (pinId-16), 1 << (pinId-16));
        }
        else if(pinId <= 36)
        {
            rtl83xx_setPhyRegBits(28, 31, 4, 0 << (pinId-32), 1 << (pinId-32));
        }

        /*remove debouncing*/
        if((pinId >= 31) && (pinId <= 36))
        {
            rtl83xx_setPhyRegBits(28, 31, 1, 0 << (pinId-21), 1 << (pinId-21));
        }
    }
    else if(RTL8231_PIN_GPI == state)
    {
        /*enable SEL_GPIO*/
        if(pinId <= 15)
        {
            rtl83xx_setPhyRegBits(28, 31, 2, 1 << pinId, 1 << pinId);
        }
        else if(pinId <= 31)
        {
            rtl83xx_setPhyRegBits(28, 31, 3, 1 << (pinId-16), 1 << (pinId-16));
        }
        else if(pinId <= 36)
        {
            rtl83xx_setPhyRegBits(28, 31, 4, 1 << (pinId-32), 1 << (pinId-32));
        }

        /*set GPIO_MASK to be GPI*/
        if(pinId <= 15)
        {
            rtl83xx_setPhyRegBits(28, 31, 5, 1 << pinId, 1 << pinId);
        }
        else if(pinId <= 31)
        {
            rtl83xx_setPhyRegBits(28, 31, 6, 1 << (pinId-16), 1 << (pinId-16));
        }
        else if(pinId <= 36)
        {
            rtl83xx_setPhyRegBits(28, 31, 4, 1 << (pinId-27), 1 << (pinId-27));
        }
    }
    else if(RTL8231_PIN_GPO == state)
    {
        /*enable SEL_GPIO*/
        if(pinId <= 15)
        {
            rtl83xx_setPhyRegBits(28, 31, 2, 1 << pinId, 1 << pinId);
        }
        else if(pinId <= 31)
        {
            rtl83xx_setPhyRegBits(28, 31, 3, 1 << (pinId-16), 1 << (pinId-16));
        }
        else if(pinId <= 36)
        {
            rtl83xx_setPhyRegBits(28, 31, 4, 1 << (pinId-32), 1 << (pinId-32));
        }

        /*set GPIO_MASK to be GPO*/
        if(pinId <= 15)
        {
            rtl83xx_setPhyRegBits(28, 31, 5, 0 << pinId, 1 << pinId);
        }
        else if(pinId <= 31)
        {
            rtl83xx_setPhyRegBits(28, 31, 6, 0 << (pinId-16), 1 << (pinId-16));
        }
        else if(pinId <= 36)
        {
            rtl83xx_setPhyRegBits(28, 31, 4, 0 << (pinId-27), 1 << (pinId-27));
        }
    }
    return;
}

/* Function Name:
 *      rtl8231_setPinDeBouncing
 * Description:
 *      Enable/Disable pin debouncing for RTL8231.
 * Input:
 *      pinId - pin ID, range from 31 to 36
 *      enable - debouncing enable status
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
int rtl8231_setPinDeBouncing(Tuint8 pinId, Tuint8 enable)
{
    if(pinId < 31 || pinId > 36)
        return -1;
    
    rtl83xx_setPhyRegBits(28, 31, 0, enable << (pinId-21), 1 << (pinId-21));
    
    return 0;
}
#endif

/* Function Name:
 *      rtl8231_pin_direction_set
 * Description:
 *      set 8231 pin direction.
 * Input:
 *      pinId - pin ID, range from 0 to 36
 *      dir - rtl8231_PIN_DIR_IN or rtl8231_PIN_DIR_OUT
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8231_pin_direction_set(rtl8231_pin_t pin, rtl8231_pin_direction_t dir)
{
    uint32 val;

    rtk_smiRead(RTL8231_ADDR, RTL8231_PIN_DIR_REG, &val);
    if(dir == rtl8231_PIN_DIR_IN)
        val |= (1<<pin);
    else
        val &= (~(1<<pin));
    rtk_smiWrite(RTL8231_ADDR, RTL8231_PIN_DIR_REG, val);
}

/* Function Name:
 *      rtl8231_pin_data_set
 * Description:
 *      set 8231 pin status.
 * Input:
 *      pinId - pin ID, range from 0 to 36
 *      data - set value
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8231_pin_data_set(rtl8231_pin_t pin, uint32 data)
{
    uint32 val;

    rtk_smiRead(RTL8231_ADDR, RTL8231_PIN_DATA_REG, &val);
    if(data==0)
        val &= (~(1<<pin));
    else
        val |= (1<<pin);
    rtk_smiWrite(RTL8231_ADDR, RTL8231_PIN_DATA_REG, val);
}

/* Function Name:
 *      rtl8231_pin_data_get
 * Description:
 *      get rtl8231 pin status.
 * Input:
 *      pinId - pin ID, range from 0 to 36
 *      *pData - result
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8231_pin_data_get(rtl8231_pin_t pin, uint32 *pData)
{
    uint32 val;

    rtk_smiRead(RTL8231_ADDR, RTL8231_PIN_DATA_REG, &val);
    if((1<<pin) & val)
        *pData = 1;
    else
        *pData = 0;
}



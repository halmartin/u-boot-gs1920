/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 30020 $
 * $Date: 2012-06-18 15:09:58 +0800 (Mon, 18 Jun 2012) $
 *
 * Purpose : Definition those public APIs to acess RTL8231 from RTL8390 family.
 *
 * Feature : The file have include the following module and sub-modules
 *            1) mdc read & write
 */

#include <rtk_type.h>
#include <rtk_debug.h>
#include <rtk_reg.h>
#include <rtk/mac/rtl8390/rtl8390_swcore_reg.h>
#include <rtk/drv/gpio/gpio.h>
#include <rtk/drv/gpio/ext_gpio.h>


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
 * Macro Definition
 */
#define EXT_GPIO_EN_OFFSET  18
#define EXT_GPIO_EN_MASK    (0X7 << EXT_GPIO_EN_OFFSET)

#define GPIO_READ       0
#define GPIO_WRITE      1

#define CHECKBUSY_TIMES (150000)

#define RTL8231_BUSY_WAIT_LOOP(REG, MASK)\
{\
    uint32 i;\
    uint32 regVal;\
    for (i = 0; i < CHECKBUSY_TIMES; i++)\
    {\
        regVal = MEM32_READ(REG);\
        if (0 == (regVal & MASK))\
        {\
            break;\
        }\
    }\
    if (CHECKBUSY_TIMES == i)\
    {\
        return 0;\
    }\
}


/*
 * Function Declaration
 */
void rtl8390_rtl8231_init(void)
{
    int i;

    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR | 0xE4, EXT_GPIO_EN_OFFSET, EXT_GPIO_EN_MASK, 0x4);

    for (i = 0; i <= 36; ++i)
        extGpio_pin_dir_set(3, i, EXT_GPIO_DIR_IN);

    extGpio_pin_dir_set(3, 6, EXT_GPIO_DIR_OUT);
    extGpio_pin_data_set(3, 6, 1);
    extGpio_pin_dir_set(3, 8, EXT_GPIO_DIR_OUT);

    extGpio_pin_dir_set(3, 13, EXT_GPIO_DIR_OUT);   /*PoE disable pin, extPin13=0, active*/
    extGpio_pin_data_set(3, 13, 0);

    extGpio_pin_dir_set(3, 17, EXT_GPIO_DIR_OUT);   /* extPin17 output, low*/
    extGpio_pin_data_set(3, 17, 0);

    extGpio_pin_dir_set(3, 23, EXT_GPIO_DIR_OUT);
    extGpio_pin_data_set(3, 23, 0);
    extGpio_pin_dir_set(3, 29, EXT_GPIO_DIR_OUT);
    extGpio_pin_data_set(3, 29, 0);

    extGpio_dev_enable(3, 1);
}

int rtl8390_rtl8231_read(uint32 phy_id, uint32 reg_addr, uint32 *pData)
{
    uint32  temp;

    /* initialize variable */
    temp = 0;

    /* Input parameters:
     * If RWOP = 0(read), then INDATA[15:0] = {Reserved & PORT_ID[4:0]}
     * If RWOP = 1(write), then INDATA[15:0] = DATA[15:0]
     */
    temp |= ((phy_id << RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_PHY_ADDR_OFFSET) & RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_PHY_ADDR_MASK);

    /* Select register number to access */
    temp |= ((reg_addr << RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_REG_OFFSET) & RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_REG_MASK);

    /* Read/Write operation
     * 0b0: read
     * 0b1: write
     */
    temp |= ((GPIO_READ << RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_RWOP_OFFSET) & RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_RWOP_MASK);

    /* Request MAC to access PHY MII register
     * 0b0: complete access
     * 0b1: execute access
     * Note: When MAC completes access, it will clear this bit.
     */
    temp |= ((1 << RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_CMD_OFFSET) & RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_CMD_MASK);

    /* write register to active the read operation */
    MEM32_WRITE(SWCORE_BASE_ADDR | RTL8390_EXT_GPIO_INDRT_ACCESS_ADDR, temp);

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    RTL8231_BUSY_WAIT_LOOP(SWCORE_BASE_ADDR | RTL8390_EXT_GPIO_INDRT_ACCESS_ADDR, 0x1);

    /* get the read operation result to temp */
    temp = MEM32_READ(SWCORE_BASE_ADDR | RTL8390_EXT_GPIO_INDRT_ACCESS_ADDR);

    /* fill the DATA[15:0] from temp to pData */
    (*pData) = (temp & RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_DATA_MASK) >> RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_DATA_OFFSET;

    return 1;
}

int rtl8390_rtl8231_write(uint32 phy_id, uint32 reg_addr, uint32 data)
{
    uint32  temp;


    /* initialize variable */
    temp = 0;

    /* Input parameters:
     * If RWOP = 0(read), then INDATA[15:0] = {Reserved & PORT_ID[4:0]}
     * If RWOP = 1(write), then INDATA[15:0] = DATA[15:0]
     */
    temp |= ((phy_id << RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_PHY_ADDR_OFFSET) & RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_PHY_ADDR_MASK);

    /* Select register number to access */
    temp |= ((reg_addr << RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_REG_OFFSET) & RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_REG_MASK);

    /* Read/Write operation
     * 0b0: read
     * 0b1: write
     */
    temp |= ((GPIO_WRITE << RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_RWOP_OFFSET) & RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_RWOP_MASK);

    /* Input parameters:
     * If RWOP = 0(read), then GPIO_DATA[15:0] = input data[15:0]
     * If RWOP = 1(write), then GPIO_DATA [15:0] = output data[15:0]
     */
    temp |= ((data << RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_DATA_OFFSET) & RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_DATA_MASK);

    /* Request MAC to access PHY MII register
     * 0b0: complete access
     * 0b1: execute access
     * Note: When MAC completes access, it will clear this bit.
     */
    temp |= ((1 << RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_CMD_OFFSET) & RTL8390_EXT_GPIO_INDRT_ACCESS_GPIO_CMD_MASK);

    /* write register to active the read operation */
    MEM32_WRITE(SWCORE_BASE_ADDR | RTL8390_EXT_GPIO_INDRT_ACCESS_ADDR, temp);

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    RTL8231_BUSY_WAIT_LOOP(SWCORE_BASE_ADDR | RTL8390_EXT_GPIO_INDRT_ACCESS_ADDR, 0x1);

    return 1;
}


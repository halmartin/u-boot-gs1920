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
 * Purpose : Definition those public APIs to acess RTL8231 from RTL8380 family.
 *
 * Feature : The file have include the following module and sub-modules
 *            1) mdc read & write
 */

#include <rtk_type.h>
#include <rtk_debug.h>
#include <rtk/drv/gpio/gpio.h>
#include <rtk_reg.h>
#include <rtk/mac/rtl8380/rtl8380_swcore_reg.h>

#include <config.h>
#include <rtk_osal.h>
#include <rtk_switch.h>
#include <init.h>
#include <rtk/mac/rtl8380/rtl8380_rtk.h>


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

#define GPIO_READ       0
#define GPIO_WRITE      1

#define CHECKBUSY_TIMES (3000)

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

typedef enum GPIO_PIN_E
{
    GPIO_A7 = 31,
    GPIO_A6 = 30,
    GPIO_A5 = 29,
    GPIO_A4 = 28,
    GPIO_A3 = 27,
    GPIO_A2 = 26,
    GPIO_A1 = 25,
    GPIO_A0 = 24,
    GPIO_B7 = 23,
    GPIO_B6 = 22,
    GPIO_B5 = 21,
    GPIO_B4 = 20,
    GPIO_B3 = 19,
    GPIO_B2 = 18,
    GPIO_B1 = 17,
    GPIO_B0 = 16,
    GPIO_C7 = 15,
    GPIO_C6 = 14,
    GPIO_C5 = 13,
    GPIO_C4 = 12,
    GPIO_C3 = 11,
    GPIO_C2 = 10,
    GPIO_C1 = 9,
    GPIO_C0 = 8,
    GPIO_END
}gpio_pin_t;



#define GPIO_CTRL_REG_BASE (0xb8003500)
#define GPIO_PABC_CNR   (GPIO_CTRL_REG_BASE +0x0)
#define GPIO_PABC_DIR   (GPIO_CTRL_REG_BASE +0x8)
#define GPIO_PABC_DATA  (GPIO_CTRL_REG_BASE +0xc)
#define GPIO_PABC_ISR  (GPIO_CTRL_REG_BASE +0x10)
#define GPIO_PAB_IMR  (GPIO_CTRL_REG_BASE +0x14)
#define GPIO_PC_IMR  (GPIO_CTRL_REG_BASE +0x18)


#define DELAY 2
#define CLK_DURATION(clk)                  { int i; for(i=0; i<clk; i++); }






#define RTL8380_VERSION_UNKWN  0x0
#define RTL8380_VERSION_A_CUT         0x1
#define RTL8380_VERSION_B_CUT         0x2

#define RTL8380_VERSION_IS_A_CUT (rtl8380_sw_version == RTL8380_VERSION_A_CUT)


static uint32 rtl8380_sw_version;


static void rtl8380_swVersion_get(uint32 *version)
{
    /*Chip Version Control*/
    uint32 data_int;
    uint32 data_chipRd;
    uint32 chip_info;

    if(version == NULL)
    	return;

    /* Save original value */
    data_int = MEM32_READ(0xBB000058);
    data_chipRd = MEM32_READ(0xBB0000D8);

    MEM32_WRITE(0xBB000058, data_int | 0x3);
    MEM32_WRITE(0xBB0000D8, data_chipRd | 0xA0000000);

    /* Get Chip Info */
    chip_info = MEM32_READ(0xBB0000D8);

    /* Restore original value */
    MEM32_WRITE(0xBB0000D8, data_chipRd);
    MEM32_WRITE(0xBB000058, data_int);

    if (((chip_info & 0xFFFF) == 0x6275))
    {
        if(((chip_info>>16) & 0x1F) == 0x1)
        {
            *version = RTL8380_VERSION_A_CUT;
        }
        else if(((chip_info>>16) & 0x1F) == 0x2)
        {
            *version = RTL8380_VERSION_B_CUT;
        }
	 else
 	{
            *version = RTL8380_VERSION_B_CUT;
 	}
    }
    else
    {
            *version = RTL8380_VERSION_UNKWN;
    }
}




/*******************************Access RTL8231 Using GPIO Simulating SMI Waveform**************************/

static uint32 smi_MDC;         /* GPIO used for SMI Clock Generation */
static uint32 smi_MDIO;       /* GPIO used for SMI Data signal */


 static  void _setGpioDir(uint32 pin, drv_gpio_direction_t dir)
{
    uint32 val;
    /*configure pin direction*/
    val = REG32(GPIO_PABC_DIR);
    if(dir == GPIO_DIR_IN)
        REG32(GPIO_PABC_DIR) = val & (~(1<<pin));
    else
        REG32(GPIO_PABC_DIR) = val | (1<<pin);
}

 static  void _setGpioData(uint32 pin, uint32 bitData)
{
    uint32 val;
    val = REG32(GPIO_PABC_DATA);
    if(bitData == 0)
        REG32(GPIO_PABC_DATA) = val & (~(1<<pin));
    else
        REG32(GPIO_PABC_DATA) = val | (1<<pin);
}

 static  void _getGpioData(uint32 pin, uint32 *pBitData)
{
    uint32 val;
    val = REG32(GPIO_PABC_DATA);
    if(val & (1<<pin))
        *pBitData = 1;
    else
        *pBitData = 0;
}

 static void _writeBit(uint32 bitData)
{
    _setGpioData(smi_MDIO, bitData);

    CLK_DURATION(DELAY);
    _setGpioData(smi_MDC, 0);
    CLK_DURATION(DELAY);
    _setGpioData(smi_MDC, 1);
}

static void smi_write_bits(uint32 data, uint32 bitLen)
{
    int i;
    for(i=bitLen-1; i>=0; i--)
        _writeBit(data & (1<<i));
}

 static void _readBit(uint32 *pBitData)
{
    CLK_DURATION(DELAY);
    _setGpioData(smi_MDC, 0);
    CLK_DURATION(DELAY);
    _setGpioData(smi_MDC, 1);

    _getGpioData(smi_MDIO, pBitData);
}

static void smi_read_bits(uint32 *pData, uint32 bitLen)
{
    int i;
    uint32 bitData;
    uint32 rawData = 0;

    for(i=bitLen-1; i>=0; i--)
    {
        _readBit(&bitData);
        rawData |= (bitData<<i);
    }

    *pData = rawData & 0xffff;
}

static void _smi_start(void)
{
    _setGpioDir(smi_MDC, GPIO_DIR_OUT);
    _setGpioDir(smi_MDIO, GPIO_DIR_OUT);
    smi_write_bits(0xffffffff, 32);
}

 static void _smiZbit(void)
{
    //_setGpioData(smi_MDIO, 0);
    _setGpioDir(smi_MDIO, GPIO_DIR_IN);

    CLK_DURATION(DELAY);
    _setGpioData(smi_MDC, 0);
    CLK_DURATION(DELAY);
    _setGpioData(smi_MDC, 1);
}


static int32 rtl8380_gpioSimulated_rtl8231_init(uint32 pinMDC, uint32 pinMDIO)
{
    int32 val;

    smi_MDC = pinMDC;
    smi_MDIO = pinMDIO;

    /* configure as gpio pin*/
    val = REG32(GPIO_PABC_CNR);
    REG32(GPIO_PABC_CNR) = val & (~((1<<pinMDC) | (1<<pinMDIO)));

    /*configure pin direction*/
    val = REG32(GPIO_PABC_DIR);
    REG32(GPIO_PABC_DIR) = val | ((GPIO_DIR_OUT<<pinMDC) | (GPIO_DIR_OUT<<pinMDIO));

    return SUCCESS;
}


static void rtl8380_gpioSimulated_rtl8231_write(uint32 phyad, uint32 regad, uint32 data)
{
    _smi_start();                           /*output 32bit 1 preamble*/
    smi_write_bits(0x5, 4);           /*output 0b0101, write ST+writeOP*/

    smi_write_bits(phyad, 5);      /*output 5bit phy address*/
    smi_write_bits(regad, 5);      /*output 5bit reg address*/

    smi_write_bits(0x2, 2);           /*output 0b10, write TA*/

    smi_write_bits(data, 16);           /*output data*/

    _smiZbit();
}

static void rtl8380_gpioSimulated_rtl8231_read(uint32 phyad, uint32 regad, uint32* pData)
{
    uint32 data = 0;

    _smi_start();                           /*output 32bit 1 preamble*/
    smi_write_bits(0x6, 4);           /*output 0b0110, write ST+readOP*/

    smi_write_bits(phyad, 5);      /*output 5bit phy address*/
    smi_write_bits(regad, 5);      /*output 5bit reg address*/

    _smiZbit();;           /*output 0b10, write TA*/
    //smi_read_bits(&data, 1);

    /*read data*/
    data = 0;
    smi_read_bits(&data, 16);

    _smiZbit();

    *pData = data;
}


/***********************Access RTL8231 Using Indirect Access Interface**************************/

static void rtl8380_indirectAccess_rtl8231_read(uint32 phy_id, uint32 reg_addr, uint32 *pData)
{
    uint32  temp;

    /* initialize variable */
    temp = 0;

    /* Input parameters:
     * If RWOP = 0(read), then INDATA[15:0] = {Reserved & PORT_ID[4:0]}
     * If RWOP = 1(write), then INDATA[15:0] = DATA[15:0]
     */
    temp |= ((phy_id << RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_PHY_ADDR_OFFSET) \
    				& RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_PHY_ADDR_MASK);

    /* Select register number to access */
    temp |= ((reg_addr << RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_REG_OFFSET) \
    				& RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_REG_MASK);

    /* Read/Write operation
     * 0b0: read
     * 0b1: write
     */
    temp |= ((GPIO_READ << RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_RWOP_OFFSET) \
    			& RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_RWOP_MASK);

    /* Request MAC to access PHY MII register
     * 0b0: complete access
     * 0b1: execute access
     * Note: When MAC completes access, it will clear this bit.
     */
    temp |= ((1 << RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_CMD_OFFSET) \
    		& RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_CMD_MASK);

    /* write register to active the read operation */
    MEM32_WRITE(SWCORE_BASE_ADDR | RTL8380_EXT_GPIO_INDRT_ACCESS_ADDR, temp);


    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    RTL8231_BUSY_WAIT_LOOP(SWCORE_BASE_ADDR | RTL8380_EXT_GPIO_INDRT_ACCESS_ADDR, 0x1);

    /* get the read operation result to temp */
    temp = MEM32_READ(SWCORE_BASE_ADDR | RTL8380_EXT_GPIO_INDRT_ACCESS_ADDR);

    /* fill the DATA[15:0] from temp to pData */
    (*pData) = (temp & RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_DATA_MASK) \
    			>> RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_DATA_OFFSET;
}


static void rtl8380_indirectAccess_rtl8231_write(uint32 phy_id, uint32 reg_addr, uint32 data)
{
    uint32  temp;

    /* initialize variable */
    temp = 0;

    /* Input parameters:
     * If RWOP = 0(read), then INDATA[15:0] = {Reserved & PORT_ID[4:0]}
     * If RWOP = 1(write), then INDATA[15:0] = DATA[15:0]
     */
    temp |= ((phy_id << RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_PHY_ADDR_OFFSET) \
    			& RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_PHY_ADDR_MASK);

    /* Select register number to access */
    temp |= ((reg_addr << RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_REG_OFFSET) \
    			& RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_REG_MASK);

    /* Read/Write operation
     * 0b0: read
     * 0b1: write
     */
    temp |= ((GPIO_WRITE << RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_RWOP_OFFSET) \
    			& RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_RWOP_MASK);

    /* Input parameters:
     * If RWOP = 0(read), then GPIO_DATA[15:0] = input data[15:0]
     * If RWOP = 1(write), then GPIO_DATA [15:0] = output data[15:0]
     */
    temp |= ((data << RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_DATA_OFFSET) \
    			& RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_DATA_MASK);

    /* Request MAC to access PHY MII register
     * 0b0: complete access
     * 0b1: execute access
     * Note: When MAC completes access, it will clear this bit.
     */
    temp |= ((1 << RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_CMD_OFFSET) \
    			& RTL8380_EXT_GPIO_INDRT_ACCESS_GPIO_CMD_MASK);

    /* write register to active the read operation */
    MEM32_WRITE(SWCORE_BASE_ADDR | RTL8380_EXT_GPIO_INDRT_ACCESS_ADDR, temp);

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    RTL8231_BUSY_WAIT_LOOP(SWCORE_BASE_ADDR | RTL8380_EXT_GPIO_INDRT_ACCESS_ADDR, 0x1);
}



/*******************************Access RTL8231 Exported APIs**************************/
/*
 * Function Declaration
 */
void rtl8380_rtl8231_init(void)
{
    uint32 pin_mdc;
    uint32 pin_mdio;
    uint32 ctrl_reg;
    uint32  temp;
    uint32 val;
    rtk_switch_model_t *pModel;

    pModel = gSwitchModel;

    /* Add GPIO output driver current */
    ctrl_reg = MEM32_READ(SWCORE_BASE_ADDR | RTL8380_IO_DRIVING_ABILITY_CTRL_ADDR);
    ctrl_reg |= (0x2UL);
    MEM32_WRITE(SWCORE_BASE_ADDR | RTL8380_IO_DRIVING_ABILITY_CTRL_ADDR, ctrl_reg);

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

    /* Get switch core version */
    rtl8380_swVersion_get(&rtl8380_sw_version);

    /* Initialize Switch Core */
    if(RTL8380_VERSION_IS_A_CUT)
    {
        /* Enable RTL8380 GPIO-Simulated mode */
        ctrl_reg = MEM32_READ(SWCORE_BASE_ADDR | RTL8380_EXTRA_GPIO_CTRL_ADDR);
        ctrl_reg &= ~(0x1UL);
        MEM32_WRITE(SWCORE_BASE_ADDR | RTL8380_EXTRA_GPIO_CTRL_ADDR, ctrl_reg);

        /* Set GPIOA2/GPIOA3 as MDC/MDIO pins */
        pin_mdc = 26;
        pin_mdio = 27;
	    rtl8380_gpioSimulated_rtl8231_init(pin_mdc, pin_mdio);
    }
    else
    {
	 /* Enable RTL8380 Indirect access RTL8231 mode */
        ctrl_reg = MEM32_READ(SWCORE_BASE_ADDR | RTL8380_EXTRA_GPIO_CTRL_ADDR);
        ctrl_reg |= 0x1UL;
        MEM32_WRITE(SWCORE_BASE_ADDR | RTL8380_EXTRA_GPIO_CTRL_ADDR, ctrl_reg);


        ctrl_reg = MEM32_READ(SWCORE_BASE_ADDR | RTL8380_DMY_REG5_ADDR);
        ctrl_reg &= ~0x3UL;
        ctrl_reg |= 0x1UL;
        MEM32_WRITE(SWCORE_BASE_ADDR | RTL8380_DMY_REG5_ADDR, ctrl_reg);
    }

    /* Select all the PINs as GPIO */
    rtl8380_rtl8231_write(0, 2, 0xffff);
    rtl8380_rtl8231_write(0, 3, 0xffff);
    rtl8380_rtl8231_read(0, 4, &temp);
    temp |= 0x1f;
    rtl8380_rtl8231_write(0, 4, temp);

    /* Set all the PINs Direction as GPI, Because GPO is not safe */
    rtl8380_rtl8231_read(0, 4, &temp);
    temp |= 0x1f<<5;
    rtl8380_rtl8231_write(0, 4, temp);
    rtl8380_rtl8231_write(0, 5, 0xffff);
    rtl8380_rtl8231_write(0, 6, 0xffff);

    /*Set 8231 PIN5/6/8 output*/
    extGpio_pin_dir_set(0, RTL8231_PIN_5, 1);
    extGpio_pin_dir_set(0, RTL8231_PIN_6, 1);
    extGpio_pin_dir_set(0, RTL8231_PIN_8, 1);
    extGpio_pin_dir_set(0, RTL8231_PIN_13, 1); /*PoE disable pin, extPin13=0, active*/
    extGpio_pin_dir_set(0, RTL8231_PIN_23, 1);
    extGpio_pin_dir_set(0, RTL8231_PIN_29, 1);

    /*Set 8231 PIN5/6/8 output high*/
    extGpio_pin_data_set(0, RTL8231_PIN_5, 0x1);
    extGpio_pin_data_set(0, RTL8231_PIN_6, 0x1);
    extGpio_pin_data_set(0, RTL8231_PIN_8, 0x1);
    extGpio_pin_data_set(0, RTL8231_PIN_13, 0x0);
    extGpio_pin_data_set(0, RTL8231_PIN_23, 0x0);
    extGpio_pin_data_set(0, RTL8231_PIN_29, 0x0);

    /*
          For GS1900_8HPv2 and GS1900_10HP:
          external gpio 15,16,17,18 should be inited as output low
        */
    if (NULL == pModel)
        return;

    if((0 == strcmp(pModel->name, "ZyXEL_GS1900_8HPv2"))||
        (0 == strcmp(pModel->name, "ZyXEL_GS1900_10HP")))
    {
        extGpio_pin_dir_set(0, RTL8231_PIN_15, 1); /*init extPin15 as output low*/
        extGpio_pin_data_set(0, RTL8231_PIN_15, 0x0);

        extGpio_pin_dir_set(0, RTL8231_PIN_16, 1); /*init extPin16 as output low*/
        extGpio_pin_data_set(0, RTL8231_PIN_16, 0x0);

        extGpio_pin_dir_set(0, RTL8231_PIN_17, 1); /*init extPin17 as output low*/
        extGpio_pin_data_set(0, RTL8231_PIN_17, 0x0);

        extGpio_pin_dir_set(0, RTL8231_PIN_18, 1); /*init extPin18 as output low*/
        extGpio_pin_data_set(0, RTL8231_PIN_18, 0x0);
    }
}


/*
 * Function Declaration
 */
int rtl8380_rtl8231_read(uint32 phy_id, uint32 reg_addr, uint32 *pData)
{
    if(RTL8380_VERSION_IS_A_CUT)
    {
	rtl8380_gpioSimulated_rtl8231_read(phy_id, reg_addr,  pData);
    }
    else
    {
	rtl8380_indirectAccess_rtl8231_read(phy_id, reg_addr,  pData);
    }

    return 1;
}

/*
 * Function Declaration
 */
int rtl8380_rtl8231_write(uint32 phy_id, uint32 reg_addr, uint32 data)
{
    if(RTL8380_VERSION_IS_A_CUT)
    {
	rtl8380_gpioSimulated_rtl8231_write(phy_id, reg_addr,  data);
    }
    else
    {
	rtl8380_indirectAccess_rtl8231_write(phy_id, reg_addr,  data);
    }

    return 1;
}



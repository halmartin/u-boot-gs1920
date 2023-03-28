/*
* Copyright (C) 2010 Realtek Semiconductor Corp.
* All Rights Reserved.
*
* This program is the proprietary software of Realtek Semiconductor
* Corporation and/or its licensors, and only be used, duplicated,
* modified or distributed under the authorized license from Realtek.
*
* ANY USE OF THE SOFTWARE OTEHR THAN AS AUTHORIZED UNDER
* THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
*
* $Revision: 9863 $
* $Date: 2010-05-28 11:51:11 +0800 $
*
* Purpose : MDC/MDIO example code
*
* Feature :  This file consists of following modules:
*
*
*/

#include <rtk_type.h>
#include <rtk_reg.h>
#include <common.h>

#include <rtk/drv/gpio/gpio.h>
#include <rtk/mac/rtl8390/rtl8390_mdc_mdio.h>


uint32 smi_MDC;         /* GPIO used for SMI Clock Generation */
uint32 smi_MDIO;       /* GPIO used for SMI Data signal */


int32 rtl8390_smiInit(uint32 pinMDC, uint32 pinMDIO)
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

 void smi_write_bits(uint32 data, uint32 bitLen)
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

 void smi_read_bits(uint32 *pData, uint32 bitLen)
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

 void _smi_start(void)
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

 void rtl8390_smiWrite(uint32 phyad, uint32 regad, uint32 data)
{
    _smi_start();                           /*output 32bit 1 preamble*/
    smi_write_bits(0x5, 4);           /*output 0b0101, write ST+writeOP*/

    smi_write_bits(phyad, 5);      /*output 5bit phy address*/
    smi_write_bits(regad, 5);      /*output 5bit reg address*/

    smi_write_bits(0x2, 2);           /*output 0b10, write TA*/

    smi_write_bits(data, 16);           /*output data*/

    _smiZbit();
}

 void rtl8390_smiRead(uint32 phyad, uint32 regad, uint32* pData)
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


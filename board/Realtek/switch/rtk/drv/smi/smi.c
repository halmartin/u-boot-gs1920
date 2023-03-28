/*
 * Copyright (C) 2009 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 40834 $
 * $Date: 2013-07-08 15:46:33 +0800 (Mon, 08 Jul 2013) $
 *
 * Purpose : DRV APIs definition.
 *
 * Feature : SMI relative API
 *
 */

/*
 * Include Files
 */
/*
 * Include Files
 */
#include <common.h>
#include <command.h>
//#include "rtk_soc_common.h"
#include <rtk_reg.h>
#include <rtk_osal.h>
//#include <customer/customer_diag.h>
#include <rtk/drv/rtl8231/rtl8231_drv.h>

#include <rtk/drv/swled/swctrl_led_main.h>
#if defined(CONFIG_RTL8380)
#include <rtk/mac/rtl8380/rtl8380_rtk.h>
#include <rtk/mac/rtl8380/rtl8380_mdc_mdio.h>
#include <rtk/mac/rtl8380/rtl8380_swcore_reg.h>
#endif

#if defined(CONFIG_RTL8390)
#include <rtk/mac/rtl8390/rtl8390_drv.h>
#include <rtk/mac/rtl8390/rtl8390_mdc_mdio.h>
#include <rtk/mac/rtl8390/rtl8390_swcore_reg.h>
#endif
#include <rtk/drv/smi/smi.h>
#include <rtk/drv/gpio/ext_gpio.h>
/*
 * Symbol Definition
 */
#define ACK_TIMER                       5
#define DELAY_4000                      4000 /*100KHz*/
#define DELAY_1800                      1800 /*100KHz*/
#define SFP_CHIPID                      0x50
#define FAN_CHIPID											0x1B
#define SFP_TYPE                        SMI_TYPE_8BITS_DEV
#define SMI_DEVICE_MAX                  8
#define RT_ERR_CHK(op, ret)\
do {\
    if ((ret = (op)) != RT_ERR_OK)\
        return ret;\
} while(0)
/*
 * Data Declaration
 */
static dev_i2c_t smi_dev[SMI_DEVICE_MAX];

/*
 * Macro Declaration
 */

/*
 * Function Declaration
 */
void drv_smi_list(void)
{
	int i;
	printf("\n Index SCK_DEV SCK_PIN SDA_DEV SDA_PIN TYPE   ID   Delay Name \n");
	printf("=================================================================\n");
	for (i=0;i<SMI_DEVICE_MAX;i++)
	{
		if (smi_dev[i].valid==1)
		{
			printf("    %.2d      %.2d      %.2d      %.2d      %.2d ",	i,smi_dev[i].sck.devId,smi_dev[i].sck.pinId,
			smi_dev[i].sda.devId,smi_dev[i].sda.pinId);
			if (SMI_TYPE_8BITS_DEV == smi_dev[i].smi_TYPE)
				printf(" %.2d  ",8);
			if (SMI_TYPE_16BITS_DEV == smi_dev[i].smi_TYPE)
				printf(" %.2d  ",16);
			printf("  %x   %.5d  %s\n",	smi_dev[i].smi_CHIPID,smi_dev[i].smi_DELAY,smi_dev[i].name);
		}
	}
	return;
}

/* Function Name:
 *      drv_smi_type_set
 * Description:
 *      SMI init function
 * Input:
 *      portSCK - SCK port id
 *      pinSCK  - SCK pin
 *      portSDA - SDA port id
 *      pinSDA  - SDA pin
 *      dev     - dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 drv_smi_type_set(uint32 smi_type, uint32 chipid, uint32 delay, uint32 dev,uint8 *name)
{
	  if (dev >= SMI_DEVICE_MAX)
		{
			printf("\n dev index out of range! \n");
			return RT_ERR_FAILED;
		}

	  if (smi_dev[dev].valid==1)
		{
    	smi_dev[dev].smi_TYPE = smi_type;
    	smi_dev[dev].smi_CHIPID = chipid;
    	smi_dev[dev].smi_DELAY = delay;
    	strncpy((char *)smi_dev[dev].name,(char *)name,(size_t)MAX_DEV_NAME);
    	return RT_ERR_OK;
    }

    return RT_ERR_FAILED;
} /* end of drv_smi_type_set */

/* Function Name:
 *      drv_smi_type_get
 * Description:
 *      SMI init function
 * Input:
 *      portSCK - SCK port id
 *      pinSCK  - SCK pin
 *      portSDA - SDA port id
 *      pinSDA  - SDA pin
 *      dev     - dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 drv_smi_type_get(uint32 * ptype, uint32 * pchipid, uint32 * pdelay, uint32 dev)
{
	  if (dev >= SMI_DEVICE_MAX)
		{
			printf("\n dev index out of range! \n");
			return RT_ERR_FAILED;
		}
		
		if (smi_dev[dev].valid==1)
		{
    	*ptype = smi_dev[dev].smi_TYPE;
    	*pchipid = smi_dev[dev].smi_CHIPID;
    	*pdelay = smi_dev[dev].smi_DELAY;
    	return RT_ERR_OK;
    }

    return RT_ERR_FAILED;
} /* end of drv_smi_type_get */

/* Function Name:
 *      _general_smi_start
 * Description:
 *      SMI start function
 * Input:
 *      dev - dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
static int32 _general_smi_start(uint32 dev)
{
    int32 ret = RT_ERR_OK;

  extGpio_pin_dir_set(RTL8231_ADDR,smi_dev[dev].sda.pinId, GPIO_DIR_OUT);
  extGpio_pin_dir_set(RTL8231_ADDR,smi_dev[dev].sck.pinId, GPIO_DIR_OUT);

    /* Initial state: SCK: 0, SDA: 1 */
	extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sck.pinId, 0);
	extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, 1);
	
    /* CLK 1: 0 -> 1, 1 -> 0 */
    CLK_DURATION(smi_dev[dev].smi_DELAY);
	extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sck.pinId, 1);


    CLK_DURATION(smi_dev[dev].smi_DELAY);
	extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sck.pinId, 0);
	

    /* CLK 2: */
    CLK_DURATION(smi_dev[dev].smi_DELAY);
	extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sck.pinId, 1);
	

    CLK_DURATION(smi_dev[dev].smi_DELAY);
	extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, 0);
	

    CLK_DURATION(smi_dev[dev].smi_DELAY);
	extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sck.pinId, 0);
	

    CLK_DURATION(smi_dev[dev].smi_DELAY);
	extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, 1);

    return ret;
} /* end of _general_smi_start */

/* Function Name:
 *      _general_smi_stop
 * Description:
 *      SMI stop function
 * Input:
 *      dev  - dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
static int32 _general_smi_stop(uint32 dev)
{
    int32 ret = RT_ERR_OK;

    CLK_DURATION(smi_dev[dev].smi_DELAY);

//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SDA[dev], 0), ret);
	extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, 0);
	

    CLK_DURATION(smi_dev[dev].smi_DELAY);
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SCK[dev], 1), ret);
	extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sck.pinId, 1);

    CLK_DURATION(smi_dev[dev].smi_DELAY);
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SDA[dev], 1), ret);
	extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, 1);

    CLK_DURATION(smi_dev[dev].smi_DELAY);
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SCK[dev], 1), ret);
	extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sck.pinId, 1);

    CLK_DURATION(smi_dev[dev].smi_DELAY);
    CLK_DURATION(smi_dev[dev].smi_DELAY);
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SCK[dev], 0), ret);
	extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sck.pinId, 0);
	

    CLK_DURATION(smi_dev[dev].smi_DELAY);
    CLK_DURATION(smi_dev[dev].smi_DELAY);
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SCK[dev], 1), ret);
	extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sck.pinId, 1);
	
    /* add a click */
    CLK_DURATION(smi_dev[dev].smi_DELAY);
    CLK_DURATION(smi_dev[dev].smi_DELAY);
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SCK[dev], 0), ret);
	extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sck.pinId, 0);
	

    CLK_DURATION(smi_dev[dev].smi_DELAY);
    CLK_DURATION(smi_dev[dev].smi_DELAY);
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SCK[dev], 1), ret);
	extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sck.pinId, 1);

    /* change GPIO pin to Output only */
//    RT_ERR_CHK(drv_gpio_init(smi_SDA[dev], GPIO_CTRLFUNC_NORMAL, GPIO_DIR_IN, GPIO_INT_DISABLE), ret);
//    RT_ERR_CHK(drv_gpio_init(smi_SCK[dev], GPIO_CTRLFUNC_NORMAL, GPIO_DIR_IN, GPIO_INT_DISABLE), ret);
	
	extGpio_pin_dir_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, GPIO_DIR_OUT);
	extGpio_pin_dir_set(RTL8231_ADDR, smi_dev[dev].sck.pinId, GPIO_DIR_OUT);

    return ret;
} /* end of _general_smi_stop */


/* Function Name:
 *      _general_smi_readBit
 * Description:
 *      SMI read bit function
 * Input:
 *      bitLen  - bit length
 *      dev     - dev id
 * Output:
 *      pRdata - data read
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
static int32 _general_smi_readBit(uint32 bitLen, uint32 *pRdata, uint32 dev)
{
    uint32 data = 0;
    uint32 delay = smi_dev[dev].smi_DELAY;

/*  Remove from orignal static defined DELAY_1800, now it is user confiurable.
    if(SMI_DEVICE_SI3452 == dev)
    {
        delay = DELAY_1800;
    }
*/

    /* change GPIO pin to Input only */
//    RT_ERR_CHK(drv_gpio_init(smi_SDA[dev], GPIO_CTRLFUNC_NORMAL, GPIO_DIR_IN, GPIO_INT_DISABLE), ret);
	CLK_DURATION(delay);

	extGpio_pin_dir_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, GPIO_DIR_IN);

    for (*pRdata = 0; bitLen > 0; bitLen--)
    {
        CLK_DURATION(delay);
        CLK_DURATION(delay);

        /* clocking */
//        RT_ERR_CHK(drv_gpio_dataBit_set(smi_SCK[dev], 1), ret);
		extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sck.pinId, 1);
		

        CLK_DURATION(delay);

//        RT_ERR_CHK(drv_gpio_dataBit_get(smi_SDA[dev], &data), ret);
		extGpio_pin_data_get(RTL8231_ADDR, smi_dev[dev].sda.pinId, &data);
		
		
        CLK_DURATION(delay);

//        RT_ERR_CHK(drv_gpio_dataBit_set(smi_SCK[dev], 0), ret);
		extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sck.pinId, 0);

        *pRdata |= (data << (bitLen - 1));
    }

    /* change GPIO pin to Output only */
    //RT_ERR_CHK(drv_gpio_init(smi_SDA[dev], GPIO_CTRLFUNC_NORMAL, GPIO_DIR_OUT, GPIO_INT_DISABLE), ret);
	extGpio_pin_dir_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, GPIO_DIR_OUT);

    return RT_ERR_OK;
} /* end of _general_smi_readBit */

/* Function Name:
 *      _general_smi_writeBit
 * Description:
 *      SMI write bit function
 * Input:
 *      signal - ctrl code
 *      bitLen - bit length
 *      dev    - dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OUT_OF_RANGE
 *      RT_ERR_OK
 * Note:
 *      None
 */
static int32 _general_smi_writeBit(uint16 signal, uint32 bitLen, uint32 dev)
{
    uint32 delay = smi_dev[dev].smi_DELAY;

    for ( ; bitLen > 0; bitLen--)
    {
        CLK_DURATION(delay);

        /* prepare data */
        if (signal & (1 << (bitLen - 1)))
        {
			extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, 1);
			
        }
        else
        {
			extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, 0);
			
        }

        CLK_DURATION(delay);

        /* clocking */
		extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sck.pinId, 1);
		
        CLK_DURATION(delay);
        CLK_DURATION(delay);
		extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sck.pinId, 0);
		
    }

    return RT_ERR_OK;
} /* end of _general_smi_writeBit */

/* Function Name:
 *      _general_8bits_dev_start
 * Description:
 *      8bits device start function
 * Input:
 *      dev - dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
static int32 _general_8bits_dev_start(uint32 dev)
{
    int32 ret = RT_ERR_OK;

    /* change GPIO pin to Output only */

	extGpio_pin_dir_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, GPIO_DIR_OUT);
	extGpio_pin_dir_set(RTL8231_ADDR, smi_dev[dev].sck.pinId, GPIO_DIR_OUT);

	extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sck.pinId, 0);
	
    CLK_DURATION(smi_dev[dev].smi_DELAY);
	extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sck.pinId, 1);

    CLK_DURATION(smi_dev[dev].smi_DELAY);
	extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, 0);
	

    CLK_DURATION(smi_dev[dev].smi_DELAY);
	extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sck.pinId, 0);

    return ret;
} /* end of _general_8bits_dev2_start */

/* Function Name:
 *      _general_8bits_dev_smi_read
 * Description:
 *      8bits device read function
 * Input:
 *      mAddrs  - address
 *      dev     - dev id
 * Output:
 *      pRdata - data read
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
static int32 _general_8bits_dev_read(uint32 mAddrs, uint8 *pRdata, uint32 dev)
{
    uint32 rawData = 0;
    uint32 ack = 0;
    uint32 slave = 0;
    int32  ret = RT_ERR_FAILED;
	uint16 regAddr;

    /*Initial data memory*/
    slave = smi_dev[dev].smi_CHIPID & 0xff;
    printf("[Read]devid 0x%x addr 0x%x ack 0x%x", slave, mAddrs, ack);
    mAddrs = mAddrs & 0xff;
    *pRdata = 0;

    /* CTRL code: 7'slave adress */
    RT_ERR_CHK(_general_smi_writeBit(slave, 7, dev), ret);

    /* 0: issue WRITE command */
    RT_ERR_CHK(_general_smi_writeBit(0x0, 1, dev), ret);
//	RT_ERR_CHK(_general_smi_writeBit(0x1, 1, dev), ret);

    /* Read ack for issuing WRITE command*/
    RT_ERR_CHK(_general_smi_readBit(1, &ack, dev), ret);

    /* Check ack value, it should be zero. */
    if (0 != ack)
    {
        //RT_LOG(LOG_DEBUG, MOD_GENERAL, "[Read]devid 0x%x addr 0x%x ack 0x%x\n", slave, mAddrs, ack);
        //RT_LOG(LOG_DEBUG, MOD_GENERAL, "[si3452_smi_read] Read first ack faild\n");
        //RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 1:,Send:Dev[6:0]_0, no Ack");
        printf("\n[Read]devid 0x%x addr 0x%x ack 0x%x\n", slave, mAddrs, ack);
        printf("\n[si3452_smi_read] Read first ack faild\n");
        printf("\nPoE Read Error 1:,Send:Dev[6:0]_0, no Ack");		
        return RT_ERR_FAILED;
    }

    /* Set data pin to high */
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SDA[dev], 1), ret);
	extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, 1);
	

    /* Note: 8-bit Reg Address */
	regAddr = (uint16)(mAddrs & 0xFF);
//    RT_ERR_CHK(_general_smi_writeBit((mAddrs & 0xFF), 8, dev), ret);
	RT_ERR_CHK(_general_smi_writeBit(regAddr, 8, dev), ret);


    /* Read ack for issuing WRITE command*/
    RT_ERR_CHK(_general_smi_readBit(1, &ack, dev), ret);

    /* Check ack value, it should be zero. */
    if (0 != ack)
    {
        //RT_LOG(LOG_DEBUG, MOD_GENERAL, "[Read]devid 0x%x addr 0x%x ack 0x%x\n", slave, mAddrs, ack);
        //RT_LOG(LOG_DEBUG, MOD_GENERAL, "[si3452_smi_read] Read second ack faild\n");
        //RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 2:Dev[6:0]_0|A|Addr[7:0], no Ack");
        printf("\n[Read]devid 0x%x addr 0x%x ack 0x%x\n", slave, mAddrs, ack);
        printf("\n[si3452_smi_read] Read second ack faild\n");
        printf("\nPoE Read Error 2:Dev[6:0]_0|A|Addr[7:0], no Ack");		
        return RT_ERR_FAILED;
    }

    /* Set data pin to high */
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SDA[dev], 1), ret);
	extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, 1);
	
    /* Start signal for Read */
    RT_ERR_CHK(_general_smi_start(dev), ret);

    /* Device Address & Read */
    RT_ERR_CHK(_general_smi_writeBit(slave, 7, dev), ret);

     /* 1: issue READ command */
    RT_ERR_CHK(_general_smi_writeBit(0x1, 1, dev), ret);

    /* Read ack for issuing READ command*/
    RT_ERR_CHK(_general_smi_readBit(1, &ack, dev), ret);

    /* Check ack value, it should be zero. */
    if (ack != 0)
    {
        //RT_LOG(LOG_DEBUG, MOD_GENERAL, "[Read]devid 0x%x addr 0x%x ack 0x%x\n", slave, mAddrs, ack);
        //RT_LOG(LOG_DEBUG, MOD_GENERAL, "[si3452_smi_read] Read third ack faild\n");
        //RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 3:Dev[6:0]_0|A|Addr[7:0]|A|Dev[6:0], no Ack");
        printf("\n[Read]devid 0x%x addr 0x%x ack 0x%x\n", slave, mAddrs, ack);
        printf("\n[si3452_smi_read] Read third ack faild\n");
        printf("\nPoE Read Error 3:Dev[6:0]_0|A|Addr[7:0]|A|Dev[6:0], no Ack");		
        return RT_ERR_FAILED;
    }

    /* Read 1 bytes data, Read DATA [7:0] */
    RT_ERR_CHK(_general_smi_readBit(8, &rawData, dev), ret);

    *pRdata |= rawData & 0xFF;

    /* Return Nack = 1*/
    RT_ERR_CHK(_general_smi_writeBit(0x01, 1, dev), ret);


    return ret;
} /* end of _general_8bits_dev_smi_read */

/* Function Name:
 *      _general_8bits_dev_smi_write
 * Description:
 *      8bits device write function
 * Input:
 *      mAddrs  - address
 *      wData   - data write
 *      dev     - dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
static int32 _general_8bits_dev_write(uint32 mAddrs, uint32 wData, uint32 dev)
{
    uint32 ack = 0;
    uint32 slave = 0;
    int32  ret = RT_ERR_FAILED;

    /*Initial data memory*/
    slave = smi_dev[dev].smi_CHIPID & 0xff;
    //RT_DBG(LOG_DEBUG, MOD_GENERAL, "Write add: %02X ", slave);
    mAddrs = mAddrs & 0xff;

    /* CTRL code: 7'slave adress */
    RT_ERR_CHK(_general_smi_writeBit(slave, 7, dev), ret);

    /* 0: issue WRITE command */
    RT_ERR_CHK(_general_smi_writeBit(0x0, 1, dev), ret);

    /* Set data pin to high */
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SDA[dev], 1), ret);
	extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, 1);


    /* Read ack for issuing WRITE command*/
    RT_ERR_CHK(_general_smi_readBit(1, &ack, dev), ret);

    /* Check ack value, it should be zero. */
    if (0 != ack)
    {
        //RT_LOG(LOG_DEBUG, MOD_GENERAL, "[Write]devid 0x%x addr 0x%x ack 0x%x\n", slave, mAddrs, ack);
        //RT_LOG(LOG_DEBUG, MOD_GENERAL, "[si3452_smi_write] Read first ack faild\n");
        return RT_ERR_FAILED;
    }

//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SDA[dev], 1), ret);
	extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, 1);
	

    /* Set reg_addr[7:0] */
    RT_ERR_CHK(_general_smi_writeBit((mAddrs & 0xFF), 8, dev), ret);

    /* Read ack for issuing WRITE command*/
    RT_ERR_CHK(_general_smi_readBit(1, &ack, dev), ret);

    /* Check ack value, it should be zero. */
    if (0 != ack)
    {
        //RT_LOG(LOG_DEBUG, MOD_GENERAL, "[Write]devid 0x%x addr 0x%x ack 0x%x\n", slave, mAddrs, ack);
        //RT_LOG(LOG_DEBUG, MOD_GENERAL, "[si3452_smi_write] Read second ack faild\n");
        return RT_ERR_FAILED;
    }


//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SDA[dev], 1), ret);
	extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, 1);
	
    /* Write Data [7:0] out */
    RT_ERR_CHK(_general_smi_writeBit((wData & 0xFF), 8, dev), ret);

    /* Read ack for issuing WRITE command*/
    RT_ERR_CHK(_general_smi_readBit(1, &ack, dev), ret);

    /* Check ack value, it should be zero. */
    if (0 != ack)
    {
        //RT_LOG(LOG_DEBUG, MOD_GENERAL, "[Write]devid 0x%x addr 0x%x ack 0x%x\n", slave, mAddrs, ack);
        //RT_LOG(LOG_DEBUG, MOD_GENERAL, "[si3452_smi_write] Read third ack faild\n");
        return RT_ERR_FAILED;
    }

//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SDA[dev], 1), ret);
	extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, 1);
	
    return ret;
} /* end of _general_8bits_dev_smi_write */


/* Function Name:
 *      _general_8bits_dev_stop
 * Description:
 *      8bits device stop function
 * Input:
 *      dev - dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
static int32 _general_8bits_dev_stop(uint32 dev)
{

    CLK_DURATION(smi_dev[dev].smi_DELAY);
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SDA[dev], 0), ret);
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SCK[dev], 1), ret);
	extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, 0);
	extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sck.pinId, 1);
	

    CLK_DURATION(smi_dev[dev].smi_DELAY);
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SDA[dev], 1), ret);
	extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, 1);
	

    CLK_DURATION(smi_dev[dev].smi_DELAY);
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SCK[dev], 0), ret);
	extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sck.pinId, 0);

    /* change GPIO pin to Output only */
//    RT_ERR_CHK(drv_gpio_init(smi_SDA[dev], GPIO_CTRLFUNC_NORMAL, GPIO_DIR_IN, GPIO_INT_DISABLE), ret);
//    RT_ERR_CHK(drv_gpio_init(smi_SCK[dev], GPIO_CTRLFUNC_NORMAL, GPIO_DIR_IN, GPIO_INT_DISABLE), ret);
	extGpio_pin_dir_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, GPIO_DIR_OUT);
	extGpio_pin_dir_set(RTL8231_ADDR, smi_dev[dev].sck.pinId, GPIO_DIR_OUT);

    return RT_ERR_OK;
} /* end of _general_8bits_dev_stop */


/* Function Name:
 *      _general_16bits_start
 * Description:
 *      SMI start function
 * Input:
 *      dev - dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      PD64012
 */
static int32 _general_16bits_start(uint32 dev)
{

    /* change GPIO pin to Output only */
	extGpio_pin_dir_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, GPIO_DIR_OUT);
	extGpio_pin_dir_set(RTL8231_ADDR, smi_dev[dev].sck.pinId, GPIO_DIR_OUT);
    //RT_ERR_CHK(drv_gpio_init(smi_SCK[dev], GPIO_CTRLFUNC_NORMAL, GPIO_DIR_OUT, GPIO_INT_DISABLE), ret);
    //RT_ERR_CHK(drv_gpio_init(smi_SDA[dev], GPIO_CTRLFUNC_NORMAL, GPIO_DIR_OUT, GPIO_INT_DISABLE), ret);

	extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, 1);
	extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sck.pinId, 1);
    //RT_ERR_CHK(drv_gpio_dataBit_set(smi_SDA[dev], 1), ret);
    //RT_ERR_CHK(drv_gpio_dataBit_set(smi_SCK[dev], 1), ret);

    CLK_DURATION(smi_dev[dev].smi_DELAY);
    //RT_ERR_CHK(drv_gpio_dataBit_set(smi_SDA[dev], 0), ret);
	extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, 0);

    /* Initial state: SCK: 0, SDA: 1 */
    CLK_DURATION(smi_dev[dev].smi_DELAY);
    //RT_ERR_CHK(drv_gpio_dataBit_set(smi_SCK[dev], 0), ret);
	extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sck.pinId, 0);

    return RT_ERR_OK;
} /* end of _general_16bits_start */

/* Function Name:
 *      _general_16bits_read
 * Description:
 *      ina209 read function
 * Input:
 *      mAddrs  - address
 *      dev     - dev id
 * Output:
 *      pRdata - data read
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      PD64012.
 */
int32 _general_16bits_read(uint32 chipid, uint32 mAddrs, uint32 *pRdata, uint32 dev)
{
    uint32 rawData = 0;
    uint32 ack = 0;
    int32  ret = RT_ERR_FAILED;
    uint16 devid = 0;
    uint8  con = 0;

    /*Initial data memory*/
    *pRdata = 0;

    devid = chipid & 0xff;
    //RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read:device:%04d, addr:%04d", devid, mAddrs);

    /* Start SMI */
    ret = _general_16bits_start(dev);
    if (RT_ERR_OK != ret)
    {
        //RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 1:Start");
        return ret;
    }

    /* Device Address & Write */

    ret = _general_smi_writeBit(devid, 7, dev);              /* CTRL code: 7'PD64012_MasterAddress */
    if (RT_ERR_OK != ret)
    {
        //RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 2:write devid");
        return ret;
    }

    ret = _general_smi_writeBit(0x0, 1, dev);               /* 1: issue READ command */
    if (RT_ERR_OK != ret)
    {
        //RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 3:write read_bit 0");
        return ret;
    }

	  extGpio_pin_dir_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, GPIO_DIR_IN);
	
    do
    {
        con++;
        ret = _general_smi_readBit(1, &ack, dev);           /* ack for issuing READ command*/
        if (RT_ERR_OK != ret)
        {
            //RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 5:read bit");
            return ret;
        }
    } while ((0 != ack) && (con < ACK_TIMER));

	  extGpio_pin_dir_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, GPIO_DIR_OUT);

    if (0 != ack)
    {
        //RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 9: Dev[6:0]_0, no Ack");
        return RT_ERR_FAILED;
    }

    /* Note: 8-bit Reg Address */
    ret = _general_smi_writeBit((mAddrs & 0xFF), 8, dev);   /* Set reg_addr[7:0] */

    if (RT_ERR_OK != ret)
    {
        //RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error A: Dev[6:0]_0|A|Addr[7:0]");
        return ret;
    }
	
	  extGpio_pin_dir_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, GPIO_DIR_IN);

    con = 0;
    do
    {
        con++;
        ret = _general_smi_readBit(1, &ack, dev);           /* ack for setting reg_addr[7:0] */
        if (RT_ERR_OK != ret)
        {
            //RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error C");
            return ret;
        }
    } while ((0 != ack) && (con < ACK_TIMER));

	  extGpio_pin_dir_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, GPIO_DIR_OUT);

    if (0 != ack)
    {
        //RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error G: Dev[6:0]_0|A|Addr[7:0], no Ack");
        return RT_ERR_FAILED;
    }

    ret = _general_smi_writeBit((mAddrs >> 8), 8, dev);      /* Set reg_addr[15:8] */
    if (RT_ERR_OK != ret)
    {
        //RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error H: Dev[6:0]_0|A|Addr[7:0]|A|Addr[15:8]");
        return ret;
    }
	
	  extGpio_pin_dir_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, GPIO_DIR_IN);

    con = 0;
    do
    {
        con++;
        ret = _general_smi_readBit(1, &ack, dev);           /* ack for setting reg_addr[15:8] */
        if (RT_ERR_OK != ret)
        {
            //RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 11");
            return ret;
        }
    } while ((0 != ack) && (con < ACK_TIMER));

	  extGpio_pin_dir_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, GPIO_DIR_OUT);

    if (0 != ack)
    {
        //RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 12: Dev[6:0]_0|A|Addr[7:0]|A|Addr[15:8], no Ack");
        return RT_ERR_FAILED;
    }

    /* Start signal for Read */
    ret = _general_smi_start(dev);                          /* Start SMI */
    if (RT_ERR_OK != ret)
    {
        //RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 13: Dev[6:0]_0|A|Addr[7:0]|A|Addr[15:8]|A, start");
        return ret;
    }

    /* Device Address & Read */
    ret = _general_smi_writeBit(devid, 7, dev);              /* CTRL code: 7'PD64012_MasterAddress */
    if (RT_ERR_OK != ret)
    {
        //RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 14: Dev[6:0]_0|A|Addr[7:0]|A|Addr[15:8]|A|Dev[6:0]");
        return ret;
    }

    ret = _general_smi_writeBit(0x1, 1, dev);               /* 1: issue READ command */
    if (RT_ERR_OK != ret)
    {
        //RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 17: Dev[6:0]_0|A|Addr[7:0]|A|Addr[15:8]|A|Dev[6:0]_1");
        return ret;
    }

	  extGpio_pin_dir_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, GPIO_DIR_IN);

    con = 0;
    do
    {
        con++;
        ret = _general_smi_readBit(1, &ack, dev);           /* ack for issuing READ command*/
        if (RT_ERR_OK != ret)
        {
            //RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 18");
            return ret;
        }
    } while ((0 != ack) && (con < ACK_TIMER));

    if (ack != 0)
    {
        //RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 19: Dev[6:0]_0|A|Addr[7:0]|A|Addr[15:8]|A|Dev[6:0]_1, no Ack");
        return RT_ERR_FAILED;
    }

    /* Read 2 bytes data */
    ret = _general_smi_readBit(8, &rawData, dev);           /* Read DATA [7:0] */
    if (RT_ERR_OK != ret)
    {
        //RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 20: Dev[6:0]_0|A|Addr[7:0]|A|Addr[15:8]|A|Dev[6:0]_1|A, no datat read");
        return ret;
    }
	  extGpio_pin_dir_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, GPIO_DIR_OUT);

    *pRdata |= rawData & 0xFF;

    ret = _general_smi_writeBit(0x00, 1, dev);              /* ack by Master(CPU) */
    if (RT_ERR_OK != ret)
    {
        //RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 21: Dev[6:0]_0|A|Addr[7:0]|A|Addr[15:8]|A|Dev[6:0]_1|A|DATA[7:0], send Ack fail");
        return ret;
    }

	  extGpio_pin_dir_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, GPIO_DIR_IN);

    ret = _general_smi_readBit(8, &rawData, dev);           /* Read DATA [15: 8] */
    if (RT_ERR_OK != ret)
    {
        //RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 22: Dev[6:0]_0|A|Addr[7:0]|A|Addr[15:8]|A|Dev[6:0]_1|A|DATA[7:0]|A no data read");
        return ret;
    }
        CLK_DURATION(smi_dev[dev].smi_DELAY);

	  extGpio_pin_dir_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, GPIO_DIR_OUT);

    *pRdata |= rawData << 8;

    /* Stop SMI */
    ret = _general_smi_stop(dev);
    if (RT_ERR_OK != ret)
    {
        //RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 23: Dev[6:0]_0|A|Addr[7:0]|A|Addr[15:8]|A|Dev[6:0]_1|A|DATA[7:0]|A|DATA[15:8], no stop");
        return RT_ERR_FAILED;
    }

    return ret;
} /* end of _general_16bits_read */


/* Function Name:
 *      _general_16bits_write
 * Description:
 *      pd64012 write function
 * Input:
 *      mAddrs  - address
 *      wData   - data write
 *      dev     - dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      PD64012
 */
#define PD64012_Master_DeviceID    0x24 /*0x48 Address #9  0b010,010,00 ->inverse 0b000,1001 0x9 */
int32 _general_16bits_write(uint32 chipid, uint32 mAddrs, uint32 wData, uint32 dev)
{
    uint32 ack = 0;
    int32  ret = RT_ERR_FAILED;
    uint16 devid = PD64012_Master_DeviceID;
    int8   con = 0;

    devid = chipid & 0xff;
    //RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read:device:%04d, addr:%04d",devid,mAddrs);

    /* Start SMI */
    RT_ERR_CHK(_general_16bits_start(dev), ret);

    /* Start signal for Read */
    RT_ERR_CHK(_general_smi_writeBit(devid, 7, dev), ret);              /* CTRL code: 7'PD64012_MasterAddress */
    RT_ERR_CHK(_general_smi_writeBit(0x0, 1, dev), ret);                /* 0: issue WRITE command */

    extGpio_pin_dir_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, GPIO_DIR_IN);

    do
    {
        con++;
        RT_ERR_CHK(_general_smi_readBit(1, &ack, dev), ret);           /* ack for issuing WRITE command*/
    } while ((0 != ack) && (con < ACK_TIMER));

    extGpio_pin_dir_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, GPIO_DIR_OUT);

    if (0 != ack)
    {
        return RT_ERR_FAILED;
    }

    RT_ERR_CHK(_general_smi_writeBit((mAddrs & 0xFF), 8, dev), ret);   /* Set reg_addr[7:0] */
    extGpio_pin_dir_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, GPIO_DIR_IN);

    con = 0;
    do
    {
        con++;
        RT_ERR_CHK(_general_smi_readBit(1, &ack, dev), ret);           /* ack for setting reg_addr[7:0] */
    } while ((0 != ack) && (con < ACK_TIMER));
	
    extGpio_pin_dir_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, GPIO_DIR_OUT);

    if (0 != ack)
    {
        return RT_ERR_FAILED;
    }

    RT_ERR_CHK(_general_smi_writeBit((mAddrs >> 8), 8, dev), ret);      /* Set reg_addr[15:8] */

    extGpio_pin_dir_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, GPIO_DIR_IN);

    con = 0;
    do
    {
        con++;
        RT_ERR_CHK(_general_smi_readBit(1, &ack, dev), ret);           /* ack for setting reg_addr[15:8] */
    } while ((0 != ack) && (con < ACK_TIMER));

    extGpio_pin_dir_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, GPIO_DIR_OUT);

    if (0 != ack)
    {
        return RT_ERR_FAILED;
    }

    RT_ERR_CHK(_general_smi_writeBit((wData & 0xFF), 8, dev), ret);    /* Write Data [7:0] out */
	
    extGpio_pin_dir_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, GPIO_DIR_IN);

    con = 0;
    do
    {
        con++;
        RT_ERR_CHK(_general_smi_readBit(1, &ack, dev), ret);           /* ack for writting data [7:0] */
    } while ((0 != ack) && (con < ACK_TIMER));

    extGpio_pin_dir_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, GPIO_DIR_OUT);

    if (0 != ack)
    {
        return RT_ERR_FAILED;
    }

    RT_ERR_CHK(_general_smi_writeBit((wData >> 8), 8, dev), ret);      /*Write Data [15:8] out */
    extGpio_pin_dir_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, GPIO_DIR_IN);

    con = 0;
    do
    {
        con++;
        RT_ERR_CHK(_general_smi_readBit(1, &ack, dev), ret);           /* ack for writting data [15:8] */
    } while ((0 != ack) && (con < ACK_TIMER));
	
    extGpio_pin_dir_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, GPIO_DIR_OUT);

    if (0 != ack)
    {
        return RT_ERR_FAILED;
    }

    /* Stop SMI */
    ret = _general_smi_stop(dev);
    if (RT_ERR_OK != ret)
    {
        return RT_ERR_FAILED;
    }
    return ret;
} /* end of _general_16bits_write */


/* Function Name:
 *      drv_smi_slavePresent
 * Description:
 *      Detect if slave is exist
 * Input:
 *      slave  - slave address
 *      dev     - dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK : slave exist
 *      RT_ERR_FAILED: slave not exist
 * Note:
 *      None
 */
int32 drv_smi_slavePresent(uint32 slave, uint32 dev)
{
    uint32 ack = 0;
    int32  ret = RT_ERR_FAILED;

    /* Start SMI */
    RT_ERR_CHK(_general_8bits_dev_start(dev), ret);
    RT_ERR_CHK(_general_smi_writeBit(slave, 7, dev), ret);
    RT_ERR_CHK(_general_smi_writeBit(0x0, 1, dev), ret);
    extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, 1);

    /*Read ack for issuing WRITE command*/
    RT_ERR_CHK(_general_smi_readBit(1, &ack, dev), ret);

    /* Check ack value, it should be zero. */
    if (0 != ack)
    {
        return RT_ERR_FAILED;
    }

    extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, 1);

    /* Stop SMI */
    RT_ERR_CHK(_general_8bits_dev_stop(dev), ret);

    return ((0 == ack) ? RT_ERR_OK : RT_ERR_FAILED);
} /* end of drv_smi_slavePresent */


/* Function Name:
 *      drv_smi_group_get
 * Description:
 *      SMI init function
 * Input:
 *      pDevSCK - SCK device id
 *      pinSCK  - SCK pin
 *      pDevDA - SDA device id
 *      pinSDA  - SDA pin
 *      dev     - dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 drv_smi_group_get(uint32 * pDevSCK, uint32 * pPinSCK, uint32 * pDevSDA, uint32 * pPinSDA, uint32 dev)
{
    *pDevSCK = GPIO_PORT(smi_dev[dev].sck.devId);
    *pPinSCK = GPIO_PIN(smi_dev[dev].sck.pinId);
    *pDevSDA = GPIO_PORT(smi_dev[dev].sda.devId);
    *pPinSDA = GPIO_PIN(smi_dev[dev].sda.pinId);

    return RT_ERR_OK;
} /* end of drv_smi_group_get */


/* Function Name:
 *      drv_smi_init
 * Description:
 *      SMI init function
 * Input:
 *      devSCK - SCK GPIO dev id
 *      pinSCK  - SCK pin
 *      devSDA - SDA GPIO dev id
 *      pinSDA  - SDA pin
 *      dev     - dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
int32 drv_smi_init(uint32 devSCK, uint32 pinSCK, uint32 devSDA, uint32 pinSDA, uint32 dev)
{
//    uint32 gpioId = 0;
			if (dev >= SMI_DEVICE_MAX)
			{
				printf("\n dev index out of range! \n");
				return RT_ERR_FAILED;
			}
		  memset(&smi_dev[dev],0,sizeof(dev_i2c_t));
    /* parameter check */
//    RT_PARAM_CHK(!GPIO_PORT_CHK(portSCK), RT_ERR_OUT_OF_RANGE);
//    RT_PARAM_CHK((pinSCK & (~GPIO_ID_PIN_MASK)), RT_ERR_INPUT);
//    RT_PARAM_CHK(!GPIO_PORT_CHK(portSDA), RT_ERR_OUT_OF_RANGE);
//    RT_PARAM_CHK((pinSDA & (~GPIO_ID_PIN_MASK)), RT_ERR_INPUT);
    //RT_PARAM_CHK(!SMI_DEVICE_CHK(dev), RT_ERR_OUT_OF_RANGE);

    /* Initialize GPIO port 'portSDA', pin 'pinSDA' as SMI SDA */
    smi_dev[dev].smi_TYPE = SFP_TYPE;
    smi_dev[dev].smi_CHIPID = SFP_CHIPID;
    smi_dev[dev].smi_DELAY = DELAY_4000;
	
    extGpio_pin_dir_set(RTL8231_ADDR, pinSDA, GPIO_DIR_OUT);

	  smi_dev[dev].sda.devId = devSDA;
	  smi_dev[dev].sda.pinId= pinSDA;

    /* Initialize GPIO port 'portSCK', pin 'pinSCK' as SMI SCK */

	  extGpio_pin_dir_set(RTL8231_ADDR, pinSCK, GPIO_DIR_OUT);

	  smi_dev[dev].sck.devId = devSCK;
	  smi_dev[dev].sck.pinId= pinSCK;
	
    extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sda.pinId, 1);
	  extGpio_pin_data_set(RTL8231_ADDR, smi_dev[dev].sck.pinId, 1);

    smi_dev[dev].valid=1;
    return RT_ERR_OK;
} /* end of drv_smi_init */

/* Function Name:
 *      drv_smi_write
 * Description:
 *      SMI write wrapper function
 * Input:
 *      mAddrs  - address
 *      wData   - data write
 *      dev     - dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
int32 drv_smi_write(uint32 mAddrs, uint32 wData, uint32 dev)
{
    int32 ret = RT_ERR_FAILED;

    /* parameter check */
    //RT_PARAM_CHK(!SMI_DEVICE_CHK(dev), RT_ERR_OUT_OF_RANGE);

    switch (smi_dev[dev].smi_TYPE)
    {
        case SMI_TYPE_8BITS_DEV:
            /* Start SMI */
            ret = _general_8bits_dev_start(dev);
            if (RT_ERR_OK != ret)
            {
                goto smi_general_write_end;
            }

            ret = _general_8bits_dev_write(mAddrs, wData, dev);

            /* Stop SMI */
            ret = _general_8bits_dev_stop(dev);
            break;
        case SMI_TYPE_16BITS_DEV:
            ret = _general_16bits_write(smi_dev[dev].smi_CHIPID, mAddrs, wData, dev);
            break;			

        default:
            break;
    }

		printf("\n[Write]devid 0x%x addr 0x%x data 0x%x ", 
	            smi_dev[dev].smi_CHIPID & 0xff, mAddrs, wData);
		printf("%s!\n",(ret==RT_ERR_OK)?"OK":"FAIL");
smi_general_write_end:
    return ret;
} /* end of drv_smi_write */

/* Function Name:
 *      drv_smi_read
 * Description:
 *      SMI read wrapper function
 * Input:
 *      mAddrs  - address
 *      dev     - dev id
 * Output:
 *      pRdata  - data read
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
int32 drv_smi_read(uint32 mAddrs, uint32 *pRdata, uint32 dev)
{
    uint8 value;
    int32 ret = RT_ERR_FAILED;

    /* parameter check */
    //RT_PARAM_CHK((NULL == pRdata), RT_ERR_NULL_POINTER);
    //RT_PARAM_CHK(!SMI_DEVICE_CHK(dev), RT_ERR_OUT_OF_RANGE);
    *pRdata = 0;
    switch (smi_dev[dev].smi_TYPE)
    {
        case SMI_TYPE_8BITS_DEV:
            /* Start SMI */
            ret = _general_8bits_dev_start(dev);
            if (RT_ERR_OK != ret)
            {
                goto smi_general_read_end;
            }

            ret = _general_8bits_dev_read(mAddrs, &value, dev);
            *pRdata = value;

            /* Stop SMI */
            ret = _general_8bits_dev_stop(dev);
            break;
			
		case SMI_TYPE_16BITS_DEV:
				ret = _general_16bits_read(smi_dev[dev].smi_CHIPID, mAddrs, pRdata, dev);
				break;

        default:
            break;
    }

    printf("\n[Read]devid 0x%x addr 0x%x data 0x%x\n", 
              smi_dev[dev].smi_CHIPID & 0xff, mAddrs, *pRdata);
smi_general_read_end:
    return ret;
} /* end of drv_smi_read */




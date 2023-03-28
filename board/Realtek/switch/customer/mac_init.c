/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * Purpose : Related definition of the customer MAC init for U-Boot.
 *
 * Feature : customer MAC init function
 *
 */


/*
 * Include Files
 */
#include <config.h>
#include <rtk_reg.h>
#include <rtk_type.h>
#include <rtk_osal.h>
#include <rtk_debug.h>
#include <rtk_switch.h>
#include <rtk/phy/rtl8214f.h>
#include <rtk/phy/rtl8218.h>
#include <rtk/drv/rtl8231/rtl8231_drv.h>

#include <rtk/drv/gpio/gpio.h>
#include <rtk/drv/gpio/ext_gpio.h>


/*
 * Symbol Definition
 */
#define CHIPVER_B_CUT       (0)                   /* B-CUT */


/*
 * Macro Definition
 */
#ifndef VALUE_CHG
#define VALUE_CHG(var,mask,val)     do { var = ((var & ~(mask)) | ((val) & (mask))); } while(0)
#endif
#ifndef VALUE_CHK
#define VALUE_CHK(var,val)          do { \
    if (var != val) \
        printf("Error:%d - Var 0x%08X != 0x%08X\n", __LINE__, var, val); \
        } while(0)
#endif
#ifndef REG32_CHG
#define REG32_CHG(reg,mask,val)     do { REG32(reg) = ((REG32(reg) & ~(mask)) | ((val) & (mask))); } while(0)
#endif
#ifndef REG32_CHK
#define REG32_CHK(reg,val)          do { \
    if (REG32(reg) != val) \
        printf("Error:%d - Reg(0x%08X), 0x%08X != 0x%08X\n", __LINE__, reg, REG32(reg), val); \
        } while(0)
#endif
#ifndef CHIPVER_CHAR
#define CHIPVER_CHAR(ver)           (ver + 0x41 + 1)
#endif
#ifndef PORTMASK_NUM
#define PORTMASK_NUM(port_num)      (0x1 << (port_num))
#endif
#ifndef PORTMASK_COUNT
#define PORTMASK_COUNT(port_count)  ((0x1 << (port_count)) - 1)
#endif

/*
 * Data Declaration
 */
extern const rtk_mac_drv_t *gMacDrv;

/*
 * Function Declaration
 */

/* Function Name:
 *      customer_mac_config_init
 * Description:
 *      Mac Configuration code for customer specified
 * Input:
 *      pModel - pointer to switch model of platform
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void customer_mac_config_init(const rtk_switch_model_t *pModel)
{
    if (pModel == NULL)
    {
        OSAL_PRINTF("%s():%d - pModel is NULL\n", __FUNCTION__, __LINE__);
        return;
    }

    /* Wait for customer to implement their configuration */

    return;
} /* end of customer_mac_config_init */

/* Function Name:
 *      customer_phy_config_init
 * Description:
 *      PHY Configuration code for customer specified
 * Input:
 *      pModel - pointer to switch model of platform
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void customer_phy_config_init(const rtk_switch_model_t *pModel)
{
    if (pModel == NULL)
    {
        OSAL_PRINTF("%s():%d - pModel is NULL\n", __FUNCTION__, __LINE__);
        return;
    }

    /* Wait for customer to implement their configuration */

    return;
} /* end of customer_phy_config_init */

void customer_mac_misc_config_init(const rtk_switch_model_t *pModel)
{
    uint32 val;

    if (pModel == NULL)
    {
        OSAL_PRINTF("%s():%d - pModel is NULL\n", __FUNCTION__, __LINE__);
        return;
    }

    /* Wait for customer to implement their configuration */

/*=======================================*/
/*==========For ZyXEL GS1900-X Begin Here==========*/
/*=======================================*/
    if(0 == strcmp(pModel->name, "ZyXEL_GS1810_8HP"))
    {
        MEM32_WRITE(0xbb00a010, 0xFF00);
    }

    /*Patch serdes parameters for direct fiber EMI, for ZyXEL_GS1900_8HPv2*/
    if(0 == strcmp(pModel->name, "ZyXEL_GS1900_8HPv2"))
    {
        MEM32_WRITE(0xbb00f7b8, 0x6882);
        MEM32_WRITE(0xbb00f8b8, 0x6882);
    }

    /*Patch serdes parameters for direct fiber EMI, for ZyXEL_GS1900_10HP*/
    if(0 == strcmp(pModel->name, "ZyXEL_GS1900_10HP"))
    {
        MEM32_WRITE(0xbb00f7b8, 0x6882);
        MEM32_WRITE(0xbb00f8b8, 0x6882);
    }

    /* Set SW control LED */
    if(0 == strcmp(pModel->name, "ZyXEL_GS1900_24HP"))
        MEM32_WRITE(0xbb00a010, 0xFFFFFF);
    else if (0 == strcmp(pModel->name, "ZyXEL_GS1900_24HPv2"))
        MEM32_WRITE(0xbb00a014, 0xFFFFFF);
    else if (0 == strcmp(pModel->name, "ZyXEL_GS1900_24EP"))
        MEM32_WRITE(0xbb00a014, 0xFFF);

    /*config combo port to fiber mode*/
    if((0 == strcmp(pModel->name, "ZyXEL_GS1900_24"))   ||
       (0 == strcmp(pModel->name, "ZyXEL_GS1900_24HP")) ||
       (0 == strcmp(pModel->name, "ZyXEL_GS1900_24HPv2")))
    {
        val = MEM32_READ(0xbb00a000);
        val &= ~(0x3<<7);
        val &= ~(0x1<<17);
        MEM32_WRITE(0xbb00a000, val);
    }

    if((0 == strcmp(pModel->name, "zyxel_gs1810_16_FIRST_16PHY"))&&((pModel->led.count / 8) == 2))
    {
        val = MEM32_READ(0xbb00a008);
        val &= ~0xFFFFFF;
        val |= 0x00FFFF;
        MEM32_WRITE(0xbb00a008, val);

        val = MEM32_READ(0xbb00a17c);
        val &= ~0xFFFFFF;
        val |= 0x00FFFF;
        MEM32_WRITE(0xbb00a17c, val);
    }

    if(0 == strcmp(pModel->name, "ZyXEL_GS1900_48HPv2"))
    {
        if (NULL == gMacDrv)
        {
            OSAL_PRINTF("%s():%d - gMacDrv is NULL\n", __FUNCTION__, __LINE__);
            return;
        }

        /* Serdes Patch for 48HPv2 */
        gMacDrv->drv_miim_write(32, 0xa40, 0x1e, 0x8);
        gMacDrv->drv_miim_write(32, 0x465, 0x13, 0x2);
    }

/*=======================================*/
/*==========For ZyXEL GS1900-X End Here==========*/
/*=======================================*/
#if defined(CONFIG_RTL8390)
    extGpio_pin_dir_set(3, 8, EXT_GPIO_DIR_OUT);
    extGpio_pin_data_set(3, 8, 1);
#endif
    return;
} /* end of customer_mac_misc_config_init */

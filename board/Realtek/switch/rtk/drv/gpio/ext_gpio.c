/*
 * Copyright(c) Realtek Semiconductor Corporation, 2013
 * All rights reserved.
 *
 * Purpose : Related definition of the ext-gpio for U-Boot.
 *
 * Feature : ext-gpio init function
 *
 */


/*
 * Include Files
 */
#include <rtk/drv/gpio/ext_gpio.h>
#include <rtk_osal.h>
#include <rtk_debug.h>
#if defined(CONFIG_RTL8390)
#include <rtk/drv/rtl8231/rtl8390_8231_drv.h>
#endif

#if defined(CONFIG_RTL8380)
#include <rtk/drv/rtl8231/rtl8380_8231_drv.h>
#endif


/*
 * Symbol Definition
 */

/*
 * Macro Definition
 */

/*
 * Data Declaration
 */
extern const rtk_extGpio_drv_t *gExtGpio;
#if defined(CONFIG_RTL8390)
static rtk_extGpio_drv_t rtl8390_drv;
#endif /* end of #if defined(CONFIG_RTL8390) */
#if defined(CONFIG_RTL8380)
static rtk_extGpio_drv_t rtl8380_drv;
#endif /* end of #if defined(CONFIG_RTL8390) */


/*
 * Macro Definition
 */
#define RTL8231_LED_FUNC0_ADDR                                  (0)
#define RTL8231_LED_FUNC0_LED_START_OFFSET                      (1)
#define RTL8231_LED_FUNC0_LED_START_MASK                        (0x1 << RTL8231_LED_FUNC0_LED_START_OFFSET)
#define RTL8231_GPIO_PIN_SEL_ADDR(gpio)                         ((0x0002) + ((gpio) >> 4))
#define RTL8231_GPIO_PIN_SEL0_ADDR                              (0x0002)
#define RTL8231_GPIO_PIN_SEL1_ADDR                              (0x0003)
#define RTL8231_GPIO_PIN_SEL2_ADDR                              (0x0004)
#define RTL8231_GPIO_IO_SEL_ADDR(gpio)                          ((0x0005) + ((gpio) >> 4))
#define RTL8231_GPIO_PIN_SEL2_IOMASK_32_OFFSET                  (5)
#define RTL8231_GPIO_CTRL_ADDR(gpio)                            ((0x001C) + ((gpio) >> 4))

/*
 * Function Declaration
 */
void extGpio_drv_init(int chip_index, rtk_extGpio_drv_t **ppExtGpioDrv)
{
    switch (chip_index)
    {
#if defined(CONFIG_RTL8390)
        case RTK_CHIP_RTL8392M:
        case RTK_CHIP_RTL8393M:
        case RTK_CHIP_RTL8396M:
        case RTK_CHIP_RTL8353M:
            rtl8390_drv.extGpio_init = rtl8390_rtl8231_init;
            rtl8390_drv.extGpio_reg_read = rtl8390_rtl8231_read;
            rtl8390_drv.extGpio_reg_write = rtl8390_rtl8231_write;
            (*ppExtGpioDrv) = &rtl8390_drv;
            break;
#endif /* end of #if defined(CONFIG_RTL8390) */
#if defined(CONFIG_RTL8380)
        case RTK_CHIP_RTL8382M:
        case RTK_CHIP_RTL8380M:
        case RTK_CHIP_RTL8332M:
        case RTK_CHIP_RTL8330M:
            rtl8380_drv.extGpio_init = rtl8380_rtl8231_init;
            rtl8380_drv.extGpio_reg_read = rtl8380_rtl8231_read;
            rtl8380_drv.extGpio_reg_write = rtl8380_rtl8231_write;
            (*ppExtGpioDrv) = &rtl8380_drv;

            break;
#endif /* end of #if defined(CONFIG_RTL8380) */
        default:
            break;
    }
}

void extGpio_init(void)
{
    if (gExtGpio)
        gExtGpio->extGpio_init();
}

int extGpio_reg_read(uint32 phy_id, uint32 reg, uint32 *pData)
{
    if (gExtGpio == NULL)
    {
        OSAL_PRINTF("gExtGpio is NULL\n");
        return 0;
    }

    if (gExtGpio->extGpio_reg_read != NULL)
    {
        return gExtGpio->extGpio_reg_read(phy_id, reg, pData);
    }
}

int extGpio_reg_write(uint32 phy_id, uint32 reg, uint32 data)
{
    if (gExtGpio == NULL)
    {
        OSAL_PRINTF("gExtGpio is NULL\n");
        return 0;
    }

    if (gExtGpio->extGpio_reg_write != NULL)
    {
        return gExtGpio->extGpio_reg_write(phy_id, reg, data);
    }
}

void extGpio_dev_enable(uint32 phy_id, uint32 enable)
{
    uint32  regVal = 0;

    extGpio_reg_read(phy_id, RTL8231_LED_FUNC0_ADDR, &regVal);
    regVal = (regVal & ~RTL8231_LED_FUNC0_LED_START_MASK) | (enable << RTL8231_LED_FUNC0_LED_START_OFFSET);
    extGpio_reg_write(phy_id, RTL8231_LED_FUNC0_ADDR, regVal);

}

void extGpio_pin_dir_get(uint32 phy_id, uint32 gpioId, ext_gpio_direction_t *pData)
{
    uint32  sel_gpio = 0;
    uint32  regVal = 0;


    extGpio_reg_read(phy_id, RTL8231_GPIO_PIN_SEL_ADDR(gpioId), &regVal);
    sel_gpio = (regVal >> (gpioId % 16)) & 0x1;
    if (sel_gpio)
    {
        /* Direction */
        if (gpioId < EXT_GPIO_ID32)
        {
            extGpio_reg_read(phy_id, RTL8231_GPIO_IO_SEL_ADDR(gpioId), &regVal);
            if ((regVal >> (gpioId % 16)) & 0x1)
                *pData = EXT_GPIO_DIR_IN;
            else
                *pData = EXT_GPIO_DIR_OUT;
        }
        else
        {
            extGpio_reg_read(phy_id, RTL8231_GPIO_PIN_SEL2_ADDR, &regVal);
            if ((regVal >> ((gpioId % 32) + RTL8231_GPIO_PIN_SEL2_IOMASK_32_OFFSET)) & 0x1)
                *pData = EXT_GPIO_DIR_IN;
            else
                *pData = EXT_GPIO_DIR_OUT;
        }
    }
    else
    {
        return;
    }
}

void extGpio_pin_dir_set(uint32 phy_id, uint32 gpioId, ext_gpio_direction_t data)
{
    uint32  regVal = 0, value;

    /* GPIO Selection */
    extGpio_reg_read(phy_id, RTL8231_GPIO_PIN_SEL_ADDR(gpioId), &regVal);
    regVal = (regVal | (1 << (gpioId % 16)));
    extGpio_reg_write(phy_id, RTL8231_GPIO_PIN_SEL_ADDR(gpioId), regVal);

    /* Direction */
    if (data == EXT_GPIO_DIR_IN)
        value = 1;
    else
        value = 0;

    if (gpioId < EXT_GPIO_ID32)
    {
        extGpio_reg_read(phy_id, RTL8231_GPIO_IO_SEL_ADDR(gpioId), &regVal);
        regVal = (regVal & ~(1 << (gpioId % 16))) | (value << (gpioId % 16));
        extGpio_reg_write(phy_id, RTL8231_GPIO_IO_SEL_ADDR(gpioId), regVal);
    }
    else
    {
        extGpio_reg_read(phy_id, RTL8231_GPIO_PIN_SEL2_ADDR, &regVal);
        regVal = (regVal & ~(1 << ((gpioId % 32) + RTL8231_GPIO_PIN_SEL2_IOMASK_32_OFFSET))) | (value << ((gpioId % 32) + RTL8231_GPIO_PIN_SEL2_IOMASK_32_OFFSET));
        extGpio_reg_write(phy_id, RTL8231_GPIO_PIN_SEL2_ADDR, regVal);
    }
}

int extGpio_pin_data_get(uint32 phy_id, uint32 gpioId, uint32 *pData)
{
    uint32  regVal = 0;

    if (0 == extGpio_reg_read(phy_id, RTL8231_GPIO_CTRL_ADDR(gpioId), &regVal))
        return 0;

    *pData = (regVal & (1 << (gpioId % 16))) >> (gpioId % 16);

    return 1;
}

int extGpio_pin_data_set(uint32 phy_id, uint32 gpioId, uint32 data)
{
    uint32  regVal = 0;

    if (0 == extGpio_reg_read(phy_id, RTL8231_GPIO_CTRL_ADDR(gpioId), &regVal))
        return 0;

    regVal = (regVal & ~(1 << (gpioId % 16))) | (data << (gpioId % 16));
    if (0 == extGpio_reg_write(phy_id, RTL8231_GPIO_CTRL_ADDR(gpioId), regVal))
        return 0;

    return 1;
}

void rtl8231_pin_status_get(unsigned int pinNum, unsigned int *pinStatus)
{
#if defined(CONFIG_RTL8380)
    uint32 phy_id = 0;
#elif defined(CONFIG_RTL8390)
    uint32 phy_id = 3;
#endif
		extGpio_pin_dir_set(phy_id,pinNum,rtl8231_PIN_DIR_IN);
		extGpio_pin_data_get(phy_id,pinNum,pinStatus);
		return;
}

void rtl8231_pin_status_set(unsigned int pinNum, unsigned int pinStatus)
{
#if defined(CONFIG_RTL8380)
    uint32 phy_id = 0;
#elif defined(CONFIG_RTL8390)
    uint32 phy_id = 3;
#endif

    extGpio_pin_dir_set(phy_id,pinNum,rtl8231_PIN_DIR_OUT);
	extGpio_pin_data_set(phy_id,pinNum,pinStatus);
}


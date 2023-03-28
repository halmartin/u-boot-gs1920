/*
 * Copyright(c) Realtek Semiconductor Corporation, 2013
 * All rights reserved.
 *
 * $Revision: 27747 $
 * $Date: 2012-04-05 14:08:35 +0800 (Thu, 05 Apr 2012) $
 *
 * Purpose :
 *
 * Feature :
 *
 */


/*
 * Include Files
 */
#include <rtk_debug.h>
#include <rtk_switch.h>
#include <rtk/drv/gpio/gpio.h>

#if defined(CONFIG_RTL8390)
#include <rtk/drv/gpio/rtl8390_gpio_drv.h>
#endif

#if defined(CONFIG_RTL8380)
#include <rtk/drv/gpio/rtl8380_gpio_drv.h>
#endif


/*
 * Data Declaration
 */
extern rtk_gpio_drv_t *gIntGpio;
#if defined(CONFIG_RTL8390)
static rtk_gpio_drv_t rtl8390_intGpio_drv;
#endif /* end of #if defined(CONFIG_RTL8390) */
#if defined(CONFIG_RTL8380)
static rtk_gpio_drv_t rtl8380_intGpio_drv;
#endif /* end of #if defined(CONFIG_RTL8390) */

/*
 * Function Declaration
 */
void intGpio_drv_init(int chip_index, rtk_gpio_drv_t **ppintGpioDrv)
{
    switch (chip_index)
    {
#if defined(CONFIG_RTL8390)
        case RTK_CHIP_RTL8392M:
        case RTK_CHIP_RTL8393M:
        case RTK_CHIP_RTL8396M:
        case RTK_CHIP_RTL8353M:
	    rtl8390_intGpio_drv.intGpio_pindata_get = rtl8390_intGpio_pindata_get;
            rtl8390_intGpio_drv.intGpio_pindata_set = rtl8390_intGpio_pindata_set;
            (*ppintGpioDrv) = &rtl8390_intGpio_drv;
            break;
#endif /* end of #if defined(CONFIG_RTL8390) */
#if defined(CONFIG_RTL8380)
        case RTK_CHIP_RTL8382M:
        case RTK_CHIP_RTL8380M:
        case RTK_CHIP_RTL8332M:
        case RTK_CHIP_RTL8330M:
            rtl8380_intGpio_drv.intGpio_pindata_get = rtl8380_intGpio_pindata_get;
            rtl8380_intGpio_drv.intGpio_pindata_set = rtl8380_intGpio_pindata_set;
            (*ppintGpioDrv) = &rtl8380_intGpio_drv;
			
            break;
#endif /* end of #if defined(CONFIG_RTL8380) */
        default:
            break;
    }
}

void intGpio_PinSet(uint32 pin, uint32 bitData)
{
    if (gIntGpio == NULL)
    {
        OSAL_PRINTF("gIntGpio is NULL\n");
        return;
    }
    
    if (gIntGpio->intGpio_pindata_set != NULL)
    {
        gIntGpio->intGpio_pindata_set(pin,bitData);
    }
}

void intGpio_PinGet(uint32 pin, uint32 *pBitData)
{
		*pBitData = 0x0;
    if (gIntGpio == NULL)
    {
        OSAL_PRINTF("gIntGpio is NULL\n");
        return;
    }
    
    if (gIntGpio->intGpio_pindata_get != NULL)
    {
        gIntGpio->intGpio_pindata_get(pin,pBitData);
    }
    return;
}

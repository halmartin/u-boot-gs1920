
#ifndef ____RTL8380_GPIO_DRV_H____
#define ____RTL8380_GPIO_DRV_H____

/*
 * Include Files
 */
#include <rtk/drv/gpio/gpio.h>

extern void rtl8380_intGpio_pindata_get(uint32 pin, uint32 *pBitData);
extern void rtl8380_intGpio_pindata_set(uint32 pin, uint32 bitData);

#endif	/* ____RTL8380_GPIO_DRV_H____ */


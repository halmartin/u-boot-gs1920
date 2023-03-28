/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * Purpose : Related definition of the RTL8231 driver for U-Boot.
 *
 * Feature : RTL8231 driver function
 *
 */

#ifndef	__RTL8231_DRV_H__
#define	__RTL8231_DRV_H__

/*
 * Include Files
 */
#include <rtk_switch.h>

/*
 * Symbol Definition
 */
typedef enum rtl8231_pin_state_e
{
    RTL8231_PIN_NORMAL = 0,
    RTL8231_PIN_GPI,
    RTL8231_PIN_GPO,
    RTL8231_PIN_STAT_END
} rtl8231_pin_state_t;

typedef enum rtl8231_pin_direction_e
{
    rtl8231_PIN_DIR_IN = 0,
    rtl8231_PIN_DIR_OUT,
    rtl8231_PIN_DIR_END
}rtl8231_pin_direction_t;


typedef enum rtl8231_pin_e
{
    RTL8231_PIN_0 = 0,
    RTL8231_PIN_1,
    RTL8231_PIN_2,
    RTL8231_PIN_3,
    RTL8231_PIN_4,
    RTL8231_PIN_5,
    RTL8231_PIN_6,
    RTL8231_PIN_7,
    RTL8231_PIN_8,
    RTL8231_PIN_9,
    RTL8231_PIN_10,
    RTL8231_PIN_11,
    RTL8231_PIN_12,
    RTL8231_PIN_13,
    RTL8231_PIN_14,
    RTL8231_PIN_15,
    RTL8231_PIN_16,
    RTL8231_PIN_17,
    RTL8231_PIN_18,
    RTL8231_PIN_19,
    RTL8231_PIN_20,
    RTL8231_PIN_21,
    RTL8231_PIN_22,
    RTL8231_PIN_23,
    RTL8231_PIN_24,
    RTL8231_PIN_25,
    RTL8231_PIN_26,
    RTL8231_PIN_27,
    RTL8231_PIN_28,
    RTL8231_PIN_29,
    RTL8231_PIN_30,
    RTL8231_PIN_31,
    RTL8231_PIN_32,
    RTL8231_PIN_33,
    RTL8231_PIN_34,
    RTL8231_PIN_35,
    RTL8231_PIN_36,
    RTL8231_PIN_END
}rtl8231_pin_t;

#define RTL8231_ADDR  0x0
#define RTL8231_PIN_SEL_REG 0x2
#define RTL8231_PIN_DIR_REG 0x5
#define RTL8231_PIN_DATA_REG 0x1c

#define GPIO_CTRL_REG_BASE (0xb8003500)
#define GPIO_PABC_CNR   (GPIO_CTRL_REG_BASE +0x0)
#define GPIO_PABC_DIR   (GPIO_CTRL_REG_BASE +0x8)
#define GPIO_PABC_DATA  (GPIO_CTRL_REG_BASE +0xc)
#define GPIO_PABC_ISR  (GPIO_CTRL_REG_BASE +0x10)
#define GPIO_PAB_IMR  (GPIO_CTRL_REG_BASE +0x14)
#define GPIO_PC_IMR  (GPIO_CTRL_REG_BASE +0x18)


/*
 * Function Declaration
 */
extern void rtl8231_setStartStatus(Tuint8 start);
extern void rtl8231_setPinStatus(Tuint8 pinId, rtl8231_pin_state_t state);
extern int rtl8231_setPinDeBouncing(Tuint8 pinId, Tuint8 enable);
extern void rtl8231_pin_direction_set(rtl8231_pin_t pin, rtl8231_pin_direction_t dir);
extern void rtl8231_pin_data_set(rtl8231_pin_t pin, uint32 data);
extern void rtl8231_pin_data_get(rtl8231_pin_t pin, uint32 *pData);

#endif	/* __RTL8231_DRV_H__ */

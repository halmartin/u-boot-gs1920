/*
 * Copyright(c) Realtek Semiconductor Corporation, 2013
 * All rights reserved.
 *
 * Purpose : Related definition of the ext-gpio for U-Boot.
 *
 * Feature : ext-gpio init function
 *
 */

#ifndef	__EXT_GPIO_H__
#define	__EXT_GPIO_H__

#include <rtk_switch.h>

typedef enum drv_extGpio_id_e
{
    EXT_GPIO_ID0 = 0,
    EXT_GPIO_ID1,
    EXT_GPIO_ID2,
    EXT_GPIO_ID3,
    EXT_GPIO_ID4,
    EXT_GPIO_ID5,
    EXT_GPIO_ID6,
    EXT_GPIO_ID7,
    EXT_GPIO_ID8,
    EXT_GPIO_ID9,
    EXT_GPIO_ID10,
    EXT_GPIO_ID11,
    EXT_GPIO_ID12,
    EXT_GPIO_ID13,
    EXT_GPIO_ID14,
    EXT_GPIO_ID15,
    EXT_GPIO_ID16,
    EXT_GPIO_ID17,
    EXT_GPIO_ID18,
    EXT_GPIO_ID19,
    EXT_GPIO_ID20,
    EXT_GPIO_ID21,
    EXT_GPIO_ID22,
    EXT_GPIO_ID23,
    EXT_GPIO_ID24,
    EXT_GPIO_ID25,
    EXT_GPIO_ID26,
    EXT_GPIO_ID27,
    EXT_GPIO_ID28,
    EXT_GPIO_ID29,
    EXT_GPIO_ID30,
    EXT_GPIO_ID31,
    EXT_GPIO_ID32,
    EXT_GPIO_ID33,
    EXT_GPIO_ID34,
    EXT_GPIO_ID35,
    EXT_GPIO_ID36,
    EXT_GPIO_ID_END
} drv_extGpio_id_t;

typedef enum EXT_GPIO_DIRECTION_E
{
    EXT_GPIO_DIR_IN = 0,
    EXT_GPIO_DIR_OUT,
    EXT_GPIO_DIR_END
} ext_gpio_direction_t;

extern void extGpio_drv_init(int chip_index, rtk_extGpio_drv_t **ppExtGpioDrv);
extern void extGpio_init(void);
extern int extGpio_reg_read(uint32 phy_id, uint32 reg, uint32 *pData);
extern int extGpio_reg_write(uint32 phy_id, uint32 reg, uint32 data);
extern void extGpio_dev_enable(uint32 phy_id, uint32 enable);
extern void extGpio_pin_dir_get(uint32 phy_id, uint32 gpioId, ext_gpio_direction_t *pData);
extern void extGpio_pin_dir_set(uint32 phy_id, uint32 gpioId, ext_gpio_direction_t data);
extern int extGpio_pin_data_get(uint32 phy_id, uint32 gpioId, uint32 *pData);
extern int extGpio_pin_data_set(uint32 phy_id, uint32 gpioId, uint32 data);
extern void rtl8231_pin_status_get(unsigned int pinNum, unsigned int *pinStatus);
extern void rtl8231_pin_status_set(unsigned int pinNum, unsigned int pinStatus);
#endif	/* __EXT_GPIO_H__ */


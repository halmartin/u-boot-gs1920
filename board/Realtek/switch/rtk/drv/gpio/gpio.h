#ifndef __GPIO_H__
#define __GPIO_H__

/*
 * Include Files
 */
#include <rtk_type.h>
#include <rtk/drv/swled/swctrl_led_main.h>


/*
 * Symbol Definition
 */
typedef uint32 gpioID;

/* define GPIO port */
typedef enum drv_intGpio_port_e
{
    GPIO_PORT_A = 0,
    GPIO_PORT_B,
    GPIO_PORT_C,
    GPIO_PORT_D,
    GPIO_PORT_E,
    GPIO_PORT_F,
    GPIO_PORT_G,
    GPIO_PORT_END
} drv_gpio_port_t;

/* define GPIO pin function */
typedef enum drv_intGpio_func_e
{
    GPIO_FUNC_CONTROL = 0,
    GPIO_FUNC_DIRECTION,
    GPIO_FUNC_DATA,
    GPIO_FUNC_INTERRUPT_STATUS,
    GPIO_FUNC_INTERRUPT_ENABLE,
    GPIO_FUNC_END
} drv_gpio_func_t;

/* define internal GPIO control function */
typedef enum drv_intGpio_control_e
{
    GPIO_CTRLFUNC_NORMAL = 0,
    GPIO_CTRLFUNC_DEDICATE_PERIPHERAL,
    GPIO_CTRLFUNC_END
} drv_gpio_control_t;

/* define internal GPIO direction */
typedef enum drv_intGpio_direction_e
{
    GPIO_DIR_IN = 0,
    GPIO_DIR_OUT,
    GPIO_DIR_END
} drv_gpio_direction_t;

/* define internal GPIO Interrupt Type */
typedef enum drv_gpio_interruptType_e
{
    GPIO_INT_DISABLE = 0,
    GPIO_INT_FALLING_EDGE,
    GPIO_INT_RISING_EDGE,
    GPIO_INT_BOTH_EDGE,
    GPIO_INT_TYPE_END
} drv_gpio_interruptType_t;

typedef struct rtk_gpio_drv_s
{
    void (*intGpio_pindata_get)(uint32, uint32 *);
    void (*intGpio_pindata_set)(uint32, uint32);
} rtk_gpio_drv_t;


/*
 * Macro Definition
 */

/* GPIO control registers
*/

#define GPIO_CTRL_REG_BASE (0xb8003500)
#define GPIO_PABC_CNR   (GPIO_CTRL_REG_BASE +0x0)
#define GPIO_PABC_DIR   (GPIO_CTRL_REG_BASE +0x8)
#define GPIO_PABC_DATA  (GPIO_CTRL_REG_BASE +0xc)
#define GPIO_PABC_ISR  (GPIO_CTRL_REG_BASE +0x10)
#define GPIO_PAB_IMR  (GPIO_CTRL_REG_BASE +0x14)
#define GPIO_PC_IMR  (GPIO_CTRL_REG_BASE +0x18)

#define GPIO_BASE                           0xB8003500
#define PABCDCNR                            (0x000 + GPIO_BASE)     /* Port ABCD control */
#define PABCDPTYPE                          (0x004 + GPIO_BASE)     /* Port ABCD type */
#define PABCDDIR                            (0x008 + GPIO_BASE)     /* Port ABCD direction */
#define PABCDDAT                            (0x00C + GPIO_BASE)     /* Port ABCD data */
#define PABCDISR                            (0x010 + GPIO_BASE)     /* Port ABCD interrupt status */
#define PABIMR                              (0x014 + GPIO_BASE)     /* Port AB interrupt mask */
#define PCDIMR                              (0x018 + GPIO_BASE)     /* Port CD interrupt mask */
#define PEFGHCNR                            (0x01C + GPIO_BASE)     /* Port ABCD control */
#define PEFGHPTYPE                          (0x020 + GPIO_BASE)     /* Port ABCD type */
#define PEFGHDIR                            (0x024 + GPIO_BASE)     /* Port ABCD direction */
#define PEFGHDAT                            (0x028 + GPIO_BASE)     /* Port ABCD data */
#define PEFGHISR                            (0x02C + GPIO_BASE)     /* Port ABCD interrupt status */
#define PEFIMR                              (0x030 + GPIO_BASE)     /* Port AB interrupt mask */
#define PGHIMR                              (0x034 + GPIO_BASE)     /* Port CD interrupt mask */


#define GPIO_ID_PORT_OFFSET         3
#define GPIO_ID_PIN_MASK            ((1 << GPIO_ID_PORT_OFFSET) - 1)
#define GPIO_ID(port,pin)           (((uint32)port << GPIO_ID_PORT_OFFSET) | ((uint32)pin & GPIO_ID_PIN_MASK))
#define GPIO_PORT(id)               (id >> GPIO_ID_PORT_OFFSET)

#define GPIO_PIN(id)                (id & GPIO_ID_PIN_MASK)
#define GPIO_PIN_MIN                0
#define GPIO_PIN_MAX                7

#define GPIO_PIN_CHK(pin)           ((pin >= GPIO_PIN_MIN) && (pin <= GPIO_PIN_MAX))
#define GPIO_DIR_CHK(dir)           ((dir >= GPIO_DIR_IN) && (dir < GPIO_DIR_END))
#define GPIO_INT_CHK(type)          ((type >= GPIO_INT_DISABLE) && (type < GPIO_INT_TYPE_END))



extern void intGpio_drv_init(int chip_index, rtk_gpio_drv_t **ppintGpioDrv);
extern void intGpio_PinSet(uint32 pin, uint32 bitData);
extern void intGpio_PinGet(uint32 pin, uint32 *pBitData);

#endif  /*__GPIO_H__*/


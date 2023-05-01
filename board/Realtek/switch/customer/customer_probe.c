/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * Purpose : Related definition of the board probe for U-Boot.
 *
 * Feature : board probe function
 *
 */


/*
 * Include Files
 */
#include <config.h>
#include <rtk_osal.h>
#include <rtk_switch.h>
#include <init.h>
#include <rtk/drv/gpio/ext_gpio.h>
#include <rtk/drv/rtl8231/rtl8231_drv.h>
#include <customer/loader/customer_board.c>


/*
 * Symbol Definition
 */
#define ENV_BOARD_MODEL     "boardmodel"

#define CONFIG_BOARD_SEL_TYPE_CONFIG   1
#define CONFIG_BOARD_SEL_TYPE_GPIO     0

/*
 * Data Declaration
 */
const uint32 boardModelExtGpio[] = {
    RTL8231_PIN_0,
    RTL8231_PIN_1,
    RTL8231_PIN_2,
    RTL8231_PIN_4
};

const char* evn_boardModel[] = {
    "ZyXEL_GS1900_8",
    "ZyXEL_GS1900_8HP",
    "ZyXEL_GS1900_16",
    "ZyXEL_GS1900_24E",
    "ZyXEL_GS1900_24",
    "ZyXEL_GS1900_24HP",
    "ZyXEL_GS1900_48",
    "ZyXEL_GS1900_48HP",
    "ZyXEL_GS1900_8HPv2",
    "ZyXEL_GS1900_10HP",
    "ZyXEL_GS1900_24EP",
    "ZyXEL_GS1900_24HPv2",
    "ZyXEL_GS1900_48HPv2",
    "ZyXEL_GS1920_24HP"
};

extern int board_model_set(int modelId);

/*
 * Macro Definition
 */

/*
 * Function Declaration
 */
#if CONFIG_BOARD_SEL_TYPE_GPIO
static int _customer_board_model_get(uint32 *model)
{
    uint32 data = 0;
    uint32 readVal = 0;
    uint32 gpioId;
#if defined(CONFIG_RTL8380)
    uint32 phy_id = 0;
#endif
#if defined(CONFIG_RTL8390)
    uint32 phy_id = 3;
#endif

    extGpio_init();
    extGpio_dev_enable(phy_id, 1);

    for(gpioId = 0; gpioId < sizeof(boardModelExtGpio)/sizeof(boardModelExtGpio[0]); gpioId++)
    {
        extGpio_pin_dir_set(phy_id, boardModelExtGpio[gpioId], EXT_GPIO_DIR_IN);
    }

    for(gpioId = 0; gpioId < sizeof(boardModelExtGpio)/sizeof(boardModelExtGpio[0]); gpioId++)
    {
        if (0 == extGpio_pin_data_get(phy_id, boardModelExtGpio[gpioId], &data))
            return 1;

        readVal |= (data << gpioId);
    }

    *model = readVal;
    return 0;
}
#endif

/* Function Name:
 *      customer_board_probe
 * Description:
 *      Probe the customer board
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      0 - Success
 * Note:
 *      None
 */
int customer_board_probe(rtk_switch_model_t **pSwitchModel)
{
    uint32 model = 0;
#if CONFIG_BOARD_SEL_TYPE_CONFIG
    char    *board_model;
#endif
#if CONFIG_BOARD_SEL_TYPE_GPIO
    int ret = 1;
#endif

/*=================================
  * Board model probe by menu configuration
  *=================================*/
#if CONFIG_BOARD_SEL_TYPE_CONFIG
    board_model = getenv(ENV_BOARD_MODEL);

#if defined(CONFIG_ZYXEL_GS1810_48)
    if (board_model && ((0 == strcmp(board_model, "ZyXEL_GS1900_48"))))
    {
        gSwitchModel = &zyxel_gs1810_48;
        model = 6;
    }
    else
#endif
#if defined(CONFIG_ZYXEL_GS1810_48_HP)
    if (board_model && ((0 == strcmp(board_model, "ZyXEL_GS1900_48HP"))))
    {
        gSwitchModel = &zyxel_gs1810_48_hp;
        model = 7;
    }
    else
#endif
#if defined(CONFIG_ZYXEL_GS1810_24)
    if (board_model && ((0 == strcmp(board_model, "ZyXEL_GS1900_24"))))
    {
        gSwitchModel = &zyxel_gs1810_24;
        model = 4;
    }
    else
#endif
#if defined(CONFIG_ZYXEL_GS1810_24E)
    if (board_model && ((0 == strcmp(board_model, "ZyXEL_GS1900_24E"))))
    {
        gSwitchModel = &zyxel_gs1810_24E;
        model = 3;
    }
    else
#endif
#if defined(CONFIG_ZYXEL_GS1810_24HP)
    if (board_model && ((0 == strcmp(board_model, "ZyXEL_GS1900_24HP"))))
    {
        gSwitchModel = &zyxel_gs1810_24HP;
        model = 5;
    }
    else
#endif
#if defined(CONFIG_ZYXEL_GS1810_16)
    if (board_model && ((0 == strcmp(board_model, "ZyXEL_GS1900_16"))))
    {
        gSwitchModel = &zyxel_gs1810_16;
        model = 2;
    }
    else
#endif
#if defined(CONFIG_ZYXEL_GS1810_16_FIRST_16PHY)
    if (board_model && ((0 == strcmp(board_model, "zyxel_gs1810_16_FIRST_16PHY"))))
    {
        gSwitchModel = &zyxel_gs1810_16_FIRST_16PHY;
        model = 2;
    }
    else
#endif
#if defined(CONFIG_ZYXEL_GS1810_8)
    if (board_model && ((0 == strcmp(board_model, "ZyXEL_GS1900_8"))))
    {
        gSwitchModel = &zyxel_gs1810_8;
        model = 0;
    }
    else
#endif
#if defined(CONFIG_ZYXEL_GS1810_8HP)
    if (board_model && ((0 == strcmp(board_model, "ZyXEL_GS1900_8HP"))))
    {
        gSwitchModel = &zyxel_gs1810_8HP;
        model = 1;
    }
    else
#endif
#if defined(CONFIG_ZYXEL_GS1810_8HPv2)
    if (board_model && ((0 == strcmp(board_model, "ZyXEL_GS1900_8HPv2"))))
    {
        gSwitchModel = &zyxel_gs1810_8HPv2;
        model = 8;
    }
    else
#endif
#if defined(CONFIG_ZYXEL_GS1810_10HP)
    if (board_model && ((0 == strcmp(board_model, "ZyXEL_GS1900_10HP"))))
    {
        gSwitchModel = &zyxel_gs1810_10HP;
        model = 9;
    }
    else
#endif
#if defined(CONFIG_GS1900v2)
    if (board_model && ((0 == strcmp(board_model, "ZyXEL_GS1900_24EP"))))
    {
        gSwitchModel = &zyxel_gs1810_24EP;
        model = 10;
    }
    else
    if (board_model && ((0 == strcmp(board_model, "ZyXEL_GS1900_24HPv2"))))
    {
        gSwitchModel = &zyxel_gs1810_24HPv2;
        model = 11;
    }
    else
    if (board_model && ((0 == strcmp(board_model, "ZyXEL_GS1900_48HPv2"))))
    {
        gSwitchModel = &zyxel_gs1810_48HPv2;
        model = 12;
    }
    else
#endif
#if defined(CONFIG_GS1920)
    if (board_model && ((0 == strcmp(board_model, "ZyXEL_GS1920_24HP"))))
    {
    	gSwitchModel = &zyxel_gs1920_24HP;
    	model = 13;
    }
    else
#endif
    {   /* Not match customer board */
        (*pSwitchModel) = NULL;
        model = 0;
    }
#endif

/*=================================
  * Board model probe by GPIO
  *=================================*/
#if CONFIG_BOARD_SEL_TYPE_GPIO
#if defined(CONFIG_MDC_MDIO_EXT_SUPPORT)
    while(1 == ret)
    {
        ret = _customer_board_model_get(&model);
    }

    switch (model)
    {
        case 0x0:
            gSwitchModel = &zyxel_gs1810_8;
            break;
        case 0x1:
            gSwitchModel = &zyxel_gs1810_8HP;
            break;
        case 0x2:
            gSwitchModel = &zyxel_gs1810_16;
            break;
        case 0x3:
            gSwitchModel = &zyxel_gs1810_24E;
            break;
        case 0x4:
            gSwitchModel = &zyxel_gs1810_24;
            break;
        case 0x5:
            gSwitchModel = &zyxel_gs1810_24HP;
            break;
        case 0x6:
            gSwitchModel = &zyxel_gs1810_48;
            break;
        case 0x7:
            gSwitchModel = &zyxel_gs1810_48_hp;
            break;
        case 0x8:
            gSwitchModel = &zyxel_gs1810_8HPv2;
            break;
        case 0x9:
            gSwitchModel = &zyxel_gs1810_10HP;
            break;
        case 0xA:
            gSwitchModel = &zyxel_gs1810_24EP;
            break;
        case 0xB:
            gSwitchModel = &zyxel_gs1810_24HPv2;
            break;
        case 0xC:
            gSwitchModel = &zyxel_gs1810_48HPv2;
            break;
        default:
            /* Not match customer board */
            (*pSwitchModel) = NULL;
    }

    setenv("boardmodel", evn_boardModel[model]);
#endif
#endif
    board_model_set(model);

    return 0;
} /* end of customer_board_probe */

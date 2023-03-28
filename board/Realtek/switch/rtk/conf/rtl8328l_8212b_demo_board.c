/*
 * Copyright(c) Realtek Semiconductor Corporation, 2010
 * All rights reserved.
 *
 * Purpose : Related implementation of the RTL8328L_8212B demo board for U-Boot.
 *
 * Feature : RTL8328L_8212B demo board database
 *
 */


/*
 * Include Files
 */


/*
 * Symbol Definition
 */


/*
 * Data Declaration
 */
static const rtk_switch_model_t rtl8328l_8212b_demo = {
    .name = "RTL8328L_8212B_DEMO",
    .chip = RTK_CHIP_RTL8328L,
    .led.offset = 0,
    .led.count = 24,
    .led.num = 4,
    .led.sel_pwr_on_led = 0xD,
    .led.sel_p0_p23_led_mod = 0x5980,
    .led.sel_p24_p27_led_mod = 0x6870,
    .port.offset = 0,
    .port.count = 26,
    .port.list = {
        /* Port 0 ~ 7 */
        { .phy_idx = 0, .phy = 0 }, { .phy_idx = 0, .phy = 1 }, { .phy_idx = 0, .phy = 2 }, { .phy_idx = 0, .phy = 3 },
        { .phy_idx = 0, .phy = 4 }, { .phy_idx = 0, .phy = 5 }, { .phy_idx = 0, .phy = 6 }, { .phy_idx = 0, .phy = 7 },
        /* Port 8 ~ 15 */
        { .phy_idx = 1, .phy = 0 }, { .phy_idx = 1, .phy = 1 }, { .phy_idx = 1, .phy = 2 }, { .phy_idx = 1, .phy = 3 },
        { .phy_idx = 1, .phy = 4 }, { .phy_idx = 1, .phy = 5 }, { .phy_idx = 1, .phy = 6 }, { .phy_idx = 1, .phy = 7 },
        /* Port 16 ~ 23 */
        { .phy_idx = 2, .phy = 0 }, { .phy_idx = 2, .phy = 1 }, { .phy_idx = 2, .phy = 2 }, { .phy_idx = 2, .phy = 3 },
        { .phy_idx = 2, .phy = 4 }, { .phy_idx = 2, .phy = 5 }, { .phy_idx = 2, .phy = 6 }, { .phy_idx = 2, .phy = 7 },
        /* Port 24 ~ 25 */
        { .phy_idx = 3, .phy = 0 }, { .phy_idx = 3, .phy = 1 },
    },  /* port.list */
    .serdes.offset = 0,
    .serdes.count = 3,
    .serdes.list = {
        { .phy_idx = 3, .mii = RTK_MII_RSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 3, .mii = RTK_MII_RSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 2, .mii = RTK_MII_RSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
    },  /* serdes.list */
    .phy.baseid = 0,
    .phy.count = 4,
    .phy.list = {
        [0] =   { .chip = RTK_CHIP_RTL8208D, .phyid = 0,  .phy_max = 8 },
        [1] =   { .chip = RTK_CHIP_RTL8208D, .phyid = 8,  .phy_max = 8 },
        [2] =   { .chip = RTK_CHIP_RTL8208D, .phyid = 16,  .phy_max = 8 },
        [3] =   { .chip = RTK_CHIP_RTL8212B, .phyid = 24, .phy_max = 2 },
    }   /* .phy.list */
};

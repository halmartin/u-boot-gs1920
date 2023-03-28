/*
 * Copyright(c) Realtek Semiconductor Corporation, 2012
 * All rights reserved.
 *
 * Purpose : Related implementation of the RTL8382M_8218B_8218FB demo board for U-Boot.
 *
 * Feature : RTL8382M_8282B_8218BF demo board database
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
static const rtk_switch_model_t rtl8330m_intphy_8212b_demo = {
    .name = "RTL8330M_INTPHY_8212B_DEMO",
    .chip = RTK_CHIP_RTL8330M,

    .led.offset = 0,
    .led.count = 10,
    .led.num = 3,
    .led.p0_p23_led_num = 3,
    .led.p24_p27_led_num = 3,
    .led.sel_pwr_on_led = 0xd,
    .led.sel_p0_p23_led_mod = 0xF5018B & 0x7fff,
    .led.sel_p24_p27_led_mod = (0xF5018B >> 15) & 0x7fff,

    .port.offset = 0,
    .port.count = 10,
    .port.list = {
        /* Port 8 ~ 15 */
        { .mac_id = 8,  .phy_idx = 0, .phy = 0 },
        { .mac_id = 9,  .phy_idx = 0, .phy = 1 },
        { .mac_id = 10, .phy_idx = 0, .phy = 2 },
        { .mac_id = 11, .phy_idx = 0, .phy = 3 },
        { .mac_id = 12, .phy_idx = 0, .phy = 4 },
        { .mac_id = 13, .phy_idx = 0, .phy = 5 },
        { .mac_id = 14, .phy_idx = 0, .phy = 6 },
        { .mac_id = 15, .phy_idx = 0, .phy = 7 },
        /* Port 24~ 27 */
        { .mac_id = 24, .phy_idx = 1, .phy = 0 },
        { .mac_id = 25, .phy_idx = 1, .phy = 1 },
    },  /* port.list */

    .serdes.offset = 0,
    .serdes.count = 6,
    .serdes.list = {
        { .phy_idx = 0, .mii = RTK_MII_XSMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 0, .mii = RTK_MII_XSMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 0, .mii = RTK_MII_XSMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 0, .mii = RTK_MII_XSMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 1, .mii = RTK_MII_RSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 1, .mii = RTK_MII_NONE, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
    },  /* serdes.list */

    .phy.baseid = 0,
    .phy.count = 2,
    .phy.list = {
        [0] =   { .chip = RTK_CHIP_RTL8218B, .mac_id = 8,  .phy_max = 8 },
        [1] =   { .chip = RTK_CHIP_RTL8214FB, .mac_id = 24,  .phy_max = 2 },
    }   /* .phy.list */
};

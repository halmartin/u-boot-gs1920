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
static const rtk_switch_model_t rtl8382m_8218b_intphy_8218b_8214fc_demo = {
    .name = "RTL8382M_8218B_INTPHY_8218B_8214FC_DEMO",
    .chip = RTK_CHIP_RTL8382M,
    .led.offset = 0,
    .led.count = 28,
    .led.num = 2,
    .led.p0_p23_led_num = 2,
    .led.p24_p27_led_num = 2,
    .led.sel_pwr_on_led = 0xd,
    .led.sel_p0_p23_led_mod = 0x1ea,
    .led.sel_p24_p27_led_mod = 0x1ea,

    .port.offset = 0,  /*Should Always be zero*/
    .port.count = 28,
    .port.list = {
        /* Port 0 ~ 7 */
        { .mac_id = 0,  .phy_idx = 0, .phy = 0 },
        { .mac_id = 1,  .phy_idx = 0, .phy = 1 },
        { .mac_id = 2,  .phy_idx = 0, .phy = 2 },
        { .mac_id = 3,  .phy_idx = 0, .phy = 3 },
        { .mac_id = 4,  .phy_idx = 0, .phy = 4 },
        { .mac_id = 5,  .phy_idx = 0, .phy = 5 },
        { .mac_id = 6,  .phy_idx = 0, .phy = 6 },
        { .mac_id = 7,  .phy_idx = 0, .phy = 7 },
        /* Port 8 ~ 15 */
        { .mac_id = 8,  .phy_idx = 1, .phy = 0 },
        { .mac_id = 9,  .phy_idx = 1, .phy = 1 },
        { .mac_id = 10, .phy_idx = 1, .phy = 2 },
        { .mac_id = 11, .phy_idx = 1, .phy = 3 },
        { .mac_id = 12, .phy_idx = 1, .phy = 4 },
        { .mac_id = 13, .phy_idx = 1, .phy = 5 },
        { .mac_id = 14, .phy_idx = 1, .phy = 6 },
        { .mac_id = 15, .phy_idx = 1, .phy = 7 },
        /* Port 16 ~ 23 */
        { .mac_id = 16, .phy_idx = 2, .phy = 0 },
        { .mac_id = 17, .phy_idx = 2, .phy = 1 },
        { .mac_id = 18, .phy_idx = 2, .phy = 2 },
        { .mac_id = 19, .phy_idx = 2, .phy = 3 },
        { .mac_id = 20, .phy_idx = 2, .phy = 4 },
        { .mac_id = 21, .phy_idx = 2, .phy = 5 },
        { .mac_id = 22, .phy_idx = 2, .phy = 6 },
        { .mac_id = 23, .phy_idx = 2, .phy = 7 },
        /* Port 24~ 27 */
        { .mac_id = 24, .phy_idx = 3, .phy = 0 },
        { .mac_id = 25, .phy_idx = 3, .phy = 1 },
        { .mac_id = 26, .phy_idx = 3, .phy = 2 },
        { .mac_id = 27, .phy_idx = 3, .phy = 3 },
    },  /* port.list */

    .serdes.offset = 0,
    .serdes.count = 6,
    .serdes.list = {
        { .phy_idx = 0, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 0, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 2, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 2, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 3, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 3, .mii = RTK_MII_NONE, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
    },  /* serdes.list */

    .phy.baseid = 0,
    .phy.count = 4,
    .phy.list = {
        [0] =   { .chip = RTK_CHIP_RTL8218B, .mac_id = 0,  .phy_max = 8 },
        [1] =   { .chip = RTK_CHIP_RTL8218B, .mac_id = 8,  .phy_max = 8 },
        [2] =   { .chip = RTK_CHIP_RTL8218B, .mac_id = 16,  .phy_max = 8 },
        [3] =   { .chip = RTK_CHIP_RTL8214FC, .mac_id = 24,  .phy_max = 4 },
    }   /* .phy.list */
};

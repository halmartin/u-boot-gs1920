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
static const rtk_switch_model_t rtl8332m_8208l_8208l_8214b_qa = {
    .name = "RTL8332M_8208L_8208L_8214B_QA",
    .chip = RTK_CHIP_RTL8332M,
    .led.offset = 0,
    .led.count = 28,
    .led.num = 3,
    .led.p0_p23_led_num = 3,
    .led.p24_p27_led_num = 3,
    .led.sel_pwr_on_led = 0xd,
    .led.sel_p0_p23_led_mod = 0xF5018B & 0x7fff,
    .led.sel_p24_p27_led_mod = (0xF5018B >> 15) & 0x7fff,
    .port.offset = 0,
    .port.count = 28,
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
        /* Port 24~ 27 */
        { .phy_idx = 3, .phy = 0 }, { .phy_idx = 3, .phy = 1 }, { .phy_idx = 3, .phy = 2 }, { .phy_idx = 3, .phy = 3 },
    },  /* port.list */
    .serdes.offset = 0,
    .serdes.count = 6,
    .serdes.list = {
        { .phy_idx = 0, .mii = RTK_MII_XSMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 0, .mii = RTK_MII_XSMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 2, .mii = RTK_MII_XSMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 2, .mii = RTK_MII_XSMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 3, .mii = RTK_MII_RSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 3, .mii = RTK_MII_RSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
    },  /* serdes.list */
    .phy.baseid = 0,
    .phy.count = 4,
    .phy.list = {
        [0] =   { .chip = RTK_CHIP_RTL8208L, .mac_id = 0,  .phy_max = 8 },
        [1] =   { .chip = RTK_CHIP_RTL8218B, .mac_id = 8,  .phy_max = 8 },
        [2] =   { .chip = RTK_CHIP_RTL8208L, .mac_id = 16,  .phy_max = 8 },
        [3] =   { .chip = RTK_CHIP_RTL8214FB, .mac_id = 24,  .phy_max = 8 },
    }   /* .phy.list */
};

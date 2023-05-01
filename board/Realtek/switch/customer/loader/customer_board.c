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
 static const rtk_switch_model_t zyxel_gs1810_8 = {
     .name = "ZyXEL_GS1900_8",
     .chip = RTK_CHIP_RTL8382M,
     .led.offset = 0,
     .led.count = 8,
     .led.p0_p23_led_num = 1,
     .led.p24_p27_led_num = 1,
     .led.sel_pwr_on_led = 0xd,
     .led.sel_p0_p23_led_mod = 0x100,
     .led.sel_p24_p27_led_mod = 0x107,
     .port.offset = 0,
     .port.count = 8,
     .port.list = {
         /* Port 8 ~ 15 */
         { .mac_id = 8,  .phy_idx = 0, .phy = 0 },
         { .mac_id = 9,  .phy_idx = 0, .phy = 1 },
         { .mac_id = 10,  .phy_idx = 0, .phy = 2 },
         { .mac_id = 11,  .phy_idx = 0, .phy = 3 },
         { .mac_id = 12,  .phy_idx = 0, .phy = 4 },
         { .mac_id = 13,  .phy_idx = 0, .phy = 5 },
         { .mac_id = 14,  .phy_idx = 0, .phy = 6 },
         { .mac_id = 15,  .phy_idx = 0, .phy = 7 },
     },  /* port.list */
     .serdes.offset = 0,
     .serdes.count = 6,
     .serdes.list = {
         { .phy_idx = 0, .mii = RTK_MII_NONE, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
         { .phy_idx = 0, .mii = RTK_MII_NONE, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
         { .phy_idx = 2, .mii = RTK_MII_NONE, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
         { .phy_idx = 2, .mii = RTK_MII_NONE, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
         { .phy_idx = 3, .mii = RTK_MII_1000BX_FIBER, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
         { .phy_idx = 3, .mii = RTK_MII_1000BX_FIBER, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
     },  /* serdes.list */
     .phy.baseid = 0,
     .phy.count = 1,
     .phy.list = {
         [0] =   { .chip = RTK_CHIP_RTL8218B, .mac_id = 8,  .phy_max = 8 },
     }   /* .phy.list */
 };

static const rtk_switch_model_t zyxel_gs1810_8HP = {
    .name = "ZyXEL_GS1900_8HP",
    .chip = RTK_CHIP_RTL8382M,
    .led.offset = 0,
    .led.count = 8,
    .led.p0_p23_led_num = 2,
    .led.p24_p27_led_num = 2,
    .led.sel_pwr_on_led = 0xd,
    .led.sel_p0_p23_led_mod = 0x008,
    .led.sel_p24_p27_led_mod = 0x107,
    .port.offset = 0,
    .port.count = 8,
    .port.list = {
        /* Port 8 ~ 15 */
        { .mac_id = 8,  .phy_idx = 0, .phy = 0 },
        { .mac_id = 9,  .phy_idx = 0, .phy = 1 },
        { .mac_id = 10,  .phy_idx = 0, .phy = 2 },
        { .mac_id = 11,  .phy_idx = 0, .phy = 3 },
        { .mac_id = 12,  .phy_idx = 0, .phy = 4 },
        { .mac_id = 13,  .phy_idx = 0, .phy = 5 },
        { .mac_id = 14,  .phy_idx = 0, .phy = 6 },
        { .mac_id = 15,  .phy_idx = 0, .phy = 7 },
    },  /* port.list */
    .serdes.offset = 0,
    .serdes.count = 6,
    .serdes.list = {
        { .phy_idx = 0, .mii = RTK_MII_NONE, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 0, .mii = RTK_MII_NONE, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 2, .mii = RTK_MII_NONE, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 2, .mii = RTK_MII_NONE, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 3, .mii = RTK_MII_1000BX_FIBER, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 3, .mii = RTK_MII_1000BX_FIBER, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
    },  /* serdes.list */
    .phy.baseid = 0,
    .phy.count = 1,
    .phy.list = {
        [0] =   { .chip = RTK_CHIP_RTL8218B, .mac_id = 8,  .phy_max = 8 },
    }   /* .phy.list */
};

static const rtk_switch_model_t zyxel_gs1810_16_FIRST_16PHY = {
    .name = "zyxel_gs1900_16_FIRST_16PHY",
    .chip = RTK_CHIP_RTL8382M,
    .led.offset = 0,
    .led.count = 16,
    .led.p0_p23_led_num = 1,
    .led.p24_p27_led_num = 1,
    .led.sel_pwr_on_led = 0xd,
    .led.sel_p0_p23_led_mod = 0x100,
    .led.sel_p24_p27_led_mod = 0x107,
    .port.offset = 0,
    .port.count = 16,
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
    },  /* port.list */
    .serdes.offset = 0,
    .serdes.count = 6,
    .serdes.list = {
        { .phy_idx = 0, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 0, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 2, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 2, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 3, .mii = RTK_MII_1000BX_FIBER, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 3, .mii = RTK_MII_1000BX_FIBER, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
    },  /* serdes.list */
    .phy.baseid = 0,
    .phy.count = 2,
    .phy.list = {
        [0] =   { .chip = RTK_CHIP_RTL8218B, .mac_id = 0,  .phy_max = 8 },
        [1] =   { .chip = RTK_CHIP_RTL8218B, .mac_id = 8,  .phy_max = 8 },
    }   /* .phy.list */
};

static const rtk_switch_model_t zyxel_gs1810_16 = {
    .name = "ZyXEL_GS1900_16",
    .chip = RTK_CHIP_RTL8382M,
    .led.offset = 0,
    .led.count = 16,
    .led.p0_p23_led_num = 1,
    .led.p24_p27_led_num = 1,
    .led.sel_pwr_on_led = 0xd,
    .led.sel_p0_p23_led_mod = 0x100,
    .led.sel_p24_p27_led_mod = 0x107,
    .port.offset = 0,
    .port.count = 16,
    .port.list = {
        /* Port 0 ~ 7 */
        { .mac_id = 8,  .phy_idx = 0, .phy = 0 },
        { .mac_id = 9,  .phy_idx = 0, .phy = 1 },
        { .mac_id = 10, .phy_idx = 0, .phy = 2 },
        { .mac_id = 11, .phy_idx = 0, .phy = 3 },
        { .mac_id = 12, .phy_idx = 0, .phy = 4 },
        { .mac_id = 13, .phy_idx = 0, .phy = 5 },
        { .mac_id = 14, .phy_idx = 0, .phy = 6 },
        { .mac_id = 15, .phy_idx = 0, .phy = 7 },
        /* Port 8 ~ 15 */
        { .mac_id = 16, .phy_idx = 1, .phy = 0 },
        { .mac_id = 17, .phy_idx = 1, .phy = 1 },
        { .mac_id = 18, .phy_idx = 1, .phy = 2 },
        { .mac_id = 19, .phy_idx = 1, .phy = 3 },
        { .mac_id = 20, .phy_idx = 1, .phy = 4 },
        { .mac_id = 21, .phy_idx = 1, .phy = 5 },
        { .mac_id = 22, .phy_idx = 1, .phy = 6 },
        { .mac_id = 23, .phy_idx = 1, .phy = 7 },
    },  /* port.list */
    .serdes.offset = 0,
    .serdes.count = 6,
    .serdes.list = {
        { .phy_idx = 0, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 0, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 2, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 2, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 3, .mii = RTK_MII_1000BX_FIBER, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 3, .mii = RTK_MII_1000BX_FIBER, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
    },  /* serdes.list */
    .phy.baseid = 0,
    .phy.count = 2,
    .phy.list = {
        [0] =   { .chip = RTK_CHIP_RTL8218B, .mac_id = 8,  .phy_max = 8 },
        [1] =   { .chip = RTK_CHIP_RTL8218B, .mac_id = 16,  .phy_max = 8 },
    }   /* .phy.list */
};

static const rtk_switch_model_t zyxel_gs1810_24 = {
    .name = "ZyXEL_GS1900_24",
    .chip = RTK_CHIP_RTL8382M,
    .led.offset = 0,
    .led.count = 28,
    .led.p0_p23_led_num = 1,
    .led.p24_p27_led_num = 1,
    .led.sel_pwr_on_led = 0xd,
    .led.sel_p0_p23_led_mod = 0x0,
    .led.sel_p24_p27_led_mod = 0x0,
    .port.offset = 0,
    .port.count = 26,
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
        /* Port 24 & 26 */
        { .mac_id = 24, .phy_idx = 3, .phy = 0 },
        { .mac_id = 26, .phy_idx = 3, .phy = 1 },
    },  /* port.list */
    .serdes.offset = 0,
    .serdes.count = 6,
    .serdes.list = {
        { .phy_idx = 0, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 0, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 2, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 2, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 3, .mii = RTK_MII_1000BX_FIBER, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 3, .mii = RTK_MII_1000BX_FIBER, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
    },  /* serdes.list */
    .phy.baseid = 0,
    .phy.count = 4,
    .phy.list = {
        [0] =   { .chip = RTK_CHIP_RTL8218B, .mac_id = 0,  .phy_max = 8 },
        [1] =   { .chip = RTK_CHIP_RTL8218B, .mac_id = 8,  .phy_max = 8 },
        [2] =   { .chip = RTK_CHIP_RTL8218B, .mac_id = 16,  .phy_max = 8 },
        [3] =   { .chip = RTK_CHIP_NONE,     .mac_id = 24,  .phy_max = 2 },
    }   /* .phy.list */
};

static const rtk_switch_model_t zyxel_gs1810_24E = {
    .name = "ZyXEL_GS1900_24E",
    .chip = RTK_CHIP_RTL8382M,
    .led.offset = 0,
    .led.count = 26,
    .led.p0_p23_led_num = 1,
    .led.p24_p27_led_num = 3,
    .led.sel_pwr_on_led = 0xd,
    .led.sel_p0_p23_led_mod = 0x100,
    .led.sel_p24_p27_led_mod = 0x107,
    .port.offset = 0,
    .port.count = 24,
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
    },  /* port.list */
    .serdes.offset = 0,
    .serdes.count = 6,
    .serdes.list = {
        { .phy_idx = 0, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 0, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 2, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 2, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 3, .mii = RTK_MII_1000BX_FIBER, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 3, .mii = RTK_MII_1000BX_FIBER, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
    },  /* serdes.list */
    .phy.baseid = 0,
    .phy.count = 3,
    .phy.list = {
        [0] =   { .chip = RTK_CHIP_RTL8218B, .mac_id = 0,  .phy_max = 8 },
        [1] =   { .chip = RTK_CHIP_RTL8218B, .mac_id = 8,  .phy_max = 8 },
        [2] =   { .chip = RTK_CHIP_RTL8218B, .mac_id = 16,  .phy_max = 8 },
    }   /* .phy.list */
};

static const rtk_switch_model_t zyxel_gs1810_24HP = {
    .name = "ZyXEL_GS1900_24HP",
    .chip = RTK_CHIP_RTL8382M,
    .led.offset = 0,
    .led.count = 28,
    .led.p0_p23_led_num = 2,
    .led.p24_p27_led_num = 2,
    .led.sel_pwr_on_led = 0xd,
    .led.sel_p0_p23_led_mod = 0x0,
    .led.sel_p24_p27_led_mod = 0x0,
    .port.offset = 0,
    .port.count = 26,
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
            /* Port 24 & 26 */
            { .mac_id = 24, .phy_idx = 3, .phy = 0 },
            { .mac_id = 26, .phy_idx = 3, .phy = 1 },
        },  /* port.list */
    .serdes.offset = 0,
    .serdes.count = 6,
    .serdes.list = {
        { .phy_idx = 0, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 0, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 2, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 2, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 3, .mii = RTK_MII_1000BX_FIBER, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 3, .mii = RTK_MII_1000BX_FIBER, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
    },  /* serdes.list */
    .phy.baseid = 0,
    .phy.count = 4,
    .phy.list = {
        [0] =   { .chip = RTK_CHIP_RTL8218B, .mac_id = 0,  .phy_max = 8 },
        [1] =   { .chip = RTK_CHIP_RTL8218B, .mac_id = 8,  .phy_max = 8 },
        [2] =   { .chip = RTK_CHIP_RTL8218B, .mac_id = 16,  .phy_max = 8 },
        [3] =   { .chip = RTK_CHIP_NONE,     .mac_id = 24,  .phy_max = 2 },
    }   /* .phy.list */
};

static const rtk_switch_model_t zyxel_gs1810_48 = {
    .name = "ZyXEL_GS1900_48",
    .chip = RTK_CHIP_RTL8393M,

    /* LED layout settting */
    .led.led_if_sel = LED_IF_SEL_SINGLE_COLOR_SCAN,
    .led.num = 1,
    .led.copr_pmask[0] = 0xFFFFFFFF,
    .led.copr_pmask[1] = 0x0000FFFF,
    .led.fib_pmask[0] = 0x00000000,
    .led.fib_pmask[1] = 0x00030000,
    .led.led_combo[0] = 0x00000000,
    .led.led_combo[1] = 0x00000000,
    /* LED definition - set 0 */
    .led.led_definition_set[0].led[0] = 0x0,    /* link/act */
    .led.led_definition_set[0].led[1] = 0xA,    /* 1000Mbps link/act */
    .led.led_definition_set[0].led[2] = 0x6,    /* duplex mode */
    /* LED definition selection (per-port) */
    .led.led_copr_set_psel_bit0_pmask[0] = 0x00000000,
    .led.led_copr_set_psel_bit1_pmask[0] = 0x00000000,
    .led.led_copr_set_psel_bit0_pmask[1] = 0x00000000,
    .led.led_copr_set_psel_bit1_pmask[1] = 0x00000000,
    .led.led_fib_set_psel_bit0_pmask[0] = 0x00000000,
    .led.led_fib_set_psel_bit1_pmask[0] = 0x00000000,
    .led.led_fib_set_psel_bit0_pmask[1] = 0x00000000,
    .led.led_fib_set_psel_bit1_pmask[1] = 0x00000000,

    /* 48 + 2 SFP */
    .port.count = 50,
    .port.list = {
        { .mac_id = 0,  .phy_idx = 0, .phy = 0 },
        { .mac_id = 1,  .phy_idx = 0, .phy = 1 },
        { .mac_id = 2,  .phy_idx = 0, .phy = 2 },
        { .mac_id = 3,  .phy_idx = 0, .phy = 3 },
        { .mac_id = 4,  .phy_idx = 0, .phy = 4 },
        { .mac_id = 5,  .phy_idx = 0, .phy = 5 },
        { .mac_id = 6,  .phy_idx = 0, .phy = 6 },
        { .mac_id = 7,  .phy_idx = 0, .phy = 7 },
        { .mac_id = 8,  .phy_idx = 1, .phy = 0 },
        { .mac_id = 9,  .phy_idx = 1, .phy = 1 },
        { .mac_id = 10, .phy_idx = 1, .phy = 2 },
        { .mac_id = 11, .phy_idx = 1, .phy = 3 },
        { .mac_id = 12, .phy_idx = 1, .phy = 4 },
        { .mac_id = 13, .phy_idx = 1, .phy = 5 },
        { .mac_id = 14, .phy_idx = 1, .phy = 6 },
        { .mac_id = 15, .phy_idx = 1, .phy = 7 },
        { .mac_id = 16, .phy_idx = 2, .phy = 0 },
        { .mac_id = 17, .phy_idx = 2, .phy = 1 },
        { .mac_id = 18, .phy_idx = 2, .phy = 2 },
        { .mac_id = 19, .phy_idx = 2, .phy = 3 },
        { .mac_id = 20, .phy_idx = 2, .phy = 4 },
        { .mac_id = 21, .phy_idx = 2, .phy = 5 },
        { .mac_id = 22, .phy_idx = 2, .phy = 6 },
        { .mac_id = 23, .phy_idx = 2, .phy = 7 },
        { .mac_id = 24, .phy_idx = 3, .phy = 0 },
        { .mac_id = 25, .phy_idx = 3, .phy = 1 },
        { .mac_id = 26, .phy_idx = 3, .phy = 2 },
        { .mac_id = 27, .phy_idx = 3, .phy = 3 },
        { .mac_id = 28, .phy_idx = 3, .phy = 4 },
        { .mac_id = 29, .phy_idx = 3, .phy = 5 },
        { .mac_id = 30, .phy_idx = 3, .phy = 6 },
        { .mac_id = 31, .phy_idx = 3, .phy = 7 },
        { .mac_id = 32, .phy_idx = 4, .phy = 0 },
        { .mac_id = 33, .phy_idx = 4, .phy = 1 },
        { .mac_id = 34, .phy_idx = 4, .phy = 2 },
        { .mac_id = 35, .phy_idx = 4, .phy = 3 },
        { .mac_id = 36, .phy_idx = 4, .phy = 4 },
        { .mac_id = 37, .phy_idx = 4, .phy = 5 },
        { .mac_id = 38, .phy_idx = 4, .phy = 6 },
        { .mac_id = 39, .phy_idx = 4, .phy = 7 },
        { .mac_id = 40, .phy_idx = 5, .phy = 0 },
        { .mac_id = 41, .phy_idx = 5, .phy = 1 },
        { .mac_id = 42, .phy_idx = 5, .phy = 2 },
        { .mac_id = 43, .phy_idx = 5, .phy = 3 },
        { .mac_id = 44, .phy_idx = 5, .phy = 4 },
        { .mac_id = 45, .phy_idx = 5, .phy = 5 },
        { .mac_id = 46, .phy_idx = 5, .phy = 6 },
        { .mac_id = 47, .phy_idx = 5, .phy = 7 },
        { .mac_id = 48, .phy_idx = 6, .phy = 0 },
        { .mac_id = 49, .phy_idx = 6, .phy = 1 },
    },  /* port.list */

    .serdes.count = 14,
    .serdes.list = {
        /* mii: SR4_CFG_FRC_SDS_MODE */
        { .sds_id = 0,  .phy_idx = 0, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 1,  .phy_idx = 0, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 2,  .phy_idx = 1, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 3,  .phy_idx = 1, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 4,  .phy_idx = 2, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 5,  .phy_idx = 2, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 6,  .phy_idx = 3, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 7,  .phy_idx = 3, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 8,  .phy_idx = 4, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 9,  .phy_idx = 4, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 10, .phy_idx = 5, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 11, .phy_idx = 5, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 12, .phy_idx = 6, .mii = RTK_MII_SGMII },
        { .sds_id = 13, .phy_idx = 6, .mii = RTK_MII_SGMII },
    },  /* serdes.list */

    .phy.baseid = 0,
    .phy.count = 7,
    .phy.list = {
        [0] = { .chip = RTK_CHIP_RTL8218B, .mac_id = 0 , .phy_max = 8 },
        [1] = { .chip = RTK_CHIP_RTL8218B, .mac_id = 8 , .phy_max = 8 },
        [2] = { .chip = RTK_CHIP_RTL8218B, .mac_id = 16, .phy_max = 8 },
        [3] = { .chip = RTK_CHIP_RTL8218B, .mac_id = 24, .phy_max = 8 },
        [4] = { .chip = RTK_CHIP_RTL8218B, .mac_id = 32, .phy_max = 8 },
        [5] = { .chip = RTK_CHIP_RTL8218B, .mac_id = 40, .phy_max = 8 },
        [6] = { .chip = RTK_CHIP_NONE,     .mac_id = 48, .phy_max = 0 },
    },   /* .phy.list */
    .phy.rstGpioType = PHY_RSTGPIOTYPE_EXTERNAL,
    .phy.rstGpio.rstExtGpio.addr = 3,
    .phy.rstGpio.rstExtGpio.pin = 6,
};

static const rtk_switch_model_t zyxel_gs1810_48_hp = {
    .name = "ZyXEL_GS1900_48HP",
    .chip = RTK_CHIP_RTL8393M,

    /* LED layout settting */
    .led.led_if_sel = LED_IF_SEL_SINGLE_COLOR_SCAN,
    .led.num = 1,
    .led.copr_pmask[0] = 0xFFFFFFFF,
    .led.copr_pmask[1] = 0x0000FFFF,
    .led.fib_pmask[0] = 0x00000000,
    .led.fib_pmask[1] = 0x000F0000,
    .led.led_combo[0] = 0x00000000,
    .led.led_combo[1] = 0x00000000,
    /* LED definition - set 0 */
    .led.led_definition_set[0].led[0] = 0x0,    /* link/act */
    .led.led_definition_set[0].led[1] = 0xA,    /* 1000Mbps link/act */
    .led.led_definition_set[0].led[2] = 0x6,    /* duplex mode */
    /* LED definition selection (per-port) */
    .led.led_copr_set_psel_bit0_pmask[0] = 0x00000000,
    .led.led_copr_set_psel_bit1_pmask[0] = 0x00000000,
    .led.led_copr_set_psel_bit0_pmask[1] = 0x00000000,
    .led.led_copr_set_psel_bit1_pmask[1] = 0x00000000,
    .led.led_fib_set_psel_bit0_pmask[0] = 0x00000000,
    .led.led_fib_set_psel_bit1_pmask[0] = 0x00000000,
    .led.led_fib_set_psel_bit0_pmask[1] = 0x00000000,
    .led.led_fib_set_psel_bit1_pmask[1] = 0x00000000,

    /* 48 + 2 SFP */
    .port.count = 50,
    .port.list = {
        { .mac_id = 0,  .phy_idx = 0, .phy = 0 },
        { .mac_id = 1,  .phy_idx = 0, .phy = 1 },
        { .mac_id = 2,  .phy_idx = 0, .phy = 2 },
        { .mac_id = 3,  .phy_idx = 0, .phy = 3 },
        { .mac_id = 4,  .phy_idx = 0, .phy = 4 },
        { .mac_id = 5,  .phy_idx = 0, .phy = 5 },
        { .mac_id = 6,  .phy_idx = 0, .phy = 6 },
        { .mac_id = 7,  .phy_idx = 0, .phy = 7 },
        { .mac_id = 8,  .phy_idx = 1, .phy = 0 },
        { .mac_id = 9,  .phy_idx = 1, .phy = 1 },
        { .mac_id = 10, .phy_idx = 1, .phy = 2 },
        { .mac_id = 11, .phy_idx = 1, .phy = 3 },
        { .mac_id = 12, .phy_idx = 1, .phy = 4 },
        { .mac_id = 13, .phy_idx = 1, .phy = 5 },
        { .mac_id = 14, .phy_idx = 1, .phy = 6 },
        { .mac_id = 15, .phy_idx = 1, .phy = 7 },
        { .mac_id = 16, .phy_idx = 2, .phy = 0 },
        { .mac_id = 17, .phy_idx = 2, .phy = 1 },
        { .mac_id = 18, .phy_idx = 2, .phy = 2 },
        { .mac_id = 19, .phy_idx = 2, .phy = 3 },
        { .mac_id = 20, .phy_idx = 2, .phy = 4 },
        { .mac_id = 21, .phy_idx = 2, .phy = 5 },
        { .mac_id = 22, .phy_idx = 2, .phy = 6 },
        { .mac_id = 23, .phy_idx = 2, .phy = 7 },
        { .mac_id = 24, .phy_idx = 3, .phy = 0 },
        { .mac_id = 25, .phy_idx = 3, .phy = 1 },
        { .mac_id = 26, .phy_idx = 3, .phy = 2 },
        { .mac_id = 27, .phy_idx = 3, .phy = 3 },
        { .mac_id = 28, .phy_idx = 3, .phy = 4 },
        { .mac_id = 29, .phy_idx = 3, .phy = 5 },
        { .mac_id = 30, .phy_idx = 3, .phy = 6 },
        { .mac_id = 31, .phy_idx = 3, .phy = 7 },
        { .mac_id = 32, .phy_idx = 4, .phy = 0 },
        { .mac_id = 33, .phy_idx = 4, .phy = 1 },
        { .mac_id = 34, .phy_idx = 4, .phy = 2 },
        { .mac_id = 35, .phy_idx = 4, .phy = 3 },
        { .mac_id = 36, .phy_idx = 4, .phy = 4 },
        { .mac_id = 37, .phy_idx = 4, .phy = 5 },
        { .mac_id = 38, .phy_idx = 4, .phy = 6 },
        { .mac_id = 39, .phy_idx = 4, .phy = 7 },
        { .mac_id = 40, .phy_idx = 5, .phy = 0 },
        { .mac_id = 41, .phy_idx = 5, .phy = 1 },
        { .mac_id = 42, .phy_idx = 5, .phy = 2 },
        { .mac_id = 43, .phy_idx = 5, .phy = 3 },
        { .mac_id = 44, .phy_idx = 5, .phy = 4 },
        { .mac_id = 45, .phy_idx = 5, .phy = 5 },
        { .mac_id = 46, .phy_idx = 5, .phy = 6 },
        { .mac_id = 47, .phy_idx = 5, .phy = 7 },
        { .mac_id = 48, .phy_idx = 6, .phy = 0 },
        { .mac_id = 49, .phy_idx = 6, .phy = 1 },
    },  /* port.list */

    .serdes.count = 14,
    .serdes.list = {
        /* mii: SR4_CFG_FRC_SDS_MODE */
        { .sds_id = 0,  .phy_idx = 0, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 1,  .phy_idx = 0, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 2,  .phy_idx = 1, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 3,  .phy_idx = 1, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 4,  .phy_idx = 2, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 5,  .phy_idx = 2, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 6,  .phy_idx = 3, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 7,  .phy_idx = 3, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 8,  .phy_idx = 4, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 9,  .phy_idx = 4, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 10, .phy_idx = 5, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 11, .phy_idx = 5, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 12, .phy_idx = 6, .mii = RTK_MII_SGMII },
        { .sds_id = 13, .phy_idx = 6, .mii = RTK_MII_SGMII },
    },  /* serdes.list */

    .phy.baseid = 0,
    .phy.count = 7,
    .phy.list = {
        [0] = { .chip = RTK_CHIP_RTL8218B, .mac_id = 0 , .phy_max = 8 },
        [1] = { .chip = RTK_CHIP_RTL8218B, .mac_id = 8 , .phy_max = 8 },
        [2] = { .chip = RTK_CHIP_RTL8218B, .mac_id = 16, .phy_max = 8 },
        [3] = { .chip = RTK_CHIP_RTL8218B, .mac_id = 24, .phy_max = 8 },
        [4] = { .chip = RTK_CHIP_RTL8218B, .mac_id = 32, .phy_max = 8 },
        [5] = { .chip = RTK_CHIP_RTL8218B, .mac_id = 40, .phy_max = 8 },
        [6] = { .chip = RTK_CHIP_NONE,     .mac_id = 48, .phy_max = 0 },
    },   /* .phy.list */
    .phy.rstGpioType = PHY_RSTGPIOTYPE_EXTERNAL,
    .phy.rstGpio.rstExtGpio.addr = 3,
    .phy.rstGpio.rstExtGpio.pin = 6,
};

static const rtk_switch_model_t zyxel_gs1810_8HPv2 = {
    .name = "ZyXEL_GS1900_8HPv2",
    .chip = RTK_CHIP_RTL8380M,
    .led.offset = 0,
    .led.count = 10,
    .led.p0_p23_led_num = 2,
    .led.p24_p27_led_num = 2,
    .led.sel_pwr_on_led = 0xd,
    .led.sel_p0_p23_led_mod = 0x1ea,
    .led.sel_p24_p27_led_mod = 0x14b,
    .port.offset = 0,
    .port.count = 8,
    .port.list = {
        /* Port 8 ~ 15 */
        { .mac_id = 8,  .phy_idx = 0, .phy = 0 },
        { .mac_id = 9,  .phy_idx = 0, .phy = 1 },
        { .mac_id = 10,  .phy_idx = 0, .phy = 2 },
        { .mac_id = 11,  .phy_idx = 0, .phy = 3 },
        { .mac_id = 12,  .phy_idx = 0, .phy = 4 },
        { .mac_id = 13,  .phy_idx = 0, .phy = 5 },
        { .mac_id = 14,  .phy_idx = 0, .phy = 6 },
        { .mac_id = 15,  .phy_idx = 0, .phy = 7 },
    },  /* port.list */
    .serdes.offset = 0,
    .serdes.count = 6,
    .serdes.list = {
        { .phy_idx = 0, .mii = RTK_MII_NONE, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 0, .mii = RTK_MII_NONE, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 2, .mii = RTK_MII_NONE, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 2, .mii = RTK_MII_NONE, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 3, .mii = RTK_MII_1000BX_FIBER, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 3, .mii = RTK_MII_1000BX_FIBER, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
    },  /* serdes.list */
    .phy.baseid = 0,
    .phy.count = 1,
    .phy.list = {
        [0] =   { .chip = RTK_CHIP_RTL8218B, .mac_id = 8,  .phy_max = 8 },
    }   /* .phy.list */
};

static const rtk_switch_model_t zyxel_gs1810_10HP = {
    .name = "ZyXEL_GS1900_10HP",
    .chip = RTK_CHIP_RTL8380M,
    .led.offset = 0,
    .led.count = 10,
    .led.p0_p23_led_num = 2,
    .led.p24_p27_led_num = 2,
    .led.sel_pwr_on_led = 0xd,
    .led.sel_p0_p23_led_mod = 0x1ea,
    .led.sel_p24_p27_led_mod = 0x14b,
    .port.offset = 0,
    .port.count = 10,
    .port.list = {
        /* Port 8 ~ 15 */
        { .mac_id = 8,   .phy_idx = 0, .phy = 0 },
        { .mac_id = 9,   .phy_idx = 0, .phy = 1 },
        { .mac_id = 10,  .phy_idx = 0, .phy = 2 },
        { .mac_id = 11,  .phy_idx = 0, .phy = 3 },
        { .mac_id = 12,  .phy_idx = 0, .phy = 4 },
        { .mac_id = 13,  .phy_idx = 0, .phy = 5 },
        { .mac_id = 14,  .phy_idx = 0, .phy = 6 },
        { .mac_id = 15,  .phy_idx = 0, .phy = 7 },
        /* Port 24 & 26 */
        { .mac_id = 24,  .phy_idx = 1, .phy = 0 },
        { .mac_id = 26,  .phy_idx = 1, .phy = 1 },
    },  /* port.list */
    .serdes.offset = 0,
    .serdes.count = 6,
    .serdes.list = {
        { .phy_idx = 0, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 0, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 0, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 0, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 1, .mii = RTK_MII_1000BX_FIBER, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 1, .mii = RTK_MII_1000BX_FIBER, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
    },  /* serdes.list */
    .phy.baseid = 0,
    .phy.count = 2,
    .phy.list = {
        [0] =   { .chip = RTK_CHIP_RTL8218B, .mac_id = 8,  .phy_max = 8 },
        [1] =   { .chip = RTK_CHIP_NONE,     .mac_id = 24, .phy_max = 2 },
    }   /* .phy.list */
};

static const rtk_switch_model_t zyxel_gs1810_24EP = {
    .name = "ZyXEL_GS1900_24EP",
    .chip = RTK_CHIP_RTL8382M,
    .led.offset = 0,
    .led.count = 26,
    .led.p0_p23_led_num = 2,
    .led.p24_p27_led_num = 0,
    .led.sel_pwr_on_led = 0xd,
    .led.sel_p0_p23_led_mod = 0x0,
    .led.sel_p24_p27_led_mod = 0x0,
    .port.offset = 0,
    .port.count = 24,
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
    },  /* port.list */
    .serdes.offset = 0,
    .serdes.count = 6,
    .serdes.list = {
        { .phy_idx = 0, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 0, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 2, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 2, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 3, .mii = RTK_MII_1000BX_FIBER, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 3, .mii = RTK_MII_1000BX_FIBER, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
    },  /* serdes.list */
    .phy.baseid = 0,
    .phy.count = 3,
    .phy.list = {
        [0] =   { .chip = RTK_CHIP_RTL8218B, .mac_id = 0,  .phy_max = 8 },
        [1] =   { .chip = RTK_CHIP_RTL8218B, .mac_id = 8,  .phy_max = 8 },
        [2] =   { .chip = RTK_CHIP_RTL8218B, .mac_id = 16,  .phy_max = 8 },
    }   /* .phy.list */
};

static const rtk_switch_model_t zyxel_gs1810_24HPv2 = {
    .name = "ZyXEL_GS1900_24HPv2",
    .chip = RTK_CHIP_RTL8382M,
    .led.offset = 0,
    .led.count = 28,
    .led.p0_p23_led_num = 2,
    .led.p24_p27_led_num = 2,
    .led.sel_pwr_on_led = 0xd,
    .led.sel_p0_p23_led_mod = 0x0,
    .led.sel_p24_p27_led_mod = 0x0,
    .port.offset = 0,
    .port.count = 26,
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
            /* Port 24 & 26 */
            { .mac_id = 24, .phy_idx = 3, .phy = 0 },
            { .mac_id = 26, .phy_idx = 3, .phy = 1 },
        },  /* port.list */
    .serdes.offset = 0,
    .serdes.count = 6,
    .serdes.list = {
        { .phy_idx = 0, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 0, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 2, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 2, .mii = RTK_MII_QSGMII, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 3, .mii = RTK_MII_1000BX_FIBER, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
        { .phy_idx = 3, .mii = RTK_MII_1000BX_FIBER, .rx_polarity = SERDES_POLARITY_NORMAL, .tx_polarity = SERDES_POLARITY_NORMAL },
    },  /* serdes.list */
    .phy.baseid = 0,
    .phy.count = 4,
    .phy.list = {
        [0] =   { .chip = RTK_CHIP_RTL8218B, .mac_id = 0,  .phy_max = 8 },
        [1] =   { .chip = RTK_CHIP_RTL8218B, .mac_id = 8,  .phy_max = 8 },
        [2] =   { .chip = RTK_CHIP_RTL8218B, .mac_id = 16,  .phy_max = 8 },
        [3] =   { .chip = RTK_CHIP_NONE,     .mac_id = 24,  .phy_max = 2 },
    }   /* .phy.list */
};

static const rtk_switch_model_t zyxel_gs1810_48HPv2 = {
    .name = "ZyXEL_GS1900_48HPv2",
    .chip = RTK_CHIP_RTL8393M,

    /* LED layout settting */
    .led.led_if_sel = LED_IF_SEL_SINGLE_COLOR_SCAN,
    .led.num = 1,
    .led.copr_pmask[0] = 0xFFFFFFFF,
    .led.copr_pmask[1] = 0x0000FFFF,
    .led.fib_pmask[0] = 0x00000000,
    .led.fib_pmask[1] = 0x000F0000,
    .led.led_combo[0] = 0x00000000,
    .led.led_combo[1] = 0x00000000,
    /* LED definition - set 0 */
    .led.led_definition_set[0].led[0] = 0x0,    /* link/act */
    .led.led_definition_set[0].led[1] = 0xA,    /* 1000Mbps link/act */
    .led.led_definition_set[0].led[2] = 0x6,    /* duplex mode */
    /* LED definition selection (per-port) */
    .led.led_copr_set_psel_bit0_pmask[0] = 0x00000000,
    .led.led_copr_set_psel_bit1_pmask[0] = 0x00000000,
    .led.led_copr_set_psel_bit0_pmask[1] = 0x00000000,
    .led.led_copr_set_psel_bit1_pmask[1] = 0x00000000,
    .led.led_fib_set_psel_bit0_pmask[0] = 0x00000000,
    .led.led_fib_set_psel_bit1_pmask[0] = 0x00000000,
    .led.led_fib_set_psel_bit0_pmask[1] = 0x00000000,
    .led.led_fib_set_psel_bit1_pmask[1] = 0x00000000,

    /* 48 + 2 SFP */
    .port.count = 50,
    .port.list = {
        { .mac_id = 0,  .phy_idx = 0, .phy = 0 },
        { .mac_id = 1,  .phy_idx = 0, .phy = 1 },
        { .mac_id = 2,  .phy_idx = 0, .phy = 2 },
        { .mac_id = 3,  .phy_idx = 0, .phy = 3 },
        { .mac_id = 4,  .phy_idx = 0, .phy = 4 },
        { .mac_id = 5,  .phy_idx = 0, .phy = 5 },
        { .mac_id = 6,  .phy_idx = 0, .phy = 6 },
        { .mac_id = 7,  .phy_idx = 0, .phy = 7 },
        { .mac_id = 8,  .phy_idx = 1, .phy = 0 },
        { .mac_id = 9,  .phy_idx = 1, .phy = 1 },
        { .mac_id = 10, .phy_idx = 1, .phy = 2 },
        { .mac_id = 11, .phy_idx = 1, .phy = 3 },
        { .mac_id = 12, .phy_idx = 1, .phy = 4 },
        { .mac_id = 13, .phy_idx = 1, .phy = 5 },
        { .mac_id = 14, .phy_idx = 1, .phy = 6 },
        { .mac_id = 15, .phy_idx = 1, .phy = 7 },
        { .mac_id = 16, .phy_idx = 2, .phy = 0 },
        { .mac_id = 17, .phy_idx = 2, .phy = 1 },
        { .mac_id = 18, .phy_idx = 2, .phy = 2 },
        { .mac_id = 19, .phy_idx = 2, .phy = 3 },
        { .mac_id = 20, .phy_idx = 2, .phy = 4 },
        { .mac_id = 21, .phy_idx = 2, .phy = 5 },
        { .mac_id = 22, .phy_idx = 2, .phy = 6 },
        { .mac_id = 23, .phy_idx = 2, .phy = 7 },
        { .mac_id = 24, .phy_idx = 3, .phy = 0 },
        { .mac_id = 25, .phy_idx = 3, .phy = 1 },
        { .mac_id = 26, .phy_idx = 3, .phy = 2 },
        { .mac_id = 27, .phy_idx = 3, .phy = 3 },
        { .mac_id = 28, .phy_idx = 3, .phy = 4 },
        { .mac_id = 29, .phy_idx = 3, .phy = 5 },
        { .mac_id = 30, .phy_idx = 3, .phy = 6 },
        { .mac_id = 31, .phy_idx = 3, .phy = 7 },
        { .mac_id = 32, .phy_idx = 4, .phy = 0 },
        { .mac_id = 33, .phy_idx = 4, .phy = 1 },
        { .mac_id = 34, .phy_idx = 4, .phy = 2 },
        { .mac_id = 35, .phy_idx = 4, .phy = 3 },
        { .mac_id = 36, .phy_idx = 4, .phy = 4 },
        { .mac_id = 37, .phy_idx = 4, .phy = 5 },
        { .mac_id = 38, .phy_idx = 4, .phy = 6 },
        { .mac_id = 39, .phy_idx = 4, .phy = 7 },
        { .mac_id = 40, .phy_idx = 5, .phy = 0 },
        { .mac_id = 41, .phy_idx = 5, .phy = 1 },
        { .mac_id = 42, .phy_idx = 5, .phy = 2 },
        { .mac_id = 43, .phy_idx = 5, .phy = 3 },
        { .mac_id = 44, .phy_idx = 5, .phy = 4 },
        { .mac_id = 45, .phy_idx = 5, .phy = 5 },
        { .mac_id = 46, .phy_idx = 5, .phy = 6 },
        { .mac_id = 47, .phy_idx = 5, .phy = 7 },
        { .mac_id = 48, .phy_idx = 6, .phy = 0 },
        { .mac_id = 49, .phy_idx = 6, .phy = 1 },
    },  /* port.list */

    .serdes.count = 14,
    .serdes.list = {
        /* mii: SR4_CFG_FRC_SDS_MODE */
        { .sds_id = 0,  .phy_idx = 0, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 1,  .phy_idx = 0, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 2,  .phy_idx = 1, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 3,  .phy_idx = 1, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 4,  .phy_idx = 2, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 5,  .phy_idx = 2, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 6,  .phy_idx = 3, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 7,  .phy_idx = 3, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 8,  .phy_idx = 4, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 9,  .phy_idx = 4, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 10, .phy_idx = 5, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 11, .phy_idx = 5, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 12, .phy_idx = 6, .mii = RTK_MII_SGMII },
        { .sds_id = 13, .phy_idx = 6, .mii = RTK_MII_SGMII },
    },  /* serdes.list */

    .phy.baseid = 0,
    .phy.count = 7,
    .phy.list = {
        [0] = { .chip = RTK_CHIP_RTL8218B, .mac_id = 0 , .phy_max = 8 },
        [1] = { .chip = RTK_CHIP_RTL8218B, .mac_id = 8 , .phy_max = 8 },
        [2] = { .chip = RTK_CHIP_RTL8218B, .mac_id = 16, .phy_max = 8 },
        [3] = { .chip = RTK_CHIP_RTL8218B, .mac_id = 24, .phy_max = 8 },
        [4] = { .chip = RTK_CHIP_RTL8218B, .mac_id = 32, .phy_max = 8 },
        [5] = { .chip = RTK_CHIP_RTL8218B, .mac_id = 40, .phy_max = 8 },
        [6] = { .chip = RTK_CHIP_NONE,     .mac_id = 48, .phy_max = 0 },
    },   /* .phy.list */
    .phy.rstGpioType = PHY_RSTGPIOTYPE_EXTERNAL,
    .phy.rstGpio.rstExtGpio.addr = 3,
    .phy.rstGpio.rstExtGpio.pin = 6,
};

static const rtk_switch_model_t zyxel_gs1920_24HP = {
    .name = "ZyXEL_GS1920_24HP",
    .chip = RTK_CHIP_RTL8392M,

    /* LED layout settting */
    .led.led_if_sel = LED_IF_SEL_NONE,

    /* 24 + 4 Combo/SFP */
    .port.count = 24, // Actually, 28
    .port.list = {
        { .mac_id = 0,  .phy_idx = 0, .phy = 0 },
        { .mac_id = 1,  .phy_idx = 0, .phy = 1 },
        { .mac_id = 2,  .phy_idx = 0, .phy = 2 },
        { .mac_id = 3,  .phy_idx = 0, .phy = 3 },
        { .mac_id = 4,  .phy_idx = 0, .phy = 4 },
        { .mac_id = 5,  .phy_idx = 0, .phy = 5 },
        { .mac_id = 6,  .phy_idx = 0, .phy = 6 },
        { .mac_id = 7,  .phy_idx = 0, .phy = 7 },
        { .mac_id = 8,  .phy_idx = 1, .phy = 0 },
        { .mac_id = 9,  .phy_idx = 1, .phy = 1 },
        { .mac_id = 10, .phy_idx = 1, .phy = 2 },
        { .mac_id = 11, .phy_idx = 1, .phy = 3 },
        { .mac_id = 12, .phy_idx = 1, .phy = 4 },
        { .mac_id = 13, .phy_idx = 1, .phy = 5 },
        { .mac_id = 14, .phy_idx = 1, .phy = 6 },
        { .mac_id = 15, .phy_idx = 1, .phy = 7 },
        { .mac_id = 16, .phy_idx = 2, .phy = 0 },
        { .mac_id = 17, .phy_idx = 2, .phy = 1 },
        { .mac_id = 18, .phy_idx = 2, .phy = 2 },
        { .mac_id = 19, .phy_idx = 2, .phy = 3 },
        { .mac_id = 20, .phy_idx = 2, .phy = 4 },
        { .mac_id = 21, .phy_idx = 2, .phy = 5 },
        { .mac_id = 22, .phy_idx = 2, .phy = 6 },
        { .mac_id = 23, .phy_idx = 2, .phy = 7 },
    },  /* port.list */

    .serdes.count = 6,
    .serdes.list = {
        /* mii: SR4_CFG_FRC_SDS_MODE */
        { .sds_id = 0,  .phy_idx = 0, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 1,  .phy_idx = 0, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 2,  .phy_idx = 1, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 3,  .phy_idx = 1, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 4,  .phy_idx = 2, .mii = RTK_MII_RSGMII_PLUS },
        { .sds_id = 5,  .phy_idx = 2, .mii = RTK_MII_RSGMII_PLUS },
    },  /* serdes.list */

    .phy.baseid = 0,
    .phy.count = 3,
    .phy.list = {
        [0] = { .chip = RTK_CHIP_RTL8218B, .mac_id = 0 , .phy_max = 8 },
        [1] = { .chip = RTK_CHIP_RTL8218B, .mac_id = 8 , .phy_max = 8 },
        [2] = { .chip = RTK_CHIP_RTL8218B, .mac_id = 16, .phy_max = 8 },
    },   /* .phy.list */
};

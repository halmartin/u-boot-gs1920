/*
 * Copyright(c) Realtek Semiconductor Corporation, 2012
 * All rights reserved.
 *
 * Purpose :
 *
 * Feature :
 *
 */


/*
 * Include Files
 */
#include <rtk_debug.h>
#include <rtk_switch.h>
#include <rtk_type.h>
#include <rtk_reg.h>
#include <init.h>
#include <interrupt.h>

#if defined(CONFIG_SOFTWARE_CONTROL_LED)
#include <rtk/drv/swled/swctrl_led_main.h>
#endif

#if defined(CONFIG_MDC_MDIO_EXT_SUPPORT)
#include <rtk/drv/rtl8231/rtl8231_drv.h>
#include <rtk/mac/rtl8390/rtl8390_mdc_mdio.h>
#endif

#include <rtk/mac/rtl8390/rtl8390_swcore_reg.h>
#include <rtk/mac/rtl8390/rtl8390_drv.h>
#include <rtk/mac/rtl8390/rtl8390_rtk.h>

#if (defined(CONFIG_RTL8214) || defined(CONFIG_RTL8214F) || defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B) || defined(CONFIG_RTL8212B))
#include <rtk/phy/rtl8214f.h>
#endif

#if (defined(CONFIG_RTL8218B) || defined(CONFIG_RTL8218FB))
#include <rtk/phy/rtl8218b.h>
#endif

#if (defined(CONFIG_RTL8214FC))
#include <rtk/phy/rtl8214fc.h>
#endif

#if (defined(CONFIG_RTL8208))
#include <rtk/phy/rtl8208.h>
#endif

#include <rtk/drv/gpio/ext_gpio.h>

/*
 * Symbol Definition
 */
typedef struct {
    unsigned int    reg;
    unsigned int    val;
} confcode_mac_regval_t;

typedef struct {
    unsigned int    reg;
    unsigned int    offset;
    unsigned int    endBit;
    unsigned int    startBit;
    unsigned int    val;
} confcode_serdes_patch_t;

#define QSGMII_MODE

#define MAX_SERDES      14

static int8     pureFiber = 0;

/*
 * Macro Definition
 */
#define MAC_REG_SET_CHK(reg, val) \
    do { \
        MEM32_WRITE((0xbb000000) + reg, val); \
        OSAL_MDELAY(1); \
        if ((reg != 0x0ff4) && \
            (reg != 0x0014) && \
            (reg != 0x1180)) \
        { \
            if (REG32((0xbb000000) + reg) != val) { \
                printf("WARN: Reg 0x%08X != 0x%08X (real: 0x%08X)\n", (0xbb000000) + reg, val, REG32((0xbb000000) + reg)); \
            } \
        } \
    } while (0)

#define SERDES_PATCH_SET(_in)                           \
    do {                                                \
        uint32  _reg, _val, _len, _mask;                \
        uint32  _i, _startBit, _endBit;                 \
                                                        \
        _reg = _in.reg + _in.offset;                    \
        _startBit = _in.startBit;                       \
        _endBit = _in.endBit;                           \
        _len = _endBit - _startBit + 1;                 \
                                                        \
        if (32 == _len)                                 \
            _val = _in.val;                             \
        else                                            \
        {                                               \
            _mask = 0;                                  \
            for (_i = _startBit; _i <= _endBit; ++_i)   \
                _mask |= (1 << _i);                     \
                                                        \
            _val = MEM32_READ(SWCORE_BASE_ADDR| _reg);  \
            _val &= ~(_mask);                           \
            _val |= (_in.val << _startBit);             \
        }                                               \
        MEM32_WRITE((0xbb000000) + _reg, _val);         \
    } while(0)

#define SERDES_PATCH_SET_CHK(_in)                       \
    do {                                                \
        uint32  _reg, _val, _len, _mask;                \
        uint32  _i, _startBit, _endBit;                 \
                                                        \
        _reg = _in.reg + _in.offset;                    \
        _startBit = _in.startBit;                       \
        _endBit = _in.endBit;                           \
        _len = _endBit - _startBit + 1;                 \
                                                        \
        if (32 == _len)                                 \
            _val = _in.val;                             \
        else                                            \
        {                                               \
            _mask = 0;                                  \
            for (_i = _startBit; _i <= _endBit; ++_i)   \
                _mask |= (1 << _i);                     \
                                                        \
            _val = MEM32_READ(SWCORE_BASE_ADDR| _reg);  \
            _val &= ~(_mask);                           \
            _val |= (_in.val << _startBit);             \
        }                                               \
                                                        \
        MAC_REG_SET_CHK(_reg, _val);                    \
    } while(0)

#define SERDES_PATCH_OFFSET_SET(_in, _offset)           \
    do {                                                \
        uint32  _reg, _val, _len, _mask;                \
        uint32  _i, _startBit, _endBit;                 \
                                                        \
        _reg = _in.reg + _offset;                       \
        _startBit = _in.startBit;                       \
        _endBit = _in.endBit;                           \
        _len = _endBit - _startBit + 1;                 \
                                                        \
        if (32 == _len)                                 \
            _val = _in.val;                             \
        else                                            \
        {                                               \
            _mask = 0;                                  \
            for (_i = _startBit; _i <= _endBit; ++_i)   \
                _mask |= (1 << _i);                     \
                                                        \
            _val = MEM32_READ(SWCORE_BASE_ADDR| _reg);  \
            _val &= ~(_mask);                           \
            _val |= (_in.val << _startBit);             \
        }                                               \
                                                        \
        MAC_REG_SET_CHK(_reg, _val);                    \
    } while(0)

#define SERDES_SET(_reg, _endBit, _startBit, _inVal)    \
    do {                                                \
        uint32  _val, _len, _mask;                      \
        uint32  _i;                                     \
                                                        \
        _len = _endBit - _startBit + 1;                 \
                                                        \
        if (32 == _len)                                 \
            _val = _inVal;                              \
        else                                            \
        {                                               \
            _mask = 0;                                  \
            for (_i = _startBit; _i <= _endBit; ++_i)   \
                _mask |= (1 << _i);                     \
                                                        \
            _val = MEM32_READ(SWCORE_BASE_ADDR + _reg); \
            _val &= ~(_mask);                           \
            _val |= (_inVal << _startBit);              \
        }                                               \
        MEM32_WRITE(SWCORE_BASE_ADDR + _reg, _val);     \
    } while(0)

/*
 * Data Declaration
 */

/* RTL835x MAC configuration code */
static confcode_mac_regval_t rtl835x_mac_conf[] = {
    //ChipInfo_En
    { 0x0ff4, 0xa0000000 },
    { 0x0008, 0x44444444 },
    { 0x000c, 0x664444 },
    //SW_Core_CLK_Sel_a
    { 0x02ac, 0x80000084 },
    //SW_Core_CLK_Sel_b
    { 0x0058, 0x15C },
    //Queue_Rst
    { 0x0014, 0x4 },
    //IPGComp_En
    { 0x02a8, 0x3c324f40 },
    //LED_En
    //{ 0x00e4, 0x260F56E },    /* depend on the boardmodel */
    //egressRate_unlimit
    { 0x60f8, 0x12972561 },
    //L2_broadcast_a
    { 0x1184, 0x7FFFFFFF },
    //L2_broadcast_b
    { 0x1188, 0xFFFFF800 },
    //L2_broadcast_c
    { 0x1180, 0x38000 },
    //chip_model
    { 0x0ff0, 0x83536800 },
    };

static confcode_mac_regval_t rtl835x_mac_serdes_pwr_save[] = {
    { 0xa0e0, 0x400 },
    { 0xa1e0, 0x400 },
    };

static confcode_mac_regval_t rtl835x_mac_2G5_serdes[] = {
    { 0xa32c, 0x12AA9482 },
    { 0xa3ac, 0x12AA9482 },
    };

static confcode_mac_regval_t rtl835x_mac_2G5_serdes_10[] = {
    { 0xa32c, 0x120F9482 },
    { 0xa3ac, 0x120F9482 },
    };

static confcode_mac_regval_t rtl835x_mac_dis_25m_sdsck_out[] = {
    { 0xa30c, 0x81C5C1F5 },
    { 0xa70c, 0x81C5C1F5 },
    { 0xab0c, 0x81C5C1F5 },
    { 0xaf0c, 0x81C5C1F5 },
    { 0xb70c, 0x81C5C1F5 },
    { 0xb304, 0x7FFB5000 },
    { 0xb384, 0x7FFB5000 },
    { 0xbb04, 0x7FFB5000 },
    { 0xbb84, 0x7FFB5000 },
    };

static confcode_mac_regval_t rtl835x_mac_serdes_0[] = {
    { 0xa300, 0xf3c70000 },
    { 0xa304, 0x0A0B838E },
    { 0xa308, 0x42097211 },
    { 0xa310, 0x5CCC8C65 },
    { 0xa320, 0xC8DB0000 },
    { 0xa324, 0x79000003 },
    { 0xa328, 0xC0C78C62 },
    { 0xa330, 0xB0020000 },
    { 0xa340, 0x10FF04AA },
    { 0xa000, 0x0F009403 },
    { 0xa004, 0x71067080 },
    { 0xa008, 0x8E0F0749 },
    { 0xa00c, 0x13598F5F },
    { 0xa010, 0x0000524B },
    { 0xa014, 0x00000CA4 },
    { 0xa018, 0x466408E4 },
    { 0xa01c, 0x00002053 },
    };

static confcode_mac_regval_t rtl835x_mac_serdes_8[] = {
    { 0xB300, 0x4000D800 },
    { 0xB304, 0x7FFB5000 },
    { 0xB308, 0x60C1001F },
    { 0xB30C, 0xFFFF60C1 },
    { 0xB310, 0xDC6FFFFF },
    { 0xB314, 0x14A51004 },
    { 0xB318, 0x0514F514 },
    { 0xB31C, 0x48C0DA41 },
    { 0xB320, 0xD08BCC06 },
    { 0xB324, 0x510897B3 },
    { 0xB328, 0x00000F03 },
    { 0xB32C, 0x0F400F40 },
    { 0xB330, 0x3DEF3DEF },
    { 0xB334, 0x00000000 },
    { 0xB338, 0x3B203B20 },
    { 0xB33C, 0x007A007A },
    { 0xB340, 0x58F558F5 },
    { 0xB344, 0x41FF41FA },
    { 0xB348, 0x39FF3A04 },
    { 0xB34C, 0x40FF00FF },
    { 0xB350, 0x007F007F },
    { 0xB354, 0x619F619F },
    { 0xB358, 0x28FB29FB },
    { 0xB35C, 0x80788078 },
    { 0xB000, 0x0F009403 },
    { 0xB004, 0x71067080 },
    { 0xB008, 0x8E0F0749 },
    { 0xB00C, 0x53598F5F },
    { 0xB010, 0x0000524B },
    { 0xB014, 0x00000CA4 },
    { 0xB018, 0x466404E4 },
    { 0xB01C, 0x00002053 },
    };

static confcode_mac_regval_t rtl835x_mac_serdes_10[] = {
    { 0xb700, 0xa3c20000 },
    { 0xb704, 0x0a4b8388 },
    { 0xb708, 0x42097211 },
    { 0xb720, 0x261b0000 },
    { 0xb728, 0xc0c78c62 },
    { 0xb728, 0xc0c78e62 },
    { 0xb728, 0xc0c78c62 },
    { 0xb400, 0x0F009403 },
    { 0xb40c, 0x53598f5f },
    { 0xb50c, 0x53598f5f },
    };

static confcode_mac_regval_t rtl835x_mac_serdes_12[] = {
    { 0xbb00, 0x4000d800 },
    { 0xbb04, 0x7ffb5000 },
    { 0xbb08, 0x60c1001f },
    { 0xbb0c, 0xffff60c1 },
    { 0xbb10, 0xdc6fffff },
    { 0xbb14, 0x14a51004 },
    { 0xbb18, 0x514e514  },
    { 0xbb1c, 0x48c08a41 },
    { 0xbb20, 0xf04af216 },
    { 0xbb24, 0xD10F97B3 },
    { 0xbba4, 0xd10f9793 },
    { 0xbb28, 0x0f03     },
    { 0xbb2c, 0x7AF07E0  },
    { 0xbb30, 0x3DEF3DEF },
    { 0xbb34, 0xffffffff },
    { 0xbb38, 0x78037803 },
    { 0xbb3c, 0xf01af01a },
    { 0xbb40, 0xf500f5   },
    { 0xbb44, 0x41ff41ff },
    { 0xbb48, 0x39ff39ff },
    { 0xbb4c, 0x00100510 },
    { 0xbb50, 0x7f007f   },
    { 0xbb54, 0x619f619f },
    { 0xbb58, 0x29fb29fb },
    { 0xbb5c, 0x806d806d },
    { 0xbbdc, 0x806d806d },
    { 0xbb78, 0x18ff3    },
    { 0xbb7c, 0x80000000 },
    { 0xbbc0, 0x18f55a75 },
    { 0xb804, 0x71467080 },
    { 0xb804, 0x71067080 },
    { 0xb904, 0x71467080 },
    { 0xb904, 0x71067080 },
    { 0xb80c, 0x53598f5f },
    { 0xbb78, 0x8553     },
    { 0xbb7c, 0x8000     },
    { 0xbbc0, 0x18f51a75 },
    { 0xbb78, 0x1c553    },
    { 0xbb7c, 0x80008000 },
    { 0xb818, 0x466408ec },
    { 0xb918, 0x466408ec },
    { 0xbb40, 0xc0f5c0f5 },
    { 0xbb40, 0x40f540f5 },
    { 0xb8e0, 0x00000400 },
    { 0xb800, 0x0f009403 },
    };

static confcode_mac_regval_t rtl835x_mac_serdes_rst[] = {
    { 0xA004, 0x71467380 },
    { 0xA004, 0x71067380 },
    { 0xA104, 0x71467380 },
    { 0xA104, 0x71067380 },
    { 0xa340, 0xBF04AA   },
    { 0xa340, 0x10BF04AA },
    { 0xa340, 0x10FF04AA },
    { 0xA404, 0x71467380 },
    { 0xA404, 0x71067380 },
    { 0xA504, 0x71467380 },
    { 0xA504, 0x71067380 },
    { 0xa740, 0xBF04AA   },
    { 0xa740, 0x10BF04AA },
    { 0xa740, 0x10FF04AA },
    { 0xA804, 0x71467380 },
    { 0xA804, 0x71067380 },
    { 0xA904, 0x71467380 },
    { 0xA904, 0x71067380 },
    { 0xab40, 0xBF04AA   },
    { 0xab40, 0x10BF04AA },
    { 0xab40, 0x10FF04AA },
    { 0xAc04, 0x71467380 },
    { 0xAc04, 0x71067380 },
    { 0xAd04, 0x71467380 },
    { 0xAd04, 0x71067380 },
    { 0xaf40, 0xBF04AA   },
    { 0xaf40, 0x10BF04AA },
    { 0xaf40, 0x10FF04AA },
    { 0xb404, 0x71467380 },
    { 0xb404, 0x71067380 },
    { 0xb504, 0x71467380 },
    { 0xb504, 0x71067380 },
    { 0xb740, 0xBF04AA   },
    { 0xb740, 0x10BF04AA },
    { 0xb740, 0x10FF04AA },
    { 0xB004, 0x71467380 },
    { 0xB004, 0x71067380 },
    { 0xB104, 0x71467380 },
    { 0xB104, 0x71067380 },
    { 0xB378, 0x00008541 },
    { 0xB378, 0x00008543 },
    { 0xB378, 0x00008553 },
    { 0xb804, 0x71467380 },
    { 0xb804, 0x71067380 },
    { 0xb904, 0x71467380 },
    { 0xb904, 0x71067380 },
    { 0xbb78, 0x1C541    },
    { 0xbb78, 0x1c543    },
    { 0xbb78, 0x1c553    },
    };

/* RTL839x MAC configuration code */
static confcode_mac_regval_t rtl839x_dis_ck25mo[] = {
    { 0xa30c, 0x81c5C1f5 },
    { 0xa70c, 0x81c5C1f5 },
    { 0xab0c, 0x81c5C1f5 },
    { 0xaf0c, 0x81c5C1f5 },
    { 0xb304, 0x7ffb5000 },
    { 0xb384, 0x7ffb5000 },
    { 0xb70c, 0x81c5C1f5 },
    { 0xbb04, 0x7ffb5000 },
    { 0xbb84, 0x7ffb5000 },
    };

static confcode_mac_regval_t rtl839x_qsgmii[] = {
    { 0x0008, 0x66666666 },
    { 0x000c, 0x00666666 },
    };

#ifdef QSGMII_MODE
static confcode_mac_regval_t rtl839x_serdes10_a2d_clk_edge[] = {
    { 0xb40c, 0x53598f5f },
    };

static confcode_mac_regval_t rtl839x_serdes11_a2d_clk_edge[] = {
    { 0xb50c, 0x53598f5f },
    };

static confcode_mac_regval_t rtl839x_5G_serdes_0[] = {
    { 0xa300, 0xf3c10000 },
    //{ 0xa300, 0x03c10000 },
    { 0xa304, 0x0A0B838E },
    { 0xa308, 0x42095211 },
    { 0xa30c, 0x81c5C1f5 },
    { 0xa310, 0x5ccc8c65 },
    { 0xa320, 0xC6DB0000 },
    { 0xa324, 0x79000003 },
    { 0xa328, 0x0ec78c60 },
    { 0xa32c, 0x14aa9482 },
    { 0xa330, 0xB0020000 },
    { 0xa340, 0x20bf04aa },
    { 0xa340, 0x30bf04aa },
    { 0xa340, 0x30ff04aa },
    { 0xa004, 0x71467080 },
    { 0xa004, 0x71067080 },
    { 0xa008, 0x8E0F0749 },
    { 0xa00c, 0x13598F5F },
    { 0xa010, 0x0000524B },
    { 0xa014, 0x00000CA4 },
    { 0xa018, 0x466408E4 },
    { 0xa01c, 0x00002053 },
    { 0xa0e0, 0x00000400 },
    { 0xa000, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_1[] = {
    { 0xa300, 0xf3c10000 },
    //{ 0xa300, 0x03c10000 },
    { 0xa304, 0x0A0B838E },
    { 0xa308, 0x42095211 },
    { 0xa30c, 0x81c5C1f5 },
    { 0xa390, 0x5ccc0000 },
    { 0xa3a0, 0xC6DB0000 },
    { 0xa3a4, 0x79000003 },
    { 0xa3a8, 0x0ec78c60 },
    { 0xa3ac, 0x14aa9482 },
    { 0xa3b0, 0xB0020000 },
    { 0xa340, 0x107f04aa },
    { 0xa340, 0x307f04aa },
    { 0xa340, 0x30ff04aa },
    { 0xa104, 0x71467080 },
    { 0xa104, 0x71067080 },
    { 0xa108, 0x8E0F0749 },
    { 0xa10c, 0x13598F5F },
    { 0xa110, 0x0000524B },
    { 0xa114, 0x00000CA4 },
    { 0xa118, 0x466408E4 },
    { 0xa11c, 0x00002053 },
    { 0xa1e0, 0x00000400 },
    { 0xa100, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_2[] = {
    { 0xa700, 0xf3c10000 },
    //{ 0xa700, 0x03c10000 },
    { 0xa704, 0x0A0B838E },
    { 0xa708, 0x42095211 },
    { 0xa70c, 0x81c5C1f5 },
    { 0xa710, 0x5ccc8c65 },
    { 0xa720, 0xC6DB0000 },
    { 0xa724, 0x79000003 },
    { 0xa728, 0x0ec78c60 },
    { 0xa72c, 0x14aa9482 },
    { 0xa730, 0xB0020000 },
    { 0xa740, 0x20bf04aa },
    { 0xa740, 0x30bf04aa },
    { 0xa740, 0x30ff04aa },
    { 0xa404, 0x71467080 },
    { 0xa404, 0x71067080 },
    { 0xa408, 0x8E0F0749 },
    { 0xa40c, 0x13598F5F },
    { 0xa410, 0x0000524B },
    { 0xa414, 0x00000CA4 },
    { 0xa418, 0x466408E4 },
    { 0xa41c, 0x00002053 },
    { 0xa4e0, 0x00000400 },
    { 0xa400, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_3[] = {
    { 0xa700, 0xf3c10000 },
    //{ 0xa700, 0x03c10000 },
    { 0xa704, 0x0A0B838E },
    { 0xa708, 0x42095211 },
    { 0xa70c, 0x81c5C1f5 },
    { 0xa790, 0x5ccc0000 },
    { 0xa7a0, 0xC6DB0000 },
    { 0xa7a4, 0x79000003 },
    { 0xa7a8, 0x0ec78c60 },
    { 0xa7ac, 0x14aa9482 },
    { 0xa7b0, 0xB0020000 },
    { 0xa740, 0x107f04aa },
    { 0xa740, 0x307f04aa },
    { 0xa740, 0x30ff04aa },
    { 0xa504, 0x71467080 },
    { 0xa504, 0x71067080 },
    { 0xa508, 0x8E0F0749 },
    { 0xa50c, 0x13598F5F },
    { 0xa510, 0x0000524B },
    { 0xa514, 0x00000CA4 },
    { 0xa518, 0x466408E4 },
    { 0xa51c, 0x00002053 },
    { 0xa5e0, 0x00000400 },
    { 0xa500, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_4[] = {
    { 0xab00, 0xf3c10000 },
    //{ 0xab00, 0x03c10000 },
    { 0xab04, 0x0A0B838E },
    { 0xab08, 0x42095211 },
    { 0xab0c, 0x81c5C1f5 },
    { 0xab10, 0x5ccc8c65 },
    { 0xab20, 0xC6DB0000 },
    { 0xab24, 0x79000003 },
    { 0xab28, 0x0ec78c60 },
    { 0xab2c, 0x14aa9482 },
    { 0xab30, 0xB0020000 },
    { 0xab40, 0x20bf04aa },
    { 0xab40, 0x30bf04aa },
    { 0xab40, 0x30ff04aa },
    { 0xa804, 0x71467080 },
    { 0xa804, 0x71067080 },
    { 0xa808, 0x8E0F0749 },
    { 0xa80c, 0x13598F5F },
    { 0xa810, 0x0000524B },
    { 0xa814, 0x00000CA4 },
    { 0xa818, 0x466408E4 },
    { 0xa81c, 0x00002053 },
    { 0xa8e0, 0x00000400 },
    { 0xa800, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_5[] = {
    { 0xab00, 0xf3c10000 },
    //{ 0xab00, 0x03c10000 },
    { 0xab04, 0x0A0B838E },
    { 0xab08, 0x42095211 },
    { 0xab0c, 0x81c5C1f5 },
    { 0xab90, 0x5ccc0000 },
    { 0xaba0, 0xC6DB0000 },
    { 0xaba4, 0x79000003 },
    { 0xaba8, 0x0ec78c60 },
    { 0xabac, 0x14aa9482 },
    { 0xabb0, 0xB0020000 },
    { 0xab40, 0x107f04aa },
    { 0xab40, 0x307f04aa },
    { 0xab40, 0x30ff04aa },
    { 0xa904, 0x71467080 },
    { 0xa904, 0x71067080 },
    { 0xa908, 0x8E0F0749 },
    { 0xa90c, 0x13598F5F },
    { 0xa910, 0x0000524B },
    { 0xa914, 0x00000CA4 },
    { 0xa918, 0x466408E4 },
    { 0xa91c, 0x00002053 },
    { 0xa9e0, 0x00000400 },
    { 0xa900, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_6[] = {
    { 0xaf00, 0xf3c10000 },
    //{ 0xaf00, 0x03c10000 },
    { 0xaf04, 0x0A0B838E },
    { 0xaf08, 0x42095211 },
    { 0xaf0c, 0x81c5C1f5 },
    { 0xaf10, 0x5ccc8c65 },
    { 0xaf20, 0xC6DB0000 },
    { 0xaf24, 0x79000003 },
    { 0xaf28, 0x0ec78c60 },
    { 0xaf2c, 0x14aa9482 },
    { 0xaf30, 0xB0020000 },
    { 0xaf40, 0x20bf04aa },
    { 0xaf40, 0x30bf04aa },
    { 0xaf40, 0x30ff04aa },
    { 0xac04, 0x71467080 },
    { 0xac04, 0x71067080 },
    { 0xac08, 0x8E0F0749 },
    { 0xac0c, 0x13598F5F },
    { 0xac10, 0x0000524B },
    { 0xac14, 0x00000CA4 },
    { 0xac18, 0x466408E4 },
    { 0xac1c, 0x00002053 },
    { 0xace0, 0x00000400 },
    { 0xac00, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_7[] = {
    { 0xaf00, 0xf3c10000 },
    //{ 0xaf00, 0x03c10000 },
    { 0xaf04, 0x0A0B838E },
    { 0xaf08, 0x42095211 },
    { 0xaf0c, 0x81c5C1f5 },
    { 0xaf90, 0x5ccc0000 },
    { 0xafa0, 0xC6DB0000 },
    { 0xafa4, 0x79000003 },
    { 0xafa8, 0x0ec78c60 },
    { 0xafac, 0x14aa9482 },
    { 0xafb0, 0xB0020000 },
    { 0xaf40, 0x107f04aa },
    { 0xaf40, 0x307f04aa },
    { 0xaf40, 0x30ff04aa },
    { 0xad04, 0x71467080 },
    { 0xad04, 0x71067080 },
    { 0xad08, 0x8E0F0749 },
    { 0xad0c, 0x13598F5F },
    { 0xad10, 0x0000524B },
    { 0xad14, 0x00000CA4 },
    { 0xad18, 0x466408E4 },
    { 0xad1c, 0x00002053 },
    { 0xade0, 0x00000400 },
    { 0xad00, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_10[] = {
    { 0xb700, 0xa3c20000 },
    { 0xb704, 0x4A0B839E },
    { 0xb708, 0x42095211 },
    { 0xb720, 0x6db0000  },
    { 0xb728, 0xd0c78c22 },
    { 0xb730, 0xf0020300 },
    { 0xb70c, 0x81c5C1f5 },
    { 0xb740, 0x20bf080f },
    { 0xb740, 0x30bf080f },
    { 0xb740, 0x30ff080f },
    { 0xb404, 0x71467080 },
    { 0xb404, 0x71067080 },
    { 0xb4e0, 0x00000400 },
    { 0xb400, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_11[] = {
    { 0xb700, 0xa3c20000 },
    { 0xb704, 0x4A0B839E },
    { 0xb708, 0x42095211 },
    { 0xb7a0, 0x165b0000 },
    { 0xb7a8, 0xd0c78c22 },
    { 0xb7b0, 0xf0020300 },
    { 0xb70c, 0x81c5C1f5 },
    { 0xb740, 0x107f080f },
    { 0xb740, 0x307f080f },
    { 0xb740, 0x30ff080f },
    { 0xb504, 0x71467080 },
    { 0xb504, 0x71067080 },
    { 0xb5e0, 0x00000400 },
    { 0xb500, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_8_9[] = {
    { 0xB300, 0x4000d800 },
    { 0xB304, 0x7ffb5000 },
    { 0xb308, 0x60c1001f },
    { 0xb30c, 0xffff60c1 },
    { 0xb310, 0xdc6fffff },
    { 0xb314, 0x14a51004 },
    { 0xb318, 0x514e514  },
    { 0xb31c, 0x48c08a41 },
    { 0xb320, 0xf04af216 },
    { 0xb324, 0xd10f9793 },
    { 0xb328, 0x0f03     },
    { 0xb32c, 0x27af27af },
    { 0xb330, 0x3cc03cc0 },
    { 0xb334, 0xffffffff },
    { 0xb338, 0x78037803 },
    { 0xb33c, 0xf01af01a },
    { 0xb340, 0xf500f5   },
    { 0xb344, 0x41ff41ff },
    { 0xb348, 0x39ff39ff },
    { 0xb34c, 0x00100010 },
    { 0xb350, 0x7f007f   },
    { 0xb354, 0x619f619f },
    { 0xb358, 0x29fb29fb },
    { 0xb35c, 0x806d806d },
    { 0xB380, 0x4000d800 },
    { 0xB384, 0x7ffb5000 },
    { 0xb388, 0x60c1001f },
    { 0xb38c, 0xffff60c1 },
    { 0xb390, 0xdc6fffff },
    { 0xb394, 0x14a51004 },
    { 0xb398, 0x514e514  },
    { 0xb39c, 0x48c08a41 },
    { 0xb3a0, 0xf04af216 },
    { 0xb3a4, 0xd10f9793 },
    { 0xb3a8, 0x0f03     },
    { 0xb3ac, 0x27af27af },
    { 0xb3b0, 0x3c403c40 },
    { 0xb3b4, 0xffffffff },
    { 0xb3b8, 0x78037803 },
    { 0xb3bc, 0xf01af01a },
    { 0xb3c0, 0xf500f5   },
    { 0xb3c4, 0x41ff41ff },
    { 0xb3c8, 0x39ff39ff },
    { 0xb3cc, 0x00100010 },
    { 0xb3d0, 0x7f007f   },
    { 0xb3d4, 0x619f619f },
    { 0xb3d8, 0x29fb29fb },
    { 0xb3dc, 0x806d806d },
    { 0xb378, 0x18ff3    },
    { 0xb37c, 0x80000000 },
    { 0xb004, 0x71467080 },
    { 0xb004, 0x71067080 },
    { 0xb104, 0x71467080 },
    { 0xb104, 0x71067080 },
    { 0xb00c, 0x53598f5f },
    { 0xb10c, 0x53598f5f },
    { 0xb378, 0x8553     },
    { 0xb37c, 0x8000     },
    { 0xb378, 0x1c553    },
    { 0xb37c, 0x80008000 },
    { 0xb018, 0x466408ec },
    { 0xb118, 0x466408ec },
    { 0xb340, 0x80f580f5 },
    { 0xb340, 0x00f500f5 },
    { 0xb340, 0x40f540f5 },
    { 0xb3c0, 0x80f580f5 },
    { 0xb3c0, 0x00f500f5 },
    { 0xb3c0, 0x40f540f5 },
    { 0xb378, 0x0001c541 },
    { 0xb378, 0x0001c543 },
    { 0xb378, 0x0001c553 },
    { 0xb0e0, 0x00000400 },
    { 0xb1e0, 0x00000400 },
    { 0xb000, 0x0f009403 },
    { 0xb100, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_12[] = {
    { 0xbb00, 0x4000d800 },
    { 0xbb04, 0x7ffb5000 },
    { 0xbb08, 0x60c1001f },
    { 0xbb0c, 0xffff60c1 },
    { 0xbb10, 0xdc6fffff },
    { 0xbb14, 0x14a51004 },
    { 0xbb18, 0x514e514  },
    { 0xbb1c, 0x48c08a41 },
    { 0xbb20, 0xf04af216 },
    { 0xbb24, 0xd10f9793 },
    { 0xbba4, 0xd10f9793 },
    { 0xbb28, 0x0f03     },
    { 0xbb2c, 0x87968796 },
    { 0xbb30, 0x3dea3dea },
    { 0xbb34, 0xffffffff },
    { 0xbb38, 0x78037803 },
    { 0xbb3c, 0xf01af01a },
    { 0xbb40, 0xf500f5   },
    { 0xbb44, 0x41ff41ff },
    { 0xbb48, 0x39ff39ff },
    { 0xbb4c, 0x100010   },
    { 0xbb50, 0x7f007f   },
    { 0xbb54, 0x619f619f },
    { 0xbb58, 0x29fb29fb },
    { 0xbb5c, 0x806d806d },
    { 0xbbdc, 0x806d806d },
    { 0xbb78, 0x18ff3    },
    { 0xbb7c, 0x80000000 },
    { 0xbbc0, 0x18f55a75 },
    { 0xb804, 0x71467080 },
    { 0xb804, 0x71067080 },
    { 0xb904, 0x71467080 },
    { 0xb904, 0x71067080 },
    { 0xb80c, 0x53598f5f },
    { 0xbb78, 0x8553     },
    { 0xbb7c, 0x8000     },
    { 0xbbc0, 0x18f51a75 },
    { 0xbb78, 0x1c553    },
    { 0xbb7c, 0x80008000 },
    { 0xb818, 0x466408ec },
    { 0xb918, 0x466408ec },
    { 0xbb40, 0xc0f5c0f5 },
    { 0xbb40, 0x40f540f5 },
    { 0xbb78, 0x1C541    },
    { 0xbb78, 0x1c543    },
    { 0xbb78, 0x1c553    },
    { 0xb8e0, 0x00000400 },
    { 0xb800, 0x0f009403 },
    { 0, 0},
    };
#if 0
static confcode_mac_regval_t rtl839x_fiber_serdes_12_13[] = {
    {0x0ff4, 0xa0000000},
    /* configure serdes#12,13 speed to 1000base-x */
    {0x000c, 0x00776666},
    /* configure chip mode=0x1; 48G+2*sgmii */
    {0x02ac, 0x81},
    /* patch serdes12,13 analog registers */
    {0xbb00, 0x4000D800},
    {0xbb04, 0x7FFB5000},
    {0xbb08, 0x60C1001F},
    {0xbb0C, 0xFFFF60C1},
    {0xbb10, 0xDC6FFFFF},
    {0xbb14, 0x14A51004},
    {0xbb18, 0x0514F514},
    {0xbb1C, 0x48C0DA41},
    {0xbb20, 0xC08BCC00},
    {0xbb24, 0x510F3493},
    {0xbb28, 0x00000F03},
    {0xbb2C, 0x0F400F40},
    {0xbb30, 0x62E162E1},
    {0xbb34, 0x000F000F},
    {0xbb38, 0x2D182D18},
    {0xbb3C, 0x007A007A},
    {0xbb40, 0x4F577A65},
    {0xbb44, 0x41FF41FA},
    {0xbb48, 0x39FF7A04},
    {0xbb4C, 0x401004E3},
    {0xbb50, 0x007F007F},
    {0xbb54, 0x619F619F},
    {0xbb58, 0x29FB29FB},
    {0xbb5C, 0xA078A078},
    {0xbb80, 0x4000D800},
    {0xbb84, 0x7FFB5000},
    {0xbb88, 0x60C1001F},
    {0xbb8C, 0xFFFF60C1},
    {0xbb90, 0xDC6FFFFF},
    {0xbb94, 0x14A51004},
    {0xbb98, 0x0514F514},
    {0xbb9C, 0x48C0DA41},
    {0xbba0, 0xC08BCC00},
    {0xbba4, 0x510F3493},
    {0xbba8, 0x00000F03},
    {0xbbaC, 0x0F400F40},
    {0xbbb0, 0x62E162E1},
    {0xbbb4, 0x000F000F},
    {0xbbb8, 0x2D182D18},
    {0xbbbC, 0x007A007A},
    {0xbbc0, 0x4F577A65},
    {0xbbc4, 0x41FF41FA},
    {0xbbc8, 0x39FF7A04},
    {0xbbcC, 0x401004E3},
    {0xbbd0, 0x007F007F},
    {0xbbd4, 0x619F619F},
    {0xbbd8, 0x29FB29FB},
    {0xbbdC, 0xA078A078},
    {0xbb78, 0x8553    },
    {0xbb7C, 0x80008000},
    {0xb804, 0x71467080},
    {0xb804, 0x71067080},
    {0xb904, 0x71467080},
    {0xb904, 0x71067080},
    {0xb80C, 0x53598F5F},
    {0xb800, 0x0f009403},
    {0xb8e0, 0x00000400},
    {0xb880, 0x61091140},
    {0xb980, 0x61091140},
    {0xbb78, 0x8541    },
    {0xbb78, 0x8543    },
    {0xbb78, 0x8553    },
};
#endif
#else   /* RSGMII+ MODE (the following configuration code) */
static confcode_mac_regval_t rtl839x_serdes10_a2d_clk_edge[] = {
    { 0xb40c, 0x53598f5f },
    };

static confcode_mac_regval_t rtl839x_serdes11_a2d_clk_edge[] = {
    { 0xb50c, 0x53598f5f },
    };

static confcode_mac_regval_t rtl839x_5G_serdes_0[] = {
    { 0xa300, 0x03c10000 },
    { 0xa304, 0x4A0B839E },
    { 0xa308, 0x42095211 },
    { 0xa30c, 0x81c5C1f5 },
    { 0xa310, 0x5ccc8c65 },
    { 0xa320, 0xB61B0000 },
    { 0xa324, 0x79000003 },
    { 0xa328, 0x0ec78c60 },
    { 0xa32c, 0x14aa9482 },
    { 0xa330, 0xf0020000 },
    { 0xa340, 0x20bf04aa },
    { 0xa340, 0x30bf04aa },
    { 0xa340, 0x30ff04aa },
    { 0xa004, 0x71467080 },
    { 0xa004, 0x71067080 },
    { 0xa0e0, 0x00000400 },
    { 0xa000, 0x0f009403 },
    };

static confcode_mac_regval_t rtl839x_5G_serdes_1[] = {
    { 0xa300, 0x03c10000 },
    { 0xa304, 0x4A0B839E },
    { 0xa308, 0x42095211 },
    { 0xa30c, 0x81c5C1f5 },
    { 0xa390, 0x5ccc0000 },
    { 0xa3a0, 0xB61B0000 },
    { 0xa3a4, 0x79000003 },
    { 0xa3a8, 0x0ec78c60 },
    { 0xa3ac, 0x14aa9482 },
    { 0xa3b0, 0xf0020000 },
    { 0xa340, 0x107f04aa },
    { 0xa340, 0x307f04aa },
    { 0xa340, 0x30ff04aa },
    { 0xa104, 0x71467080 },
    { 0xa104, 0x71067080 },
    { 0xa1e0, 0x00000400 },
    { 0xa100, 0x0f009403 },
    };

static confcode_mac_regval_t rtl839x_5G_serdes_2[] = {
    { 0xa700, 0x03c10000 },
    { 0xa704, 0x4A0B839E },
    { 0xa708, 0x42095211 },
    { 0xa70c, 0x81c5C1f5 },
    { 0xa710, 0x5ccc8c65 },
    { 0xa720, 0xB61B0000 },
    { 0xa724, 0x79000003 },
    { 0xa728, 0x0ec78c60 },
    { 0xa72c, 0x14aa9482 },
    { 0xa730, 0xf0020000 },
    { 0xa740, 0x20bf04aa },
    { 0xa740, 0x30bf04aa },
    { 0xa740, 0x30ff04aa },
    { 0xa404, 0x71467080 },
    { 0xa404, 0x71067080 },
    { 0xa4e0, 0x00000400 },
    { 0xa400, 0x0f009403 },
    };

static confcode_mac_regval_t rtl839x_5G_serdes_3[] = {
    { 0xa700, 0x03c10000 },
    { 0xa704, 0x4A0B839E },
    { 0xa708, 0x42095211 },
    { 0xa70c, 0x81c5C1f5 },
    { 0xa790, 0x5ccc0000 },
    { 0xa7a0, 0xB61B0000 },
    { 0xa7a4, 0x79000003 },
    { 0xa7a8, 0x0ec78c60 },
    { 0xa7ac, 0x14aa9482 },
    { 0xa7b0, 0xf0020000 },
    { 0xa740, 0x107f04aa },
    { 0xa740, 0x307f04aa },
    { 0xa740, 0x30ff04aa },
    { 0xa504, 0x71467080 },
    { 0xa504, 0x71067080 },
    { 0xa5e0, 0x00000400 },
    { 0xa500, 0x0f009403 },
    };

static confcode_mac_regval_t rtl839x_5G_serdes_4[] = {
    { 0xab00, 0x03c10000 },
    { 0xab04, 0x4A0B839E },
    { 0xab08, 0x42095211 },
    { 0xab0c, 0x81c5C1f5 },
    { 0xab10, 0x5ccc8c65 },
    { 0xab20, 0xB61B0000 },
    { 0xab24, 0x79000003 },
    { 0xab28, 0x0ec78c60 },
    { 0xab2c, 0x14aa9482 },
    { 0xab30, 0xf0020000 },
    { 0xab40, 0x20bf04aa },
    { 0xab40, 0x30bf04aa },
    { 0xab40, 0x30ff04aa },
    { 0xa804, 0x71467080 },
    { 0xa804, 0x71067080 },
    { 0xa8e0, 0x00000400 },
    { 0xa800, 0x0f009403 },
    };

static confcode_mac_regval_t rtl839x_5G_serdes_5[] = {
    { 0xab00, 0x03c10000 },
    { 0xab04, 0x4A0B839E },
    { 0xab08, 0x42095211 },
    { 0xab0c, 0x81c5C1f5 },
    { 0xab90, 0x5ccc0000 },
    { 0xaba0, 0xB61B0000 },
    { 0xaba4, 0x79000003 },
    { 0xaba8, 0x0ec78c60 },
    { 0xabac, 0x14aa9482 },
    { 0xabb0, 0xf0020000 },
    { 0xab40, 0x107f04aa },
    { 0xab40, 0x307f04aa },
    { 0xab40, 0x30ff04aa },
    { 0xa904, 0x71467080 },
    { 0xa904, 0x71067080 },
    { 0xa9e0, 0x00000400 },
    { 0xa900, 0x0f009403 },
    };

static confcode_mac_regval_t rtl839x_5G_serdes_6[] = {
    { 0xaf00, 0x03c10000 },
    { 0xaf04, 0x8A0B83BE },
    { 0xaf08, 0x42095211 },
    { 0xaf0c, 0x81c5C1f5 },
    { 0xaf10, 0x5ccc8c65 },
    { 0xaf20, 0xB61B0000 },
    { 0xaf24, 0x79000003 },
    { 0xaf28, 0x0ec78c60 },
    { 0xaf2c, 0x14aa9482 },
    { 0xaf30, 0xf0020000 },
    { 0xaf40, 0x20bf04aa },
    { 0xaf40, 0x30bf04aa },
    { 0xaf40, 0x30ff04aa },
    { 0xac04, 0x71467080 },
    { 0xac04, 0x71067080 },
    { 0xace0, 0x00000400 },
    { 0xac00, 0x0f009403 },
    };

static confcode_mac_regval_t rtl839x_5G_serdes_7[] = {
    { 0xaf00, 0x03c10000 },
    { 0xaf04, 0x8A0B83BE },
    { 0xaf08, 0x42095211 },
    { 0xaf0c, 0x81c5C1f5 },
    { 0xaf90, 0x5ccc0000 },
    { 0xafa0, 0xB61B0000 },
    { 0xafa4, 0x79000003 },
    { 0xafa8, 0x0ec78c60 },
    { 0xafac, 0x14aa9482 },
    { 0xafb0, 0xf0020000 },
    { 0xaf40, 0x107f04aa },
    { 0xaf40, 0x307f04aa },
    { 0xaf40, 0x30ff04aa },
    { 0xad04, 0x71467080 },
    { 0xad04, 0x71067080 },
    { 0xade0, 0x00000400 },
    { 0xad00, 0x0f009403 },
    };

static confcode_mac_regval_t rtl839x_5G_serdes_10[] = {
    { 0xb700, 0xa3c20000 },
    { 0xb704, 0x8A0B83BE },
    { 0xb708, 0x42095211 },
    { 0xb720, 0x6db0000  },
    { 0xb728, 0xd0c78c22 },
    { 0xb730, 0xf0020300 },
    { 0xb70c, 0x81c5C1f5 },
    { 0xb740, 0x20bf080f },
    { 0xb740, 0x30bf080f },
    { 0xb740, 0x30ff080f },
    { 0xb404, 0x71467080 },
    { 0xb404, 0x71067080 },
    { 0xb4e0, 0x00000400 },
    { 0xb400, 0x0f009403 },
    };

static confcode_mac_regval_t rtl839x_5G_serdes_11[] = {
    { 0xb700, 0xa3c20000 },
    { 0xb704, 0x8A0B83BE },
    { 0xb708, 0x42095211 },
    { 0xb7a0, 0x169b0000 },
    { 0xb7a8, 0xd0c78c22 },
    { 0xb7b0, 0xf0020300 },
    { 0xb70c, 0x81c5C1f5 },
    { 0xb740, 0x107f080f },
    { 0xb740, 0x307f080f },
    { 0xb740, 0x30ff080f },
    { 0xb504, 0x71467080 },
    { 0xb504, 0x71067080 },
    { 0xb5e0, 0x00000400 },
    { 0xb500, 0x0f009403 },
    };

static confcode_mac_regval_t rtl839x_5G_serdes_8_9[] = {
    { 0xB300, 0x4000d800 },
    { 0xB304, 0x7ffb5000 },
    { 0xb308, 0x60c1001f },
    { 0xb30c, 0xffff60c1 },
    { 0xb310, 0xdc6fffff },
    { 0xb314, 0x14a51004 },
    { 0xb318, 0x514e514  },
    { 0xb31c, 0x48c08a41 },
    { 0xb320, 0xf04af216 },
    { 0xb324, 0xd10f97b3 },
    { 0xb328, 0x0f03     },
    { 0xb32c, 0x27af27af },
    { 0xb330, 0x3cc03cc0 },
    { 0xb334, 0xffffffff },
    { 0xb338, 0x78037803 },
    { 0xb33c, 0xf01af01a },
    { 0xb340, 0xf500f5   },
    { 0xb344, 0x41ff41ff },
    { 0xb348, 0x39ff39ff },
    { 0xb34c, 0x00100010 },
    { 0xb350, 0x7f007f   },
    { 0xb354, 0x619f619f },
    { 0xb358, 0x29fb29fb },
    { 0xb35c, 0x80608060 },
    { 0xB380, 0x4000d800 },
    { 0xB384, 0x7ffb5000 },
    { 0xb388, 0x60c1001f },
    { 0xb38c, 0xffff60c1 },
    { 0xb390, 0xdc6fffff },
    { 0xb394, 0x14a51004 },
    { 0xb398, 0x514e514  },
    { 0xb39c, 0x48c08a41 },
    { 0xb3a0, 0xf04af216 },
    { 0xb3a4, 0xd10f97b3 },
    { 0xb3a8, 0x0f03     },
    { 0xb3ac, 0x27af27af },
    { 0xb3b0, 0x3c403c40 },
    { 0xb3b4, 0xffffffff },
    { 0xb3b8, 0x78037803 },
    { 0xb3bc, 0xf01af01a },
    { 0xb3c0, 0xf500f5   },
    { 0xb3c4, 0x41ff41ff },
    { 0xb3c8, 0x39ff39ff },
    { 0xb3cc, 0x00100010 },
    { 0xb3d0, 0x7f007f   },
    { 0xb3d4, 0x619f619f },
    { 0xb3d8, 0x29fb29fb },
    { 0xb3dc, 0x80608060 },
    { 0xb378, 0x18ff3    },
    { 0xb37c, 0x80000000 },
    { 0xb004, 0x71467080 },
    { 0xb004, 0x71067080 },
    { 0xb104, 0x71467080 },
    { 0xb104, 0x71067080 },
    { 0xb00c, 0x53598f5f },
    { 0xb10c, 0x53598f5f },
    { 0xb378, 0x8553     },
    { 0xb37c, 0x8000     },
    { 0xb378, 0x1c553    },
    { 0xb37c, 0x80008000 },
    { 0xb018, 0x466408ec },
    { 0xb118, 0x466408ec },
    { 0xb340, 0x80f580f5 },
    { 0xb340, 0x00f500f5 },
    { 0xb340, 0x40f540f5 },
    { 0xb3c0, 0x80f580f5 },
    { 0xb3c0, 0x00f500f5 },
    { 0xb3c0, 0x40f540f5 },
    { 0xb378, 0x0001c541 },
    { 0xb378, 0x0001c543 },
    { 0xb378, 0x0001c553 },
    { 0xb0e0, 0x00000400 },
    { 0xb1e0, 0x00000400 },
    { 0xb000, 0x0f009403 },
    { 0xb100, 0x0f009403 },
    };

static confcode_mac_regval_t rtl839x_5G_serdes_12[] = {
    { 0xbb00, 0x4000d800 },
    { 0xbb04, 0x7ffb5000 },
    { 0xbb08, 0x60c1001f },
    { 0xbb0c, 0xffff60c1 },
    { 0xbb10, 0xdc6fffff },
    { 0xbb14, 0x14a51004 },
    { 0xbb18, 0x514e514  },
    { 0xbb1c, 0x48c08a41 },
    { 0xbb20, 0xf04af216 },
    { 0xbb24, 0xd10f97b3 },
    { 0xbb28, 0x0f03     },
    { 0xbb2c, 0x87968796 },
    { 0xbb30, 0x3dea3dea },
    { 0xbb34, 0xffffffff },
    { 0xbb38, 0x78037803 },
    { 0xbb3c, 0xf01af01a },
    { 0xbb40, 0xf500f5   },
    { 0xbb44, 0x41ff41ff },
    { 0xbb48, 0x39ff39ff },
    { 0xbb4c, 0x100010   },
    { 0xbb50, 0x7f007f   },
    { 0xbb54, 0x619f619f },
    { 0xbb58, 0x29fb29fb },
    { 0xbb5c, 0x80608060 },
    { 0xbb78, 0x18ff3    },
    { 0xbb7c, 0x80000000 },
    { 0xbbc0, 0x18f55a75 },
    { 0xb804, 0x71467080 },
    { 0xb804, 0x71067080 },
    { 0xb904, 0x71467080 },
    { 0xb904, 0x71067080 },
    { 0xb80c, 0x53598f5f },
    { 0xbb78, 0x8553     },
    { 0xbb7c, 0x8000     },
    { 0xbbc0, 0x18f51a75 },
    { 0xbb78, 0x1c553    },
    { 0xbb7c, 0x80008000 },
    { 0xb818, 0x466408ec },
    { 0xb918, 0x466408ec },
    { 0xbb40, 0xc0f5c0f5 },
    { 0xbb40, 0x40f540f5 },
    { 0xbb78, 0x1C541    },
    { 0xbb78, 0x1c543    },
    { 0xbb78, 0x1c553    },
    { 0xb8e0, 0x00000400 },
    { 0xb800, 0x0f009403 },
    };
#endif  /* end of QSGMII MODE (else) */

/* for QA board */
static confcode_mac_regval_t rtl839x_serdes10_a2d_clk_edge_qa[] = {
    { 0xb40c, 0x53598f5f },
    };

static confcode_mac_regval_t rtl839x_serdes11_a2d_clk_edge_qa[] = {
    { 0xb50c, 0x53598f5f },
    };

static confcode_mac_regval_t rtl839x_5G_serdes_0_qa[] = {
    { 0xa300, 0x03c10000 },
    { 0xa304, 0x8A0B83BE },
    { 0xa308, 0x42095211 },
    { 0xa30c, 0x81c5C1f5 },
    { 0xa310, 0x5ccc8c65 },
    { 0xa320, 0xB61B0000 },
    { 0xa324, 0x79000003 },
    { 0xa328, 0x0ec78c60 },
    { 0xa32c, 0x14aa9482 },
    { 0xa330, 0xf0020000 },
    { 0xa340, 0x20bf04aa },
    { 0xa340, 0x30bf04aa },
    { 0xa340, 0x30ff04aa },
    { 0xa004, 0x71467080 },
    { 0xa004, 0x71067080 },
    { 0xa0e0, 0x00000400 },
    { 0xa000, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_1_qa[] = {
    { 0xa300, 0x03c10000 },
    { 0xa304, 0x8A0B83BE },
    { 0xa308, 0x42095211 },
    { 0xa30c, 0x81c5C1f5 },
    { 0xa390, 0x5ccc0000 },
    { 0xa3a0, 0xB61B0000 },
    { 0xa3a4, 0x79000003 },
    { 0xa3a8, 0x0ec78c60 },
    { 0xa3ac, 0x14aa9482 },
    { 0xa3b0, 0xf0020300 },
    { 0xa340, 0x107f04aa },
    { 0xa340, 0x307f04aa },
    { 0xa340, 0x30ff04aa },
    { 0xa104, 0x71467080 },
    { 0xa104, 0x71067080 },
    { 0xa1e0, 0x00000400 },
    { 0xa100, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_2_qa[] = {
    { 0xa700, 0x03c10000 },
    { 0xa704, 0x8A0B83BE },
    { 0xa708, 0x42095211 },
    { 0xa70c, 0x81c5C1f5 },
    { 0xa710, 0x5ccc8c65 },
    { 0xa720, 0xB61B0000 },
    { 0xa724, 0x79000003 },
    { 0xa728, 0x0ec78c60 },
    { 0xa72c, 0x14aa9482 },
    { 0xa730, 0xf0020000 },
    { 0xa740, 0x20bf04aa },
    { 0xa740, 0x30bf04aa },
    { 0xa740, 0x30ff04aa },
    { 0xa404, 0x71467080 },
    { 0xa404, 0x71067080 },
    { 0xa4e0, 0x00000400 },
    { 0xa400, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_3_qa[] = {
    { 0xa700, 0x03c10000 },
    { 0xa704, 0x8A0B83BE },
    { 0xa708, 0x42095211 },
    { 0xa70c, 0x81c5C1f5 },
    { 0xa790, 0x5ccc0000 },
    { 0xa7a0, 0xB61B0000 },
    { 0xa7a4, 0x79000003 },
    { 0xa7a8, 0x0ec78c60 },
    { 0xa7ac, 0x14aa9482 },
    { 0xa7b0, 0xf0020000 },
    { 0xa740, 0x107f04aa },
    { 0xa740, 0x307f04aa },
    { 0xa740, 0x30ff04aa },
    { 0xa504, 0x71467080 },
    { 0xa504, 0x71067080 },
    { 0xa5e0, 0x00000400 },
    { 0xa500, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_4_qa[] = {
    { 0xab00, 0x03c10000 },
    { 0xab04, 0x8A0B83BE },
    { 0xab08, 0x42095211 },
    { 0xab0c, 0x81c5C1f5 },
    { 0xab10, 0x5ccc8c65 },
    { 0xab20, 0xB61B0000 },
    { 0xab24, 0x79000003 },
    { 0xab28, 0x0ec78c60 },
    { 0xab2c, 0x14aa9482 },
    { 0xab30, 0xf0020000 },
    { 0xab40, 0x20bf04aa },
    { 0xab40, 0x30bf04aa },
    { 0xab40, 0x30ff04aa },
    { 0xa804, 0x71467080 },
    { 0xa804, 0x71067080 },
    { 0xa8e0, 0x00000400 },
    { 0xa800, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_5_qa[] = {
    { 0xab00, 0x03c10000 },
    { 0xab04, 0x8A0B83BE },
    { 0xab08, 0x42095211 },
    { 0xab0c, 0x81c5C1f5 },
    { 0xab90, 0x5ccc0000 },
    { 0xaba0, 0xB61B0000 },
    { 0xaba4, 0x79000003 },
    { 0xaba8, 0x0ec78c60 },
    { 0xabac, 0x14aa9482 },
    { 0xabb0, 0xf0020000 },
    { 0xab40, 0x107f04aa },
    { 0xab40, 0x307f04aa },
    { 0xab40, 0x30ff04aa },
    { 0xa904, 0x71467080 },
    { 0xa904, 0x71067080 },
    { 0xa9e0, 0x00000400 },
    { 0xa900, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_6_qa[] = {
    { 0xaf00, 0x03c10000 },
    { 0xaf04, 0x8A0B83BE },
    { 0xaf08, 0x42095211 },
    { 0xaf0c, 0x81c5C1f5 },
    { 0xaf10, 0x5ccc8c65 },
    { 0xaf20, 0xB61B0000 },
    { 0xaf24, 0x79000003 },
    { 0xaf28, 0x0ec78c60 },
    { 0xaf2c, 0x14aa9482 },
    { 0xaf30, 0xf0020000 },
    { 0xaf40, 0x20bf04aa },
    { 0xaf40, 0x30bf04aa },
    { 0xaf40, 0x30ff04aa },
    { 0xac04, 0x71467080 },
    { 0xac04, 0x71067080 },
    { 0xace0, 0x00000400 },
    { 0xac00, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_7_qa[] = {
    { 0xaf00, 0x03c10000 },
    { 0xaf04, 0x8A0B83BE },
    { 0xaf08, 0x42095211 },
    { 0xaf0c, 0x81c5C1f5 },
    { 0xaf90, 0x5ccc0000 },
    { 0xafa0, 0xB61B0000 },
    { 0xafa4, 0x79000003 },
    { 0xafa8, 0x0ec78c60 },
    { 0xafac, 0x14aa9482 },
    { 0xafb0, 0xf0020000 },
    { 0xaf40, 0x107f04aa },
    { 0xaf40, 0x307f04aa },
    { 0xaf40, 0x30ff04aa },
    { 0xad04, 0x71467080 },
    { 0xad04, 0x71067080 },
    { 0xade0, 0x00000400 },
    { 0xad00, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_10_qa[] = {
    { 0xb700, 0xa3c20000 },
    { 0xb704, 0x8A0B83BE },
    { 0xb708, 0x42095211 },
    { 0xb720, 0x6db0000  },
    { 0xb728, 0xd0c78c22 },
    { 0xb730, 0xf0020300 },
    { 0xb70c, 0x81c5C1f5 },
    { 0xb740, 0x20bf080f },
    { 0xb740, 0x30bf080f },
    { 0xb740, 0x30ff080f },
    { 0xb404, 0x71467080 },
    { 0xb404, 0x71067080 },
    { 0xb4e0, 0x00000400 },
    { 0xb400, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_11_qa[] = {
    { 0xb700, 0xa3c20000 },
    { 0xb704, 0x8A0B83BE },
    { 0xb708, 0x42095211 },
    { 0xb7a0, 0x169b0000 },
    { 0xb7a8, 0xd0c78c22 },
    { 0xb7b0, 0xf0020300 },
    { 0xb70c, 0x81c5C1f5 },
    { 0xb740, 0x107f080f },
    { 0xb740, 0x307f080f },
    { 0xb740, 0x30ff080f },
    { 0xb504, 0x71467080 },
    { 0xb504, 0x71067080 },
    { 0xb5e0, 0x00000400 },
    { 0xb500, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_8_9_qa[] = {
    { 0xB300, 0x4000d800 },
    { 0xB304, 0x7ffb5000 },
    { 0xb308, 0x60c1001f },
    { 0xb30c, 0xffff60c1 },
    { 0xb310, 0xdc6fffff },
    { 0xb314, 0x14a51004 },
    { 0xb318, 0x514e514  },
    { 0xb31c, 0x48c08a41 },
    { 0xb320, 0xf04af216 },
    { 0xb324, 0xd10f9793 },
    { 0xb328, 0x0f03     },
    { 0xb32c, 0x27b727b7 },
    { 0xb330, 0x3cc03cc0 },
    { 0xb334, 0xffffffff },
    { 0xb338, 0x78037803 },
    { 0xb33c, 0xf01af01a },
    { 0xb340, 0xf500f5   },
    { 0xb344, 0x41ff41ff },
    { 0xb348, 0x39ff39ff },
    { 0xb34c, 0x00100010 },
    { 0xb350, 0x7f007f   },
    { 0xb354, 0x619f619f },
    { 0xb358, 0x29fb29fb },
    { 0xb35c, 0x80608071 },
    { 0xB380, 0x4000d800 },
    { 0xB384, 0x7ffb5000 },
    { 0xb388, 0x60c1001f },
    { 0xb38c, 0xffff60c1 },
    { 0xb390, 0xdc6fffff },
    { 0xb394, 0x14a51004 },
    { 0xb398, 0x514e514  },
    { 0xb39c, 0x48c08a41 },
    { 0xb3a0, 0xf04af216 },
    { 0xb3a4, 0xd10f9793 },
    { 0xb3a8, 0x0f03     },
    { 0xb3ac, 0x27b727b7 },
    { 0xb3b0, 0x3c403c40 },
    { 0xb3b4, 0xffffffff },
    { 0xb3b8, 0x78037803 },
    { 0xb3bc, 0xf01af01a },
    { 0xb3c0, 0xf500f5   },
    { 0xb3c4, 0x41ff41ff },
    { 0xb3c8, 0x39ff39ff },
    { 0xb3cc, 0x00100010 },
    { 0xb3d0, 0x7f007f   },
    { 0xb3d4, 0x619f619f },
    { 0xb3d8, 0x29fb29fb },
    { 0xb3dc, 0x80608071 },
    { 0xb378, 0x18ff3    },
    { 0xb37c, 0x80000000 },
    { 0xb004, 0x71467080 },
    { 0xb004, 0x71067080 },
    { 0xb104, 0x71467080 },
    { 0xb104, 0x71067080 },
    { 0xb00c, 0x53598f5f },
    { 0xb10c, 0x53598f5f },
    { 0xb378, 0x8553     },
    { 0xb37c, 0x8000     },
    { 0xb378, 0x1c553    },
    { 0xb37c, 0x80008000 },
    { 0xb018, 0x466408ec },
    { 0xb118, 0x466408ec },
    { 0xb340, 0x80f580f5 },
    { 0xb340, 0x00f500f5 },
    { 0xb340, 0x40f540f5 },
    { 0xb3c0, 0x80f580f5 },
    { 0xb3c0, 0x00f500f5 },
    { 0xb3c0, 0x40f540f5 },
    { 0xb378, 0x0001c541 },
    { 0xb378, 0x0001c543 },
    { 0xb378, 0x0001c553 },
    { 0xb0e0, 0x00000400 },
    { 0xb1e0, 0x00000400 },
    { 0xb000, 0x0f009403 },
    { 0xb100, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_12_qa[] = {
    { 0xbb00, 0x4000d800 },
    { 0xbb04, 0x7ffb5000 },
    { 0xbb08, 0x60c1001f },
    { 0xbb0c, 0xffff60c1 },
    { 0xbb10, 0xdc6fffff },
    { 0xbb14, 0x14a51004 },
    { 0xbb18, 0x514e514  },
    { 0xbb1c, 0x48c08a41 },
    { 0xbb20, 0xf04af216 },
    { 0xbb24, 0xd10f9793 },
    { 0xbb28, 0x0f03     },
    { 0xbb2c, 0x87968796 },
    { 0xbb30, 0x3dea3dea },
    { 0xbb34, 0xffffffff },
    { 0xbb38, 0x78037803 },
    { 0xbb3c, 0xf01af01a },
    { 0xbb40, 0xf500f5   },
    { 0xbb44, 0x41ff41ff },
    { 0xbb48, 0x39ff39ff },
    { 0xbb4c, 0x100010   },
    { 0xbb50, 0x7f007f   },
    { 0xbb54, 0x619f619f },
    { 0xbb58, 0x29fb29fb },
    { 0xbb5c, 0x80608070 },
    { 0xbb78, 0x18ff3    },
    { 0xbb7c, 0x80000000 },
    { 0xbbc0, 0x18f55a75 },
    { 0xb804, 0x71467080 },
    { 0xb804, 0x71067080 },
    { 0xb904, 0x71467080 },
    { 0xb904, 0x71067080 },
    { 0xb80c, 0x53598f5f },
    { 0xbb78, 0x8553     },
    { 0xbb7c, 0x8000     },
    { 0xbbc0, 0x18f51a75 },
    { 0xbb78, 0x1c553    },
    { 0xbb7c, 0x80008000 },
    { 0xb818, 0x466408ec },
    { 0xb918, 0x466408ec },
    { 0xbb40, 0xc0f5c0f5 },
    { 0xbb40, 0x40f540f5 },
    { 0xbb78, 0x1C541    },
    { 0xbb78, 0x1c543    },
    { 0xbb78, 0x1c553    },
    { 0xb8e0, 0x00000400 },
    { 0xb800, 0x0f009403 },
    { 0, 0},
    };

confcode_mac_regval_t rtl835x_serdes_powerOff_conf[] = {
    //#S1,S3,S5,S7,S11: frc_rx=0,frc_pdown=1,frc_cmu=0,frc_no_sds=1
    { 0xa340, 0xc400043f},
    { 0xa3c0, 0xc40043f },
    { 0xa740, 0xc400043f},
    { 0xa7c0, 0xc40043f },
    { 0xab40, 0xc400043f},
    { 0xabc0, 0xc40043f },
    { 0xaf40, 0xc400043f},
    { 0xafc0, 0xc40043f },
    { 0xb740, 0xc400043f},
    { 0xb7c0, 0xc40043f },

    //#S9: frc_pdown=1,frc_rx=0
    { 0xb3f8, 0x4c00000 },
};

confcode_serdes_patch_t rtl839x_init_fiber_1g_frc_S12[] = {
  //S12 to 1000base-x
    { 0x000c  , 0       , 19 , 16 , 0x7     },
  //force digital to 20b
    { 0xb818  , 0       , 3  , 3  , 0       },

  //force spd
    { 0xbbfc  , 0       , 16 , 16 , 1       },
    { 0xbbfc  , 0       , 19 , 18 , 0x2     },

  //DIV2 = 1
    { 0xbb58  , 0       , 0  , 0  , 1       },
    { 0xbb58  , 0       , 4  , 4  , 1       },
    { 0xbb58  , 0       , 16 , 16 , 1       },
    { 0xbb58  , 0       , 20 , 20 , 1       },

  //2013/5/8, reg_clkla_en=0, Change KI/KD/KP1/KP2/KP1_1 to 0389b
    { 0xbb38  , 0       , 15 , 0  , 0x0722  },

  //REG_PI2_M_MODE_1 = 1, REG_PI_M_MODE_1 = 1
    { 0xbb40  , 0       , 15 , 0  , 0x18f5  },

  //normal capacitor for 5GHz mode
    { 0xbb00  , 0       , 11 , 11 , 0       },

  //S12
  //disable Nway force mode
    { 0xb804  , 0       , 9  , 8  , 0x0     },
  //0xBB00B880 SDS12_13_XSG0.FR0_CFG_FIB_SPD_RD_01[6:6]  = 0x1
  //0xBB00B880 SDS12_13_XSG0.FR0_CFG_FIB_SPD_RD_00[13:13] = 0x0
  //0xBB00B880 SDS12_13_XSG0.FR0_CFG_FIB_ANEN[12:12] = 0x1
    { 0xb880  , 0       , 13 , 13 , 0       },
    { 0xb880  , 0       , 6  , 6  , 1       },
    { 0xb880  , 0       , 12 , 12 , 1       },

  //2013/4/16, s12,s13: reg_rx_sel_rxidle=1(from oobs), reg_oobs_sel_val<4:0>=0x15
    { 0xbb20  , 0       , 5  , 5  , 1       },
    { 0xbb1c  , 0       , 11 , 7  , 0x15    },

  //enable pause ability
    { 0xb888  , 0       , 7  , 7  , 1       },
    { 0xb888  , 0       , 8  , 8  , 1       },

  //rst {12} reset tx & rx
    { 0xbbf8  , 0       , 21 , 20 , 0x3     },
    { 0xbbf8  , 0       , 25 , 24 , 0x1     },
    { 0xbbf8  , 0       , 21 , 20 , 0x1     },
    { 0xbbf8  , 0       , 25 , 24 , 0x3     },
    { 0xbbf8  , 0       , 21 , 20 , 0x0     },
    { 0xbbf8  , 0       , 25 , 24 , 0x0     },
};

confcode_serdes_patch_t rtl839x_init_fiber_1g_frc_S13[] = {
  //S13 to 1000base-x
    { 0x000c  , 0       , 23 , 20 , 0x7     },
  //force digital to 20b
    { 0xb918  , 0       , 3  , 3  , 0       },

  //force spd
    { 0xbbfc  , 0       , 17 , 17 , 1       },
    { 0xbbfc  , 0       , 21 , 20 , 0x2     },

  //DIV2 = 1
    { 0xbbd8  , 0       , 0  , 0  , 1       },
    { 0xbbd8  , 0       , 4  , 4  , 1       },
    { 0xbbd8  , 0       , 16 , 16 , 1       },
    { 0xbbd8  , 0       , 20 , 20 , 1       },

  //2013/5/8, reg_clkla_en=0, Change KI/KD/KP1/KP2/KP1_1 to 0389b
    { 0xbbb8  , 0       , 15 , 0  , 0x0722  },

  //REG_PI2_M_MODE_1 = 1, REG_PI_M_MODE_1 = 1
    { 0xbbc0  , 0       , 15 , 0  , 0x18f5  },

  //normal capacitor for 5GHz mode
    { 0xbb80  , 0       , 11 , 11 , 0       },

  //S13
  //disable Nway force mode
    { 0xb904  , 0       , 9  , 8  , 0x0     },
  //0xBB00B980 SDS12_13_XSG0.FR0_CFG_FIB_SPD_RD_01[6:6]  = 0x1
  //0xBB00B980 SDS12_13_XSG0.FR0_CFG_FIB_SPD_RD_00[13:13] = 0x0
  //0xBB00B980 SDS12_13_XSG0.FR0_CFG_FIB_ANEN[12:12] = 0x1
    { 0xb980  , 0       , 13 , 13 , 0       },
    { 0xb980  , 0       , 6  , 6  , 1       },
    { 0xb980  , 0       , 12 , 12 , 1       },

  //2013/4/16, s12,s13: reg_rx_sel_rxidle=1(from oobs), reg_oobs_sel_val<4:0>=0x15
    { 0xbb20  , 0x80    , 5  , 5  , 1       },
    { 0xbb1c  , 0x80    , 11 , 7  , 0x15    },

  //enable pause ability
    { 0xb988  , 0       , 7  , 7  , 1       },
    { 0xb988  , 0       , 8  , 8  , 1       },

  //rst {13} reset tx & rx
    { 0xbbf8  , 0       , 23 , 22 , 0x3     },
    { 0xbbf8  , 0       , 27 , 26 , 0x1     },
    { 0xbbf8  , 0       , 23 , 22 , 0x1     },
    { 0xbbf8  , 0       , 27 , 26 , 0x3     },
    { 0xbbf8  , 0       , 23 , 22 , 0x0     },
    { 0xbbf8  , 0       , 27 , 26 , 0x0     },
};

/*
 * Function Declaration
 */

/* Function Name:
 *      rtl8390_platform_config_init
 * Description:
 *      Platform Configuration code in RTL8390
 * Input:
 *      pModel - pointer to switch model of platform
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
static void rtl8390_platform_config_init(const rtk_switch_model_t *pModel)
{
    char *s_mac = NULL;
    int     i, j;

    /* write MAC addr to register */
    s_mac = getenv("ethaddr");
    if (s_mac != NULL)
    {
        char enetaddr[6], *e = NULL;

        for (i=0; i<6; i++)
        {
            enetaddr[i] = (unsigned char)(s_mac ? simple_strtoul(s_mac, &e, 16) : 0);
            if (s_mac)
            s_mac = (*e) ? e+1 : e;
        }

#if 0
        REG32(SWCORE_BASE_ADDR| RTL8390_MAC_ADDR_CTRL_ADDR) = (*(unsigned int *)&enetaddr[0] & 0x0000FFFFU);
        REG32(SWCORE_BASE_ADDR| RTL8390_MAC_ADDR_CTRL_ADDR+4) = (*(unsigned int *)&enetaddr[2] & 0xFFFFFFFFU);
#else
        REG32(SWCORE_BASE_ADDR| RTL8390_MAC_ADDR_CTRL_ADDR) = (((uint8)enetaddr[0]) << 8) | ((uint8)enetaddr[1]);
        REG32(SWCORE_BASE_ADDR| (RTL8390_MAC_ADDR_CTRL_ADDR+4)) = (((uint8)enetaddr[2]) << 24) | (((uint8)enetaddr[3]) << 16) | (((uint8)enetaddr[4]) << 8) | ((uint8)enetaddr[5]);
#endif
    }

    MEM32_WRITE(SWCORE_BASE_ADDR | RTL8390_SMI_PORT_POLLING_CTRL_ADDR(0), 0);
    MEM32_WRITE(SWCORE_BASE_ADDR | RTL8390_SMI_PORT_POLLING_CTRL_ADDR(32), 0);

    for (i = 0; i < pModel->port.count; ++i)
    {
        j = pModel->port.list[i].mac_id;
        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR | RTL8390_SMI_PORT_POLLING_CTRL_ADDR(j), \
            RTL8390_SMI_PORT_POLLING_CTRL_SMI_POLLING_PMSK_OFFSET(j),
            RTL8390_SMI_PORT_POLLING_CTRL_SMI_POLLING_PMSK_MASK(j), 0x1);
    }

#ifdef CONFIG_RTL8390_FPGA
    /* set PHY start address to 1 */
    MEM32_WRITE(SWCORE_BASE_ADDR| RTL8390_SMI_GLB_CTRL_ADDR, 0x81);

    /* lower FC threshold to fit FPGA packet buffer size */
    REG32(SWCORE_BASE_ADDR | RTL8390_FC_DROP_THR_ADDR)         = 0x000003FF;
    REG32(SWCORE_BASE_ADDR | RTL8390_FC_GLB_HI_THR_ADDR)       = 0x02320226;
    REG32(SWCORE_BASE_ADDR | RTL8390_FC_GLB_LO_THR_ADDR)       = 0x0168015E;
    REG32(SWCORE_BASE_ADDR | RTL8390_FC_GLB_FCOFF_HI_THR_ADDR) = 0x02320226;
    REG32(SWCORE_BASE_ADDR | RTL8390_FC_GLB_FCOFF_LO_THR_ADDR) = 0x0168015E;
    REG32(SWCORE_BASE_ADDR | RTL8390_FC_P_HI_THR_ADDR(0))      = 0x0064005E;
    REG32(SWCORE_BASE_ADDR | RTL8390_FC_P_LO_THR_ADDR(0))      = 0x00140014;
    REG32(SWCORE_BASE_ADDR | RTL8390_FC_P_FCOFF_HI_THR_ADDR(0))= 0x0064005E;
#endif

    return;
} /* end of rtl8390_platform_config_init */

/* Function Name:
 *      rtl8390_mac_config_init
 * Description:
 *      Mac Configuration code in RTL8390
 * Input:
 *      pModel - pointer to switch model of platform
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
static void rtl8390_mac_config_init(const rtk_switch_model_t *pModel)
{
    uint32 chip_info = 0;
    uint32 idx;
    uint32 i, active;

    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_CHIP_INFO_ADDR, \
        RTL8390_CHIP_INFO_CHIP_INFO_EN_OFFSET, \
        RTL8390_CHIP_INFO_CHIP_INFO_EN_MASK, \
        0xA);

    chip_info = MEM32_READ(SWCORE_BASE_ADDR| RTL8390_CHIP_INFO_ADDR);

    if ((chip_info & RTL8390_CHIP_INFO_RL_ID_MASK) == 0x0399)
    {
        if ((pModel->chip == RTK_CHIP_RTL8352M) || \
            (pModel->chip == RTK_CHIP_RTL8353M))
        {
            /* MAC basic configuration */
            for (idx=0; idx<(sizeof(rtl835x_mac_conf)/sizeof(confcode_mac_regval_t)); idx++)
            {
                MAC_REG_SET_CHK(rtl835x_mac_conf[idx].reg, rtl835x_mac_conf[idx].val);
            }
        }   /* RTK_CHIP_RTL835xM */
        else if ((pModel->chip == RTK_CHIP_RTL8392M) || \
            (pModel->chip == RTK_CHIP_RTL8393M) || \
            (pModel->chip == RTK_CHIP_RTL8396M))
        {
            MEM32_WRITE(SWCORE_BASE_ADDR| RTL8390_MAC_EFUSE_CTRL_ADDR, 0x00000080);

            //set Queue_Rst {0x0014 0x4}
            MEM32_WRITE(SWCORE_BASE_ADDR| RTL8390_RST_GLB_CTRL_ADDR, 0x00000004);

            //set IPGComp_En {0x02a8 0x3c324f40}
            MEM32_WRITE(SWCORE_BASE_ADDR| RTL8390_MAC_GLB_CTRL_ADDR, 0x3c324f40);

            #if 0   /* depend on the setting of boardmodel */
            //set LED_En {0x00e4 0x260F56E}
            MEM32_WRITE(SWCORE_BASE_ADDR| RTL8390_LED_GLB_CTRL_ADDR, 0x0260f56e);
            #endif

            //set egressRate_unlimit {0x60f8 0x12972561}
            MEM32_WRITE(SWCORE_BASE_ADDR| RTL8390_SCHED_LB_TICK_TKN_CTRL_ADDR, 0x1297b961);
        }

        /* L2 Table default entry */
        MEM32_WRITE(SWCORE_BASE_ADDR| RTL8390_TBL_ACCESS_L2_DATA_ADDR(0), 0x7FFFFFFF);
        MEM32_WRITE(SWCORE_BASE_ADDR| RTL8390_TBL_ACCESS_L2_DATA_ADDR(1), 0xFFFFF800);
        MEM32_WRITE(SWCORE_BASE_ADDR| RTL8390_TBL_ACCESS_L2_CTRL_ADDR, 0x38000);

        /* Initial (Tick, Token) value */
        if ((pModel->chip == RTK_CHIP_RTL8352M) || \
            (pModel->chip == RTK_CHIP_RTL8353M))
        {   /* 50MHz */
            /* Scheduling */
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_SCHED_LB_TICK_TKN_CTRL_ADDR, \
                RTL8390_SCHED_LB_TICK_TKN_CTRL_TICK_PERIOD_OFFSET, \
                RTL8390_SCHED_LB_TICK_TKN_CTRL_TICK_PERIOD_MASK, \
                37);
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_SCHED_LB_TICK_TKN_CTRL_ADDR, \
                RTL8390_SCHED_LB_TICK_TKN_CTRL_BYTE_PER_TKN_OFFSET, \
                RTL8390_SCHED_LB_TICK_TKN_CTRL_BYTE_PER_TKN_MASK, \
                97);

            /* Storm Control - pps */
            /* Giga Port */
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| (RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_ADDR + 4), \
                RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_TICK_PERIOD_OFFSET, \
                RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_TICK_PERIOD_MASK, \
                240);
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| (RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_ADDR + 4), \
                RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_TKN_OFFSET, \
                RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_TKN_MASK, \
                5);
            /* 10G Port */
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| (RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_ADDR + 0), \
                RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_TICK_PERIOD_OFFSET, \
                RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_TICK_PERIOD_MASK, \
                240);
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| (RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_ADDR + 0), \
                RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_TKN_OFFSET, \
                RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_TKN_MASK, \
                5);

            /* Protocol Storm - pps */
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_STORM_CTRL_SPCL_LB_TICK_TKN_CTRL_ADDR, \
                RTL8390_STORM_CTRL_SPCL_LB_TICK_TKN_CTRL_TICK_PERIOD_OFFSET, \
                RTL8390_STORM_CTRL_SPCL_LB_TICK_TKN_CTRL_TICK_PERIOD_MASK, \
                195313);

            /* ACL Policer - bps */
            for (idx=0; idx<16; idx++)
            {
                MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_METER_LB_TICK_TKN_CTRL_ADDR(idx), \
                    RTL8390_METER_LB_TICK_TKN_CTRL_TICK_PERIOD_OFFSET, \
                    RTL8390_METER_LB_TICK_TKN_CTRL_TICK_PERIOD_MASK, \
                    214);
                MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_METER_LB_TICK_TKN_CTRL_ADDR(idx), \
                    RTL8390_METER_LB_TICK_TKN_CTRL_TKN_OFFSET, \
                    RTL8390_METER_LB_TICK_TKN_CTRL_TKN_MASK, \
                    561);

            }

            /* Input BW - bps */
            /* Giga Port */
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| (RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_ADDR + 4), \
                RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_TICK_PERIOD_OFFSET, \
                RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_TICK_PERIOD_MASK, \
                214);
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| (RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_ADDR + 4), \
                RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_TKN_OFFSET, \
                RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_TKN_MASK, \
                561);
        }
        else if ((pModel->chip == RTK_CHIP_RTL8392M) || \
                 (pModel->chip == RTK_CHIP_RTL8393M) || \
                 (pModel->chip == RTK_CHIP_RTL8396M))
        {   /* 250MHz */
            /* Scheduling */
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_SCHED_LB_TICK_TKN_CTRL_ADDR, \
                RTL8390_SCHED_LB_TICK_TKN_CTRL_TICK_PERIOD_OFFSET, \
                RTL8390_SCHED_LB_TICK_TKN_CTRL_TICK_PERIOD_MASK, \
                185);
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_SCHED_LB_TICK_TKN_CTRL_ADDR, \
                RTL8390_SCHED_LB_TICK_TKN_CTRL_BYTE_PER_TKN_OFFSET, \
                RTL8390_SCHED_LB_TICK_TKN_CTRL_BYTE_PER_TKN_MASK, \
                97);
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_SCHED_LB_TICK_TKN_CTRL_ADDR, \
                RTL8390_SCHED_LB_TICK_TKN_CTRL_TICK_PERIOD_10G_OFFSET, \
                RTL8390_SCHED_LB_TICK_TKN_CTRL_TICK_PERIOD_10G_MASK, \
                18);
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_SCHED_LB_TICK_TKN_CTRL_ADDR, \
                RTL8390_SCHED_LB_TICK_TKN_CTRL_BYTE_PER_TKN_10G_OFFSET, \
                RTL8390_SCHED_LB_TICK_TKN_CTRL_BYTE_PER_TKN_10G_MASK, \
                151);

            /* Storm Control - pps */
            /* Giga Port */
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| (RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_ADDR + 4), \
                RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_TICK_PERIOD_OFFSET, \
                RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_TICK_PERIOD_MASK, \
                238);
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| (RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_ADDR + 4), \
                RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_TKN_OFFSET, \
                RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_TKN_MASK, \
                1);
            /* 10G Port */
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| (RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_ADDR + 0), \
                RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_TICK_PERIOD_OFFSET, \
                RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_TICK_PERIOD_MASK, \
                238);
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| (RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_ADDR + 0), \
                RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_TKN_OFFSET, \
                RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_TKN_MASK, \
                1);

            /* Protocol Storm - pps */
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_STORM_CTRL_SPCL_LB_TICK_TKN_CTRL_ADDR, \
                RTL8390_STORM_CTRL_SPCL_LB_TICK_TKN_CTRL_TICK_PERIOD_OFFSET, \
                RTL8390_STORM_CTRL_SPCL_LB_TICK_TKN_CTRL_TICK_PERIOD_MASK, \
                976563);

            /* ACL Policer - bps */
            for (idx=0; idx<16; idx++)
            {
                MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_METER_LB_TICK_TKN_CTRL_ADDR(idx), \
                    RTL8390_METER_LB_TICK_TKN_CTRL_TICK_PERIOD_OFFSET, \
                    RTL8390_METER_LB_TICK_TKN_CTRL_TICK_PERIOD_MASK, \
                    288);
                MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_METER_LB_TICK_TKN_CTRL_ADDR(idx), \
                    RTL8390_METER_LB_TICK_TKN_CTRL_TKN_OFFSET, \
                    RTL8390_METER_LB_TICK_TKN_CTRL_TKN_MASK, \
                    151);

            }

            /* Input BW - bps */
            /* Giga Port */
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| (RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_ADDR + 4), \
                RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_TICK_PERIOD_OFFSET, \
                RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_TICK_PERIOD_MASK, \
                246);
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| (RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_ADDR + 4), \
                RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_TKN_OFFSET, \
                RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_TKN_MASK, \
                129);
            /* 10G Port */
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| (RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_ADDR + 0), \
                RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_TICK_PERIOD_OFFSET, \
                RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_TICK_PERIOD_MASK, \
                18);
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| (RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_ADDR + 0), \
                RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_TKN_OFFSET, \
                RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_TKN_MASK, \
                151);
        }
    }

    /*
     * LED initialize
     */
    /* config LED interface */
    if (LED_IF_SEL_NONE != pModel->led.led_if_sel)
    {
        switch (pModel->led.led_if_sel)
        {
            case LED_IF_SEL_SERIAL:
                i = 0;
                active = 0; /* Low */
                break;
            case LED_IF_SEL_SINGLE_COLOR_SCAN:
                i = 1;
                active = 1; /* High */
                break;
            case LED_IF_SEL_BI_COLOR_SCAN:
                i = 2;
                active = 1; /* High */
                break;
            default:
                printf("Invalid LED IF SEL\n");
                return;
        }

        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_GLB_CTRL_ADDR,
                RTL8390_LED_GLB_CTRL_LED_IF_SEL_OFFSET,
                RTL8390_LED_GLB_CTRL_LED_IF_SEL_MASK, i);

        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_GLB_CTRL_ADDR,
                RTL8390_LED_GLB_CTRL_LED_ACTIVE_OFFSET,
                RTL8390_LED_GLB_CTRL_LED_ACTIVE_MASK, active);
    }

    /* config LED number */
    if (0 != pModel->led.num &&
            pModel->led.num < (1 << RTL8390_LED_GLB_CTRL_LED_NUM_SEL_OFFSET))
    {
        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_GLB_CTRL_ADDR,
                RTL8390_LED_GLB_CTRL_LED_NUM_SEL_OFFSET,
                RTL8390_LED_GLB_CTRL_LED_NUM_SEL_MASK, pModel->led.num);
    }

    /* enable LED */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_GLB_CTRL_ADDR,
            RTL8390_LED_GLB_CTRL_LED_EN_OFFSET,
            RTL8390_LED_GLB_CTRL_LED_EN_MASK, 1);

    MEM32_WRITE(SWCORE_BASE_ADDR| RTL8390_LED_COPR_PMASK_CTRL_ADDR(0),  pModel->led.copr_pmask[0]);
    MEM32_WRITE(SWCORE_BASE_ADDR| RTL8390_LED_COPR_PMASK_CTRL_ADDR(32), pModel->led.copr_pmask[1]);
    MEM32_WRITE(SWCORE_BASE_ADDR| RTL8390_LED_FIB_PMASK_CTRL_ADDR(0),  pModel->led.fib_pmask[0]);
    MEM32_WRITE(SWCORE_BASE_ADDR| RTL8390_LED_FIB_PMASK_CTRL_ADDR(32), pModel->led.fib_pmask[1]);
    MEM32_WRITE(SWCORE_BASE_ADDR| RTL8390_LED_COMBO_CTRL_ADDR(0),  pModel->led.led_combo[0]);
    MEM32_WRITE(SWCORE_BASE_ADDR| RTL8390_LED_COMBO_CTRL_ADDR(32), pModel->led.led_combo[1]);

    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_SET_0_1_CTRL_ADDR, \
        RTL8390_LED_SET_0_1_CTRL_SET0_LED0_SEL_OFFSET,
        RTL8390_LED_SET_0_1_CTRL_SET0_LED0_SEL_MASK,
        pModel->led.led_definition_set[0].led[0]);
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_SET_0_1_CTRL_ADDR, \
        RTL8390_LED_SET_0_1_CTRL_SET0_LED1_SEL_OFFSET,
        RTL8390_LED_SET_0_1_CTRL_SET0_LED1_SEL_MASK,
        pModel->led.led_definition_set[0].led[1]);
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_SET_0_1_CTRL_ADDR, \
        RTL8390_LED_SET_0_1_CTRL_SET0_LED2_SEL_OFFSET,
        RTL8390_LED_SET_0_1_CTRL_SET0_LED2_SEL_MASK,
        pModel->led.led_definition_set[0].led[2]);
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_SET_0_1_CTRL_ADDR, \
        RTL8390_LED_SET_0_1_CTRL_SET1_LED0_SEL_OFFSET,
        RTL8390_LED_SET_0_1_CTRL_SET1_LED0_SEL_MASK,
        pModel->led.led_definition_set[1].led[0]);
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_SET_0_1_CTRL_ADDR, \
        RTL8390_LED_SET_0_1_CTRL_SET1_LED1_SEL_OFFSET,
        RTL8390_LED_SET_0_1_CTRL_SET1_LED1_SEL_MASK,
        pModel->led.led_definition_set[1].led[1]);
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_SET_0_1_CTRL_ADDR, \
        RTL8390_LED_SET_0_1_CTRL_SET1_LED2_SEL_OFFSET,
        RTL8390_LED_SET_0_1_CTRL_SET1_LED2_SEL_MASK,
        pModel->led.led_definition_set[1].led[2]);

    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_SET_2_3_CTRL_ADDR, \
        RTL8390_LED_SET_2_3_CTRL_SET2_LED0_SEL_OFFSET,
        RTL8390_LED_SET_2_3_CTRL_SET2_LED0_SEL_MASK,
        pModel->led.led_definition_set[2].led[0]);
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_SET_2_3_CTRL_ADDR, \
        RTL8390_LED_SET_2_3_CTRL_SET2_LED1_SEL_OFFSET,
        RTL8390_LED_SET_2_3_CTRL_SET2_LED1_SEL_MASK,
        pModel->led.led_definition_set[2].led[1]);
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_SET_2_3_CTRL_ADDR, \
        RTL8390_LED_SET_2_3_CTRL_SET2_LED2_SEL_OFFSET,
        RTL8390_LED_SET_2_3_CTRL_SET2_LED2_SEL_MASK,
        pModel->led.led_definition_set[2].led[2]);
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_SET_2_3_CTRL_ADDR, \
        RTL8390_LED_SET_2_3_CTRL_SET3_LED0_SEL_OFFSET,
        RTL8390_LED_SET_2_3_CTRL_SET3_LED0_SEL_MASK,
        pModel->led.led_definition_set[3].led[0]);
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_SET_2_3_CTRL_ADDR, \
        RTL8390_LED_SET_2_3_CTRL_SET3_LED1_SEL_OFFSET,
        RTL8390_LED_SET_2_3_CTRL_SET3_LED1_SEL_MASK,
        pModel->led.led_definition_set[3].led[1]);
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_SET_2_3_CTRL_ADDR, \
        RTL8390_LED_SET_2_3_CTRL_SET3_LED2_SEL_OFFSET,
        RTL8390_LED_SET_2_3_CTRL_SET3_LED2_SEL_MASK,
        pModel->led.led_definition_set[3].led[2]);

    for (idx=0; idx<pModel->port.count; idx++)
    {
        Tuint32 macid = pModel->port.list[idx].mac_id;
        Tuint32 value;

        //printf("[LED] port %u => mac_idx (%u)\n", idx, macid);

        value = ((pModel->led.led_copr_set_psel_bit1_pmask[(macid/32)] & (1<<(macid%32)))? 2:0) | \
                ((pModel->led.led_copr_set_psel_bit0_pmask[(macid/32)] & (1<<(macid%32)))? 1:0);
        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_COPR_SET_SEL_CTRL_ADDR(macid), \
            RTL8390_LED_COPR_SET_SEL_CTRL_LED_COPR_SET_PSEL_OFFSET(macid),
            RTL8390_LED_COPR_SET_SEL_CTRL_LED_COPR_SET_PSEL_MASK(macid),
            value);

        value = ((pModel->led.led_fib_set_psel_bit1_pmask[(macid/32)] & (1<<(macid%32)))? 2:0) | \
                ((pModel->led.led_fib_set_psel_bit0_pmask[(macid/32)] & (1<<(macid%32)))? 1:0);
        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_FIB_SET_SEL_CTRL_ADDR(macid), \
            RTL8390_LED_FIB_SET_SEL_CTRL_LED_FIB_SET_PSEL_OFFSET(macid),
            RTL8390_LED_FIB_SET_SEL_CTRL_LED_FIB_SET_PSEL_MASK(macid),
            value);
    }

    /* EEEP configuration */
    MEM32_WRITE(0xBB00047C, 0x1414FF15);
    MEM32_WRITE(0xBB000480, 0x1414FF0D);
    MEM32_WRITE(0xBB000488, 0x1414FF0B);

    return;
} /* end of rtl8390_mac_config_init */

/* Function Name:
 *      rtl8390_phy_config_init
 * Description:
 *      PHY Configuration code that connect with RTL8390
 * Input:
 *      pModel - pointer to switch model of platform
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
static void rtl8390_phy_config_init(const rtk_switch_model_t *pModel)
{
    unsigned int phy_idx;
    unsigned int port_id;

    /* Disable MAC polling PHY setting */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR | RTL8390_SMI_GLB_CTRL_ADDR, \
            RTL8390_SMI_GLB_CTRL_MDX_POLLING_EN_OFFSET,
            RTL8390_SMI_GLB_CTRL_MDX_POLLING_EN_MASK, 0x0);

    for (phy_idx=0; phy_idx<pModel->phy.count; phy_idx++)
    {
        Tuint8 phy0_macid = pModel->phy.list[phy_idx].mac_id;
        #if (defined(CONFIG_RTL8214FC))
        Tuint8 portNum = pModel->phy.list[phy_idx].phy_max;
        #endif

        //printf("[INFO] config PHY (%u) => mac_id = %u\n", phy_idx, phy0_macid);

        switch (pModel->phy.list[phy_idx].chip)
        {
            #if (defined(CONFIG_RTL8208))
            case RTK_CHIP_RTL8208D:
            case RTK_CHIP_RTL8208L:
                {
                    rtl8208_config(phy0_macid);
                }
                break;
            #endif
            #if (defined(CONFIG_RTL8218B))
            case RTK_CHIP_RTL8218B:
                {
                    rtl8218b_rtl8390_config(phy0_macid);
                }
                break;
            #endif
            #if (defined(CONFIG_RTL8218FB))
            case RTK_CHIP_RTL8218FB:
                {
                    rtl8218fb_rtl8390_config(phy0_macid);
                }
                break;
            #endif
            #if (defined(CONFIG_RTL8214FC))
            case RTK_CHIP_RTL8214FC:
                {
                    rtl8214fc_rtl8390_config(phy0_macid, portNum);
                }
                break;
            #endif
            #if (defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B) || defined(CONFIG_RTL8212B))
            case RTK_CHIP_RTL8214FB:
            case RTK_CHIP_RTL8214B:
            case RTK_CHIP_RTL8212B:
            {
                rtl8214fb_config(phy0_macid);
            }
            break;
            #endif
            default:
                break;
        }
    }

    /* Park Page to 0 */
    for (port_id=pModel->port.offset; port_id<(pModel->port.offset+pModel->port.count); port_id++)
    {
        int mac_id = pModel->port.list[port_id].mac_id;

        gMacDrv->drv_miim_write(mac_id, gMacDrv->miim_max_page, 31, 0);
    }

    rtl8390_phyPowerOn();

    OSAL_MDELAY(500);

    rtl8390_phyPowerOff();

    /* Restore MAC polling PHY setting */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR | RTL8390_SMI_GLB_CTRL_ADDR, \
            RTL8390_SMI_GLB_CTRL_MDX_POLLING_EN_OFFSET,
            RTL8390_SMI_GLB_CTRL_MDX_POLLING_EN_MASK, 0x1);

    return;
} /* end of rtl8390_phy_config_init */

confcode_serdes_patch_t rtl839x_eee_enable0[] = {
    { 0, 0, 14, 10, 0x8},
    { 0, 0,  9,  5, 0x10},
    { 0, 0,  4,  0, 0x10},
};

confcode_serdes_patch_t rtl839x_eee_enable1[] = {
    //#qsgmii_lpi_tx_en=1, qsgmii_lpi_rx_en=1
    { 0, 0,  9,  8, 0x3},
};

void rtl8390_serdes_eee_enable(const int sds_num)
{
    int eee_sds_addr0 = 0xa0e0;
    int eee_sds_addr1 = 0xa014;
    int eee_sds_addr_ofst = 0x400;
    int addr_ofst;
    int idx;

    addr_ofst = (eee_sds_addr_ofst * (sds_num / 2)) + (0x100 * (sds_num % 2));

    for (idx = 0; idx < (sizeof(rtl839x_eee_enable0)/sizeof(confcode_serdes_patch_t)); ++idx)
    {
        rtl839x_eee_enable0[idx].reg = eee_sds_addr0 + addr_ofst;
        SERDES_PATCH_SET_CHK(rtl839x_eee_enable0[idx]);
    }

    for (idx = 0; idx < (sizeof(rtl839x_eee_enable1)/sizeof(confcode_serdes_patch_t)); ++idx)
    {
        rtl839x_eee_enable1[idx].reg = eee_sds_addr1 + addr_ofst;
        SERDES_PATCH_SET_CHK(rtl839x_eee_enable1[idx]);
    }

    return ;
}   /* end of rtl8390_serdes_eee_enable */

static confcode_mac_regval_t *rtl839x_5G_serdesDB_qa[] =
{
    rtl839x_5G_serdes_0_qa,
    rtl839x_5G_serdes_1_qa,
    rtl839x_5G_serdes_2_qa,
    rtl839x_5G_serdes_3_qa,
    rtl839x_5G_serdes_4_qa,
    rtl839x_5G_serdes_5_qa,
    rtl839x_5G_serdes_6_qa,
    rtl839x_5G_serdes_7_qa,
    rtl839x_5G_serdes_8_9_qa,
    rtl839x_5G_serdes_8_9_qa,
    rtl839x_5G_serdes_10_qa,
    rtl839x_5G_serdes_11_qa,
    rtl839x_5G_serdes_12_qa,
    rtl839x_5G_serdes_12_qa,
};

static confcode_mac_regval_t *rtl839x_5G_serdesDB[] =
{
    rtl839x_5G_serdes_0,
    rtl839x_5G_serdes_1,
    rtl839x_5G_serdes_2,
    rtl839x_5G_serdes_3,
    rtl839x_5G_serdes_4,
    rtl839x_5G_serdes_5,
    rtl839x_5G_serdes_6,
    rtl839x_5G_serdes_7,
    rtl839x_5G_serdes_8_9,
    rtl839x_5G_serdes_8_9,
    rtl839x_5G_serdes_10,
    rtl839x_5G_serdes_11,
    rtl839x_5G_serdes_12,
};

int rtl8390_serdes_chk(const rtk_switch_model_t *pModel, const int sdsId)
{
    uint32  base = 0xbb00a078, chkPos;
    int     id;

    chkPos = base + (0x400 * (sdsId / 2)) + (0x100 * (sdsId % 2));

    if ((pModel->chip == RTK_CHIP_RTL8352M) || \
            (pModel->chip == RTK_CHIP_RTL8353M))
    {
        id = sdsId / 2;
    }
    else
        id = sdsId;

    if (MEM32_READ(chkPos) != 0x1ff0000)
    {
        OSAL_PRINTF("[WARN] Serdes %u initail fail %x %x\n", id, chkPos, MEM32_READ(chkPos));
        return 1;
    }
    else
    {
        //OSAL_PRINTF("Serdes %u initail OK\n", id);
        return 0;
    }

    return 0;
}

void
rtl8390_5G_serdes_config(const rtk_switch_model_t *pModel,
    const int sdsId, int phyIdx)
{
    confcode_mac_regval_t   *serdesConfig;
    int32                   try, idx;

    try = 1;
    do {
        if (RTK_CHIP_NONE == pModel->phy.list[phyIdx].chip)
        {
            #if 1
            MAC_REG_SET_CHK(0xbb78, 0x1c005);
            return;
            #else
            for (idx = 0; idx < (sizeof(rtl839x_fiber_serdes_12_13) / sizeof(confcode_mac_regval_t)); idx++)
            {
                MAC_REG_SET_CHK(rtl839x_fiber_serdes_12_13[idx].reg, rtl839x_fiber_serdes_12_13[idx].val);
            }
            #endif
        }
        else if (NULL != strstr(pModel->name, "_QA"))
        {
            if (10 == sdsId)
            {
                for (idx = 0; idx < (sizeof(rtl839x_serdes10_a2d_clk_edge_qa)/sizeof(confcode_mac_regval_t)); idx++)
                {
                    MAC_REG_SET_CHK(rtl839x_serdes10_a2d_clk_edge_qa[idx].reg, rtl839x_serdes10_a2d_clk_edge_qa[idx].val);
                }
            }
            else if (11 == sdsId)
            {
                for (idx = 0; idx < (sizeof(rtl839x_serdes11_a2d_clk_edge_qa)/sizeof(confcode_mac_regval_t)); idx++)
                {
                    MAC_REG_SET_CHK(rtl839x_serdes11_a2d_clk_edge_qa[idx].reg, rtl839x_serdes11_a2d_clk_edge_qa[idx].val);
                }
            }

            serdesConfig = rtl839x_5G_serdesDB_qa[sdsId];
            idx = 0;
            while (serdesConfig[idx].reg != 0)
            {
                MAC_REG_SET_CHK(serdesConfig[idx].reg, serdesConfig[idx].val);
                ++idx;
            }
        }
        else
        {
            if (10 == sdsId)
            {
                for (idx = 0; idx < (sizeof(rtl839x_serdes10_a2d_clk_edge)/sizeof(confcode_mac_regval_t)); idx++)
                {
                    MAC_REG_SET_CHK(rtl839x_serdes10_a2d_clk_edge[idx].reg, rtl839x_serdes10_a2d_clk_edge[idx].val);
                }
            }
            else if (11 == sdsId)
            {
                for (idx = 0; idx < (sizeof(rtl839x_serdes11_a2d_clk_edge)/sizeof(confcode_mac_regval_t)); idx++)
                {
                    MAC_REG_SET_CHK(rtl839x_serdes11_a2d_clk_edge[idx].reg, rtl839x_serdes11_a2d_clk_edge[idx].val);
                }
            }

            serdesConfig = rtl839x_5G_serdesDB[sdsId];
            idx = 0;
            while (serdesConfig[idx].reg != 0)
            {
                MAC_REG_SET_CHK(serdesConfig[idx].reg, serdesConfig[idx].val);
                ++idx;
            }
        }

        OSAL_MDELAY(500);

        if (0 == rtl8390_serdes_chk(pModel, sdsId))
            break;
    } while (1);

    return;
}   /* end of rtl8390_5G_serdes_config */

void rtl839x_serdes_rst(const uint32 sds_num)
{
    uint32 addr_ofst = 0x400;
    uint32 ofst;

    ofst = addr_ofst*(sds_num/2);
    //if {[expr int(fmod(sds_num,2))] == 1} {set ofst [expr addr_ofst*(sds_num/2) + 0x100]} {set ofst [expr addr_ofst*(sds_num/2)]}
    if (sds_num < 8 || sds_num == 10 || sds_num == 11) {
        // CMU RST
        //for 5G S0-S7, S10-11 CMU RST
        SERDES_SET(0xa3c0 + ofst,  31 , 16 , 0x0050);
        SERDES_SET(0xa3c0 + ofst,  31 , 16 , 0x00f0);
        SERDES_SET(0xa3c0 + ofst,  31 , 16 , 0x0);
    } else if (sds_num == 8 || sds_num == 9) {
        SERDES_SET(0xb3f8,  31 , 16 , 0x0005);
        SERDES_SET(0xb3f8,  31 , 16 , 0x000f);
        SERDES_SET(0xb3f8,  31 , 16 , 0x0);
    } else if (sds_num == 12 || sds_num == 13) {
        SERDES_SET(0xbbf8,  31 , 16 , 0x0005);
        SERDES_SET(0xbbf8,  31 , 16 , 0x000f);
        SERDES_SET(0xbbf8,  31 , 16 , 0x0);
    } else {
        printf( "sds number doesn't exist");
        return;
    }
    //digital soft reset
    SERDES_SET(0xa004 + ofst,  31 , 16 , 0x7146);
    SERDES_SET(0xa004 + ofst,  31 , 16 , 0x7106);
    SERDES_SET(0xa004 + ofst + 0x100,  31 , 16 , 0x7146);
    SERDES_SET(0xa004 + ofst + 0x100,  31 , 16 , 0x7106);
}   /* end of rtl839x_serdes_rst */

void rtl839x_serdes_patch_init(void)
{
    uint32 ofst_list[] = {0,0x800};
    uint32 ofst_list1[] = {0x80,0x880};
    uint32 ofst_list2[] = {0,0x80,0x400,0x480,0x800,0x880,0xc00,0xc80,0x1400,0x1480};
    //uint32  sdsList[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
    uint32  sdsId, i, ofst;

    //======= Change 10G reg value to same as defined at datasheet===================
    //S8 patch
    SERDES_SET(0xb300,  15 , 0  , 0x5800);
    SERDES_SET(0xb300,  31 , 16 , 0x4000);
    SERDES_SET(0xb304,  15 , 0  , 0x5400);
    SERDES_SET(0xb304,  31 , 16 , 0x0000);
    SERDES_SET(0xb308,  15 , 0  , 0x0000);
    SERDES_SET(0xb308,  31 , 16 , 0x4000);
    SERDES_SET(0xb30c,  15 , 0  , 0x4000);
    SERDES_SET(0xb30c,  31 , 16 , 0xffff);
    SERDES_SET(0xb310,  15 , 0  , 0xffff);
    SERDES_SET(0xb310,  31 , 16 , 0x806f);
    SERDES_SET(0xb314,  15 , 0  , 0x0004);
    SERDES_SET(0xb314,  31 , 16 , 0x0000);
    SERDES_SET(0xb318,  15 , 0  , 0x0000);
    SERDES_SET(0xb318,  31 , 16 , 0x0000);
    SERDES_SET(0xb31c,  15 , 0  , 0x0a00);
    SERDES_SET(0xb31c,  31 , 16 , 0x2000);
    SERDES_SET(0xb320,  15 , 0  , 0xf00e);
    SERDES_SET(0xb320,  31 , 16 , 0xf04a);
    SERDES_SET(0xb324,  15 , 0  , 0x97b3);
    SERDES_SET(0xb324,  31 , 16 , 0x5318);
    SERDES_SET(0xb328,  15 , 0  , 0x0f03);
    SERDES_SET(0xb328,  31 , 16 , 0x0);
    SERDES_SET(0xb32c,  15 , 0  , 0x0000);
    SERDES_SET(0xb32c,  31 , 16 , 0x0000);
    SERDES_SET(0xb330,  15 , 0  , 0x0000);
    SERDES_SET(0xb330,  31 , 16 , 0x0000);
    SERDES_SET(0xb334,  15 , 0  , 0xffff);
    SERDES_SET(0xb334,  31 , 16 , 0x0000);
    SERDES_SET(0xb338,  15 , 0  , 0x1203);
    SERDES_SET(0xb338,  31 , 16 , 0x0000);
    SERDES_SET(0xb33c,  15 , 0  , 0xa052);
    SERDES_SET(0xb33c,  31 , 16 , 0x9a00);
    SERDES_SET(0xb340,  15 , 0  , 0x00f5);
    SERDES_SET(0xb340,  31 , 16 , 0xf000);
    SERDES_SET(0xb344,  15 , 0  , 0x41ff);
    SERDES_SET(0xb344,  31 , 16 , 0x0000);
    SERDES_SET(0xb348,  15 , 0  , 0x39ff);
    SERDES_SET(0xb348,  31 , 16 , 0x3340);
    SERDES_SET(0xb34c,  15 , 0  , 0x40aa);
    SERDES_SET(0xb34c,  31 , 16 , 0x0000);
    SERDES_SET(0xb350,  15 , 0  , 0x801f);
    SERDES_SET(0xb350,  31 , 16 , 0x0000);
    SERDES_SET(0xb354,  15 , 0  , 0x619c);
    SERDES_SET(0xb354,  31 , 16 , 0xffed);
    SERDES_SET(0xb358,  15 , 0  , 0x29ff);
    SERDES_SET(0xb358,  31 , 16 , 0x29ff);
    SERDES_SET(0xb35c,  15 , 0  , 0x4e10);
    SERDES_SET(0xb35c,  31 , 16 , 0x4e10);
    SERDES_SET(0xb360,  15 , 0  , 0x0000);
    SERDES_SET(0xb360,  31 , 16 , 0x0000);
    //s9 patch
    SERDES_SET(0xb380,  15 , 0  , 0x5800);
    SERDES_SET(0xb380,  31 , 16 , 0x4000);
    SERDES_SET(0xb384,  15 , 0  , 0x5000);
    SERDES_SET(0xb384,  31 , 16 , 0x0000);
    SERDES_SET(0xb388,  15 , 0  , 0x0000);
    SERDES_SET(0xb388,  31 , 16 , 0x4000);
    SERDES_SET(0xb38c,  15 , 0  , 0x4000);
    SERDES_SET(0xb38c,  31 , 16 , 0xffff);
    SERDES_SET(0xb390,  15 , 0  , 0xffff);
    SERDES_SET(0xb390,  31 , 16 , 0x806f);
    SERDES_SET(0xb394,  15 , 0  , 0x0004);
    SERDES_SET(0xb394,  31 , 16 , 0x0000);
    SERDES_SET(0xb398,  15 , 0  , 0x0000);
    SERDES_SET(0xb398,  31 , 16 , 0x0000);
    SERDES_SET(0xb39c,  15 , 0  , 0x0a00);
    SERDES_SET(0xb39c,  31 , 16 , 0x2000);
    SERDES_SET(0xb3a0,  15 , 0  , 0xf00e);
    SERDES_SET(0xb3a0,  31 , 16 , 0xfdab);
    SERDES_SET(0xb3a4,  15 , 0  , 0x96ea);
    SERDES_SET(0xb3a4,  31 , 16 , 0x5318);
    SERDES_SET(0xb3a8,  15 , 0  , 0x0f03);
    SERDES_SET(0xb3a8,  31 , 16 , 0);
    SERDES_SET(0xb3ac,  15 , 0  , 0x0000);
    SERDES_SET(0xb3ac,  31 , 16 , 0x0000);
    SERDES_SET(0xb3b0,  15 , 0  , 0x0000);
    SERDES_SET(0xb3b0,  31 , 16 , 0x0000);
    SERDES_SET(0xb3b4,  15 , 0  , 0xffff);
    SERDES_SET(0xb3b4,  31 , 16 , 0x0000);
    SERDES_SET(0xb3b8,  15 , 0  , 0x1203);
    SERDES_SET(0xb3b8,  31 , 16 , 0x0000);
    SERDES_SET(0xb3bc,  15 , 0  , 0xa052);
    SERDES_SET(0xb3bc,  31 , 16 , 0x9a00);
    SERDES_SET(0xb3c0,  15 , 0  , 0x00f5);
    SERDES_SET(0xb3c0,  31 , 16 , 0xf000);
    SERDES_SET(0xb3c4,  15 , 0  , 0x4079);
    SERDES_SET(0xb3c4,  31 , 16 , 0x0000);
    SERDES_SET(0xb3c8,  15 , 0  , 0x93fa);
    SERDES_SET(0xb3c8,  31 , 16 , 0x3340);
    SERDES_SET(0xb3cc,  15 , 0  , 0x4280);
    SERDES_SET(0xb3cc,  31 , 16 , 0x0000);
    SERDES_SET(0xb3d0,  15 , 0  , 0x801f);
    SERDES_SET(0xb3d0,  31 , 16 , 0x0000);
    SERDES_SET(0xb3d4,  15 , 0  , 0x619c);
    SERDES_SET(0xb3d4,  31 , 16 , 0xffed);
    SERDES_SET(0xb3d8,  15 , 0  , 0x29ff);
    SERDES_SET(0xb3d8,  31 , 16 , 0x29ff);
    SERDES_SET(0xb3dc,  15 , 0  , 0x4c50);
    SERDES_SET(0xb3dc,  31 , 16 , 0x4c50);
    SERDES_SET(0xb3e0,  15 , 0  , 0x0000);
    SERDES_SET(0xb3e0,  31 , 16 , 0x0000);
    //S12 patch
    SERDES_SET(0xbb00,  15 , 0  , 0x5800);
    SERDES_SET(0xbb00,  31 , 16 , 0x4000);
    SERDES_SET(0xbb04,  15 , 0  , 0x5400);
    SERDES_SET(0xbb04,  31 , 16 , 0x0000);
    SERDES_SET(0xbb08,  15 , 0  , 0x0000);
    SERDES_SET(0xbb08,  31 , 16 , 0x4000);
    SERDES_SET(0xbb0c,  15 , 0  , 0x4000);
    SERDES_SET(0xbb0c,  31 , 16 , 0xffff);
    SERDES_SET(0xbb10,  15 , 0  , 0xffff);
    SERDES_SET(0xbb10,  31 , 16 , 0x806f);
    SERDES_SET(0xbb14,  15 , 0  , 0x0004);
    SERDES_SET(0xbb14,  31 , 16 , 0x0000);
    SERDES_SET(0xbb18,  15 , 0  , 0x0000);
    SERDES_SET(0xbb18,  31 , 16 , 0x0000);
    SERDES_SET(0xbb1c,  15 , 0  , 0x0a00);
    SERDES_SET(0xbb1c,  31 , 16 , 0x2000);
    SERDES_SET(0xbb20,  15 , 0  , 0xf00e);
    SERDES_SET(0xbb20,  31 , 16 , 0xf04a);
    SERDES_SET(0xbb24,  15 , 0  , 0x97b3);
    SERDES_SET(0xbb24,  31 , 16 , 0x5318);
    SERDES_SET(0xbb28,  15 , 0  , 0x0f03);
    SERDES_SET(0xbb28,  31 , 16 , 0x0);
    SERDES_SET(0xbb2c,  15 , 0  , 0x0000);
    SERDES_SET(0xbb2c,  31 , 16 , 0x0000);
    SERDES_SET(0xbb30,  15 , 0  , 0x0000);
    SERDES_SET(0xbb30,  31 , 16 , 0x0000);
    SERDES_SET(0xbb34,  15 , 0  , 0xffff);
    SERDES_SET(0xbb34,  31 , 16 , 0x0000);
    SERDES_SET(0xbb38,  15 , 0  , 0x1203);
    SERDES_SET(0xbb38,  31 , 16 , 0x0000);
    SERDES_SET(0xbb3c,  15 , 0  , 0xa052);
    SERDES_SET(0xbb3c,  31 , 16 , 0x9a00);
    SERDES_SET(0xbb40,  15 , 0  , 0x00f5);
    SERDES_SET(0xbb40,  31 , 16 , 0xf000);
    SERDES_SET(0xbb44,  15 , 0  , 0x41ff);
    SERDES_SET(0xbb44,  31 , 16 , 0x0000);
    SERDES_SET(0xbb48,  15 , 0  , 0x39ff);
    SERDES_SET(0xbb48,  31 , 16 , 0x3340);
    SERDES_SET(0xbb4c,  15 , 0  , 0x40aa);
    SERDES_SET(0xbb4c,  31 , 16 , 0x0000);
    SERDES_SET(0xbb50,  15 , 0  , 0x801f);
    SERDES_SET(0xbb50,  31 , 16 , 0x0000);
    SERDES_SET(0xbb54,  15 , 0  , 0x619c);
    SERDES_SET(0xbb54,  31 , 16 , 0xffed);
    SERDES_SET(0xbb58,  15 , 0  , 0x29ff);
    SERDES_SET(0xbb58,  31 , 16 , 0x29ff);
    SERDES_SET(0xbb5c,  15 , 0  , 0x4e10);
    SERDES_SET(0xbb5c,  31 , 16 , 0x4e10);
    SERDES_SET(0xbb60,  15 , 0  , 0x0000);
    SERDES_SET(0xbb60,  31 , 16 , 0x0000);
    //s13 patch
    SERDES_SET(0xbb80,  15 , 0  , 0x5800);
    SERDES_SET(0xbb80,  31 , 16 , 0x4000);
    SERDES_SET(0xbb84,  15 , 0  , 0x5000);
    SERDES_SET(0xbb84,  31 , 16 , 0x0000);
    SERDES_SET(0xbb88,  15 , 0  , 0x0000);
    SERDES_SET(0xbb88,  31 , 16 , 0x4000);
    SERDES_SET(0xbb8c,  15 , 0  , 0x4000);
    SERDES_SET(0xbb8c,  31 , 16 , 0xffff);
    SERDES_SET(0xbb90,  15 , 0  , 0xffff);
    SERDES_SET(0xbb90,  31 , 16 , 0x806f);
    SERDES_SET(0xbb94,  15 , 0  , 0x0004);
    SERDES_SET(0xbb94,  31 , 16 , 0x0000);
    SERDES_SET(0xbb98,  15 , 0  , 0x0000);
    SERDES_SET(0xbb98,  31 , 16 , 0x0000);
    SERDES_SET(0xbb9c,  15 , 0  , 0x0a00);
    SERDES_SET(0xbb9c,  31 , 16 , 0x2000);
    SERDES_SET(0xbba0,  15 , 0  , 0xf00e);
    SERDES_SET(0xbba0,  31 , 16 , 0xfdab);
    SERDES_SET(0xbba4,  15 , 0  , 0x96ea);
    SERDES_SET(0xbba4,  31 , 16 , 0x5318);
    SERDES_SET(0xbba8,  15 , 0  , 0x0f03);
    SERDES_SET(0xbba8,  31 , 16 , 0);
    SERDES_SET(0xbbac,  15 , 0  , 0x0000);
    SERDES_SET(0xbbac,  31 , 16 , 0x0000);
    SERDES_SET(0xbbb0,  15 , 0  , 0x0000);
    SERDES_SET(0xbbb0,  31 , 16 , 0x0000);
    SERDES_SET(0xbbb4,  15 , 0  , 0xffff);
    SERDES_SET(0xbbb4,  31 , 16 , 0x0000);
    SERDES_SET(0xbbb8,  15 , 0  , 0x1203);
    SERDES_SET(0xbbb8,  31 , 16 , 0x0000);
    SERDES_SET(0xbbbc,  15 , 0  , 0xa052);
    SERDES_SET(0xbbbc,  31 , 16 , 0x9a00);
    SERDES_SET(0xbbc0,  15 , 0  , 0x00f5);
    SERDES_SET(0xbbc0,  31 , 16 , 0xf000);
    SERDES_SET(0xbbc4,  15 , 0  , 0x4079);
    SERDES_SET(0xbbc4,  31 , 16 , 0x0000);
    SERDES_SET(0xbbc8,  15 , 0  , 0x93fa);
    SERDES_SET(0xbbc8,  31 , 16 , 0x3340);
    SERDES_SET(0xbbcc,  15 , 0  , 0x4280);
    SERDES_SET(0xbbcc,  31 , 16 , 0x0000);
    SERDES_SET(0xbbd0,  15 , 0  , 0x801f);
    SERDES_SET(0xbbd0,  31 , 16 , 0x0000);
    SERDES_SET(0xbbd4,  15 , 0  , 0x619c);
    SERDES_SET(0xbbd4,  31 , 16 , 0xffed);
    SERDES_SET(0xbbd8,  15 , 0  , 0x29ff);
    SERDES_SET(0xbbd8,  31 , 16 , 0x29ff);
    SERDES_SET(0xbbdc,  15 , 0  , 0x4c50);
    SERDES_SET(0xbbdc,  31 , 16 , 0x4c50);
    SERDES_SET(0xbbe0,  15 , 0  , 0x0000);
    SERDES_SET(0xbbe0,  31 , 16 , 0x0000);
    //===================== SERDES DIGITAL PATCH ===================================
    //10G to 40b mode
    SERDES_SET(0xb018,  15 , 0  , 0x08ec);
    SERDES_SET(0xb118,  15 , 0  , 0x08ec);
    SERDES_SET(0xb818,  15 , 0  , 0x08ec);
    SERDES_SET(0xb918,  15 , 0  , 0x08ec);
    //force 10G spd = 11
    SERDES_SET(0xb3fc,  31 , 16 , 0x3f);
    SERDES_SET(0xbbfc,  31 , 16 , 0x3f);
    //change S8-S12 clock edge
    SERDES_SET(0xb00c,  30 , 30 , 1);
    SERDES_SET(0xb10c,  30 , 30 , 1);
    SERDES_SET(0xb40c,  30 , 30 , 1);
    SERDES_SET(0xb50c,  30 , 30 , 1);
    SERDES_SET(0xb80c,  30 , 30 , 1);
    SERDES_SET(0xb90c,  30 , 30 , 1);
    //===============================================================================
    //=====================10G(S8,S12) SERDES ANALOG PATCH====================
    for (i = 0; i < sizeof(ofst_list)/sizeof(uint32); ++i)
    {
        ofst = ofst_list[i];
        //reg_bypass = 10
        SERDES_SET(0xb350 + ofst,  31 , 16 , 0x417f);
        //KD = 0, KP1_1 = 2, KP1 = 3b'101 , KP2 = 0, KI= 0
        SERDES_SET(0xb338 + ofst,  9  , 9  , 0);
        SERDES_SET(0xb338 + ofst,  12 , 10 , 0x0);
        SERDES_SET(0xb338 + ofst,  5  , 3  , 0x5);
        SERDES_SET(0xb338 + ofst,  8  , 6  , 0x0);
        SERDES_SET(0xb338 + ofst,  2  , 0  , 0x2);
        //LEQ
        SERDES_SET(0xb340 + ofst,  31 , 16 , 0xc440);
        //IB
        SERDES_SET(0xb34c + ofst,  3  , 3  , 0);
        //Change para to same as 0389B
        SERDES_SET(0xb308 + ofst,  31 , 16 , 0x8000);
        SERDES_SET(0xb30c + ofst,  15 , 0  , 0x8000);
        SERDES_SET(0xb314 + ofst,  15 , 0  , 0x0);
        SERDES_SET(0xb33c + ofst,  15 , 0  , 0x2);
        SERDES_SET(0xb33c + ofst,  31 , 16 , 0xbe00);
        //2013/3/22 03:44と, ICP_LBW=0, R_LBW=0
        SERDES_SET(0xb35c + ofst,  10 , 10 , 0);
        SERDES_SET(0xb35c + ofst,  26 , 26 , 0);
        SERDES_SET(0xb35c + ofst,  14 , 14 , 0);
        SERDES_SET(0xb35c + ofst,  30 , 30 , 0);
        //disable OOBS Jim: 5/8
        SERDES_SET(0xb320 + ofst,  5  , 5  , 0);
        //w32b [expr 0xb31c + ofst] 11-7 0x16
        //disable DFE
        SERDES_SET(0xb350 + ofst,  24 , 24 , 0);
        //
        //REG_DFE_CALI_INIT<3:0>
        SERDES_SET(0xb304 + ofst,  31 , 28 , 0xf);
        //0xB33C/0xBB3C [29:28] REG_DFE_IVGA_ADJ[1:0] 0xB33C/0xBB3C[27:25] REG_DFE_AFC_VRTL[2:0]
        SERDES_SET(0xb33c + ofst,  29 , 28 , 0x3);
        SERDES_SET(0xb33c + ofst,  27 , 25 , 0x7);
        //10G LEQ, 31-28= Bypass LEQ[1:4], 0=enable, 27-25=LEQ_BOOST1, BOOST2, BOOST 3, BOOST4
        SERDES_SET(0xb340 + ofst,  31 , 31 , 1);
        SERDES_SET(0xb340 + ofst,  30 , 30 , 1);
        SERDES_SET(0xb340 + ofst,  29 , 29 , 0);
        SERDES_SET(0xb340 + ofst,  28 , 28 , 0);
        SERDES_SET(0xb340 + ofst,  27 , 25 , 0x2);
        SERDES_SET(0xb340 + ofst,  24 , 22 , 0x2);
        SERDES_SET(0xb340 + ofst,  21 , 19 , 0x0);
        SERDES_SET(0xb340 + ofst,  18 , 16 , 0x0);
        //Tx Emphasis
        SERDES_SET(0xb358 + ofst,  9  , 9  , 1);
        SERDES_SET(0xb358 + ofst,  25 , 25 , 1);
        SERDES_SET(0xb350 + ofst,  5  , 5  , 1);
        SERDES_SET(0xb350 + ofst,  6  , 6  , 0);
        //reg_clkla_en = 0
        SERDES_SET(0xb338 + ofst,  15 , 15 , 0);
        //disable DC calibration circuit
        SERDES_SET(0xb320 + ofst,  15 , 12 , 0x0);
        //disable empedance auto-calibration, force R=8
        SERDES_SET(0xb324 + ofst,  20 , 20 , 0);
        SERDES_SET(0xb324 + ofst,  25 , 25 , 0);
        SERDES_SET(0xb324 + ofst,  19 , 16 , 0x8);
        SERDES_SET(0xb324 + ofst,  24 , 21 , 0x8);
    }
    //=====================10G(S9,S13) SERDES ANALOG PATCH====================
    for (i = 0; i < sizeof(ofst_list1)/sizeof(uint32); ++i)
    {
        ofst = ofst_list1[i];
        //reg_bypass = 10
        SERDES_SET(0xb350 + ofst,  31 , 16 , 0x417f);
        //KD = 0, KP1_1 = 2, KP1 = 3b'101 , KP2 = 0, KI= 0
        SERDES_SET(0xb338 + ofst,  9  , 9  , 0);
        SERDES_SET(0xb338 + ofst,  12 , 10 , 0x0);
        SERDES_SET(0xb338 + ofst,  5  , 3  , 0x5);
        SERDES_SET(0xb338 + ofst,  8  , 6  , 0x0);
        SERDES_SET(0xb338 + ofst,  2  , 0  , 0x2);
        //LEQ
        SERDES_SET(0xb340 + ofst,  31 , 16 , 0xc440);
        //IB
        //w32b [expr 0xb34c + ofst] 3 0
        //Change para to same as 0389B
        SERDES_SET(0xb308 + ofst,  31 , 16 , 0x8000);
        SERDES_SET(0xb30c + ofst,  15 , 0  , 0x8000);
        SERDES_SET(0xb314 + ofst,  15 , 0  , 0x0);
        SERDES_SET(0xb33c + ofst,  15 , 0  , 0x2);
        SERDES_SET(0xb33c + ofst,  31 , 16 , 0xbe00);
        //2013/3/22 03:44と, ICP_LBW=0, R_LBW=0
        //w32b [expr 0xb35c  + ofst] 10 0 26 0 14 0 30 0
        //disable OOBS Jim: 5/8
        SERDES_SET(0xb320 + ofst,  5  , 5  , 0);
        //w32b [expr 0xb31c + ofst] 11-7 0x16
        //disable DFE
        SERDES_SET(0xb350 + ofst,  24 , 24 , 0);
        //REG_DFE_CALI_INIT<3:0>
        SERDES_SET(0xb304 + ofst,  31 , 28 , 0xf);
        //0xB33C/0xBB3C [29:28] REG_DFE_IVGA_ADJ[1:0] 0xB33C/0xBB3C[27:25] REG_DFE_AFC_VRTL[2:0]
        SERDES_SET(0xb33c + ofst,  29 , 28 , 0x3);
        SERDES_SET(0xb33c + ofst,  27 , 25 , 0x7);
        //10G LEQ, 31-28= Bypass LEQ[1:4], 0=enable, 27-25=LEQ_BOOST1, BOOST2, BOOST 3, BOOST4
        SERDES_SET(0xb340 + ofst,  31 , 31 , 1);
        SERDES_SET(0xb340 + ofst,  30 , 30 , 1);
        SERDES_SET(0xb340 + ofst,  29 , 29 , 0);
        SERDES_SET(0xb340 + ofst,  28 , 28 , 0);
        SERDES_SET(0xb340 + ofst,  27 , 25 , 0x2);
        SERDES_SET(0xb340 + ofst,  24 , 22 , 0x2);
        SERDES_SET(0xb340 + ofst,  21 , 19 , 0x0);
        SERDES_SET(0xb340 + ofst,  18 , 16 , 0x0);
        //Tx Emphasis
        SERDES_SET(0xb358 + ofst,  9  , 9  , 1);
        SERDES_SET(0xb358 + ofst,  25 , 25 , 1);
        SERDES_SET(0xb350 + ofst,  5  , 5  , 1);
        SERDES_SET(0xb350 + ofst,  6  , 6  , 0);
        //reg_clkla_en = 0
        SERDES_SET(0xb338 + ofst,  15 , 15 , 0);
        //disable DC calibration circuit
        SERDES_SET(0xb320 + ofst,  15 , 12 , 0x0);
        //disable empedance auto-calibration, force R=8
        SERDES_SET(0xb324 + ofst,  20 , 20 , 0);
        SERDES_SET(0xb324 + ofst,  25 , 25 , 0);
        SERDES_SET(0xb324 + ofst,  19 , 16 , 0x8);
        SERDES_SET(0xb324 + ofst,  24 , 21 , 0x8);
    }
    //============================================================================

    //=====================5G(S0-S7,S10,S11) SERDES ANALOG PATCH==================
    //S4 and S10
    //Bit0 Should be 0 for S4S5 and S10S11, FULL Swing
    SERDES_SET(0xab10,  15 , 0  , 0x8c6a);
    SERDES_SET(0xb710,  15 , 0  , 0x8c6a);

    for (i = 0; i < sizeof(ofst_list2)/sizeof(uint32); ++i)
    {
        ofst = ofst_list2[i];
        //RX_IQDSEL=0
        //w32b [expr 0xa328 + ofst] 23 0
        //KD,KI, KP1, KP2
        //KP1 = 2(0x2), KP2 = 1(0x3)
        SERDES_SET(0xa320 + ofst,  31 , 31 , 0);
        SERDES_SET(0xa320 + ofst,  30 , 28 , 0x1);
        SERDES_SET(0xa320 + ofst,  27 , 25 , 0x2);
        SERDES_SET(0xa320 + ofst,  24 , 22 , 0x3);
        //OFFSET_DIG_EN = 0
        SERDES_SET(0xa32c + ofst,  15 , 15 , 0);
        //CMU_LCVCO_IBX_SEL = 0
        SERDES_SET(0xa310 + ofst,  3  , 3  , 0);
        //REG_BG = 00
        SERDES_SET(0xa30c + ofst,  25 , 25 , 0);
        SERDES_SET(0xa30c + ofst,  24 , 24 , 0);
        //disable auto-k
        //w32b [expr 0xa308 + ofst] 13 0
        //force band
        //w32b [expr 0xa304 + ofst] 15-11 0x14
        //force RX_EQIN = 1100
        //force RX_EQ = 1100(Binary = 1000
        SERDES_SET(0xa328 + ofst,  1  , 1  , 1);
        //RX_EQ = 1100
        SERDES_SET(0xa328 + ofst,  31 , 28 , 0xc);
        //REG_RX_PS_AFE, AFE is always ON
        SERDES_SET(0xa32c + ofst,  12 , 12 , 0);
        //Tx Emphasis
        SERDES_SET(0xa330 + ofst,  5  , 0  , 0x6);
        //disable Tx empedance AUTO-K, force R=0x8
        SERDES_SET(0xa310 + ofst,  6  , 6  , 0);
        SERDES_SET(0xa310 + ofst,  11 , 11 , 0);
        SERDES_SET(0xa310 + ofst,  15 , 12 , 0x8);
        SERDES_SET(0xa310 + ofst,  10 , 7  , 0x8);
    }

    for (sdsId = 0; sdsId < MAX_SERDES; ++sdsId)
        rtl839x_serdes_rst(sdsId);
    //===============================================================================
}   /* end of rtl839x_serdes_patch_init */

//for RTL8353M serdes patch
void rtl839x_5x_serdes_patch_init(void)
{
    //force digital to 20b
    SERDES_SET(0xb018,  15 , 0  , 0x08e4);
    SERDES_SET(0xb118,  15 , 0  , 0x08e4);

    //force spd =10
    SERDES_SET(0xb3fc,  31 , 16 , 0x2b);
    //w32b 0xbbfc 31-16 0x2b

    //2013/5/8, reg_clkla_en=0, Change KI/KD/KP1/KP2/KP1_1 to 0389b
    SERDES_SET(0xb338,  15 , 0  , 0x0722);
    SERDES_SET(0xb3b8,  15 , 0  , 0x0722);

    //REG_PI2_M_MODE_1 = 1, REG_PI_M_MODE_1 = 1
    SERDES_SET(0xb340,  15 , 0  , 0x18f5);
    SERDES_SET(0xb3c0,  15 , 0  , 0x18f5);

    //2013/3/25, power down unused analog S1,S3,S5,S7,S9,S11
    //S1,S3,S5,S7,S11: frc_rx=0,frc_pdown=1,frc_cmu=0,frc_no_sds=1
    SERDES_SET(0xa340, 31, 0, 0xc400043f);
    SERDES_SET(0xa3c0, 31, 0, 0xc40043f);
    SERDES_SET(0xa740, 31, 0, 0xc400043f);
    SERDES_SET(0xa7c0, 31, 0, 0xc40043f);
    SERDES_SET(0xab40, 31, 0, 0xc400043f);
    SERDES_SET(0xabc0, 31, 0, 0xc40043f);
    SERDES_SET(0xaf40, 31, 0, 0xc400043f);
    SERDES_SET(0xafc0, 31, 0, 0xc40043f);
    SERDES_SET(0xb740, 31, 0, 0xc400043f);
    SERDES_SET(0xb7c0, 31, 0, 0xc40043f);
    //S9: frc_pdown=1,frc_rx=0; 10G-serdes CMU⊿縒ミ秨闽,2甅ノCMU
    SERDES_SET(0xb3f8, 31, 0, 0x4c00000);
}   /* end of rtl839x_5x_serdes_patch_init */

void rtl839x_serdes_cmu(uint32 enable, uint32 sds_num)
{
    uint32 addr5g;
    uint32 addr10g;
    uint32 val;
    uint32 addr_ofst = 0x400;
    uint32 ofst;

    ofst = addr_ofst*(sds_num/2);
    if (sds_num % 2 == 0) {
        addr5g = 20;
        addr10g = 16;
    } else {
        addr5g = 22;
        addr10g = 18;
    }

    if (enable == 1) {
        val = 1;
    } else {
        val = 0;
    }

    if (sds_num < 8 || sds_num == 10 || sds_num == 11) {
        // CMU RST
        //for 5G S0-S7, S10-11 CMU
        SERDES_SET(0xa3c0 + ofst,  addr5g, addr5g, 1);
        SERDES_SET(0xa3c0 + ofst,  (addr5g + 1), (addr5g + 1), val);
    } else if (sds_num == 8 || sds_num == 9) {
        SERDES_SET(0xb3f8,  addr10g, addr10g, 1);
        SERDES_SET(0xb3f8,  (addr10g + 1), (addr10g + 1), val);
    } else if (sds_num == 12 || sds_num == 13) {
        SERDES_SET(0xbbf8,  addr10g, addr10g, 1);
        SERDES_SET(0xbbf8,  (addr10g + 1), (addr10g + 1), val);
    } else {
        printf( "sds number doesn't exist");
        return;
    }
}

void rtl839x_93m_rst_sys (void)
{
    rtl8390_drv_macPhyPatch1();

    //1.  analog serdes 把计
    //2. SW_SERDES_RST = 1 (auto clear to 0)
    //3. э digital serdes register (р digital serdes 璶 patch 秈)

    //SW_SERDES_RST=1 (auto clear to 0)
    SERDES_SET(0x0014,  4  , 4  , 1);

    OSAL_MDELAY(500);
    //==Patch digital serdes registers==
    SERDES_SET(0xb018,  15 , 0  , 0x08ec);
    SERDES_SET(0xb118,  15 , 0  , 0x08ec);
    SERDES_SET(0xb818,  15 , 0  , 0x08ec);
    SERDES_SET(0xb918,  15 , 0  , 0x08ec);
    //force 10G spd = 11
    SERDES_SET(0xb3fc,  31 , 16 , 0x3f);
    SERDES_SET(0xbbfc,  31 , 16 , 0x3f);
    //change S8-S12 clock edge
    SERDES_SET(0xb00c,  30 , 30 , 1);
    SERDES_SET(0xb10c,  30 , 30 , 1);
    SERDES_SET(0xb40c,  30 , 30 , 1);
    SERDES_SET(0xb50c,  30 , 30 , 1);
    SERDES_SET(0xb80c,  30 , 30 , 1);
    SERDES_SET(0xb90c,  30 , 30 , 1);

    rtl8390_drv_macPhyPatch2();
}

void rtl839x_53m_rst_sys (void)
{
    uint32 sdsList[] = {1, 3, 5, 7, 11};
    uint32 i;

    rtl8390_drv_macPhyPatch1();

    //1.  analog serdes 把计
    //2. SW_SERDES_RST = 1 (auto clear to 0)
    //3. э digital serdes register (р digital serdes 璶 patch 秈)

    //SW_SERDES_RST=1 (auto clear to 0)
    SERDES_SET(0x0014,  4  , 4  , 1);

    OSAL_MDELAY(500);
    //==Patch digital serdes registers==
    SERDES_SET(0xb018,  15 , 0  , 0x08ec);
    SERDES_SET(0xb118,  15 , 0  , 0x08ec);
    SERDES_SET(0xb818,  15 , 0  , 0x08ec);
    SERDES_SET(0xb918,  15 , 0  , 0x08ec);
    //force 10G spd = 11
    SERDES_SET(0xb3fc,  31 , 16 , 0x3f);
    SERDES_SET(0xbbfc,  31 , 16 , 0x3f);
    //change S8-S12 clock edge
    SERDES_SET(0xb00c,  30 , 30 , 1);
    SERDES_SET(0xb10c,  30 , 30 , 1);
    SERDES_SET(0xb40c,  30 , 30 , 1);
    SERDES_SET(0xb50c,  30 , 30 , 1);
    SERDES_SET(0xb80c,  30 , 30 , 1);
    SERDES_SET(0xb90c,  30 , 30 , 1);

    SERDES_SET(0xb018,  15 , 0  , 0x08e4);
    SERDES_SET(0xb118,  15 , 0  , 0x08e4);
    //force spd =10
    SERDES_SET(0xb3fc,  21 , 16 , 0x2b);

    //disable serdes cmu, S1,3,5,7,11

    for (i = 0; i < sizeof(sdsList)/sizeof(uint32); ++i)
        rtl839x_serdes_cmu(0, sdsList[i]);

    //disable s9/s13 tx&rx
    //pdown
    SERDES_SET(0xb3f8,  23 , 23 , 1);
    SERDES_SET(0xb3f8,  22 , 22 , 1);
    SERDES_SET(0xb3f8,  26 , 26 , 1);
    SERDES_SET(0xb3f8,  27 , 27 , 0);
    SERDES_SET(0xbbf8,  23 , 23 , 1);
    SERDES_SET(0xbbf8,  22 , 22 , 1);
    SERDES_SET(0xbbf8,  26 , 26 , 1);
    SERDES_SET(0xbbf8,  27 , 27 , 0);

    rtl8390_drv_macPhyPatch2();
}

/* Function Name:
 *      rtl8390_serdes_config_init
 * Description:
 *      Serdes Configuration code that connect with RTL8390
 * Input:
 *      pModel - pointer to switch model of platform
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
static void rtl8390_serdes_config_init(const rtk_switch_model_t *pModel)
{
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR | RTL8390_CHIP_INFO_ADDR, \
        RTL8390_CHIP_INFO_CHIP_INFO_EN_OFFSET, RTL8390_CHIP_INFO_CHIP_INFO_EN_MASK, 0xA);

    if ((MEM32_READ(SWCORE_BASE_ADDR | RTL8390_CHIP_INFO_ADDR) & 0xffff) == 0x0399)
    {
        DBG_PRINT(1, "### rtl8390_serdes_config_init: Set CFG_NEG_CLKWR_A2D to 1 of Serdes 8~13 ###\n");
#if 0   /* the width of register is over 32bits, need to fix it */
        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR | RTL8390_SDS8_9_XSG0_ADDR, \
            RTL8390_SDS8_9_XSG0_SR7_CFG_NEG_CLKWR_A2D_OFFSET, RTL8390_SDS8_9_XSG0_SR7_CFG_NEG_CLKWR_A2D_MASK, 1);
        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR | RTL8390_SDS8_9_XSG1_ADDR, \
            RTL8390_SDS8_9_XSG1_SR7_CFG_NEG_CLKWR_A2D_OFFSET, RTL8390_SDS8_9_XSG1_SR7_CFG_NEG_CLKWR_A2D_MASK, 1);
        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR | RTL8390_SDS10_11_XSG0_ADDR, \
            RTL8390_SDS10_11_XSG0_SR7_CFG_NEG_CLKWR_A2D_OFFSET, RTL8390_SDS8_9_XSG0_SR7_CFG_NEG_CLKWR_A2D_MASK, 1);
        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR | RTL8390_SDS10_11_XSG1_ADDR, \
            RTL8390_SDS10_11_XSG1_SR7_CFG_NEG_CLKWR_A2D_OFFSET, RTL8390_SDS8_9_XSG1_SR7_CFG_NEG_CLKWR_A2D_MASK, 1);
        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR | RTL8390_SDS12_13_XSG0_ADDR, \
            RTL8390_SDS12_13_XSG0_SR7_CFG_NEG_CLKWR_A2D_OFFSET, RTL8390_SDS12_13_XSG0_SR7_CFG_NEG_CLKWR_A2D_MASK, 1);
        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR | RTL8390_SDS12_13_XSG1_ADDR, \
            RTL8390_SDS12_13_XSG1_SR7_CFG_NEG_CLKWR_A2D_OFFSET, RTL8390_SDS12_13_XSG1_SR7_CFG_NEG_CLKWR_A2D_MASK, 1);
#endif

        if ((pModel->chip == RTK_CHIP_RTL8352M) || \
            (pModel->chip == RTK_CHIP_RTL8353M))
        {
            uint32 sds;
            uint32 idx, i;
            uint32 try;

            /* serdes_PWR_save */
            for (i=0; i<=6; i++)
            {
                for (idx=0; idx<(sizeof(rtl835x_mac_serdes_pwr_save)/sizeof(confcode_mac_regval_t)); idx++)
                {
                    MAC_REG_SET_CHK(rtl835x_mac_serdes_pwr_save[idx].reg + (i * 0x400), \
                                    rtl835x_mac_serdes_pwr_save[idx].val);
                }
            }

            /* Serdes Reset: ew 0xbb000014=0x00000010 */
            MEM32_WRITE(SWCORE_BASE_ADDR | RTL8390_RST_GLB_CTRL_ADDR, 0x00000010);

            // Disable 25M clock output
            for (idx=0; idx<(sizeof(rtl835x_mac_dis_25m_sdsck_out)/sizeof(confcode_mac_regval_t)); idx++)
            {
                MAC_REG_SET_CHK(rtl835x_mac_dis_25m_sdsck_out[idx].reg, \
                                rtl835x_mac_dis_25m_sdsck_out[idx].val);
            }

            // #=========Serdes 0, 2, 4, 6 patch ====================
            for (sds=0; sds<=6; sds+=2)
            {
                // 2G5_serdes
                for (idx=0; idx<(sizeof(rtl835x_mac_2G5_serdes)/sizeof(confcode_mac_regval_t)); idx++)
                {
                    MAC_REG_SET_CHK(rtl835x_mac_2G5_serdes[idx].reg + (0x400 * (sds/2)), \
                                    rtl835x_mac_2G5_serdes[idx].val);
                }

                // serdes_0
                try = 1;
                do {
                    for (idx=0; idx<(sizeof(rtl835x_mac_serdes_0)/sizeof(confcode_mac_regval_t)); idx++)
                    {
                        MAC_REG_SET_CHK(rtl835x_mac_serdes_0[idx].reg + (0x400 * (sds/2)), \
                                        rtl835x_mac_serdes_0[idx].val);
                    }
                } while (0);
            }


            //#=========Serdes 10 patch ====================
            // serdes_10
            try = 1;
            do {
                // 2G5_serdes
                for (idx=0; idx<(sizeof(rtl835x_mac_2G5_serdes_10)/sizeof(confcode_mac_regval_t)); idx++)
                {
                    MAC_REG_SET_CHK(rtl835x_mac_2G5_serdes_10[idx].reg + (0x400 * (10/2)), \
                                    rtl835x_mac_2G5_serdes_10[idx].val);
                }

                // serdes_10
                for (idx=0; idx<(sizeof(rtl835x_mac_serdes_10)/sizeof(confcode_mac_regval_t)); idx++)
                {
                    MAC_REG_SET_CHK(rtl835x_mac_serdes_10[idx].reg, \
                                    rtl835x_mac_serdes_10[idx].val);
                }
            } while (0);


            //#=========Serdes 8 patch==================================
            // serdes_8
            try = 1;
            do {
                for (idx=0; idx<(sizeof(rtl835x_mac_serdes_8)/sizeof(confcode_mac_regval_t)); idx++)
                {
                    MAC_REG_SET_CHK(rtl835x_mac_serdes_8[idx].reg, \
                                    rtl835x_mac_serdes_8[idx].val);
                }
            } while (0);

            //#=========Serdes 12 patch==================================

            // serdes_12
            try = 1;
            do {
                for (idx=0; idx<(sizeof(rtl835x_mac_serdes_12)/sizeof(confcode_mac_regval_t)); idx++)
                {
                    MAC_REG_SET_CHK(rtl835x_mac_serdes_12[idx].reg, \
                                    rtl835x_mac_serdes_12[idx].val);
                }
            } while (0);

            /* Reset Serdes */
            for (idx=0; idx<(sizeof(rtl835x_mac_serdes_rst)/sizeof(confcode_mac_regval_t)); idx++)
            {
                MAC_REG_SET_CHK(rtl835x_mac_serdes_rst[idx].reg, \
                                rtl835x_mac_serdes_rst[idx].val);
            }

            /* Check Serdes Status */
            OSAL_MDELAY(500);
            /* sds = 0 ~ 12*/
            for (sds=0; sds<=12; sds+=2)
            {
                if (MEM32_READ(0xbb00a078 + (0x400 * (sds/2))) != 0x1ff0000)
                {
                    OSAL_PRINTF("[WARN] Serdes %u initail fail\n", (sds / 2));
                }
                else
                {
                    OSAL_PRINTF("[INFO] Serdes %u OK\n", (sds / 2));
                }
            }
        }
        else if ((pModel->chip == RTK_CHIP_RTL8392M) || \
                 (pModel->chip == RTK_CHIP_RTL8393M) || \
                 (pModel->chip == RTK_CHIP_RTL8396M))
        {
            uint32  idx;
            int8    sdsMask[MAX_SERDES];

            //disable 25M clock
            for (idx=0; idx<(sizeof(rtl839x_dis_ck25mo)/sizeof(confcode_mac_regval_t)); idx++)
            {
                MAC_REG_SET_CHK(rtl839x_dis_ck25mo[idx].reg, rtl839x_dis_ck25mo[idx].val);
            }
            OSAL_MDELAY(500);

    #ifdef  QSGMII_MODE
            //configure serdes to QSGMII mode
            for (idx=0; idx<(sizeof(rtl839x_qsgmii)/sizeof(confcode_mac_regval_t)); idx++)
            {
                MAC_REG_SET_CHK(rtl839x_qsgmii[idx].reg, rtl839x_qsgmii[idx].val);
            }
    #endif

            memset(sdsMask, MAX_SERDES, MAX_SERDES);

            for (idx = 0; idx < pModel->serdes.count; ++idx)
            {
                sdsMask[pModel->serdes.list[idx].sds_id] = pModel->serdes.list[idx].phy_idx;
            }

            for (idx = 0; idx < MAX_SERDES; ++idx)
            {
                if (MAX_SERDES != sdsMask[idx])
                {
                    rtl8390_5G_serdes_config(pModel, idx, sdsMask[idx]);
                    if (8 == idx)
                    {
                        ++idx;
                    }
                }
                /* #power down */
                else
                {
                    switch (idx)
                    {
                        case 6:
                            if (MAX_SERDES != sdsMask[idx + 1])
                            {
                                MAC_REG_SET_CHK(0xaf40, 0xcf0104aa);
                                ++idx;
                            }
                            break;
                        case 8:
                            if (MAX_SERDES != sdsMask[idx + 1])
                            {
                                MAC_REG_SET_CHK(0xb378, 0x1c005);
                                ++idx;
                            }
                            break;
                        case 10:
                            if (MAX_SERDES != sdsMask[idx + 1])
                            {
                                MAC_REG_SET_CHK(0xb740, 0xcf01080f);
                                ++idx;
                            }
                            break;
                        case 12:
                            if (MAX_SERDES != sdsMask[idx + 1])
                            {
                                MAC_REG_SET_CHK(0xbb78, 0x1c005);
                                ++idx;
                            }
                            break;
                    }
                }
            }
        }
    }
    else
    {
        uint32  sdsId, idx;
        int8    sdsMask[MAX_SERDES];

        rtl839x_serdes_patch_init();

        if ((pModel->chip == RTK_CHIP_RTL8352M) || \
            (pModel->chip == RTK_CHIP_RTL8353M))
        {
            rtl839x_5x_serdes_patch_init();
        }

        memset(sdsMask, MAX_SERDES, MAX_SERDES);

        for (idx = 0; idx < pModel->serdes.count; ++idx)
        {
            sdsMask[pModel->serdes.list[idx].sds_id] = pModel->serdes.list[idx].phy_idx;
        }

        for (sdsId = 0; sdsId < MAX_SERDES; ++sdsId)
        {
            if (MAX_SERDES != sdsMask[sdsId])
            {
                if (RTK_CHIP_NONE == pModel->phy.list[(int)sdsMask[sdsId]].chip)
                {
                    pureFiber = 1;
                    switch (sdsId)
                    {
                        case 12:
                            for (idx = 0; idx < (sizeof(rtl839x_init_fiber_1g_frc_S12)/sizeof(confcode_serdes_patch_t)); ++idx)
                            {
                                SERDES_PATCH_SET_CHK(rtl839x_init_fiber_1g_frc_S12[idx]);
                            }
                            break;
                        case 13:
                            for (idx = 0; idx < (sizeof(rtl839x_init_fiber_1g_frc_S13)/sizeof(confcode_serdes_patch_t)); ++idx)
                                SERDES_PATCH_SET_CHK(rtl839x_init_fiber_1g_frc_S13[idx]);
                            break;
                        default:
                            printf("The serdes can't pure fiber\n");
                    }
                }
            }
            else if ((pModel->chip == RTK_CHIP_RTL8392M) || \
                 (pModel->chip == RTK_CHIP_RTL8393M) || \
                 (pModel->chip == RTK_CHIP_RTL8396M))
            {
                /* power off */
                MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_MAC_SERDES_IF_CTRL_ADDR(sdsId), \
                        RTL8390_MAC_SERDES_IF_CTRL_SERDES_SPD_SEL_OFFSET(sdsId), \
                        RTL8390_MAC_SERDES_IF_CTRL_SERDES_SPD_SEL_MASK(sdsId), 0);
            }
        }

        for (sdsId = 0; sdsId < MAX_SERDES; ++sdsId)
        {
            if (MAX_SERDES != sdsMask[sdsId])
                rtl839x_serdes_rst(sdsId);
        }

        if ((pModel->chip == RTK_CHIP_RTL8392M) || \
                 (pModel->chip == RTK_CHIP_RTL8393M) || \
                 (pModel->chip == RTK_CHIP_RTL8396M))
        {
            rtl839x_93m_rst_sys();
        }
        else
            rtl839x_53m_rst_sys();

        /* digit serdes config must be after MAC serdes reset */
        for (sdsId = 0; sdsId < MAX_SERDES; ++sdsId)
        {
            if (MAX_SERDES != sdsMask[sdsId])
            {
                if (RTK_CHIP_NONE == pModel->phy.list[(int)sdsMask[sdsId]].chip)
                {
                    switch (sdsId)
                    {
                        case 12:
                            for (idx = 0; idx < (sizeof(rtl839x_init_fiber_1g_frc_S12)/sizeof(confcode_serdes_patch_t)); ++idx)
                            {
                                SERDES_PATCH_SET_CHK(rtl839x_init_fiber_1g_frc_S12[idx]);
                            }
                            break;
                        case 13:
                            for (idx = 0; idx < (sizeof(rtl839x_init_fiber_1g_frc_S13)/sizeof(confcode_serdes_patch_t)); ++idx)
                                SERDES_PATCH_SET_CHK(rtl839x_init_fiber_1g_frc_S13[idx]);
                            break;
                        default:
                            printf("The serdes can't pure fiber\n");
                    }
                }

                rtl8390_serdes_eee_enable(sdsId);
            }
        }

        rtl8390_phyPowerOff();

        /* power off */
        if ((pModel->chip == RTK_CHIP_RTL8352M) || (pModel->chip == RTK_CHIP_RTL8353M))
        {
            for (idx = 0; idx < (sizeof(rtl835x_serdes_powerOff_conf)/sizeof(confcode_mac_regval_t)); ++idx)
            {
                MAC_REG_SET_CHK(rtl835x_serdes_powerOff_conf[idx].reg, rtl835x_serdes_powerOff_conf[idx].val);
            }
        }

        /* serdes link check */
        OSAL_MDELAY(500);

        for (idx = 0; idx < pModel->serdes.count; ++idx)
        {
            if ((1 == pureFiber) && (pModel->serdes.list[idx].sds_id == 12 ||
                    pModel->serdes.list[idx].sds_id == 13))
            {
                continue;
            }

            rtl8390_serdes_chk(pModel, pModel->serdes.list[idx].sds_id);
        }
    }

    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR | RTL8390_CHIP_INFO_ADDR, \
        RTL8390_CHIP_INFO_CHIP_INFO_EN_OFFSET, RTL8390_CHIP_INFO_CHIP_INFO_EN_MASK, 0x0);

    return;
} /* end of rtl8390_serdes_config_init */

confcode_serdes_patch_t rtl839x_init_fiber_100_S12[] = {
  //init_fiber_1g_frc_S12
    { 0x000c  , 0       , 19 , 16 , 0x8     },

  //force spd =0
    { 0xbbfc  , 0       , 16 , 16 , 0       },
    { 0xbbfc  , 0       , 19 , 18 , 0x0     },

  //DIV2 = 0
    { 0xbb58  , 0       , 0  , 0  , 0       },
    { 0xbb58  , 0       , 4  , 4  , 0       },
    { 0xbb58  , 0       , 16 , 16 , 0       },
    { 0xbb58  , 0       , 20 , 20 , 0       },

    { 0xb880  , 0       , 12 , 12 , 0       },
    { 0xb880  , 0       , 13 , 13 , 1       },
    { 0xb880  , 0       , 6  , 6  , 0       },
  //2013/4/16, s12,s13: reg_rx_sel_rxidle=1(from oobs), reg_oobs_sel_val<4:0>=0x15
    { 0xbb20  , 0       , 5  , 5  , 1       },
    { 0xbb1c  , 0       , 11 , 7  , 0x15    },
};

confcode_serdes_patch_t rtl839x_init_fiber_100_S13[] = {
  //init_fiber_1g_frc_S13
    { 0x000c  , 0       , 23 , 20 , 0x8     },

  //force spd =0
    { 0xbbfc  , 0       , 17 , 17 , 0       },
    { 0xbbfc  , 0       , 21 , 20 , 0x0     },

  //DIV2 = 0
    { 0xbbd8  , 0       , 0  , 0  , 0       },
    { 0xbbd8  , 0       , 4  , 4  , 0       },
    { 0xbbd8  , 0       , 16 , 16 , 0       },
    { 0xbbd8  , 0       , 20 , 20 , 0       },

    { 0xb980  , 0       , 12 , 12 , 0       },
    { 0xb980  , 0       , 13 , 13 , 1       },
    { 0xb980  , 0       , 6  , 6  , 0       },
  //2013/4/16, s12,s13: reg_rx_sel_rxidle=1(from oobs), reg_oobs_sel_val<4:0>=0x15
    { 0xbb20  , 0x80    , 5  , 5  , 1       },
    { 0xbb1c  , 0x80    , 11 , 7  , 0x15    },
};

confcode_serdes_patch_t rtl839x_sfp_rst_S12[] = {
    #if 1
    //#reset tx & rx
    { 0xbbf8  , 0       , 21 , 20 , 0x0003  },
    { 0xbbf8  , 0       , 25 , 24 , 0x0001  },
    { 0xbbf8  , 0       , 21 , 20 , 0x0001  },
    { 0xbbf8  , 0       , 25 , 24 , 0x0003  },
    { 0xbbf8  , 0       , 21 , 20 , 0x0000  },
    { 0xbbf8  , 0       , 25 , 24 , 0x0000  },
    #else
    //#cmu reset for S12 fiber:
    { 0xbbf8  , 0       , 19 , 16 , 0x0005  },
    { 0xbbf8  , 0       , 19 , 16 , 0x000f  },
    #endif
    //#digital soft reset for S12
    { 0xb804, 0, 22,  22,  1},
    { 0xb804, 0, 22,  22,  0},
};

confcode_serdes_patch_t rtl839x_sfp_rst_S13[] = {
    #if 1
    { 0xbbf8  , 0       , 23 , 22 , 0x0003  },
    { 0xbbf8  , 0       , 27 , 26 , 0x0001  },
    { 0xbbf8  , 0       , 23 , 22 , 0x0001  },
    { 0xbbf8  , 0       , 27 , 26 , 0x0003  },
    { 0xbbf8  , 0       , 23 , 22 , 0x0000  },
    { 0xbbf8  , 0       , 27 , 26 , 0x0000  },
    #else
    //#cmu reset for S13 fiber:
    { 0xbbf8  , 0       , 19 , 16 , 0x0005  },
    { 0xbbf8  , 0       , 19 , 16 , 0x000f  },
    #endif
    //#digital soft reset for S13
    { 0xb904, 0, 22,  22,  1},
    { 0xb904, 0, 22,  22,  0},
};

/* Function Name:
 *      rtl8390_misc_config_init
 * Description:
 *      Misc Configuration code in the RTL8390
 * Input:
 *      pModel - pointer to switch model of platform
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
static void rtl8390_misc_config_init(const rtk_switch_model_t *pModel)
{
    uint32 port;

#if defined(CONFIG_SOFTWARE_CONTROL_LED)
    if ((pModel->chip == RTK_CHIP_RTL8352M) || (pModel->chip == RTK_CHIP_RTL8353M))
    {
        /*Setup the board LED information to swCtrl_led module*/
        swCtrl_led_init();

        uboot_isr_register(RTK_DEV_TC0, swCtrl_led_intr_handler, NULL);
        common_enable_irq(29);/* TC0_IRQ */
        common_enable_interrupt();
    }
#endif

    rtk_eee_off(pModel);

    /* enable special congest and set congest timer to 2 sec */
    for (port=0; port < 52; port++)
    {
        MEM32_WRITE(SWCORE_BASE_ADDR| RTL8390_SC_P_EN_ADDR(port), (0x2<<0) | (0x2<<16));
    }

    /* Disable 48 pass 1,
       so high speed to low speed Tx experiment in half mode won't drop packet by this function */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR | RTL8390_MAC_GLB_CTRL_ADDR, \
            RTL8390_MAC_GLB_CTRL_MAC_DROP_48PASS1_EN_OFFSET,        \
            RTL8390_MAC_GLB_CTRL_MAC_DROP_48PASS1_EN_MASK, 0x0);

    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR | RTL8390_MAC_GLB_CTRL_ADDR, \
            RTL8390_MAC_GLB_CTRL_HALF_48PASS1_EN_OFFSET,            \
            RTL8390_MAC_GLB_CTRL_HALF_48PASS1_EN_MASK, 0x0);

    return;
} /* end of rtl8390_misc_config_init */

void rtl8390_gpio_init(void)
{
    return;
}

#if defined(CONFIG_MDC_MDIO_EXT_SUPPORT)
void rtl8231_init(void)
{
    extGpio_init();
}
#endif

/* Function Name:
 *      rtl8390_config
 * Description:
 *      Configuration code for RTL8390
 * Input:
 *      pModel - pointer to switch model of platform
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8390_config(const rtk_switch_model_t *pModel)
{
    uint32  model_info;

    model_info = MEM32_READ(SWCORE_BASE_ADDR| RTL8390_MODEL_NAME_INFO_ADDR);

    OSAL_PRINTF("Model Info: %x\n", model_info);

    /*Internal GPIO init*/
    DBG_PRINT(1, "### Internal GPIO Init ###\n");
    rtl8390_gpio_init();

    /*External GPIO init*/
    #if defined(CONFIG_MDC_MDIO_EXT_SUPPORT)
    DBG_PRINT(1, "### Init RTL8231 ###\n");
    rtl8231_init();
    #endif

    /* Disable MAC polling PHY setting */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR | RTL8390_SMI_GLB_CTRL_ADDR,
            RTL8390_SMI_GLB_CTRL_MDX_POLLING_EN_OFFSET,
            RTL8390_SMI_GLB_CTRL_MDX_POLLING_EN_MASK, 0x0);

    DBG_PRINT(1, "### Reset PHY ###\n");
    rtl8390_phyReset(gSwitchModel);

    /* Enable MAC polling PHY */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR | RTL8390_SMI_GLB_CTRL_ADDR,
            RTL8390_SMI_GLB_CTRL_MDX_POLLING_EN_OFFSET,
            RTL8390_SMI_GLB_CTRL_MDX_POLLING_EN_MASK, 0x1);

    DBG_PRINT(1, "### Platform Config ###\n");
    rtl8390_platform_config_init(gSwitchModel);

    DBG_PRINT(1, "### MAC Config ###\n");
    rtl8390_mac_config_init(gSwitchModel);

#if defined(CONFIG_CUSTOMER_BOARD)
    DBG_PRINT(1, "### Customer MAC Config ###\n");
    customer_mac_config_init(gSwitchModel);
#endif

    DBG_PRINT(1, "### Power down PHY (RTL82XX) ###\n");
    rtl8390_phyPowerOff();

    DBG_PRINT(1, "### PHY Config (RTL82XX) ###\n");
    rtl8390_phy_config_init(gSwitchModel);

#if defined(CONFIG_CUSTOMER_BOARD)
    DBG_PRINT(1, "### Customer PHY Config (RTL82XX) ###\n");
    customer_phy_config_init(gSwitchModel);
#endif

    DBG_PRINT(1, "### Serdes Config ###\n");
    rtl8390_serdes_config_init(gSwitchModel);

    DBG_PRINT(1, "### Misc Config ###\n");
    rtl8390_misc_config_init(gSwitchModel);

#if defined(CONFIG_CUSTOMER_BOARD)
    DBG_PRINT(1, "### Customer Misc Config ###\n");
    customer_mac_misc_config_init(gSwitchModel);
#endif

    return;
} /* end of rtl8390_config */

void rtl8390_sfp_speed_set(int port, int speed)
{
    int idx;

    if (0 == pureFiber)
        return;

    if (1000 == speed)
    {
        if (0 == port)
        {
            for (idx = 0; idx < (sizeof(rtl839x_init_fiber_1g_frc_S12)/sizeof(confcode_serdes_patch_t)); ++idx)
                SERDES_PATCH_SET(rtl839x_init_fiber_1g_frc_S12[idx]);

            for (idx = 0; idx < (sizeof(rtl839x_sfp_rst_S12)/sizeof(confcode_serdes_patch_t)); ++idx)
                SERDES_PATCH_SET(rtl839x_sfp_rst_S12[idx]);
        }
        else if (1 == port)
        {
            for (idx = 0; idx < (sizeof(rtl839x_init_fiber_1g_frc_S13)/sizeof(confcode_serdes_patch_t)); ++idx)
                SERDES_PATCH_SET(rtl839x_init_fiber_1g_frc_S13[idx]);

            for (idx = 0; idx < (sizeof(rtl839x_sfp_rst_S13)/sizeof(confcode_serdes_patch_t)); ++idx)
                SERDES_PATCH_SET(rtl839x_sfp_rst_S13[idx]);
        }
    }
    else if (100 == speed)
    {
        if (0 == port)
        {
            for (idx = 0; idx < (sizeof(rtl839x_init_fiber_100_S12)/sizeof(confcode_serdes_patch_t)); ++idx)
                SERDES_PATCH_SET(rtl839x_init_fiber_100_S12[idx]);

            for (idx = 0; idx < (sizeof(rtl839x_sfp_rst_S12)/sizeof(confcode_serdes_patch_t)); ++idx)
                SERDES_PATCH_SET(rtl839x_sfp_rst_S12[idx]);
        }
        else if (1 == port)
        {
            for (idx = 0; idx < (sizeof(rtl839x_init_fiber_100_S13)/sizeof(confcode_serdes_patch_t)); ++idx)
                SERDES_PATCH_SET(rtl839x_init_fiber_100_S13[idx]);

            for (idx = 0; idx < (sizeof(rtl839x_sfp_rst_S13)/sizeof(confcode_serdes_patch_t)); ++idx)
                SERDES_PATCH_SET(rtl839x_sfp_rst_S13[idx]);
        }
    }

    return;
}

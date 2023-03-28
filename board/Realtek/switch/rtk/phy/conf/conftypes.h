/*
 * Copyright (C) 2010 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 15022 $
 * $Date: 2010-12-24 18:27:25 +0800 (Fri, 24 Dec 2010) $
 *
 * Purpose :
 *
 * Feature :
 *
 */

#ifndef __CONFTYPES_H__
#define __CONFTYPES_H__


/*
 * Gerneral format
 */

/* per-chip: {phy, page, reg, mask, val} */
typedef struct {
    unsigned char  phy:3;
    unsigned char  page:5;
    unsigned char  reg;
    unsigned short mask;
    unsigned short val;
} __attribute__ ((aligned(1), packed)) confcode_pprmv_t;

/* per-port: {page, reg, mask, val} */
typedef struct {
    unsigned char  page;
    unsigned char  reg;
    unsigned short mask;
    unsigned short val;
} confcode_prmv_t;


/*
 * Reduced format (for code size issue)
 */

/* per-chip: {phy, reg, val} */
typedef struct {
    unsigned char  phy:3;
    unsigned char  reg:5;
    unsigned short val;
} __attribute__ ((aligned(1), packed)) confcode_prv_t;

/* per-port: {reg, val} */
typedef struct {
    unsigned char  reg;
    unsigned short val;
} confcode_rv_t;

typedef struct {
    unsigned char   reg;
    unsigned char   endBit;
    unsigned char   startBit;
    unsigned short  val;
} confcode_phy_patch_t;

#define PHY_PATCH_SET(_p, _page, _in)                               \
    do {                                                            \
        unsigned char   _reg, _len;                                 \
        unsigned char   _i, _startBit, _endBit;                     \
        unsigned int    _val, _mask;                                \
                                                                    \
        _reg = _in.reg;                                             \
        _startBit = _in.startBit;                                   \
        _endBit = _in.endBit;                                       \
        _len = _endBit - _startBit + 1;                             \
                                                                    \
        if (32 == _len)                                             \
            _val = _in.val;                                         \
        else                                                        \
        {                                                           \
            _mask = 0;                                              \
            for (_i = _startBit; _i <= _endBit; ++_i)               \
                _mask |= (1 << _i);                                 \
                                                                    \
            gMacDrv->drv_miim_read(_p, _page, _reg, &_val);         \
            _val &= ~(_mask);                                       \
            _val |= (_in.val << _startBit);                         \
        }                                                           \
        gMacDrv->drv_miim_write(_p, _page, _reg, (int)_val);        \
    } while(0);


#endif /* __CONFTYPES_H__ */


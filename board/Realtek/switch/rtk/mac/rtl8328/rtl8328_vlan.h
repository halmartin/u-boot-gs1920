/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * Purpose : Related definition of the RTL8328 driver for RTK command.
 *
 * Feature : RTL8328 driver for RTK VLAN command
 *
 */

#ifndef	__RTL8328_VLAN_H__
#define	__RTL8328_VLAN_H__

#include <common.h>
#include "rtl8328_asicregs.h"
#include <rtk_switch.h>

extern const rtk_switch_model_t *gSwitchModel;
extern const rtk_mac_drv_t *gMacDrv;

typedef struct rtk_vlan_table_s
{
    uint32 reserved_0:24;
    uint32 fid:7;
    uint32 vlan_base_fwd:1;
    uint32 reserved_1:3;
    uint32 utag:29;
    uint32 reserved_2:3;
    uint32 mbr:29;
} rtk_vlan_table_t;

extern int32 rtk_pvid_set(uint32 port, uint32 vid);
extern int32 rtk_pvid_get(uint32 port, uint32 *pVid);
extern int32 rtk_vlanTbl_get(uint32 vid, rtk_vlan_table_t *pEntry);
extern int32 rtk_vlanTbl_set(uint32 vid, rtk_vlan_table_t *pEntry);

#endif	/* __RTL8328_VLAN_H__ */


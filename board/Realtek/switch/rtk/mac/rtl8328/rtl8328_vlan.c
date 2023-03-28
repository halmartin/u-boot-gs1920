/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * Purpose : RTL8328 SOC commands for U-Boot.
 *
 * Feature : 
 *
 */


/*  
 * Include Files 
 */
#include <common.h>
#include <command.h>
#include <rtk/mac/rtl8328/rtl8328_rtk.h>
#include <rtk/mac/rtl8328/rtl8328_drv.h>
#include <rtk/mac/rtl8328/rtl8328_vlan.h>
#include "rtl8328_asicregs.h"

/* 
 * Symbol Definition 
 */
typedef enum rtk_table_list_e
{
    VLAN = 0,
    L2_ISNOTFTIDX,
    L2_ISFTIDX,
    CAM_ISNOTFTIDX,
    CAM_ISFTIDX,
    FORWARDING,
    ACL,
    ENTRY_HIT_INDI_STA,
    ACL_COUNTER_32,
    ACL_COUNTER_64,
    POLICER = 10,
    PIE,
    CBCCR,
    MOVE_PIE,
    SPT,
    TABLE_LIST_END = 15
} rtk_table_list_t;

typedef struct rtk_table_s
{
    uint32 type;              /* access table type */
    uint32 width;             /* table entry width */
    uint32 size;              /* table size */
    uint32 datareg_num;       /* total data registers */
} rtk_table_t;

typedef union rtk_vlan_entry_s
{
    rtk_vlan_table_t entry;
    uint32 data[3];
}rtk_vlan_entry_t;

/* 
 * Data Declaration 
 */
const rtk_table_t rtl8316s_table_list[] =
{
    {   /* table name               VLAN */
        /* access table type */     0,
        /* table entry width */     66,
        /* table size */            4096,
        /* total data registers */  3
	},
    {   /* table name               L2_ISNOTFTIDX */
        /* access table type */     1,
        /* table entry width */     64,
        /* table size */            16384,
        /* total data registers */  2
	},
    {   /* table name               L2_ISFTIDX */
        /* access table type */     1,
        /* table entry width */     60,
        /* table size */            16384,
        /* total data registers */  2
	},
    {   /* table name               CAM_ISNOTFTIDX */
        /* access table type */     2,
        /* table entry width */     75,
        /* table size */            64,
        /* total data registers */  3
	},
    {   /* table name               CAM_ISFTIDX */
        /* access table type */     2,
        /* table entry width */     72,
        /* table size */            64,
        /* total data registers */  3
	},
    {   /* table name               FORWARDING */
        /* access table type */     3,
        /* table entry width */     33,
        /* table size */            1024,
        /* total data registers */  2
	},
    {   /* table name               ACL */
        /* access table type */     4,
        /* table entry width */     86,
        /* table size */            2048,
        /* total data registers */  6
	},
    {   /* table name               ENTRY_HIT_INDI_STA */
        /* access table type */     5,
        /* table entry width */     128,
        /* table size */            16,
        /* total data registers */  4
	},
    {   /* table name               ACL_COUNTER_32 */
        /* access table type */     6,
        /* table entry width */     32,
        /* table size */            1024,
        /* total data registers */  1
	},
    {   /* table name               ACL_COUNTER_64 */
        /* access table type */     6,
        /* table entry width */     64,
        /* table size */            512,
        /* total data registers */  2
	},
    {   /* table name               POLICER */
        /* access table type */     7,
        /* table entry width */     112,
        /* table size */            256,
        /* total data registers */  4
	},
    {   /* table name               PIE */
        /* access table type */     8,
        /* table entry width */     289,
        /* table size */            2048,
        /* total data registers */  10
	},
    {   /* table name               CBCCR */
        /* access table type */     9,
        /* table entry width */     25,
        /* table size */            1,
        /* total data registers */  1
	},
    {   /* table name               MOVE_PIE */
        /* access table type */     10,
        /* table entry width */     38,
        /* table size */            1,
        /* total data registers */  2
	},
    {   /* table name               SPT */
        /* access table type */     11,
        /* table entry width */     58,
        /* table size */            128,
        /* total data registers */  2
	},
};


/*
 * Macro Definition
 */


/* 
 * Function Declaration 
 */
static int32 rtl8328_table_read(rtk_table_list_t table, uint32 addr, uint32 *data)
{
    uint32 busy = 0;
    uint32 retryCount = 512;
    uint32 i;
    uint32 val;

    if ((table >= TABLE_LIST_END) || (addr >= rtl8316s_table_list[table].size))
        return FAILED;

    /*CPUOCCUP*/
    val = MEM32_READ(0xBB000004);
    val |= (1 << 29);
    MEM32_WRITE(0xBB000004, val);    

    val = MEM32_READ(0xBB000004);
    val &= ~(0xF << 14);
    val &= ~(0x3FFF);
    val &= ~(1 << 30);
    val |= (1 << 31) | (1 << 30) |(rtl8316s_table_list[table].type << 14) | (addr << 0) ;
    MEM32_WRITE(0xBB000004, val);
 
    do {
        val = MEM32_READ(0xBB000004);
        busy = val & (1 << 31);
        retryCount--;
    } while (busy &&  (retryCount != 0));

    for (i=0; i<rtl8316s_table_list[table].datareg_num; i++)
    {
        *(data+i) = MEM32_READ(0xBB000008 + i*4);
    }

    if((retryCount == 0))
    {
        printf("Read Table Timeout!type[0x%x], Index[0x%x]\n", rtl8316s_table_list[table].type, addr);
    }

    return SUCCESS;
}

static int32 rtl8328_table_write(rtk_table_list_t table, uint32 addr, uint32 *data)
{
    uint32 busy = 0;
    uint32 retryCount = 512;
    uint32 i;
    uint32 val;

    if ((table >= TABLE_LIST_END) || (addr >= rtl8316s_table_list[table].size))
        return FAILED;

    /*CPUOCCUP*/
    val = MEM32_READ(0xBB000004);
    val |= (1 << 29);
    MEM32_WRITE(0xBB000004, val);

    for (i=0; i<rtl8316s_table_list[table].datareg_num; i++)
    {
        MEM32_WRITE(0xBB000008 + i*4, *(data+i));
    }

    val = MEM32_READ(0xBB000004);
    val &= ~(0xF << 14);
    val &= ~(0x3FFF);
    val &= ~(1 << 30);
    val |= (1 << 31) | (0 << 30) |(rtl8316s_table_list[table].type << 14) | (addr << 0) ;
    MEM32_WRITE(0xBB000004, val);
 
    do {
        val = MEM32_READ(0xBB000004);
        busy = val & (1 << 31);
        retryCount--;
    } while (busy &&  (retryCount != 0));

    if((retryCount == 0))
    {
        printf("Write Table Timeout!type[0x%x], Index[0x%x]\n", rtl8316s_table_list[table].type, addr);
    }

    return SUCCESS;
}


int32 rtk_pvid_set(uint32 port, uint32 vid)
{
    uint32 val;
    if((vid < 1) || (vid >= 4095))
        return FAILED;
    
    val = MEM32_READ(0xBB880000 + 0x100 * port);
    val &= ~(0xFFF << 19);
    val |= (vid << 19);
    MEM32_WRITE(0xBB880000 + 0x100 * port, val);
    return SUCCESS;
}

int32 rtk_pvid_get(uint32 port, uint32 *pVid)
{
    uint32 val;
    if(NULL == pVid)
        return FAILED;
    
    val = MEM32_READ(0xBB880000 + 0x100 * port);
    *pVid = (val >> 19) & 0xFFF;
    return SUCCESS;
}

int32 rtk_vlanTbl_get(uint32 vid, rtk_vlan_table_t *pEntry)
{
    rtk_vlan_entry_t vlanEntry;

    if(NULL == pEntry)
        return FAILED;

    if(vid > 4095)
        return FAILED;

    memset(&vlanEntry, 0, sizeof(rtk_vlan_entry_t));

    rtl8328_table_read(VLAN, vid, vlanEntry.data);

    memcpy(pEntry, &vlanEntry.entry, sizeof(rtk_vlan_table_t));    

    return SUCCESS;
}


int32 rtk_vlanTbl_set(uint32 vid, rtk_vlan_table_t *pEntry)
{
    rtk_vlan_entry_t vlanEntry;

    if(NULL == pEntry)
        return FAILED;

    if(vid > 4095)
        return FAILED;

    memset(&vlanEntry, 0, sizeof(rtk_vlan_entry_t));
    memcpy(&vlanEntry.entry, pEntry, sizeof(rtk_vlan_table_t));
    
    rtl8328_table_write(VLAN, vid, vlanEntry.data);

    return SUCCESS;
}



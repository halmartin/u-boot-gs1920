/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * Purpose : Related definition of the MAC init for U-Boot.
 *
 * Feature : MAC init function
 *
 */


/*
 * Include Files
 */
#include <config.h>
#include <rtk_type.h>
#include <rtk_osal.h>
#include <rtk_debug.h>
#include <rtk_switch.h>
#if defined(CONFIG_RTL8328)
#include <rtk/mac/rtl8328/rtl8328_init.h>
#include <rtk/mac/rtl8328/rtl8328_drv.h>
#endif
#if defined(CONFIG_RTL8390)
#include <rtk/mac/rtl8390/rtl8390_init.h>
#include <rtk/mac/rtl8390/rtl8390_drv.h>
#endif
#if defined(CONFIG_RTL8380)
#include <rtk/mac/rtl8380/rtl8380_init.h>
#include <rtk/mac/rtl8380/rtl8380_drv.h>
#endif

/*
 * Symbol Definition
 */

/*
 * Macro Definition
 */

/*
 * Data Declaration
 */
#if defined(CONFIG_RTL8328)
static rtk_mac_drv_t rtl8328_drv;
#endif /* end of #if defined(CONFIG_RTL8328) */
#if defined(CONFIG_RTL8390)
static rtk_mac_drv_t rtl8390_drv;
#endif /* end of #if defined(CONFIG_RTL8390) */
#if defined(CONFIG_RTL8380)
static rtk_mac_drv_t rtl8380_drv;
#endif /* end of #if defined(CONFIG_RTL8390) */
/*
 * Function Declaration
 */

/* Function Name:
 *      mac_drv_init
 * Description:
 *      Initial the mac driver
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      0 - Success
 * Note:
 *      None
 */
int mac_drv_init(int chip_index, rtk_mac_drv_t **ppMacDrv)
{
    switch (chip_index)
    {
#if defined(CONFIG_RTL8328)
        case RTK_CHIP_RTL8328M:
        case RTK_CHIP_RTL8328S:
        case RTK_CHIP_RTL8328L:
            rtl8328_drv.miim_max_page = 127;
            rtl8328_drv.drv_probe = NULL;
            rtl8328_drv.drv_miim_read = rtl8328_getPhyReg;
            rtl8328_drv.drv_miim_write = rtl8328_setPhyReg;
            rtl8328_drv.drv_miim_portmask_write = rtl8328_setPhyRegByMask;
            (*ppMacDrv) = &rtl8328_drv;
            break;
#endif /* end of #if defined(CONFIG_RTL8328) */
#if defined(CONFIG_RTL8390)
        case RTK_CHIP_RTL8392M:
        case RTK_CHIP_RTL8393M:
        case RTK_CHIP_RTL8396M:
        case RTK_CHIP_RTL8353M:
            rtl8390_drv.miim_max_page = 0x1FFF;
            rtl8390_drv.drv_probe = NULL;
            rtl8390_drv.drv_miim_read = rtl8390_getPhyReg;
            rtl8390_drv.drv_miim_write = rtl8390_setPhyReg;
            rtl8390_drv.drv_miim_portmask_write = rtl8390_setPhyRegByMask;
            (*ppMacDrv) = &rtl8390_drv;
            break;
#endif /* end of #if defined(CONFIG_RTL8390) */
#if defined(CONFIG_RTL8380)
        case RTK_CHIP_RTL8382M:
        case RTK_CHIP_RTL8380M:
        case RTK_CHIP_RTL8332M:
        case RTK_CHIP_RTL8330M:
            rtl8380_drv.miim_max_page = 4095;
            rtl8380_drv.drv_probe = NULL;
            rtl8380_drv.drv_miim_read = rtl8380_getPhyReg;
            rtl8380_drv.drv_miim_write = rtl8380_setPhyReg;
            rtl8380_drv.drv_miim_portmask_write = rtl8380_setPhyRegByMask;
            (*ppMacDrv) = &rtl8380_drv;
            break;
#endif /* end of #if defined(CONFIG_RTL8380) */
        default:
            break;
    }

    return 0;
} /* end of mac_drv_init */

/* Function Name:
 *      chip_config
 * Description:
 *      Config code of the chip.
 * Input:
 *      pModel - pointer to switch model of platform
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void chip_config(const rtk_switch_model_t *pModel)
{
    Tuint32 i;

    if (pModel == NULL)
        return;

    if(0 == strcmp(pModel->name, "ZyXEL_GS1900_8HPv2"))
        OSAL_PRINTF("Model: ZyXEL_GS1900_8HP\n");
    else
        OSAL_PRINTF("Model: %s\n", pModel->name);

    OSAL_PRINTF("Switch Chip: %s\n", \
        (pModel->chip == RTK_CHIP_RTL8328M)?  "RTL8328M" : \
        (pModel->chip == RTK_CHIP_RTL8328S)?  "RTL8328S" : \
        (pModel->chip == RTK_CHIP_RTL8328L)?  "RTL8328L" : \
        (pModel->chip == RTK_CHIP_RTL8352M)?  "RTL8352M" : \
        (pModel->chip == RTK_CHIP_RTL8353M)?  "RTL8353M" : \
        (pModel->chip == RTK_CHIP_RTL8392M)?  "RTL8392M" : \
        (pModel->chip == RTK_CHIP_RTL8393M)?  "RTL8393M" : \
        (pModel->chip == RTK_CHIP_RTL8396M)?  "RTL8396M" : \
        (pModel->chip == RTK_CHIP_RTL8382M)?  "RTL8382M" : \
        (pModel->chip == RTK_CHIP_RTL8332M)?  "RTL8332M" : \
        (pModel->chip == RTK_CHIP_RTL8380M)?  "RTL8380M" : \
        (pModel->chip == RTK_CHIP_RTL8330M)?  "RTL8330M" : \
        "Unknown");

    for (i=0; i<pModel->phy.count; i++)
    {
        DBG_PRINT(1, "  PHY[%d] %-8s : macid = %2d, phy_max = %d\n", i, \
            (pModel->phy.list[i].chip == RTK_CHIP_RTL8208)?  "RTL8208" : \
            (pModel->phy.list[i].chip == RTK_CHIP_RTL8208D)?  "RTL8208D" : \
            (pModel->phy.list[i].chip == RTK_CHIP_RTL8208L)?  "RTL8208L" : \
            (pModel->phy.list[i].chip == RTK_CHIP_RTL8214)?  "RTL8214" : \
            (pModel->phy.list[i].chip == RTK_CHIP_RTL8214F)? "RTL8214F" : \
            (pModel->phy.list[i].chip == RTK_CHIP_RTL8214FB)? "RTL8214FB" : \
            (pModel->phy.list[i].chip == RTK_CHIP_RTL8214FC)? "RTL8214FC" : \
            (pModel->phy.list[i].chip == RTK_CHIP_RTL8214B)? "RTL8214B" : \
            (pModel->phy.list[i].chip == RTK_CHIP_RTL8218)?  "RTL8218" : \
            (pModel->phy.list[i].chip == RTK_CHIP_RTL8212F)?  "RTL8212F" : \
            (pModel->phy.list[i].chip == RTK_CHIP_RTL8212B)?  "RTL8212B" : \
            (pModel->phy.list[i].chip == RTK_CHIP_RTL8218B)?  "RTL8218B" : \
            (pModel->phy.list[i].chip == RTK_CHIP_RTL8218FB)?  "RTL8218FB" : \
            "Unknown",
            pModel->phy.list[i].mac_id, pModel->phy.list[i].phy_max);
    }

    switch (pModel->chip)
    {
#if defined(CONFIG_RTL8328)
    case RTK_CHIP_RTL8328M:
    case RTK_CHIP_RTL8328S:
    case RTK_CHIP_RTL8328L:
        rtl8328_config(pModel);
        break;
#endif /* end of #if defined(CONFIG_RTL8328) */

#if defined(CONFIG_RTL8390)
    case RTK_CHIP_RTL8352M:
    case RTK_CHIP_RTL8353M:
    case RTK_CHIP_RTL8392M:
    case RTK_CHIP_RTL8393M:
    case RTK_CHIP_RTL8396M:
        rtl8390_config(pModel);
        break;
#endif /* end of #if defined(CONFIG_RTL8328) */

#if defined(CONFIG_RTL8380)
    case RTK_CHIP_RTL8382M:
    case RTK_CHIP_RTL8332M:
    case RTK_CHIP_RTL8380M:
    case RTK_CHIP_RTL8330M:
        rtl8380_config(pModel);
        break;
#endif /* end of #if defined(CONFIG_RTL8380) */

    default:
        break;
    }

    return;
} /* end of chip_config */


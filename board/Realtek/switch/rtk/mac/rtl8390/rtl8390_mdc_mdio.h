/*
* Copyright (C) 2010 Realtek Semiconductor Corp.
* All Rights Reserved.
*
* This program is the proprietary software of Realtek Semiconductor
* Corporation and/or its licensors, and only be used, duplicated,
* modified or distributed under the authorized license from Realtek.
*
* ANY USE OF THE SOFTWARE OTEHR THAN AS AUTHORIZED UNDER
* THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
*
* $Revision: 9064 $
* $Date: 2010-04-15 19:47:09 +0800 (?Ÿæ??? 15 ?›æ? 2010) $
*
* Purpose : MDC/MDIO example code
*
* Feature :  This file consists of following modules:
*
*
*/


#ifndef __RTL8390_MDCMDIO_H__
#define __RTL8390_MDCMDIO_H__

typedef enum GPIO_PIN_E
{
    GPIO_A7 = 31,
    GPIO_A6 = 30,
    GPIO_A5 = 29,
    GPIO_A4 = 28,
    GPIO_A3 = 27,
    GPIO_A2 = 26,
    GPIO_A1 = 25,
    GPIO_A0 = 24,
    GPIO_B7 = 23,
    GPIO_B6 = 22,
    GPIO_B5 = 21,
    GPIO_B4 = 20,
    GPIO_B3 = 19,
    GPIO_B2 = 18,
    GPIO_B1 = 17,
    GPIO_B0 = 16,
    GPIO_C7 = 15,
    GPIO_C6 = 14,
    GPIO_C5 = 13,
    GPIO_C4 = 12,
    GPIO_C3 = 11,
    GPIO_C2 = 10,
    GPIO_C1 = 9,
    GPIO_C0 = 8,
    GPIO_END
}gpio_pin_t;

extern uint32 smi_MDC;               /* GPIO used for SMI Clock Generation */
extern uint32 smi_MDIO;              /* GPIO used for SMI Data signal */


#define GPIO_CTRL_REG_BASE (0xb8003500)
#define GPIO_PABC_CNR   (GPIO_CTRL_REG_BASE +0x0)
#define GPIO_PABC_DIR   (GPIO_CTRL_REG_BASE +0x8)
#define GPIO_PABC_DATA  (GPIO_CTRL_REG_BASE +0xc)
#define GPIO_PABC_ISR  (GPIO_CTRL_REG_BASE +0x10)
#define GPIO_PAB_IMR  (GPIO_CTRL_REG_BASE +0x14)
#define GPIO_PC_IMR  (GPIO_CTRL_REG_BASE +0x18)


#define DELAY 2
#define CLK_DURATION(clk)                  { int i; for(i=0; i<clk; i++); }


extern int32 rtl8390_smiInit(uint32 pinMDC, uint32 pinMDIO);
void rtl8390_smiRead(uint32 phyad, uint32 regad, uint32 * data);

#if defined(CONFIG_MDC_MDIO_EXT_SUPPORT)
void rtl8390_smiWrite(uint32 phyad, uint32 regad, uint32 data);
#endif

#endif /*__RTL8390_MDCMDIO_H__*/

/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * Purpose : RTL83xx SOC and SWCORE commands for U-Boot.
 *
 * Feature :
 *
 */


/*
 * Include Files
 */
#include <rtk_osal.h>
#include <config.h>
#include <common.h>
#include <command.h>
#include <rtk/mac/rtl8328/rtl8328_rtk.h>
#include <rtk/mac/rtl8380/rtl8380_rtk.h>
#include <rtk/mac/rtl8390/rtl8390_rtk.h>

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */

/*
 * Macro Definition
 */
#ifndef REG32
#define REG32(reg)   (*(volatile unsigned int *)((unsigned int)reg))
#endif
#ifndef PORTMASK_COUNT
#define PORTMASK_COUNT(port_count)  ((0x1 << (port_count)) - 1)
#endif


/*
 * Function Declaration
 */

/* Function Name:
 *      do_rtk
 * Description:
 *      Main function of RTK commands.
 * Input:
 *      cmdtp, flag, argc, argv handled by the parser
 * Output:
 *      None
 * Return:
 *      [FIXME]
 * Note:
 *      None
 */
int do_rtk(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
    if (argc < 2)
    {
        goto usage;
    }

    /* default */
    if (0 == strcmp(argv[1], "default"))
    {
        printf("Resetting the confinguration ...\n");
        rtk_default();
        return 0;
    }

    /* network */
    if (0 == strcmp(argv[1], "network"))
    {
        if (0 == strcmp(argv[2], "on"))        /* network on */
        {
            printf("Enable network\n");
            rtk_network_on();

            return 0;
        }
        else if (0 == strcmp(argv[2], "off"))  /* network off */
        {
            printf("Disable network\n");
            rtk_network_off();

            return 0;
        }

        goto usage;
    }

#if defined(CONFIG_RTL8328)
    if (0 == strcmp(argv[1], "linkdown-patch")) /* For RTL8328M only */
    {
        rtk_linkdown_powersaving_patch();

        return 0;
    }
#endif

    /* comboport */
    if (0 == strcmp(argv[1], "comboport"))
    {
        if (0 == strcmp(argv[2], "copper"))        /* comboport copper */
        {
            printf("ComboPort: Copper mode\n");
            rtk_comboport_copper();

            return 0;
        }
        else if (0 == strcmp(argv[2], "fiber"))  /* comboport fiber */
        {
            printf("ComboPort: Fiber mode\n");
            rtk_comboport_fiber();

            return 0;
        }

        goto usage;
    }

    /* phyreg get portid page reg */
    if ((0 == strcmp(argv[1], "phyreg")) && (0 == strcmp(argv[2], "get")))
    {
        int portid = simple_strtoul(argv[3], NULL, 10);
        int page = simple_strtoul(argv[4], NULL, 10);
        int reg = simple_strtoul(argv[5], NULL, 10);
        int val;

        gMacDrv->drv_miim_read(portid, page, reg, &val);
        printf("Get Port %02d page %02d reg %02d val: 0x%04X \n", portid, page, reg, val);

        return 1;
    }

    /* phyreg get portid page reg */
    if ((0 == strcmp(argv[1], "phyreg")) && (0 == strcmp(argv[2], "dump-top")))
    {
        int portid = simple_strtoul(argv[3], NULL, 10);

        #ifdef CONFIG_RTL8214FC
        rtk_8214fc_dumpTop(portid);
        #endif

        return 1;
    }

    /* phyreg set */
    if ((0 == strcmp(argv[1], "phyreg")) && (0 == strcmp(argv[2], "set")))
    {
        int portid = simple_strtoul(argv[3], NULL, 10);
        int page = simple_strtoul(argv[4], NULL, 10);
        int reg = simple_strtoul(argv[5], NULL, 10);
        int val = simple_strtoul(argv[6], NULL, 10);

        gMacDrv->drv_miim_write(portid, page, reg, val);
        printf("Set Port %02d page %02d reg %02d val: 0x%04X \n", portid, page, reg, val);

        return 1;
    }

    /* phyreg setbymask */
    if ((strcmp(argv[1], "phyreg") == 0) && (strcmp(argv[2], "setbymask") == 0)) {
        unsigned int port_mask = simple_strtoul(argv[3], NULL, 10);
        int page = simple_strtoul(argv[4], NULL, 10);
        int reg = simple_strtoul(argv[5], NULL, 10);
        int val = simple_strtoul(argv[6], NULL, 10);

        gMacDrv->drv_miim_portmask_write(port_mask, page, reg, val);
        printf("Set PortMask %08x page %02d reg %02d val: 0x%04X \n", port_mask, page, reg, val);

        return 1;
    }

    /* testmode */
    if (strcmp(argv[1], "testmode") == 0)
    {
        unsigned int mode;
        unsigned int port;
        unsigned int chn;

        if (argc < 4)
        {
            printf("Usage: testmode <mode> <port> [channel]\n channel: 0=A,1=B,2=C,3=D\n\n");
        }
        else
        {
        	  mode = simple_strtoul(argv[2], NULL, 10);
        	  port = simple_strtoul(argv[3], NULL, 10);
        	  chn = (argc < 5)? 0 : simple_strtoul(argv[4], NULL, 10);

            if ((port < 0) || (port >= gSwitchModel->port.count))
            {
                printf("The port (%d) is invalid.\n", port);
            }
            else
            {
                unsigned int phyid = gSwitchModel->port.list[port].phy;
                unsigned int phy0_macid = gSwitchModel->phy.list[gSwitchModel->port.list[port].phy_idx].mac_id;

                printf("test mode: %d\n", mode);
                printf("test port: %d [PHY%01d]\n", port, phyid);

                switch (gSwitchModel->phy.list[gSwitchModel->port.list[port].phy_idx].chip)
                {
                case RTK_CHIP_RTL8214FB:
                case RTK_CHIP_RTL8214B:
                case RTK_CHIP_RTL8212B:
                    {
                        printf("RTL8214FB/RTL8214B/RTL8212B Test Mode (PHYID: %d + %d)\n", phy0_macid, phyid);

                        switch (mode)
                        {
                        case 1:
                            {
                                /* Disable LDPS */
                                gMacDrv->drv_miim_write(phy0_macid+phyid,0,21,0x0006); // disable LDPS
                                /* Test Mode 1 */
                                gMacDrv->drv_miim_write(phy0_macid+phyid,0,0,0x1140);  // power on PHY
                                gMacDrv->drv_miim_write(phy0_macid+phyid,0,9,0x2E00);  // test mode 1
                                OSAL_MDELAY(100);    /* delay 100mS and wait for Mirco-P completed */
                                /* Adjust Amplitude */
                                gMacDrv->drv_miim_write(phy0_macid+phyid,2,21,0xAA00); // adjust amplitude
                            }
                            break;

                        case 4:
                            {
                                /* Disable LDPS */
                                gMacDrv->drv_miim_write(phy0_macid+phyid,0,21,0x0006); // disable LDPS
                                /* Test Mode 4 */
                                gMacDrv->drv_miim_write(phy0_macid+phyid,0,0,0x1140);  // power on PHY
                                gMacDrv->drv_miim_write(phy0_macid+phyid,0,9,0x8E00);  // Enable Test mode 4
                                OSAL_MDELAY(100);    /* delay 100mS and wait for Mirco-P completed */
                                /* Adjust LDVbias */
                                gMacDrv->drv_miim_write(phy0_macid+phyid,2,5,0xCE68);  // adjust LDVbias
                            }
                            break;

                        default:
                            printf("The mode (%d) is not be suppoted yet.\n", mode);
                            break;
                        }
                    }
                    break;

                case RTK_CHIP_RTL8214:
                case RTK_CHIP_RTL8214F:
                    {
                        printf("RTL8214/RTL8214F Test Mode (PHYID: %d + %d)\n", phy0_macid, phyid);

                        switch (mode)
                        {
                        case 1:
                            {
                                gMacDrv->drv_miim_write(phy0_macid+0,0,9,0x0E00);  // Set Test mode: normal
                                gMacDrv->drv_miim_write(phy0_macid+1,0,9,0x0E00);  // Set Test mode: normal
                                gMacDrv->drv_miim_write(phy0_macid+2,0,9,0x0E00);  // Set Test mode: normal
                                gMacDrv->drv_miim_write(phy0_macid+3,0,9,0x0E00);  // Set Test mode: normal

                                gMacDrv->drv_miim_write(phy0_macid+phyid,0,21,0x0006); // Disable Power-Saving mode
                                gMacDrv->drv_miim_write(phy0_macid+phyid,2,21,0xAA00); // Amp+
                                gMacDrv->drv_miim_write(phy0_macid+phyid,0,9,0x2E00);  // Enable Test mode 1
                            }
                            break;

                        case 4:
                            {
                                gMacDrv->drv_miim_write(phy0_macid+0,0,9,0x0E00);  // Set Test mode: normal
                                gMacDrv->drv_miim_write(phy0_macid+1,0,9,0x0E00);  // Set Test mode: normal
                                gMacDrv->drv_miim_write(phy0_macid+2,0,9,0x0E00);  // Set Test mode: normal
                                gMacDrv->drv_miim_write(phy0_macid+3,0,9,0x0E00);  // Set Test mode: normal

                                gMacDrv->drv_miim_write(phy0_macid+phyid,0,21,0x0006); // Disable Power-Saving mode
                                gMacDrv->drv_miim_write(phy0_macid+phyid,2,21,0x5500); // Set back to normal value
                                gMacDrv->drv_miim_write(phy0_macid+phyid,0,9,0x8E00);  // Enable Test mode 4
                            }
                            break;

                        default:
                            printf("The mode (%d) is not be suppoted yet.\n", mode);
                            break;
                        }
                    }
                    break;

                case RTK_CHIP_RTL8212F:
                    {
                        unsigned int val;
                        printf("RTL8212F Test Mode (PHYID: %d + %d)\n", phy0_macid, phyid);

                        switch (mode)
                        {
                        case 1:
                            {
                                gMacDrv->drv_miim_read(phy0_macid+0, 0, 9, &val);
                                val &= ~(7<<13);
                                gMacDrv->drv_miim_write(phy0_macid+0, 0, 9, val);  // Set Test mode: normal
                                gMacDrv->drv_miim_read(phy0_macid+1, 0, 9, &val);
                                val &= ~(7<<13);
                                gMacDrv->drv_miim_write(phy0_macid+1, 0, 9, val);  // Set Test mode: normal

                                gMacDrv->drv_miim_read(phy0_macid+phyid, 0, 9, &val);
                                val |= (1 << 13);  // Enable Test mode 1
                                gMacDrv->drv_miim_write(phy0_macid+phyid, 0, 9, val);  // Enable Test mode 1

                            }
                            break;

                        case 4:
                            {
                                gMacDrv->drv_miim_read(phy0_macid+0, 0, 9, &val);
                                val &= ~(7<<13);
                                gMacDrv->drv_miim_write(phy0_macid+0, 0, 9, val);  // Set Test mode: normal
                                gMacDrv->drv_miim_read(phy0_macid+1, 0, 9, &val);
                                val &= ~(7<<13);
                                gMacDrv->drv_miim_write(phy0_macid+1, 0, 9, val);  // Set Test mode: normal

                                gMacDrv->drv_miim_read(phy0_macid+phyid, 0, 9, &val);
                                val |= (4 << 13);  // Enable Test mode 4
                                gMacDrv->drv_miim_write(phy0_macid+phyid, 0, 9,val);  // Enable Test mode 4
                            }
                            break;

                        default:
                            printf("The mode (%d) is not be suppoted yet.\n", mode);
                            break;
                        }
                    }
                    break;

                case RTK_CHIP_RTL8218:
                    {
                        printf("RTL8218 Test Mode (PHYID: %d + %d)\n", phy0_macid, phyid);

                        /* recovery test mode 2 setting to default */
                        gMacDrv->drv_miim_write(phy0_macid+1,2,0x11,0x7E00);
                        gMacDrv->drv_miim_write(phy0_macid+1,0,0x1f,0x0000);
                        gMacDrv->drv_miim_write(phy0_macid+4,2,0x11,0x7E00);
                        gMacDrv->drv_miim_write(phy0_macid+4,0,0x1f,0x0000);

                        switch (mode)
                        {
                        case 1:
                            {
                                gMacDrv->drv_miim_write(phy0_macid+4,2,31,0x0002);  // Page 2
                                gMacDrv->drv_miim_write(phy0_macid+4,2,19,0xAA00);  // PHYReg wi 4 19 0xAA00, PHY 0 ==> Giga / 10M +2.5%
                                gMacDrv->drv_miim_write(phy0_macid+4,2,20,0xAA00);  // PHYReg wi 4 20 0xAA00, PHY 1 ==> Giga / 10M +2.5%
                                gMacDrv->drv_miim_write(phy0_macid+4,2,21,0xAA00);  // PHYReg wi 4 21 0xAA00, PHY 2 ==> Giga / 10M +2.5%
                                gMacDrv->drv_miim_write(phy0_macid+4,2,22,0xFA00);  // PHYReg wi 4 22 0xFA00, PHY 3 ==> Giga / 10M +2.5%
                                gMacDrv->drv_miim_write(phy0_macid+4,2,23,0xAF00);  // PHYReg wi 4 23 0xAF00, PHY 4 ==> Giga / 10M +2.5%
                                gMacDrv->drv_miim_write(phy0_macid+4,2,24,0xAA00);  // PHYReg wi 4 24 0xAA00, PHY 5 ==> Giga / 10M +2.5%
                                gMacDrv->drv_miim_write(phy0_macid+4,2,25,0xAA00);  // PHYReg wi 4 25 0xAA00, PHY 6 ==> Giga / 10M +2.5%
                                gMacDrv->drv_miim_write(phy0_macid+4,2,26,0xAA00);  // PHYReg wi 4 26 0xAA00, PHY 7 ==> Giga / 10M +2.5%
                                gMacDrv->drv_miim_write(phy0_macid+4,0,31,0x0000);  // Page 0

                                gMacDrv->drv_miim_write(phy0_macid+0,0,9,0x0E00);  // Disable Test mode 1
                                gMacDrv->drv_miim_write(phy0_macid+1,0,9,0x0E00);  // Disable Test mode 1
                                gMacDrv->drv_miim_write(phy0_macid+2,0,9,0x0E00);  // Disable Test mode 1
                                gMacDrv->drv_miim_write(phy0_macid+3,0,9,0x0E00);  // Disable Test mode 1
                                gMacDrv->drv_miim_write(phy0_macid+4,0,9,0x0E00);  // Disable Test mode 1
                                gMacDrv->drv_miim_write(phy0_macid+5,0,9,0x0E00);  // Disable Test mode 1
                                gMacDrv->drv_miim_write(phy0_macid+6,0,9,0x0E00);  // Disable Test mode 1
                                gMacDrv->drv_miim_write(phy0_macid+7,0,9,0x0E00);  // Disable Test mode 1

                                gMacDrv->drv_miim_write(phy0_macid+phyid,0,9,0x2E00);  // Test mode 1
                            }
                            break;

                        case 2:
                            {
                                /* change the parameters for test mode 2 */
                                gMacDrv->drv_miim_write(phy0_macid+1,2,0x11,0x5E00);
                                gMacDrv->drv_miim_write(phy0_macid+1,0,0x1f,0x0000);
                                gMacDrv->drv_miim_write(phy0_macid+4,2,0x11,0x5E00);
                                gMacDrv->drv_miim_write(phy0_macid+4,0,0x1f,0x0000);

                                gMacDrv->drv_miim_write(phy0_macid+0,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+1,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+2,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+3,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+4,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+5,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+6,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+7,0,9,0x0E00);  // Disable Test mode

                                gMacDrv->drv_miim_write(phy0_macid+phyid,0,9,0x4E00);  // Test mode 2
                            }
                            break;

                        case 3:
                            {
                                printf(" test channel: %d\n", chn);

                                if (phyid < 4)
                                {
                                    //printf("PHY:%d, page:%d, reg:%d, val:0x%04X\n", phy0_macid+1, 2, 17, 0x4000 | ((phyid & 0x7) << 9));
                                    gMacDrv->drv_miim_write(phy0_macid+1, 2, 17, 0x4000 | ((phyid & 0x7) << 9));
                                    //printf("PHY:%d, page:%d, reg:%d, val:0x%04X\n", phy0_macid+1, 2, 16, 0x1100 | ((chn & 0x3) << 9));
                                    gMacDrv->drv_miim_write(phy0_macid+1, 2, 16, 0x1100 | ((chn & 0x3) << 9));
                                } else {
                                    //printf("PHY:%d, page:%d, reg:%d, val:0x%04X\n", phy0_macid+4, 2, 17, 0x4000 | ((phyid & 0x7) << 9));
                                    gMacDrv->drv_miim_write(phy0_macid+4, 2, 17, 0x4000 | ((phyid & 0x7) << 9));
                                    //printf("PHY:%d, page:%d, reg:%d, val:0x%04X\n", phy0_macid+4, 2, 16, 0x1100 | ((chn & 0x3) << 9));
                                    gMacDrv->drv_miim_write(phy0_macid+4, 2, 16, 0x1100 | ((chn & 0x3) << 9));
                                }

                                gMacDrv->drv_miim_write(phy0_macid+0,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+1,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+2,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+3,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+4,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+5,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+6,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+7,0,9,0x0E00);  // Disable Test mode

                                gMacDrv->drv_miim_write(phy0_macid+phyid,0,9,0x6E00);  // Test mode 3
                            }
                            break;

                        case 4:
                            {
                                gMacDrv->drv_miim_write(phy0_macid+0,2,7,0x3678);  // Page 2 Reg 7 = 0x3678, Ldvbias = 4 (PHY0)
                                gMacDrv->drv_miim_write(phy0_macid+0,0,31,0x0000); // Page 0
                                gMacDrv->drv_miim_write(phy0_macid+0,0,9,0x0E00);  // Disable Test mode 4
                                gMacDrv->drv_miim_write(phy0_macid+1,2,7,0x3678);  // Page 2 Reg 7 = 0x3678, Ldvbias = 4 (PHY1)
                                gMacDrv->drv_miim_write(phy0_macid+1,0,31,0x0000); // Page 0
                                gMacDrv->drv_miim_write(phy0_macid+1,0,9,0x0E00);  // Disable Test mode 4
                                gMacDrv->drv_miim_write(phy0_macid+2,2,7,0x3678);  // Page 2 Reg 7 = 0x3678, Ldvbias = 4 (PHY2)
                                gMacDrv->drv_miim_write(phy0_macid+2,0,31,0x0000); // Page 0
                                gMacDrv->drv_miim_write(phy0_macid+2,0,9,0x0E00);  // Disable Test mode 4
                                gMacDrv->drv_miim_write(phy0_macid+3,2,7,0x3678);  // Page 2 Reg 7 = 0x3678, Ldvbias = 4 (PHY3)
                                gMacDrv->drv_miim_write(phy0_macid+3,0,31,0x0000); // Page 0
                                gMacDrv->drv_miim_write(phy0_macid+3,0,9,0x0E00);  // Disable Test mode 4
                                gMacDrv->drv_miim_write(phy0_macid+4,2,7,0x3678);  // Page 2 Reg 7 = 0x3678, Ldvbias = 4 (PHY4)
                                gMacDrv->drv_miim_write(phy0_macid+4,0,31,0x0000); // Page 0
                                gMacDrv->drv_miim_write(phy0_macid+4,0,9,0x0E00);  // Disable Test mode 4
                                gMacDrv->drv_miim_write(phy0_macid+5,2,7,0x3678);  // Page 2 Reg 7 = 0x3678, Ldvbias = 4 (PHY5)
                                gMacDrv->drv_miim_write(phy0_macid+5,0,31,0x0000); // Page 0
                                gMacDrv->drv_miim_write(phy0_macid+5,0,9,0x0E00);  // Disable Test mode 4
                                gMacDrv->drv_miim_write(phy0_macid+6,2,7,0x3678);  // Page 2 Reg 7 = 0x3678, Ldvbias = 4 (PHY6)
                                gMacDrv->drv_miim_write(phy0_macid+6,0,31,0x0000); // Page 0
                                gMacDrv->drv_miim_write(phy0_macid+6,0,9,0x0E00);  // Disable Test mode 4
                                gMacDrv->drv_miim_write(phy0_macid+7,2,7,0x3678);  // Page 2 Reg 7 = 0x3678, Ldvbias = 4 (PHY7)
                                gMacDrv->drv_miim_write(phy0_macid+7,0,31,0x0000); // Page 0
                                gMacDrv->drv_miim_write(phy0_macid+7,0,9,0x0E00);  // Disable Test mode 4

                                gMacDrv->drv_miim_write(phy0_macid+phyid,0,31,0x0000);  // Page 0
                                gMacDrv->drv_miim_write(phy0_macid+phyid,0,9,0x8E00);  // PHYReg wi 0 9 0x8E00, PHY0 Test mode 4
                            }
                            break;

                        default:
                            printf("The mode (%d) is not be suppoted yet.\n", mode);
                            break;
                        }
                    }
                    break;

                case RTK_CHIP_RTL8214FC:
                case RTK_CHIP_RTL8218B:
                case RTK_CHIP_RTL8218FB:
                    {
                        printf("RTL8218 Test Mode (PHYID: %d + %d)\n", phy0_macid, phyid);

                        switch (mode)
                        {
                        case 1:
                            {
                                gMacDrv->drv_miim_write(phy0_macid+0,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+1,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+2,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+3,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+4,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+5,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+6,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+7,0,9,0x0E00);  // Disable Test mode

                                gMacDrv->drv_miim_write(phy0_macid+phyid,0,9,0x2E00);  // Test mode 1
                            }
                            break;

                        case 2:
                            {
                                gMacDrv->drv_miim_write(phy0_macid+0,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+1,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+2,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+3,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+4,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+5,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+6,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+7,0,9,0x0E00);  // Disable Test mode

                                gMacDrv->drv_miim_write(phy0_macid+phyid,0,9,0x4E00);  // Test mode 2
                            }
                            break;

                        case 3:
                            {
                                printf(" test channel: %d\n", chn);

                                if (phyid < 4)
                                {
                                    gMacDrv->drv_miim_write(phy0_macid+1, 2, 17, 0x4000 | ((phyid & 0x7) << 9));
                                    gMacDrv->drv_miim_write(phy0_macid+1, 2, 16, 0x1100 | ((chn & 0x3) << 9));
                                } else {
                                    gMacDrv->drv_miim_write(phy0_macid+4, 2, 17, 0x4000 | ((phyid & 0x7) << 9));
                                    gMacDrv->drv_miim_write(phy0_macid+4, 2, 16, 0x1100 | ((chn & 0x3) << 9));
                                }

                                gMacDrv->drv_miim_write(phy0_macid+0,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+1,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+2,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+3,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+4,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+5,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+6,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+7,0,9,0x0E00);  // Disable Test mode

                                gMacDrv->drv_miim_write(phy0_macid+phyid,0,9,0x6E00);  // Test mode 3
                            }
                            break;

                        case 4:
                            {
                                gMacDrv->drv_miim_write(phy0_macid+0,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+1,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+2,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+3,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+4,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+5,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+6,0,9,0x0E00);  // Disable Test mode
                                gMacDrv->drv_miim_write(phy0_macid+7,0,9,0x0E00);  // Disable Test mode

                                gMacDrv->drv_miim_write(phy0_macid+phyid,0,9,0x8E00);  // PHYReg wi 0 9 0x8E00, PHY0 Test mode 4
                            }
                            break;

                        default:
                            printf("The mode (%d) is not be suppoted yet.\n", mode);
                            break;
                        }
                    }
                    break;

                default:
                    printf("This chip of the port is not support the test mode yet.\n");
                    break;
                }
            }
        }

        return 1;
    }

#ifdef CONFIG_EEE
    /* eee */
    if (0 == strcmp(argv[1], "eee"))
    {
        if (0 == strcmp(argv[2], "on"))        /* eee on */
        {
            printf("Enable EEE function\n");
            rtk_eee_on(gSwitchModel);

            return 0;
        }
        else if (0 == strcmp(argv[2], "off"))  /* eee off */
        {
            printf("Disable EEE function\n");
            rtk_eee_off(gSwitchModel);

            return 0;
        }

        goto usage;
    }
#endif

#if defined(CONFIG_RTL8328) || defined(CONFIG_RTL8380)
    if ((0 == strcmp(argv[1], "l2-testmode")) && (0 == strcmp(argv[2], "on")))
    {
        rtk_l2testmode_on();
        return 0;
    }

    if ((0 == strcmp(argv[1], "l2-testmode")) && (0 == strcmp(argv[2], "off")))
    {
        rtk_l2testmode_off();
        return 0;
    }
#endif

#if defined(CONFIG_SOFTWARE_CONTROL_LED)
    if (0 == strcmp(argv[1], "software-control-led"))
    {
        if (0 == strcmp(argv[2], "on"))
        {
            printf("Enable Software LED Control function\n");
            rtk_softwareControlLed_on();

            return 0;
        }
        else if (0 == strcmp(argv[2], "off"))
        {
            printf("Disable Software LED Control function\n");
            rtk_softwareControlLed_off();

            return 0;
        }

        goto usage;
    }
#endif

#if  defined(CONFIG_RTL8380)
    if ((0 == strcmp(argv[1], "port-isolation")) && (0 == strcmp(argv[2], "on")))
    {
        rtk_port_isolation_on();
        return 0;
    }

    if ((0 == strcmp(argv[1], "port-isolation")) && (0 == strcmp(argv[2], "off")))
    {
        rtk_port_isolation_off();
        return 0;
    }
#endif

#if defined(CONFIG_RTL8390)
    if ((0 == strcmp(argv[1], "sfp-speed")) && (0 == strcmp(argv[2], "set")))
    {
        int port = simple_strtoul(argv[3], NULL, 10);
        int speed = simple_strtoul(argv[4], NULL, 10);
        rtk_sfp_speed_set(port, speed);
        return 0;
    }
    else if ((0 == strcmp(argv[1], "parameter")) && (0 == strcmp(argv[2], "version")))
    {
        printf("1.0\n");
        return 0;
    }
#endif

usage:
    printf("Usage:\n%s\n", cmdtp->usage);
    return 1;
} /* end of do_rtk */

U_BOOT_CMD(
    rtk, 7, 0, do_rtk,
    "rtk     - Realtek commands\n",
    "object action\n"
    "        - SOC commands.\n"
    "rtk network on\n"
    "        - Enable the networking function\n"
    "rtk netowkr off\n"
    "        - Disable the networking function\n"
    "rtk testmode [mode] [port]\n"
    "        - Set default value for specific testing\n"
);


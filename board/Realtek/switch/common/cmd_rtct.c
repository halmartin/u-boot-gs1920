/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * Purpose : RTCT commands for U-Boot.
 *
 * Feature : 
 *
 */


/*  
 * Include Files 
 */
#include <common.h>
#include <command.h>


/* 
 * Symbol Definition 
 */


/* 
 * Data Declaration 
 */


/*
 * Macro Definition
 */


/* 
 * Function Declaration 
 */
extern int rtct_port(int portid);

/* Function Name:
 *      do_rtct
 * Description:
 *      Main function of RTCT commands.
 * Input:
 *      cmdtp, flag, argc, argv handled by the parser
 * Output:
 *      None
 * Return:
 *      [FIXME]
 * Note:
 *      None
 */
int do_rtct(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
    if (argc < 2)
    {
        goto usage;
    }

    /* global */
    if (0 == strcmp(argv[1], "global"))
    {
        printf("Global RTCT testing ... (NOT SUPPORT)\n");
        return 0;
    }
    else if (0 == strcmp(argv[1], "port"))
    {
        int portid = simple_strtoul(argv[2], NULL, 10);
        
        printf("Port (%d) RTCT testing ...\n", portid);
        rtct_port(portid);
        return 0;
    }
    else if (0 == strcmp(argv[1], "portmask"))
    {
        int portmask = simple_strtoul(argv[2], NULL, 16);
        int i;
        
        printf("PortMask (%08X) RTCT testing ...\n", portmask);
        for (i=0; i<32; i++) {
            if ((0x1 << i) & portmask)
                rtct_port(i);
        }
        return 0;
    }

usage:
    printf("Usage:\n%s\n", cmdtp->usage);
    return 1;
} /* end of do_rtct */

U_BOOT_CMD(
    rtct, 3, 1, do_rtct,
    "rtct    - Realtek Cable Tester (RTCT) commands\n",
    "object action\n"
    "        - RTCT commands.\n"
);


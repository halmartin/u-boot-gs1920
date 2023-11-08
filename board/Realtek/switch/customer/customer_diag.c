/*
 * Copyright(c) Realtek Semiconductor Corporation, 2012
 * All rights reserved.
 *
 * Purpose : RTL8380 SOC commands for U-Boot.
 *
 * Feature :
 *
 */


/*
 * Include Files
 */
#include <common.h>
#include <command.h>
#include <rtk_reg.h>
#include <rtk_osal.h>
#include <customer/customer_diag.h>
#include <rtk/drv/rtl8231/rtl8231_drv.h>
#include <rtk/drv/gpio/ext_gpio.h>
#include <spi_flash.h>

#if defined(CONFIG_RTL8380)
#include <rtk/mac/rtl8380/rtl8380_rtk.h>
#include <rtk/mac/rtl8380/rtl8380_mdc_mdio.h>
#endif

#if defined(CONFIG_RTL8390)
#include <rtk/mac/rtl8390/rtl8390_drv.h>
#include <rtk/mac/rtl8390/rtl8390_mdc_mdio.h>
#include <rtk/mac/rtl8390/rtl8390_rtk.h>
#endif

/*
 * Symbol Definition
 */
#define ENV_BOARD_MODEL     "boardmodel"
#define ENV_RUNNING_HTP     "runHTP"
#define ENV_HAVE_HTP_LOG    "HTPLog"

/*
 * Data Declaration
 */
uchar pkt[] = { /* ARP: Who has 192.168.154.80? Tell 192.168.154.89 */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x52, 0x45, 0x41, 0x4c, 0x30, 0x59, 0x08, 0x06, 0x00, 0x01,
    0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0x52, 0x45, 0x41, 0x4c, 0x30, 0x59, 0xc0, 0xa8, 0x9a, 0x59,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xa8, 0x9a, 0x50, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20
    };

uchar pkt_buff[PORT_LOOPBACK_TEST_PKT_MAX_LEN];

/*
 * Macro Definition
 */
#ifdef CONFIG_GS1900v2
int htpModeIf = 0;
int htpBreakIf = 1;
u_long htpFailMsg = 0;
#endif

/*
 * Function Declaration
 */
int32 is_ctrlc(void)
{
	if (tstc()) {
		switch (getc ()) {
		case 0x03:		/* ^C - Control C */
			return 1;
		default:
			break;
		}
	}
	return 0;
}

int32 rtk_rstDeft_detect(void)
{
    uint32 delay1,delay2;
    int32   ret = 1;

    for(delay1=0;delay1<0xFFFF;delay1++)
    {
        for(delay2=0;delay2<0x6;delay2++)
        {
    		if (is_ctrlc())
            {
                printf("\n");
    			goto END;
    		}

            ret = rtk_rstDeftGpio_detect();
            if (1 == ret)
            {
                printf("Reset to factory default ....\n");
                ret = 0;
                goto END;
            }
        }
    }

END:
    return ret;
}

int32 rtk_dram_test(uint32 round, vu_long addr_start, vu_long addr_end)
{
    uint32 i;
	vu_long	*addr, *start, *end;

	vu_long	addr_len;
	vu_long	offset;
	vu_long	pattern;
	vu_long	temp;
	vu_long	anti_pattern;

    start = (ulong *)addr_start;
    end = (ulong *)addr_end;
    //printf ("Testing %08x ... %08x:\n", (uint)start, (uint)end);

    for(i=0; i<round; i++)
    {
        printf("\r[round:%d]",i);

    /*ctrl+c to break*/
       if (is_ctrlc())
       {
           printf("\n");
           return 1;
       }

        addr = start;

        addr_len = (ulong)end - (ulong)start;
        pattern = (vu_long) 0xaaaaaaaa;
        anti_pattern = (vu_long) 0x55555555;

      /*
         * Write the default pattern at each of the
         * power-of-two offsets.
         */

        for (offset = 0; offset < addr_len; offset += 4)
        {
            start[offset] = pattern;
        }

      /*
         * Check for address bits stuck high.
         */
        for (offset = 0; offset < addr_len; offset += 4)
        {
            temp = start[offset];
            if (temp != pattern)
            {
                printf ("\nFAILURE: Address bit stuck high @ 0x%.8lx:"
                    " expected 0x%.8lx, actual 0x%.8lx\n",
                    (ulong)&start[offset], pattern, temp);
                return 1;
            }
        }

        for (offset = 0; offset < addr_len; offset += 4)
        {
            start[offset] = anti_pattern;
        }

      /*
         * Check for address bits stuck high.
         */
        for (offset = 0; offset < addr_len; offset += 4)
        {
            temp = start[offset];
            if (temp != anti_pattern)
            {
                printf ("\nFAILURE: Address bit stuck high @ 0x%.8lx:"
                    " expected 0x%.8lx, actual 0x%.8lx\n",
                    (ulong)&start[offset], anti_pattern, temp);
                return 1;
            }
        }

        for (offset = 0; offset < addr_len; offset += 4)
        {
            start[offset] = offset;
        }

      /*
         * Check for address bits stuck high.
         */
        for (offset = 0; offset < addr_len; offset += 4)
        {
            temp = start[offset];
            if (temp != offset)
            {
                printf ("\nFAILURE: Address bit stuck high @ 0x%.8lx:"
                    " expected 0x%.8lx, actual 0x%.8lx\n",
                    (ulong)&start[offset], offset, temp);
                return 1;
            }
        }
    }

    return 0;
}


#define RTL8231_GPIO_ACCESS_DELAY (0xFFF)
#define RTL8231_GPIO_READ_TIMES (10)
#define RTL8231_GPIO_VALUE_THRD (3)

#define RTK_FC_DRAM_TEST_ROUND  (1000)
#define RTK_FC_DRAM_TEST_ADDR_START (0x81000000)
#define RTK_FC_DRAM_TEST_ADDR_END (0x81020000)  //128KB

#define RTK_FC_FLASH_TEST_ROUND  (200)  /*test about 1 hour*/
//#define	FC_FLASH_TEST_SIZE_PER_PATTREN	(0x20000) //128KB
#define	FC_FLASH_TEST_SIZE_PER_PATTREN	(0x10000) //64KB
#define RTK_FC_FLASH_TEST_ADDR_START (0xB4FF0000)
//#define RTK_FC_FLASH_TEST_ADDR_END (FLASH_TOP_ADDRESS)
#define RTK_FC_FLASH_TEST_ADDR_END (RTK_FC_FLASH_TEST_ADDR_START+(FC_FLASH_TEST_SIZE_PER_PATTREN)*1)

/*
 * Protection Flags for flash_protect():
 */
#define FLAG_PROTECT_SET	0x01
#define FLAG_PROTECT_CLEAR	0x02
#define	FLAG_PROTECT_INVALID	0x03

#if 0
int32 rtk_flash_test(uint32 round, vu_long addr_start, vu_long addr_end)
{
    uint32 i;
    int32 ret;
    uint32 flash_sect_num;
    uint32 j;
    uint32 sect_start_addr;

    flash_sect_num = (addr_end - addr_start)/FC_FLASH_TEST_SIZE_PER_PATTREN;

    for(i=0;i<round;i++)
    {
        printf("\r[round:%d]",i);

     /*ctrl+c to break*/
        if (is_ctrlc())
        {
            printf("\n");
            return 1;
        }

    	/* 1. Non Boot loader area, in case of unrecoverable operation. */
#if 0
    /*need not to excute fully flash test*/
    	ret = _flash_test(addr_start, FC_FLASH_TEST_SIZE_PER_PATTREN, (addr_end - addr_start));
        ret = flash_normal_patterns(addr_start, FC_FLASH_TEST_SIZE_PER_PATTREN, (addr_end - addr_start));
#else
        ret = flash_walking_of_1(addr_start, FC_FLASH_TEST_SIZE_PER_PATTREN, (addr_end - addr_start));

        for(j = 0; j < flash_sect_num; j++)
        {
            sect_start_addr = addr_start + j*FC_FLASH_TEST_SIZE_PER_PATTREN;
            flash_sect_erase(sect_start_addr, (sect_start_addr+FC_FLASH_TEST_SIZE_PER_PATTREN-1));
        }
#endif

        if(ret != 0)
            return ret;
    }

    return 0;
}


int32 rtk_port_loopback_test(void)
{
    int ret = 0;
    int x;
    uint32 i;
    uint32 count;
    uint32 port;
    uint32 port_start,port_end;
    uint32 rx_pkt_len=0;
    uchar *recv_pkt;
    char   *board_model;

    board_model = getenv(ENV_BOARD_MODEL);

    if (gSwitchModel == NULL)
        return 1;

    NetInit();

    port_start = gSwitchModel->port.list[0].mac_id;
    port_end = gSwitchModel->port.list[0].mac_id + gSwitchModel->port.count;
    recv_pkt = pkt_buff;

    for(port=port_start;port<port_end;port++)
    {
        if(0 == strcmp(board_model, "ZyXEL_GS1900_10HP"))
        {
            if(port > 15)
                break;
        }

        if((0 == strcmp(board_model, "ZyXEL_GS1900_48"))||
            (0 == strcmp(board_model, "ZyXEL_GS1900_48HP"))||
            (0 == strcmp(board_model, "ZyXEL_GS1900_48HPv2")))
        {
            if(port > 47)
                break;
        }

        if((0 == strcmp(board_model, "ZyXEL_GS1900_24"))||
            (0 == strcmp(board_model, "ZyXEL_GS1900_24HP"))||
            (0 == strcmp(board_model, "ZyXEL_GS1900_24HPv2")))
            (0 == strcmp(board_model, "ZyXEL_GS1920_24")))
        {
            if(port > 23)
                break;
        }

        printf("\r[port:%d] ",port);

        rtk_phy_selfLoop_on(port);

        for(i=0;i<0xF;i++)
        {
            udelay(3000000);

            if(0x1 == rtk_portLinkStatus_get(port))
                break;
        }

        if(i == 0xF)
        {
            printf("could not linkup!\n");
            ret = 1;
            goto test_end;
        }

        NetSendPacket(pkt,sizeof(pkt));

        memset(pkt_buff, 0, sizeof(pkt_buff));

        for(count=0;count<0xF;count++)
        {
        /*ctrl+c to break*/
           if (is_ctrlc())
           {
                printf("\n");
                ret = 1;
                goto test_end;
           }

            udelay(3000000);

            rx_pkt_len = 0;
            rx_pkt_len = NetRecvPacket(&recv_pkt,PORT_LOOPBACK_TEST_PKT_MAX_LEN);

            if(rx_pkt_len != 0)
            {
                ret = OSAL_MEMCMP(pkt,recv_pkt,(rx_pkt_len-4));

                if(ret == 0)
                    break;
                else
                {
                    for (x = 0; x < (rx_pkt_len-4); ++x)
                    {
                        if (x % 10 == 0)
                            printf("\n");
                        printf("%02x ", recv_pkt[x]);
                    }

                    printf("\n");
                    printf("loop back test content failed!\n");
                    ret = 1;
                    goto test_end;
                }
            }
        }

        if(rx_pkt_len == 0)
        {
            printf("\nloop back test failed!\n");
            ret = 1;
            goto test_end;
        }

        rtk_phy_selfLoop_off(port);
    }

test_end:
    return ret;
}
#endif

void run_fc_test(unsigned int detectGPIO)
{
    uint32 htp_gpio_val_sum=0;
    int32 ret_val=0,ret_val_temp=0;

    rtk_rstDeftGpio_init();

    if (detectGPIO && 0 == rtk_htp_detect())
        return;

    ret_val = 0;
    if(htp_gpio_val_sum < RTL8231_GPIO_VALUE_THRD)
    {
        rtk_network_on();
        /*HTP switch status is '1', excute HTP function*/
        printf("============= Factory Test Begin.... =============\n");

        /**************************************************/
        /****************** RST_TO_DEFAULT ***************/
        /**************************************************/
        printf("--- Wait 10s to detect RST....[ctrl+c to skip] ---\n");
        ret_val |= rtk_rstDeft_detect();

        while (1)
        {
#if 0
            /**************************************************/
            /***************** All port loopback test **************/
            /**************************************************/
            printf("------- All port loopback test.... -------\n");
            ret_val_temp = rtk_port_loopback_test();
            ret_val |= ret_val_temp;
            if (0 != ret_val_temp)
                break;
            printf(" -------------[%s]\n",(0 == ret_val_temp)?"Pass":"Failed");
#endif
            /**************************************************/
            /****************** System DRAM test ****************/
            /**************************************************/
            printf("------- System DRAM test.... -------\n");
            ret_val_temp = rtk_dram_test(RTK_FC_DRAM_TEST_ROUND,RTK_FC_DRAM_TEST_ADDR_START,RTK_FC_DRAM_TEST_ADDR_END);
            ret_val |= ret_val_temp;
            if (0 != ret_val_temp)
                break;
            printf(" -------------[%s]\n",(0 == ret_val_temp)?"Pass":"Failed");
#if 0
            /**************************************************/
            /****************** System Flash test ****************/
            /**************************************************/
            printf("------- System Flash test.... -------\n");
            ret_val_temp = rtk_flash_test(RTK_FC_FLASH_TEST_ROUND,RTK_FC_FLASH_TEST_ADDR_START,RTK_FC_FLASH_TEST_ADDR_END);
            ret_val |= ret_val_temp;
            if (0 != ret_val_temp)
                break;
            printf(" -------------[%s]\n",(0 == ret_val_temp)?"Pass":"Failed");
#endif
        }

        printf("========= Factory Test End [Status: %s ]=========\n\n",(0==ret_val)?"PASS":"FAILED");
        rtk_network_off();

        if(ret_val)
        {/*if factory test failed, power off sys led to indicate it*/
            rtk_sys_led_off();
            printf("========= NOTE: HTP test failed, \"ctrl+c\" to continue =========\n\n");
            while(1)
            {
            /*ctrl+c to break*/
               if (is_ctrlc())
               {
                   break;
               }
            }
        }
    }
}


#ifdef CONFIG_GS1900v2
/* New HTP Test Commands */
static void _sys_htp_exit_set(void)
{
    htpBreakIf = 0;
    htpModeIf = 0;

    setenv(ENV_RUNNING_HTP, "0");
    saveenv();

    return;
}

static int _sys_htp_log_set(int entry, u_long rsn, u_long msg)
{
    int i;
    vu_long *base_ptr;
    struct spi_flash * sf;
    u_long log_info[2050];

    for(i=0; i<2050; i++)
    {
        base_ptr = (u_long *)(FLASH_HTP_LOG_ADDR + i*0x4);
        log_info[i] = (*base_ptr);
    }
    flush_cache(0x81000000, 0x10000);
    for(i=0; i<2050; i++)
    {
        base_ptr  = (u_long *)(0x81000000 + i*0x4);
        (*base_ptr) = log_info[i];
    }

    base_ptr  = (u_long *)(0x81000000 + 0x8*entry);
    *base_ptr = rsn;

    base_ptr  = (u_long *)(0x81000000 + 0x8*entry + 0x4);
    *base_ptr = msg;
    flush_cache(0x81000000, 0x10000);

    sf = spi_flash_probe(0, 0, 0, 0);
    if(!sf)
    {
        printf("HTP Test: SPI probe fail when logging!\n");
        return HTP_ERR_FLSH_SPI_PRB;
    }
    else
    {
        base_ptr  = (u_long *)(0x81000000);
        spi_flash_erase(sf, (FLASH_HTP_LOG_ADDR-FLASH_BASE_ADDRESS), 0x10000);
        spi_flash_write(sf, (FLASH_HTP_LOG_ADDR-FLASH_BASE_ADDRESS), 0x10000, (char *)base_ptr);
    }

    setenv(ENV_HAVE_HTP_LOG, "1");
    saveenv();

    return HTP_ERR_NONE;
}

int _sys_htp_info_set(int mode, int brk, int hour, int entry, int times)
{
    int i;
    u_long htpInfo;
    vu_long *base_ptr;
    struct spi_flash * sf;
    u_long log_info[2050];

    for(i=0; i<2050; i++)
    {
        base_ptr = (u_long *)(FLASH_HTP_LOG_ADDR + i*0x4);
        log_info[i] = (*base_ptr);
    }
    flush_cache(0x81000000, 0x10000);
    for(i=0; i<2050; i++)
    {
        base_ptr  = (u_long *)(0x81000000 + i*0x4);
        (*base_ptr) = log_info[i];
    }

    base_ptr  = (u_long *)(0x81000000);
    htpInfo   = ((mode  & 0xf) << 28) |
                ((brk   & 0xf) << 24) |
                ((hour & 0xff) << 16) |
                (entry & 0xffff);
    *base_ptr = htpInfo;

    base_ptr  = (u_long *)(0x81000004);
    htpInfo   = times;
    *base_ptr = htpInfo;
    flush_cache(0x81000000, 0x10000);

    sf = spi_flash_probe(0, 0, 0, 0);
    if(!sf)
    {
        printf("HTP Test: SPI probe fail when logging!\n");
        return HTP_ERR_FLSH_SPI_PRB;
    }
    else
    {
        base_ptr  = (u_long *)(0x81000000);
        spi_flash_erase(sf, (FLASH_HTP_LOG_ADDR-FLASH_BASE_ADDRESS), 0x10000);
        spi_flash_write(sf, (FLASH_HTP_LOG_ADDR-FLASH_BASE_ADDRESS), 0x10000, (char *)base_ptr);
    }

    if (0 == mode)
    {
        setenv(ENV_RUNNING_HTP, "0");
        saveenv();
    }

    return HTP_ERR_NONE;
}

int _sys_htp_info_get(int* mode, int* brk, int* hour, int* entry, int* times)
{
    u_long htpInfo;
    vu_long *base_ptr;
    uint8 *runHTP = getenv(ENV_RUNNING_HTP);
    uint8 *HTPlog = getenv(ENV_HAVE_HTP_LOG);

    base_ptr = (u_long *)(FLASH_HTP_LOG_ADDR);
    htpInfo = (*base_ptr);
    if(NULL == runHTP || '0' == runHTP[0] || 0xffffffff == htpInfo)
    {
        *mode  = 0;
        *brk   = 0;
        *hour  = 0;
        *entry = (NULL == HTPlog || '0' == HTPlog[0]) ? 0 : htpInfo & 0xffff;
        *times = 0;
    }
    else
    {
        *mode  = (htpInfo >> 28);
        *brk   = (htpInfo >> 24) & 0xf;
        *hour  = (htpInfo >> 16) & 0xff;
        *entry = htpInfo & 0xffff;
        base_ptr = (u_long *)(FLASH_HTP_LOG_ADDR + 0x4);
        *times   = (*base_ptr);
    }

    return HTP_ERR_NONE;
}

int sys_htp_enable(void)
{
    htpModeIf = 1;
    rtk_network_on();
    printf("============= Factory Test Begin =============\n");
    return RT_ERR_OK;
}

int sys_htp_clear(void)
{
    struct spi_flash * sf;

    sf = spi_flash_probe(0, 0, 0, 0);
    if(!sf)
    {
        printf("HTP Test: SPI probe fail!\n");
        return HTP_ERR_FLSH_SPI_PRB;
    }
    else
    {
        spi_flash_erase(sf, (FLASH_HTP_LOG_ADDR-FLASH_BASE_ADDRESS), 0x10000);
        printf("HTP Test: All HTP Test log cleared!\n");
    }

    setenv(ENV_HAVE_HTP_LOG, "0");
    saveenv();

    return HTP_ERR_NONE;
}

int sys_htp_print(void)
{
    int i, ret;
    u_long rsn, msg;
    vu_long *base_ptr;
    int mode, brk, rhour, entry, times;

    if((ret = _sys_htp_info_get(&mode, &brk, &rhour, &entry, &times)) != HTP_ERR_NONE)
    {
        printf("HTP Test: Get HTP test log fail!\n");
        return HTP_ERR_GET_INFO;
    }
    printf("  ------------ HTP Testlog show ------------\n");
    if(0 == entry)
    {
        printf("HTP Testlog none!\n");
        printf("  ------------ HTP Testlog End! ------------\n");
        return HTP_ERR_NONE;
    }

    for(i=1; i<=entry; i++)
    {
        base_ptr = (u_long *)(FLASH_HTP_LOG_ADDR + 0x8*i);
        rsn = (*base_ptr);
        base_ptr = (u_long *)(FLASH_HTP_LOG_ADDR + 0x8*i + 4);
        msg = (*base_ptr);

        if(HTP_ERR_NONE == (int)rsn)
            printf("HTP Test %4d: Pass! FAN Speed %u RPM.\n", i, msg);
        else if(HTP_ERR_DRAM_VAL == (int)rsn)
            printf("HTP Test %4d: DDR  Test  - memory 0x%.8lx write/read fail!\n", i, msg);
        else if(HTP_ERR_FLSH_DDR_VAL == (int)rsn)
            printf("HTP Test %4d: Flash test - memory 0x%.8lx write/read fail!\n", i, msg);
        else if(HTP_ERR_FLSH_SPI_PRB == (int)rsn)
            printf("HTP Test %4d: Flash test - SPI probe fail!\n", i);
        else if(HTP_ERR_FLSH_FLS_VAL == (int)rsn)
            printf("HTP Test %4d: Flash test - flash 0x%.8lx write/read fail!\n", i, msg);
        else if(HTP_ERR_PORT_LOP_SET == (int)rsn)
            printf("HTP Test %4d: Loop test  - port %d phy loop set fail!\n", i, (int)msg);
        else if(HTP_ERR_PORT_PKT_VAL == (int)rsn)
            printf("HTP Test %4d: Loop test  - port %d tx/rx fail!\n", i, (int)msg);
        else if(HTP_ERR_BREAK == (int)rsn)
            printf("HTP Test %4d: Break by user!\n", i);
        else if(HTP_ERR_FAN_FAIL == (int)rsn)
            printf("HTP Test %4d: Fan Speed test fail! %u RPM.\n", i, (int)msg);
        else
        {
            printf("HTP Test: Get HTP test log fail!\n");
            printf("  ------------ HTP Testlog End! ------------\n");
            return HTP_ERR_GET_INFO;
        }
    }
    printf("  ------------ HTP Testlog End! ------------\n");
    return HTP_ERR_NONE;
}

int _sys_htp_fan_speed_test(uint32 *speed)
{
    uint32 value, preValue, i, rpm = 0;
#if defined(CONFIG_RTL8380)
    uint32 phy_id = 0;
#endif
#if defined(CONFIG_RTL8390)
    uint32 phy_id = 3;
#endif

    printf("  ------------  FAN Speed Test Start  ------------\n");

    value = preValue = !FAN_GPIO_ACTIVE;

    extGpio_pin_dir_set(phy_id, FAN_EXT_GPIO_PIN, EXT_GPIO_DIR_IN);

    for (i = 0; i < FAN_READ_ROUND; i++)
    {
        extGpio_pin_data_get(phy_id, FAN_EXT_GPIO_PIN, &value);
        if (!FAN_GPIO_ACTIVE == preValue && FAN_GPIO_ACTIVE == value)
            rpm++;

        preValue = value;
    }

    /* One round needs 2 wave */
    rpm /= 2;
    rpm *= 60;

    if (speed)
        *speed = rpm;

    printf("  HTP FAN Speed Test: Speed %u RPM.\n", rpm);

    if (SYS_FAN_MIN_THRESHOLD > rpm || SYS_FAN_MAX_THRESHOLD < rpm)
        return HTP_ERR_FAN_FAIL;

    printf("  ------------  FAN Speed Test END  ------------\n");
    return HTP_ERR_NONE;
}

int _sys_hpt_ddrtest_run(void)
{
    int j, k;
    int i = 0;
    vu_long *base_ptr;
    u_long pattern, rd_value;
    u_long addr_s   = 0x81000000;
    u_long addr_e   = 0x81020000;
    u_long addr_len = addr_e - addr_s;

    printf("  ------------  DDR Test Start  ------------\n");
    for(k=0; k<(addr_len+8); k+=4)
    {
        if(0 == (i%30))
            printf("\r[round:%6d]",i);
        if (tstc())
        {
            if(0x03 == getc())
            {
                printf("\n");
                return HTP_ERR_BREAK;
            }
        }

        if(addr_len == k)
            pattern = 0x55555555;
        else if((addr_len+4) == k)
            pattern = 0xaaaaaaaa;
        else
            pattern = k;

        flush_cache(addr_s, addr_len);
        for(j=0; j<addr_len; j+=8)
        {
            base_ptr = (u_long *)(addr_s + j);
            *base_ptr = pattern;
        }
        flush_cache(addr_s, addr_len);
        for(j=0; j<addr_len; j+=8)
        {
            base_ptr = (u_long *)(addr_s + j);
            rd_value = (*base_ptr);
            if(rd_value != pattern)
            {
                htpFailMsg = (u_long)base_ptr;
                printf("\nMemory addr(0x%.8lx): 0x%.8lx != pattern(0x%.8lx) ;\n",
                        (u_long)base_ptr , rd_value, pattern);
                printf("  ------------  DDR Test Fail!  ------------\n");
                return HTP_ERR_DRAM_VAL;
            }
        }
        i++;
    }
    printf("\n  ------------  DDR Test Pass!  ------------\n");
    return HTP_ERR_NONE;
}

int _sys_htp_fltest_run(void)
{
    int j, k;
    struct spi_flash * sf;

    vu_long *base_ptr;
    u_long rd_value;
    u_long walk_pattern;
    u_long mem_s    = 0x81000000;
    u_long addr_s   = FLASH_HTP_FLASH_TST_START;
    u_long addr_e   = FLASH_HTP_FLASH_TST_END;
    u_long addr_len = addr_e - addr_s;

    printf("  ------------ Flash Test Start ------------\n");
    for(k=0; k<32; k++)
    {
        if (tstc())
        {
            if(0x03 == getc())
            {
                printf("\n");
                return HTP_ERR_BREAK;
            }
        }

        walk_pattern = (1 << k);
        base_ptr = (u_long *)mem_s;
        flush_cache(mem_s, addr_len);
        for(j=0; j<addr_len; j+=8)
        {
            base_ptr = (u_long *)(mem_s + j);
            *base_ptr = walk_pattern;
        }
        flush_cache(mem_s, addr_len);
        for(j=0; j<addr_len; j+=8)
        {
            base_ptr = (u_long *)(mem_s + j);
            rd_value = (*base_ptr);
            if(rd_value != walk_pattern)
            {
                htpFailMsg = (u_long)base_ptr;
                printf("\nMemory addr(0x%.8lx): 0x%.8lx != pattern(0x%.8lx) ;\n",
                        (u_long)base_ptr , rd_value, walk_pattern);
                printf("  ------------ Flash Test Fail! ------------\n");
                return HTP_ERR_FLSH_DDR_VAL;
            }
        }

        base_ptr = (u_long *)mem_s;
        sf = spi_flash_probe(0, 0, 0, 0);
        if(!sf)
        {
            printf("\nFlash Test: SPI probe fail!\n");
            printf("  ------------ Flash Test Fail! ------------\n");
            return HTP_ERR_FLSH_SPI_PRB;
        }
        else
        {
            spi_flash_erase(sf, (addr_s-FLASH_BASE_ADDRESS), addr_len);
            spi_flash_write(sf, (addr_s-FLASH_BASE_ADDRESS), addr_len, (char *)base_ptr);
        }
        base_ptr = (u_long *)(((u_long)addr_s) | 0x20000000);
        for(j=0; j<addr_len; j+=8)
        {
            base_ptr = (u_long *)(addr_s + j);
            rd_value = (*base_ptr);
            if(rd_value != walk_pattern)
            {
                htpFailMsg = (u_long)base_ptr;
                printf("\nFlash addr(0x%.8lx): 0x%.8lx != pattern(0x%.8lx) ;\n",
                        (u_long)base_ptr , rd_value, walk_pattern);
                printf("  ------------ Flash Test Fail! ------------\n");
                return HTP_ERR_FLSH_FLS_VAL;
            }
        }
        printf("\r[round:%d]  Flash: fill pattern(0x%.8lx) from 0x%.8lx to 0x%.8lx passed!", k, walk_pattern, addr_s, addr_e);
    }
    printf("\n  ------------ Flash Test Pass! ------------\n");
    return HTP_ERR_NONE;
}

static int __sys_htp_phyLoop_set(uint32 unit, int port, int enable)
{
    if (ENABLED == enable)
        rtk_phy_selfLoop_on(port);
    else
        rtk_phy_selfLoop_off(port);

    return HTP_ERR_NONE;
}

static void __sys_htp_rx_clear(void)
{
    memset(pkt_buff, 0, sizeof(pkt_buff));

    return;
}

int _sys_htp_looptest_run(void)
{
#if 1
    int x;
    uint32 i;
    uint32 count;
    uint32 port;
    uint32 port_start,port_end;
    uint32 rx_pkt_len=0;
    uchar *recv_pkt;
    char   *board_model;
    uint32 recv = 0;

    board_model = getenv(ENV_BOARD_MODEL);

    if (gSwitchModel == NULL)
        return 1;

    NetInit();

    port_start = gSwitchModel->port.list[0].mac_id;
    port_end = gSwitchModel->port.list[0].mac_id + gSwitchModel->port.count;
    recv_pkt = pkt_buff;

    printf("------------ Port Loop Test Start ------------\n");
    htpModeIf = 1;

    if (0 == strcmp(board_model, "ZyXEL_GS1900_48HPv2"))
        port_end = 48;
    else if (0 == strcmp(board_model, "ZyXEL_GS1900_24HPv2"))
        port_end = 24;


    for(port=port_start;port<port_end;port++)
    {
        if (__sys_htp_phyLoop_set(0, port, ENABLE) != HTP_ERR_NONE)
        {
            printf("------------ Port Loop Test Fail! ------------\n");
            htpModeIf = 0;
            htpFailMsg = (u_long)(port);
            return HTP_ERR_PORT_LOP_SET;
        }

        for (i = 0; i < 0xF; i++)
        {
            udelay(3000000);

            if (0x1 == rtk_portLinkStatus_get(port))
                break;
        }

        if (i == 0xF)
        {
            printf("could not linkup!\n");
            return HTP_ERR_PORT_LOP_SET;
        }

        __sys_htp_rx_clear();
        NetSendPacket(pkt,sizeof(pkt));

        for(count=0;count<0xF;count++)
        {
            /*ctrl+c to break*/
           if (is_ctrlc())
           {
                printf("\n");
                return HTP_ERR_BREAK;
           }

            udelay(3000000);

            rx_pkt_len = 0;
            rx_pkt_len = NetRecvPacket(&recv_pkt, PORT_LOOPBACK_TEST_PKT_MAX_LEN);

            if (0 != rx_pkt_len)
            {
                recv = 1;

                for (x = 0; x < sizeof(pkt); x++)
                {
                    if (pkt[x] != recv_pkt[x])
                    {
                        htpFailMsg = (u_long) (port);
                        printf("port[%d]: Received htpRxPkt[%d] = %d, while sent pkt[%d] = %d!\n", port, x, recv_pkt[x], x, pkt[x]);
                        printf("------------ Port Loop Test Fail! ------------\n");
                        __sys_htp_rx_clear();
                        return HTP_ERR_PORT_PKT_VAL;
                    }
                }

                break;
            }
        }

        if(0 == recv)
        {
            htpFailMsg = (u_long)(port);
            printf("Port[%d] has not received a packet!\n", port);
            printf("------------ Port Loop Test Fail! ------------\n");
            htpModeIf = 0;
            return HTP_ERR_PORT_PKT_VAL;
        }

        __sys_htp_phyLoop_set(0, port, DISABLED);

        printf("Port[%d] loop test ok!\n", port);
    }

#else
    int port, i, j, recv, count, rx_pkt_len;
    int32 rv, unit = 0, port_start, port_end;
    char *board_model;
    uchar *recv_pkt;

    board_model = getenv(ENV_BOARD_MODEL);

    if (gSwitchModel == NULL)
        return 1;

    NetInit();

    port_start = gSwitchModel->port.list[0].mac_id;
    port_end   = gSwitchModel->port.list[0].mac_id + gSwitchModel->port.count;
    recv_pkt = pkt_buff;

    if (0 == strcmp(board_model, "ZyXEL_GS1900_24HPv2"))
        port_end = 24;
    else if (0 == strcmp(board_model, "ZyXEL_GS1900_48HPv2"))
        port_end = 48;

    printf("------------ Port Loop Test Start ------------\n");
    htpModeIf = 1;

    for(port = port_start; port < port_end; port++)
    {
        recv = 0;
        if((rv = __sys_htp_phyLoop_set(unit, port, ENABLED)) != HTP_ERR_NONE)
        {
            printf("------------ Port Loop Test Fail! ------------\n");
            htpModeIf = 0;
            htpFailMsg = (u_long)(port);
            return HTP_ERR_PORT_LOP_SET;
        }

        for (i = 0; i < 0xF; i++)
        {
            udelay(3000000);

            if (0x1 == rtk_portLinkStatus_get(port))
                break;
        }

        if (i == 0xF)
        {
            printf("could not linkup!\n");
            return HTP_ERR_PORT_LOP_SET;
        }

        NetSendPacket(pkt,sizeof(pkt));

        for(count=0; count<0xF; count++)
        {
            /*ctrl+c to break*/
            if (is_ctrlc())
            {
                printf("\n");
                return HTP_ERR_BREAK;
            }

            udelay(3000000);
            rx_pkt_len = 0;
            memset(recv_pkt, 0, PORT_LOOPBACK_TEST_PKT_MAX_LEN);
            rx_pkt_len = NetRecvPacket(&recv_pkt, PORT_LOOPBACK_TEST_PKT_MAX_LEN);

            if (0 != rx_pkt_len)
            {
                recv = 1;

                for (j = 0; j < sizeof(pkt); j++)
                {
                    if (pkt[j] != recv_pkt[j])
                    {
                        htpFailMsg = (u_long) (port);
                        printf("port[%d]: Received htpRxPkt[%d] = %d, while sent pkt[%d] = %d!\n", port, j, recv_pkt[j], j, pkt[j]);
                        printf("------------ Port Loop Test Fail! ------------\n");
                        __sys_htp_rx_clear();
                        return HTP_ERR_PORT_PKT_VAL;
                    }
                }

                break;
            }
        }

        __sys_htp_phyLoop_set(unit, port, DISABLED);

        if(0 == recv)
        {
            htpFailMsg = (u_long)(port);
            printf("Port[%d] has not received a packet!\n", port);
            printf("------------ Port Loop Test Fail! ------------\n");
            htpModeIf = 0;
            return HTP_ERR_PORT_PKT_VAL;
        }
        printf("Port[%d] loop test ok!\n", port);
    }
#endif
    printf("------------ Port Loop Test Pass! ------------\n");
    __sys_htp_rx_clear();
    return 0;

}

int sys_htp_run_case(int hour)
{
    int i, ret;
    /* for now, one round need 4 minutes , so do 15 rounds in one hour */
    int round = 15; // run four per hour
    int rhour, entry, times;
    char   *board_model;
    uint32 fan_speed = 0;

    board_model = getenv(ENV_BOARD_MODEL);

    if (NULL != board_model && 0 == strcmp(board_model, "ZyXEL_GS1900_48HPv2"))
        round = 12; //For 48 Port, requires about 5 minutes per round.

    // Start and get former info
    rtk_sys_led_blink();

    _sys_htp_info_get(&htpModeIf, &htpBreakIf, &rhour, &entry, &times);
    printf("HTP log info get: htpModeIf=%d, htpBreakIf=%d, hour=%d, entry=%d, round=%d !\n", htpModeIf, htpBreakIf, rhour, entry, times);
    printf("Note: log addr %p flash test addr %p. Don't overlap with anything!\n", (void *)FLASH_HTP_LOG_ADDR, (void *)FLASH_HTP_FLASH_TST_START);

    if((1 == htpBreakIf) && (0 == htpModeIf))
    {
        rtk_sys_led_off();
        _sys_htp_exit_set();
        printf("Get HTP log fail!\n");
        printf("============= Factory Test End ! =============\n");
        return HTP_ERR_GET_INFO;
    }
    else if(0 == htpBreakIf)
    {
        rhour      = hour;
        entry      = entry + 1;
        times      = 0;
        //sys_htp_info_set(htpModeIf, htpBreakIf, rhour, entry, times);
    }
    htpModeIf  = 1;
    htpBreakIf = 1;

    if(entry > 2000)
    {
        _sys_htp_exit_set();
        printf("HTP Test log full!\n");
        printf("============= Factory Test End ! =============\n");
        return HTP_ERR_LOG_FULL;
    }

    setenv(ENV_RUNNING_HTP, "1");
    saveenv();

    // main test loop
    for(i=times; i<(round*rhour); i++)
    {
        _sys_htp_info_set(htpModeIf, htpBreakIf, rhour, entry, i);
        if((ret = _sys_hpt_ddrtest_run()) != HTP_ERR_NONE)
        {
            if(ret != HTP_ERR_BREAK)
                rtk_sys_led_off();
            _sys_htp_log_set(entry, (u_long)ret, htpFailMsg);
            htpFailMsg = 0;
            _sys_htp_exit_set();
            _sys_htp_info_set(htpModeIf, htpBreakIf, 0, entry, 0);
            printf("============= Factory Test End ! =============\n");
            return ret;
        }
        if((ret = _sys_htp_fltest_run()) != HTP_ERR_NONE)
        {
            if(ret != HTP_ERR_BREAK)
                rtk_sys_led_off();
            _sys_htp_log_set(entry, (u_long)ret, htpFailMsg);
            htpFailMsg = 0;
            _sys_htp_exit_set();
            _sys_htp_info_set(htpModeIf, htpBreakIf, 0, entry, 0);
            printf("============= Factory Test End ! =============\n");
            return ret;
        }
        if((ret = _sys_htp_looptest_run()) != HTP_ERR_NONE)
        {
            if(ret != HTP_ERR_BREAK)
                rtk_sys_led_off();
            _sys_htp_log_set(entry, (u_long)ret, htpFailMsg);
            htpFailMsg = 0;
            _sys_htp_exit_set();
            _sys_htp_info_set(htpModeIf, htpBreakIf, 0, entry, 0);
            printf("============= Factory Test End ! =============\n");
            return ret;
        }
    }

    /* Finally, check FAN speed */
    if (HTP_ERR_NONE != _sys_htp_fan_speed_test(&fan_speed))
    {
        rtk_sys_led_off();
        _sys_htp_log_set(entry, HTP_ERR_FAN_FAIL, fan_speed);
        _sys_htp_exit_set();
        _sys_htp_info_set(htpModeIf, htpBreakIf, 0, entry, 0);
        printf("============= Factory Test End ! =============\n");
        return ret;
    }

    //End and settings
    htpFailMsg = 0;
    _sys_htp_log_set(entry, HTP_ERR_NONE, fan_speed);
    _sys_htp_exit_set();
    _sys_htp_info_set(htpModeIf, htpBreakIf, 0, entry, 0);
    rtk_sys_led_on();
    printf("============= Factory Test End ! =============\n");

    return HTP_ERR_NONE;
}
#endif /* GS1900v2, New HTP command */

/*
 * Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>

#include <environment.h>
#include <spi_flash.h>

#include <image.h>
#include <rtk_flash_common.h>
#include <turnkey/sysinfo.h>

#ifndef CONFIG_ENV_SPI_BUS
#define CONFIG_ENV_SPI_BUS 0
#endif
#ifndef CONFIG_ENV_SPI_CS
#define CONFIG_ENV_SPI_CS  0
#endif
#ifndef CONFIG_ENV_SPI_MAX_HZ
#define CONFIG_ENV_SPI_MAX_HZ  1000000
#endif
#ifndef CONFIG_ENV_SPI_MODE
#define CONFIG_ENV_SPI_MODE    SPI_MODE_3
#endif

#define PARTITION_LOADER        CONFIG_SYS_FLASH_BASE
#define PARTITION_LOADER_END    PARTITION_LOADER + LOADER_SIZE - 1
#define PARTITION_BDINFO        CONFIG_SYS_FLASH_BASE + LOADER_BDINFO_START
#define PARTITION_BDINFO_END    PARTITION_BDINFO + LOADER_BDINFO_SIZE - 1
#define PARTITION_SYSINFO       CONFIG_SYS_FLASH_BASE + SYSINFO_START
#define PARTITION_SYSINFO_END   PARTITION_SYSINFO + SYSINFO_SIZE - 1
#define PARTITION_RUNTIME1      CONFIG_SYS_FLASH_BASE + KERNEL_START
#define PARTITION_RUNTIME1_END  PARTITION_RUNTIME1 + KERNEL_SIZE - 1
#if !defined(CONFIG_FLASH_LAYOUT_GS1920)
#define PARTITION_JFFS2_CFG     CONFIG_SYS_FLASH_BASE + JFFS2_CFG_START
#define PARTITION_JFFS2_CFG_END PARTITION_JFFS2_CFG + JFFS2_CFG_SIZE - 1
#define PARTITION_JFFS2_LOG     CONFIG_SYS_FLASH_BASE + JFFS2_LOG_START
#define PARTITION_JFFS2_LOG_END PARTITION_JFFS2_LOG + JFFS2_LOG_SIZE - 1
#ifdef CONFIG_DUAL_IMAGE
#define PARTITION_RUNTIME2      CONFIG_SYS_FLASH_BASE + KERNEL2_START
#define PARTITION_RUNTIME2_END  PARTITION_RUNTIME2 + KERNEL2_SIZE - 1
#endif
#endif

typedef struct partition_s {
    char name[16];
    u32 addr_start;
    u32 addr_end;
    u32 addr;
    u32 size;
} partition_t;

const partition_t partitions[] =
{
    {"LOADER",    PARTITION_LOADER,    PARTITION_LOADER_END,    LOADER_START,        LOADER_SIZE},
    {"BDINFO",    PARTITION_BDINFO,    PARTITION_BDINFO_END,    LOADER_BDINFO_START, LOADER_BDINFO_SIZE},
    {"SYSINFO",   PARTITION_SYSINFO,   PARTITION_SYSINFO_END,   SYSINFO_START,       SYSINFO_SIZE},
    {"RUNTIME1",  PARTITION_RUNTIME1,  PARTITION_RUNTIME1_END,  KERNEL_START,        KERNEL_SIZE},
#if !defined(CONFIG_FLASH_LAYOUT_GS1920)
    {"JFFS2_CFG", PARTITION_JFFS2_CFG, PARTITION_JFFS2_CFG_END, JFFS2_CFG_START,     JFFS2_CFG_SIZE},
    {"JFFS2_LOG", PARTITION_JFFS2_LOG, PARTITION_JFFS2_LOG_END, JFFS2_LOG_START,     JFFS2_LOG_SIZE},
#ifdef CONFIG_DUAL_IMAGE
    {"RUNTIME2",  PARTITION_RUNTIME2,  PARTITION_RUNTIME2_END,  KERNEL2_START,       KERNEL2_SIZE},
#endif
#endif
    {"", 0, 0}
};

static struct spi_flash *env_flash = NULL;

static int _flashErase(u32 flash_addr, size_t size)
{
    int ret = 0;

    if (!env_flash) {
        env_flash = spi_flash_probe(CONFIG_ENV_SPI_BUS,
                                    CONFIG_ENV_SPI_CS,
                                    CONFIG_ENV_SPI_MAX_HZ,
                                    CONFIG_ENV_SPI_MODE);
        if (!env_flash) {
            set_default_env("!spi_flash_probe() failed");
            return 1;
        }
    }

    //puts("Erasing SPI flash...");
    ret = spi_flash_erase(env_flash, flash_addr, size);
    if (ret)
    {
        //puts("FAIL\n");
        return ret;
    }

    ret = 0;
    //puts("DONE\n");

    return ret;
}

int do_parterase (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret = 0;
    int partition = -1;
    int partsize = sizeof(partitions)/sizeof(partition_t) - 1;

    if (argc != 3)
        return cmd_usage(cmdtp);

    partition = simple_strtoul(argv[2], NULL, 10);

    if (0 == partition)
    {
        printf("Forbid to erase LOADER partition\n");
        return 0;
    }
    else if ((partition < 0) || (partition >= partsize))
    {
        printf("Partition %d is not exist\n", partition);
        return 0;
    }

    ret = _flashErase(partitions[partition].addr, partitions[partition].size);

    return ret;
}

U_BOOT_CMD(
    flerase, 3, 0, do_parterase,
    "Erase flash partition",
    "index [index ...]\n"
    "    - Erase partition. Use flshow to get index of partition."
);


int do_partshow (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret = 0;
    int i = 0;

    printf("=============== FLASH Partition Layout ===============\n");
    printf("Index  Name       Size       Address\n");
    printf("------------------------------------------------------\n");
    while (strlen(partitions[i].name) > 0)
    {
        printf(" %-4d  %-10s 0x%-8x 0x%x-0x%x\n", i, partitions[i].name, partitions[i].size, partitions[i].addr_start, partitions[i].addr_end);
        i++;
    }
    printf("======================================================\n");

    return ret;
}

U_BOOT_CMD(
    flshow, 1, 0, do_partshow,
    "Show flash partition layout",
    ""
);

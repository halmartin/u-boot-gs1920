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

#define FILE_SOURCE_ADDR      0x81000000
#define FILE_SOURCE_ADDR_STR  "0x81000000"

#define DFLT_RUNTIME_FILENAME  "vmlinux.bix"
#define DFLT_LOADER_FILENAME   "u-boot.bin"
#define DFLT_ROM_FILENAME      "rom.bix"

#ifdef CONFIG_DUAL_IMAGE
#define ROM_SIZE (LOADER_SIZE + LOADER_BDINFO_SIZE + SYSINFO_SIZE + JFFS2_CFG_SIZE + JFFS2_LOG_SIZE + KERNEL_SIZE + KERNEL2_SIZE)
#else
#define ROM_SIZE (LOADER_SIZE + LOADER_BDINFO_SIZE + SYSINFO_SIZE + JFFS2_CFG_SIZE + JFFS2_LOG_SIZE + KERNEL_SIZE)
#endif

#ifdef CONFIG_DUAL_IMAGE
#define UPGRADE_ACTION_USAGE \
    "loader [FILENAME]\n" \
    "    - upgrade loader with image validation, such as chip family, size and CRC.\n" \
    "    - FILENAME: Specify loader file name on remote TFTP server.\n" \
    "                If not specified, '" DFLT_LOADER_FILENAME "' will be used\n" \
    "upgrade loaderforce [FILENAME]\n" \
    "    - upgrade loader without image validation.\n" \
    "    - FILENAME: Specify loader file name on remote TFTP server.\n" \
    "                If not specified, '" DFLT_LOADER_FILENAME "' will be used\n" \
    "upgrade runtime [FILENAME]\n" \
    "    - upgrade runtime image to first runtime partition.\n" \
    "    - FILENAME: Specify runtime file name on remote TFTP server.\n" \
    "                If not specified, '" DFLT_RUNTIME_FILENAME "' will be used\n" \
    "upgrade runtime2 [FILENAME]\n" \
    "    - upgrade runtime image to second runtime partition.\n" \
    "    - FILENAME: Specify runtime file name on remote TFTP server.\n" \
    "                If not specified, '" DFLT_RUNTIME_FILENAME "' will be used\n" \
    "upgrade rom [FILENAME]\n" \
    "    - upgrade loader with image validation, such as chip family, size and CRC." \
    "      this command will skip BDINFO and SYSINFO partition\n" \
    "    - FILENAME: Specify rom file name on remote TFTP server.\n" \
    "                If not specified, '" DFLT_ROM_FILENAME "' will be used\n" \
    "upgrade romforce [FILENAME]\n" \
    "    - upgrade whole rom image without image validation.\n" \
    "    - FILENAME: Specify rom file name on remote TFTP server.\n" \
    "                If not specified, '" DFLT_ROM_FILENAME "' will be used\n"
#else
#define UPGRADE_ACTION_USAGE \
    "loader [FILENAME]\n" \
    "    - upgrade loader with image validation, such as chip family, size and CRC.\n" \
    "    - FILENAME: Specify loader file name on remote TFTP server.\n" \
    "                If not specified, '" DFLT_LOADER_FILENAME "' will be used\n" \
    "upgrade loaderforce [FILENAME]\n" \
    "    - upgrade loader without image validation.\n" \
    "    - FILENAME: Specify loader file name on remote TFTP server.\n" \
    "                If not specified, '" DFLT_LOADER_FILENAME "' will be used\n" \
    "upgrade runtime [FILENAME]\n" \
    "    - upgrade runtime image to flash partition.\n" \
    "    - FILENAME: Specify runtime file name on remote TFTP server.\n" \
    "                If not specified, '" DFLT_RUNTIME_FILENAME "' will be used\n" \
    "upgrade rom [FILENAME]\n" \
    "    - upgrade loader with image validation, such as chip family, size and CRC." \
    "      this command will skip BDINFO and SYSINFO partition\n" \
    "    - FILENAME: Specify rom file name on remote TFTP server.\n" \
    "                If not specified, '" DFLT_ROM_FILENAME "' will be used\n" \
    "upgrade romforce [FILENAME]\n" \
    "    - upgrade whole rom image without image validation.\n" \
    "    - FILENAME: Specify rom file name on remote TFTP server.\n" \
    "                If not specified, '" DFLT_ROM_FILENAME "' will be used\n"
#endif

static struct spi_flash *env_flash = NULL;

static int _rtkNetworkOn(void)
{
    cmd_tbl_t *run;
    char *argu[4];

    run = find_cmd("rtk");
    if (NULL == run) {
        printf("rtk command is not supported!\n");
        return 1;
    }
    argu[0] = "rtk";
    argu[1] = "network";
    argu[2] = "on";
    argu[3] = NULL;

    return run->cmd(run, 0, 3, argu);
}

static int _tftpDownloadFile(char *filename)
{
    cmd_tbl_t *run;
    char *argu[4];

    if (_rtkNetworkOn())
        return 1;

    run = find_cmd("tftp");
    if (NULL == run) {
        printf("tftp command is not supported!\n");
        return 1;
    }
    argu[0] = "tftp";
    argu[1] = FILE_SOURCE_ADDR_STR;
    argu[2] = filename;
    argu[3] = NULL;

    return run->cmd(run, 0, 3, argu);
}

static int _compare(u32 flash_addr, size_t size, u32 offset)
{
    cmd_tbl_t *run;
    char *argu[5];
    char addrStr[16];
    char sizeStr[32];
    char fileSrcStr[16];

    run = find_cmd("cmp.b");
    if (NULL == run) {
        printf("cmp.b command is not supported!\n");
        return 1;
    }

    printf("Comparing file ......\n");
    memset(addrStr, 0, sizeof(addrStr));
    memset(sizeStr, 0, sizeof(sizeStr));
    memset(fileSrcStr, 0, sizeof(fileSrcStr));

    sprintf(addrStr, "%x", flash_addr + CONFIG_SYS_FLASH_BASE);
    sprintf(sizeStr, "%x", size);
    sprintf(fileSrcStr, "%x", FILE_SOURCE_ADDR + offset);

    argu[0] = "cmp.b";
    argu[1] = fileSrcStr;
    argu[2] = addrStr;
    argu[3] = sizeStr;
    argu[4] = NULL;

    return run->cmd(run, 0, 4, argu);
}

static int _write2flash(u32 flash_addr, size_t size, u32 offset)
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

    //puts("Writing to SPI flash...");
    ret = spi_flash_write(env_flash, flash_addr, size, (void*)(FILE_SOURCE_ADDR + offset));
    if (ret)
    {
        //puts("FAIL\n");
        return ret;
    }

    ret = 0;
    //puts("DONE\n");

    if (_compare(flash_addr, size, offset))
    {
        return 1;
    }

    return ret;
}

static uint32_t _chipFamily(void)
{
#ifdef CONFIG_RTL839XS
    return LOADER_TAIL_CHIP_839X;
#endif
#ifdef CONFIG_RTL838XS
    return LOADER_TAIL_CHIP_838X;
#endif
}

static int _loaderCheck(char *loader)
{
    int size = LOADER_SIZE;
    u32 crc = 0;
    u32 tcrc = 0;
    u32 chip = _chipFamily();
    char tailKey[8];
    char *ptr = loader;
    char *pch = NULL;
    loader_tail_t tail;
    loader_tail_t tmp_tail;

    tailKey[0] = '#';
    tailKey[1] = '-';
    tailKey[2] = 'T';
    tailKey[3] = 'A';
    tailKey[4] = 'I';
    tailKey[5] = 'L';
    tailKey[6] = '-';
    tailKey[7] = '#';

    while (size > 0)
    {
        if (NULL != (pch = memchr(ptr, '#', 1)))
        {
            if (0 == memcmp(pch, tailKey, sizeof(tailKey)))
            {
                memset(&tail, 0, sizeof(loader_tail_t));
                memset(&tmp_tail, 0, sizeof(loader_tail_t));
                memcpy(&tail, pch, sizeof(loader_tail_t));
                memcpy(&tmp_tail, pch, sizeof(loader_tail_t));

                tmp_tail.lt_tcrc = 0;
                /* Calculate loader tail crc */
                tcrc = crc32(0, (unsigned char*)&tmp_tail, sizeof(loader_tail_t));
                crc = crc32(0, (unsigned char*)loader, (pch - loader));

                printf("  Loader Chip: %x\n", tail.lt_chip);
                printf("  Loader CRC: %x\n", tail.lt_crc);
                printf("  Loader Size: %x\n", tail.lt_size);
                printf("  Loader Tail CRC: %x\n", tail.lt_tcrc);

                if (tail.lt_chip != chip)
                {
                    printf("\nCurrent Loader Chip Family %x\n", chip);
                    return 1;
                }

                if (tail.lt_size != (pch - loader))
                {
                    printf("\nCalculate Loader Size %x\n", (pch - loader));
                    return 1;
                }

                if (tail.lt_crc != crc)
                {
                    printf("\nCalculate Loader CRC %x\n", crc);
                    return 1;
                }

                if (tcrc != tail.lt_tcrc)
                {
                    printf("\nCalculate Loader Tail CRC %x\n", tcrc);
                    return 1;
                }

                return 0;
            }
        }

        ptr += 1;
        size -= 1;
    }

    return 1;
}

int upgrade_loader (char *filename, int force)
{
    int ret = 0;

    printf("Upgrade loader image [%s]......\n", filename);

    /* Reset memory value to be the same as flash erased value */
    memset((void*)FILE_SOURCE_ADDR, 0xff, LOADER_SIZE);

    if (_tftpDownloadFile(filename))
    {
        printf("Download loader image [%s] failed\n", filename);
        return 1;
    }

    if (!force)
    {
        if (_loaderCheck((void*)FILE_SOURCE_ADDR))
        {
            printf("Invalid loader image [%s]\n", filename);
            return 1;
        }
    }

    if (_write2flash(LOADER_START, LOADER_SIZE, 0))
    {
        printf("Write loader image [%s] failed\n", filename);
        return 1;
    }

    printf("Upgrade loader image [%s] success\n", filename);

    return ret;
}

int upgrade_runtime (char *filename, int partition)
{
    int ret = 0;
    char partitionStr[16];
    void *hdr = (void *)FILE_SOURCE_ADDR;

    printf("Upgrade runtime image [%s]......\n", filename);

    /* Reset memory value to be the same as flash erased value */
    memset((void*)FILE_SOURCE_ADDR, 0xff, KERNEL_SIZE);

    /* Download runtime file */
    if (_tftpDownloadFile(filename))
    {
        printf("Download runtime image [%s] failed\n", filename);
        return 1;
    }

    /* Check Image header CRC */
    if (!image_check_hcrc(hdr)) {
        printf("Bad Header Checksum\n");
        return 1;
    }

    image_print_contents(hdr);

    /* Check Image CRC */
    if (!image_check_dcrc(hdr)) {
        printf("Bad Data CRC\n");
        return 1;
    }

    /* Write to partition */
    memset(partitionStr, 0, sizeof(partitionStr));
    if (0 == partition)
    {
#ifdef CONFIG_DUAL_IMAGE
        strcpy(partitionStr, "to partition 0 ");
#endif
        if (_write2flash(KERNEL_START, KERNEL_SIZE, 0))
        {
            printf("Write runtime image [%s] %sfailed\n", filename, partitionStr);
            return 1;
        }
    }
#ifdef CONFIG_DUAL_IMAGE
    else if (1 == partition)
    {
        strcpy(partitionStr, "to partition 1 ");
        if (_write2flash(KERNEL2_START, KERNEL2_SIZE, 0))
        {
            printf("Write runtime image [%s] %sfailed\n", filename, partitionStr);
            return 1;
        }
    }
#endif

    printf("Upgrade runtime image [%s] %ssuccess\n", filename, partitionStr);

    return ret;
}

int upgrade_rom (char *filename, int force)
{
    int ret = 0;

    printf("Upgrade rom image [%s]......\n", filename);

    /* Reset memory value to be the same as flash erased value */
    memset((void*)FILE_SOURCE_ADDR, 0xff, ROM_SIZE);

    if (_tftpDownloadFile(filename))
    {
        printf("Download rom image [%s] failed\n", filename);
        return 1;
    }

    if (!force)
    {
        int offset = 0;

        if (_loaderCheck((void*)FILE_SOURCE_ADDR))
        {
            printf("Invalid rom image [%s]\n", filename);
            return 1;
        }

        if (_write2flash(LOADER_START, LOADER_SIZE, 0))
        {
            printf("Write loader image [%s] failed\n", filename);
            return 1;
        }

        offset = (LOADER_SIZE + LOADER_BDINFO_SIZE + SYSINFO_SIZE);
        if (_write2flash(JFFS2_CFG_START, (ROM_SIZE - offset), offset))
        {
            printf("Write jffs2 and runtime image [%s] failed\n", filename);
            return 1;
        }
    }
    else
    {
        if (_write2flash(LOADER_START, ROM_SIZE, 0))
        {
            printf("Write rom image [%s] failed\n", filename);
            return 1;
        }
    }

    printf("Upgrade rom image [%s] success\n", filename);

    return ret;
}


int do_upgrade (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    char filename[256];

    memset(filename, 0, sizeof(filename));

    if (argc <= 1)
    {
        return cmd_usage(cmdtp);
    }
    else
    {
        if (0 == strcmp(argv[1], "loader"))
        {
            if (3 == argc)
                strncpy(filename, argv[2], sizeof(filename));
            else
                strcpy(filename, DFLT_LOADER_FILENAME);

            upgrade_loader(filename, 0);
        }
        else if (0 == strcmp(argv[1], "loaderforce"))
        {
            if (3 == argc)
                strncpy(filename, argv[2], sizeof(filename));
            else
                strcpy(filename, DFLT_LOADER_FILENAME);

            upgrade_loader(filename, 1);
        }
        else if (0 == strcmp(argv[1], "runtime"))
        {
            if (3 == argc)
                strncpy(filename, argv[2], sizeof(filename));
            else
                strcpy(filename, DFLT_RUNTIME_FILENAME);

            upgrade_runtime(filename, 0);
        }
        else if (0 == strcmp(argv[1], "runtime2"))
        {
            if (3 == argc)
                strncpy(filename, argv[2], sizeof(filename));
            else
                strcpy(filename, DFLT_RUNTIME_FILENAME);

            upgrade_runtime(filename, 1);
        }
        else if (0 == strcmp(argv[1], "rom"))
        {
            if (3 == argc)
                strncpy(filename, argv[2], sizeof(filename));
            else
                strcpy(filename, DFLT_ROM_FILENAME);

            upgrade_rom(filename, 0);
        }
        else if (0 == strcmp(argv[1], "romforce"))
        {
            if (3 == argc)
                strncpy(filename, argv[2], sizeof(filename));
            else
                strcpy(filename, DFLT_ROM_FILENAME);

            upgrade_rom(filename, 1);
        }
        else
        {
            return cmd_usage(cmdtp);
        }
    }

    return 0;
}

U_BOOT_CMD(
    upgrade, 3, 0, do_upgrade,
    "Upgrade loader or runtime image",
    UPGRADE_ACTION_USAGE
);


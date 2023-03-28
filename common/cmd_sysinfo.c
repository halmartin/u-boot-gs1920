/*
 * (C) Copyright 2000-2009
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


/**************************************************************************
 *
 * Support for persistent environment data
 *
 * The "environment" is stored as a list of '\0' terminated
 * "name=value" strings. The end of the list is marked by a double
 * '\0'. New entries are always added at the end. Deleting an entry
 * shifts the remaining entries to the front. Replacing an entry is a
 * combination of deleting the old value and adding the new one.
 *
 * The environment is preceeded by a 32 bit CRC over the data part.
 *
 **************************************************************************
 */
#include <common.h>
#include <environment.h>
#include <malloc.h>
#include <spi_flash.h>
#include <search.h>
#include <errno.h>
#include <rtk_flash_common.h>
#include <turnkey/sysinfo.h>

/************************************************************************
************************************************************************/
#define PARTITION_SYSINFO     CONFIG_SYS_FLASH_BASE + SYSINFO_START
#define PARTITION_SYSINFO_END PARTITION_SYSINFO + SYSINFO_SIZE - 1

#ifndef CONFIG_ENV_SPI_BUS
# define CONFIG_ENV_SPI_BUS 0
#endif
#ifndef CONFIG_ENV_SPI_CS
# define CONFIG_ENV_SPI_CS  0
#endif
#ifndef CONFIG_ENV_SPI_MAX_HZ
# define CONFIG_ENV_SPI_MAX_HZ  1000000
#endif
#ifndef CONFIG_ENV_SPI_MODE
# define CONFIG_ENV_SPI_MODE    SPI_MODE_3
#endif

#ifdef CONFIG_ENV_OFFSET_REDUND
static ulong env_offset		= CONFIG_ENV_OFFSET;
static ulong env_new_offset	= CONFIG_ENV_OFFSET_REDUND;

#define ACTIVE_FLAG     1
#define OBSOLETE_FLAG   0
#endif /* CONFIG_ENV_OFFSET_REDUND */

static struct spi_flash *env_flash = NULL;

static int sys_envmatch (uchar *, int);

char *glb_sysinfo_addr = NULL;

/************************************************************************
************************************************************************/
void sysinfo_mem_init(void)
{
    glb_sysinfo_addr = malloc(SYSINFO_ENV_SIZE);
    memcpy(glb_sysinfo_addr, (const void *)PARTITION_SYSINFO, SYSINFO_ENV_SIZE);
}

/************************************************************************
 * Command interface: print one or all environment variables
 */

int do_printsys (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    int i, j, k, nxt;
    int rcode = 0;
    char *data_ptr;
    int crc;
    int calcrc;

    if (glb_sysinfo_addr == NULL)
    {
        sysinfo_mem_init();
    }

    crc = ((glb_sysinfo_addr[0] & 0xFF) << 24) + ((glb_sysinfo_addr[1] & 0xFF) << 16)
           + ((glb_sysinfo_addr[2] & 0xFF) << 8) + (glb_sysinfo_addr[3] & 0xFF);

    calcrc = crc32(0, (unsigned char*)glb_sysinfo_addr + 4, SYSINFO_ENV_SIZE - 4);
    if (crc != calcrc)
    {
        printf("Invalid system information\n");
        return 0;
    }

    data_ptr = glb_sysinfo_addr + 4; /* Skip CRC field */

    if (argc == 1) {        /* Print all env variables  */
        for (i=0; *(data_ptr+i) != '\0'; i=nxt+1)
        {
            for (nxt=i; *(data_ptr+nxt) != '\0'; ++nxt)
                ;
            for (k=i; k<nxt; ++k)
                putc(*(data_ptr+k));
            putc  ('\n');

            if (ctrlc()) {
                puts ("\n ** Abort\n");
                return 1;
            }
        }

        printf("System information size: %d/%d bytes\n", i, SYSINFO_ENV_SIZE);

        return 0;
    }

    for (i=1; i<argc; ++i) /* print single env variables   */
    {
        char *name = argv[i];

        k = -1;

        for (j=0; *(data_ptr+j) != '\0'; j=nxt+1) {

            for (nxt=j; *(data_ptr+nxt) != '\0'; ++nxt)
                ;
            k = sys_envmatch((uchar *)name, j);
            if (k < 0) {
                continue;
            }
            puts (name);
            putc ('=');
            while (k < nxt)
            {
                putc(*(data_ptr+k));
                k++;
            }
            putc ('\n');
            break;
        }
        if (k < 0) {
            printf ("## Error: \"%s\" not defined\n", name);
            rcode ++;
        }
    }
    return rcode;
}

/************************************************************************
 * Set a new environment variable,
 * or replace or delete an existing one.
 *
 * This function will ONLY work with a in-RAM copy of the environment
 */

int _do_setsys (int flag, int argc, char *argv[])
{
    int size = 0;
    char *buf;
    char *data_ptr;
    char *data_ptr2;
    char *varname;
    char *varname_end = NULL;
    int varval = 0;
    int i = 0, nxt = 0, j =0;
    int crc;
    int varSet = 0;
    int varExist = 0;
    int idx = 0;
    char *name;

    size = SYSINFO_ENV_SIZE;

    if (glb_sysinfo_addr == NULL)
    {
        sysinfo_mem_init();
    }

    buf = malloc(size);
    memset(buf, 0, size);
    data_ptr = glb_sysinfo_addr + 4; /* Skip CRC field */
    data_ptr2 = buf + 4; /* Skip CRC field */
    name = argv[1];

    for (i = 0; *(data_ptr + i) != '\0'; i = nxt + 1)
    {
        varname = data_ptr + i;
        varval = 0;
        varExist = 0;
        for (nxt = i; *(data_ptr + nxt) != '\0'; ++nxt)
        {
            if (nxt > size)
            {
                free(buf);
                return 0;
            }

            if (*(data_ptr + nxt) == '=')
            {
                if (0 == strncmp(name, varname, nxt - i))
                {
                    varname_end = data_ptr + nxt;
                    varExist = 1;
                }
                varval = nxt + 1;
            }
        }

        if (varval != 0 && varExist == 0)
        {
            strncpy((data_ptr2 + idx), varname, (data_ptr + nxt) - varname);
            idx += (data_ptr + nxt) - varname;
            data_ptr2[idx] = '\0';
            idx++;
        }
        else if (varval != 0 && varExist == 0)
        {
            if (argc > 2)
            {
                strncpy((data_ptr2 + idx), varname, varname_end - varname);
                idx += varname_end - varname;
                data_ptr2[idx] = '=';
                idx++;
                for (j = 2; j < argc; j++)
                {
                    strcpy((data_ptr2 + idx), argv[j]);
                    idx += strlen(argv[j]);
                    data_ptr2[idx] = ' ';
                    idx++;
                }
                idx--;
                data_ptr2[idx] = '\0';
            }
            varSet = 1;
        }
    }

    if (0 == varSet && argc > 2)
    {
        strncpy((data_ptr2 + idx), name, strlen(name));
        idx += strlen(name);
        data_ptr2[idx] = '=';
        idx++;
        for (j = 2; j < argc; j++)
        {
            strcpy((data_ptr2 + idx), argv[j]);
            idx += strlen(argv[j]);
            data_ptr2[idx] = ' ';
            idx++;
        }
        idx--;
        data_ptr2[idx] = '\0';
    }

    /* Update CRC field */
    crc = crc32(0, (unsigned char*)data_ptr2, size - 4);
    buf[0] = (crc >> 24) & 0xFF;
    buf[1] = (crc >> 16) & 0xFF;
    buf[2] = (crc >> 8) & 0xFF;
    buf[3] = crc & 0xFF;

    memcpy(glb_sysinfo_addr, buf, SYSINFO_ENV_SIZE);

    free(buf);

    return 0;
}

void setsys (char *varname, char *varvalue)
{
    char *argv[4] = { "setenv", varname, varvalue, NULL };
    if (varvalue == NULL)
        _do_setsys (0, 2, argv);
    else
        _do_setsys (0, 3, argv);
}

int do_setsys (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    if (argc < 2) {
        printf ("Usage:\n%s\n", cmdtp->usage);
        return 1;
    }

    return _do_setsys (flag, argc, argv);
}

/************************************************************************
 * Prompt for environment variable
 */


/************************************************************************
 * Look up variable from environment,
 * return address of storage for that variable,
 * or NULL if not found
 */

char *getsys (char *name)
{
    int i, nxt;
    char *data_ptr;

    if (glb_sysinfo_addr == NULL)
    {
        sysinfo_mem_init();
    }

    data_ptr = glb_sysinfo_addr + 4; /* Skip CRC field */

    //WATCHDOG_RESET();

    for (i=0; data_ptr[i] != '\0'; i=nxt+1) {
        int val;

        for (nxt=i; data_ptr[nxt] != '\0'; ++nxt) {
            if (nxt >= SYSINFO_ENV_SIZE) {
                return (NULL);
            }
        }
        if ((val=sys_envmatch((uchar *)name, i)) < 0)
            continue;
        return ((char *)data_ptr+val);
    }
    return (NULL);
}

int savesys(void)
{
    int ret = 1;

    if (!env_flash) {
        env_flash = spi_flash_probe(CONFIG_ENV_SPI_BUS,
            CONFIG_ENV_SPI_CS,
            CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE);
        if (!env_flash) {
            set_default_env("!spi_flash_probe() failed");
            return 1;
        }
    }

//    puts("Erasing SPI flash...");
    ret = spi_flash_erase(env_flash, SYSINFO_START, SYSINFO_ENV_SIZE);
    if (ret)
    {
        puts("fail\n");
        return ret;
    }

//    puts("Writing to SPI flash...");
    ret = spi_flash_write(env_flash, SYSINFO_START, SYSINFO_ENV_SIZE, glb_sysinfo_addr);
    if (ret)
    {
        puts("fail\n");
        return ret;
    }

    ret = 0;
//    puts("done\n");

    return ret;
}

int do_savesys(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    return (savesys());
}

/************************************************************************
 * Match a name / name=value pair
 *
 * s1 is either a simple 'name', or a 'name=value' pair.
 * i2 is the environment index for a 'name2=value2' pair.
 * If the names match, return the index for the value2, else NULL.
 */
static int
sys_envmatch (uchar *s1, int i2)
{
    char *data_ptr;

    data_ptr = glb_sysinfo_addr + 4; /* Skip CRC field */
    while (*s1 == data_ptr[i2++])
        if (*s1++ == '=')
            return(i2);
    if (*s1 == '\0' && data_ptr[i2-1] == '=')
        return(i2);
    return(-1);
}


/**************************************************/

U_BOOT_CMD(
    printsys, CONFIG_SYS_MAXARGS, 1,   do_printsys,
    "printsys - print system information variables\n",
    "\n    - print values of all system information variables\n"
    "printenv name ...\n"
    "    - print value of system information variable 'name'\n"
);

U_BOOT_CMD(
    setsys, CONFIG_SYS_MAXARGS, 0, do_setsys,
    "setsys  - set system information variables\n",
    "name value ...\n"
    "    - set system information variable 'name' to 'value ...'\n"
    "setenv name\n"
    "    - delete system information variable 'name'\n"
);

U_BOOT_CMD(
    savesys, 1, 0,  do_savesys,
    "savesys - save system information variables to persistent storage\n",
    NULL
);


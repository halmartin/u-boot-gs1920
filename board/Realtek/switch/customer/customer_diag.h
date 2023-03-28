/*
 * Copyright(c) Realtek Semiconductor Corporation, 2012
 * All rights reserved.
 *
 * Purpose : Related definition of the RTL8328 driver for RTK command.
 *
 * Feature : RTL8328 driver for RTK command
 *
 */

#ifndef	__CUSTOMER_DIAG_H__
#define	__CUSTOMER_DIAG_H__

extern void run_fc_test(unsigned int detectGPIO);
extern void rtl8231_pin_status_get(unsigned int pinNum, unsigned int *pinStatus);
extern void rtl8231_pin_status_set(unsigned int pinNum, unsigned int pinStatus);

#define PORT_LOOPBACK_TEST_PKT_MAX_LEN (1500)

#ifdef CONFIG_GS1900v2
#define ENABLED     1
#define DISABLED    0

extern int htpModeIf;
extern int htpBreakIf;

#define FLASH_BASE_ADDRESS (0xB4000000)

/* we got 13MB for runtime, use the last 128k */
#define FLASH_HTP_LOG_ADDR          0xb4170000     /* b4170000 ~ b417ffff */
#define FLASH_HTP_FLASH_TST_START   0xb4160000     /* b4160000 ~ b4170000 */
#define FLASH_HTP_FLASH_TST_END     FLASH_HTP_LOG_ADDR

/* FAN Speed threshold */
#define FAN_GPIO_ACTIVE         0

#define FAN_EXT_GPIO_PIN        21
#define SYS_FAN_MIN_THRESHOLD   2500
#define SYS_FAN_MAX_THRESHOLD   7000

#if defined(CONFIG_RTL8380)
#define FAN_READ_ROUND          100000 /* About 1 seconds */
#endif
#if defined(CONFIG_RTL8390)
#define FAN_READ_ROUND          30000 /* About 1 seconds */
#endif

typedef enum sys_htp_err_e {
    HTP_ERR_NONE = 0,
    HTP_ERR_DRAM_VAL,
    HTP_ERR_FLSH_DDR_VAL,
    HTP_ERR_FLSH_SPI_PRB,
    HTP_ERR_FLSH_FLS_VAL,
    HTP_ERR_PORT_LOP_SET,
    HTP_ERR_PORT_PKT_VAL,
    HTP_ERR_FAN_FAIL,
    HTP_ERR_GET_INFO,
    HTP_ERR_BREAK,
    HTP_ERR_LOG_FULL,
    HTP_ERR_END
} sys_htp_err_t;

extern int _sys_htp_info_set(int mode, int brk, int hour, int entry, int times);
extern int _sys_htp_info_get(int* mode, int* brk, int* hour, int* entry, int* times);

extern int _sys_hpt_ddrtest_run(void);
extern int _sys_htp_fltest_run(void);
extern int _sys_htp_looptest_run(void);
extern int _sys_htp_fan_speed_test(unsigned int *);

extern int sys_htp_enable(void);
extern int sys_htp_run_case(int hour);
extern int sys_htp_print(void);
extern int sys_htp_clear(void);
#endif

#endif  /*__CUSTOMER_DIAG_H__*/


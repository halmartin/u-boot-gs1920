/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * Purpose : Related definition of the switch probe for U-Boot.
 *
 * Feature : switch probe function
 *
 */

#ifndef __PROBE_H__
#define __PROBE_H__

/*
 * Include Files
 */
#include <rtk_type.h>
#include <rtk_switch.h>

/*
 * Data Declaration
 */

/*
 * Function Declaration
 */
extern int board_probe(rtk_switch_model_t **pSwitchModel);
extern int board_model_set(int modelId);
extern int board_model_get(int *modelId);
#endif  /* __PROBE_H__ */


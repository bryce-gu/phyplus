/*
 ******************************************************************************
 * Name:        user_dhub.h
 *
 * description: defines:
 * 				implementation of CATE device plug & unplug detection using pulling.
 *
 *  History:     Date;           Author;         Description
 *              18 Mar. 2016;   Chen, George;   file creation.
 ******************************************************************************
 */

#ifndef __USER_APPL_H__
#define __USER_APPL_H__

/*
 ******************************************************************************
 * Includes
 ******************************************************************************
 */
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 ******************************************************************************
 * defines
 ******************************************************************************
 */
#define START_DEVICE_EVT               0x0001
#define USER_CHCK_MOTI_EVNT            0X0002
#define USER_CFGS_CPIS_EVNT            0X0004
#define USER_CHCK_KEYS_EVNT            0x0008
#define USER_CHCK_WHLS_EVNT            0x0010
#define USER_APPL_KEY_SCAN_EVT         0x0020
#define USER_APPL_WHEEL_SEND_EVT       0x0040
#define USER_POWR_OFFS_EVNT            0X4000
/*
 ******************************************************************************
 * public variables
 ******************************************************************************
 */

/*
 ******************************************************************************
 * public functions
 ******************************************************************************
 */
uint16
user_appl_evnt_hdlr(uint8 task, uint16 evnt);

void
user_appl_ini(uint8 task);

void
user_appl_chk_mode(void);

uint8
user_appl_get_mode(void);

#ifdef __cplusplus
}
#endif

#endif /* __USER_APPL_H__ */

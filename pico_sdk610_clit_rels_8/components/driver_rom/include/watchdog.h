/**************************************************************************************************
 
  Phyplus Microelectronics Limited confidential and proprietary. 
  All rights reserved.

  IMPORTANT: All rights of this software belong to Phyplus Microelectronics 
  Limited ("Phyplus"). Your use of this Software is limited to those 
  specific rights granted under  the terms of the business contract, the 
  confidential agreement, the non-disclosure agreement and any other forms 
  of agreements as a customer or a partner of Phyplus. You may not use this 
  Software unless you agree to abide by the terms of these agreements. 
  You acknowledge that the Software may not be modified, copied, 
  distributed or disclosed unless embedded on a Phyplus Bluetooth Low Energy 
  (BLE) integrated circuit, either as a product or is integrated into your 
  products.  Other than for the aforementioned purposes, you may not use, 
  reproduce, copy, prepare derivative works of, modify, distribute, perform, 
  display or sell this Software and/or its documentation for any purposes.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED AS IS WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  PHYPLUS OR ITS SUBSIDIARIES BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
  
**************************************************************************************************/
#ifndef __WATCHDOG_ROM_H__
#define __WATCHDOG_ROM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"
#include "bus_dev.h"

typedef enum
{
    WDG_2S =   0,
    WDG_4S =   1,
    WDG_8S =   2,
    WDG_16S =  3,
    WDG_32S =  4,
    WDG_64S =  5,
    WDG_128S = 6,
    WDG_256S = 7
}WDG_CYCLE_Type_e;

//when use watchdog,please umount its task and event,Watchdog_Init and Watchdog_ProcessEvent.
void Watchdog_Init(uint8 task_id);
uint16 Watchdog_ProcessEvent(uint8 task_id, uint16 events);

//watchdog default config:
//WDG_CYCLE_Type_e r_wdg_cycle = WDG_2S;
//bool r_wdg_int_mode = FALSE;
//uint32_t wdg_ms_cycle = 1000ms
//
//if you want change,please modify the following before watchdog start.
//for example:
//r_wdg_cycle = WDG_4S;
//r_wdg_int_mode = TRUE;

//if you want change feed watchdog cycle,please change wdg_ms_cycle.
//for example:
//wdg_ms_cycle = 1500;

//in most case,just use default setting is enough.

bool watchdog_init(WDG_CYCLE_Type_e cycle,bool int_mode);
void watchdog_feed(void);
int hal_watchdog_config(WDG_CYCLE_Type_e cycle);
#ifdef __cplusplus
}
#endif

#endif

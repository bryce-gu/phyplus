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

/**************************************************************************************************
  Filename:       bus_dev.h
  Revised:         
  Revision:        

  Description:    Describe the purpose and contents of the file.


 
**************************************************************************************************/

#ifndef _HAL_MCU_H
#define _HAL_MCU_H
 


/* ------------------------------------------------------------------------------------------------
 *                                           Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "types.h"
#include <stdint.h>

//enum{
//  MCU_UNDEF    = 0,
//  MCU_PRIME_A1 = 1,
//  MCU_PRIME_A2 = 2,
//  MCU_BUMBEE_M0,
//  MCU_BUMBEE_CK802
//};

#define MCU_UNDEF 0
#define MCU_PRIME_A1 1
#define MCU_PRIME_A2 2
#define MCU_BUMBEE_M0 3
#define MCU_BUMBEE_CK802 4


/******** ROM memory layout **********/
/*
1fff0000 ~ 1fff0200   jump table and global config
1fff0200 ~ 1fff0800   rom variable and rom common stack
1fff0800 ~ 1fff1800   4KB, program mode: command buffer and data buffer
1fff1c00 ~ 1fff1d00   256B, DWC buffer, overlap with programing mode stack, time-shared
1fff1a00 ~ 1fff1fe0   1504 bytes at most, programing mode stack
*/

/* only used in ate mode
1fff1000 ~ 1fff1804  used by ate otp  : otp_default_value_check
1fff1000 ~ 1fff1084  used by ate sram ret test
*/
/*************  end    ***************/


#define SRAM_BASE_ADDR          0x1fff0000
#define SRAM_END_ADDR           0x1fff1fff

#define ROM_SRAM_JUMPTABLE      SRAM_BASE_ADDR
#define ROM_SRAM_GLOBALCFG      (ROM_SRAM_JUMPTABLE+0x100)
//#define ROM_SRAM_JUMPTABLE_MIRROR   0x1fff1800
//#define ROM_SRAM_GLOBALCFG_MIRROR   (ROM_SRAM_JUMPTABLE_MIRROR+0x100)

#define ROM_SRAM_HEAP           0x1fff0800
#define ROM_SRAM_HEAP_SIZE      (1024*4)

#define ROM_SRAM_DWC_BUF        0x1fff1c00
#define ROM_SRAM_DWC_SIZE       (64*4)



#define ROM_PROG_MODE_STACK_TOP 0x1fff1fe0 //don't touch the last 32 bytes


#define APP_SRAM_START_ADDR     0x1fff0800
#define ATE_SRAM_RET_RESULT_0   0x1fff0800
#define ATE_SRAM_RET_RESULT_1   0x1fff1000


/* ------------------------------------------------------------------------------------------------
 *                                        Target Defines
 * ------------------------------------------------------------------------------------------------
 */
 
#define MAXMEMHEAP 4096

#define HAL_ISER   *((volatile uint32_t *)(0xe000e100))
#define HAL_ICER   *((volatile uint32_t *)(0xe000e180))


//#ifdef NO_STACK
//delete this to removing compile error of testcase31_lltest.
//#else

//subWriteReg: write value to register zone: bit[high:low]

#if 1
#define   subWriteReg(addr,high,low,value)    {write_reg(addr,(read_reg(addr)&\
                                              (~((((unsigned int)1<<((high)-(low)+1))-1)<<(low))))|\
                                                  ((unsigned int)(value)<<(low)));__asm volatile("nop");}
#else
extern void subWriteReg(uint32_t addr,uint8_t high,uint8_t low,uint32_t value);
#endif
//#endif
 
																										
#define TIME_BASE               (0x003fffff) // 24bit count shift 2 bit as 1us/bit
#define TIME_DELTA(x,y)         ( (x>=y) ? x-y : TIME_BASE-y+x )
																									
/*------------------------------------------------------
*  add arg for drv_enable_irq suport YoC software tspend
*/
//extern void drv_irq_init(void);
//extern int drv_enable_irq(void);
//extern int drv_disable_irq(void);
//#define HAL_CRITICAL_SECTION_INIT()   drv_irq_init()
//#define HAL_ENTER_CRITICAL_SECTION()  drv_disable_irq()
//#define HAL_EXIT_CRITICAL_SECTION()   drv_enable_irq()

/*-----------------------------------------------------
* for rom symoble use the _HAL_CS_ALLOC_
*/
extern void drv_irq_init(void);
extern int drv_enable_irq(int cs);
extern int drv_disable_irq(void);
extern int drv_enable_irqx(int cs);
extern int drv_disable_irqx(void);
#define HAL_CRITICAL_SECTION_INIT()             drv_irq_init()


#if(__BUILD_DRV_IRQ_CS_ALLOC__)
//#warning" !!! HAS CS ALLOC !!!"
#define _HAL_CS_ALLOC_()                        int _drv_irq_cs

#define HAL_ENTER_CRITICAL_SECTION()            { _drv_irq_cs = drv_disable_irq();} 
#define HAL_EXIT_CRITICAL_SECTION()             { drv_enable_irq(_drv_irq_cs);} 

#define DRV_ENTER_CRITICAL_SECTION()            {drv_disable_irqx();}
#define DRV_EXIT_CRITICAL_SECTION()             {drv_enable_irqx(0);}

#else                       

//#ifndef FPGA_ROM_DRIVER_TEST

//#define HAL_ENTER_CRITICAL_SECTION()            { ;} 
//#define HAL_EXIT_CRITICAL_SECTION()             { ;} 
//#define DRV_ENTER_CRITICAL_SECTION()            { ;}
//#define DRV_EXIT_CRITICAL_SECTION()             { ;}

//#else
	
#define _HAL_CS_ALLOC_()                        int _drv_irq_cs;

#define HAL_ENTER_CRITICAL_SECTION()            { _drv_irq_cs = drv_disable_irq();} 
#define HAL_EXIT_CRITICAL_SECTION()             { drv_enable_irq(_drv_irq_cs);} 

#define DRV_ENTER_CRITICAL_SECTION()            {drv_disable_irqx();}
#define DRV_EXIT_CRITICAL_SECTION()             {drv_enable_irqx(0);}

//#endif

#endif

/**************************************************************************************************
 */
#endif

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
#include "rom_sym_def.h"
#include "types.h"
#include "ll_sleep.h"
#include "bus_dev.h"
#include "string.h"

#include "pwrmgr.h"
#include "error.h"
#include "gpio.h"
#include "log.h"
#include "clock.h"
#include "otp.h"
#include "jump_function.h"

extern uint32_t s_config_swClk0;
extern uint32_t s_config_swClk1;

#define HAL_PWRMGR_TASK_MAX_NUM   5 
uint32_t s_num = HAL_PWRMGR_TASK_MAX_NUM;
static pwrmgr_Ctx_t s_pwrmgrCtx[HAL_PWRMGR_TASK_MAX_NUM];
/*
    osal_idle_task will be call
*/

void osal_idle_task (void)
{
    AP_WDT_FEED;
//    extern void runtime_check_dll(void);
//    runtime_check_dll();
    osal_pwrmgr_powerconserve0();
}
int hal_pwrmgr_init(void)
{
	
#if(CFG_SLEEP_MODE == PWR_MODE_NO_SLEEP)
	pwrmgr_config(PWR_MODE_NO_SLEEP);
#else
	pwrmgr_config(PWR_MODE_SLEEP);
#endif
	
  pwrmgr_init(s_pwrmgrCtx, HAL_PWRMGR_TASK_MAX_NUM);
	
  //OTP should be the 1st MOD in app wakeup process
  pwrmgr_register(MOD_OTP,NULL,otp_cache_init);
	
  s_config_swClk0 = DEF_CLKG_CONFIG_0;
  s_config_swClk1 = DEF_CLKG_CONFIG_1;
#if(CFG_WDT_ENABLE==1)
  /*
      if wdt enable, set osal idle task to feed wdt before powerconserve
  */
  //if(AP_WDT_ENABLE_STATE)
  JUMP_FUNCTION_SET(OSAL_POWER_CONSERVE,(uint32_t)&osal_idle_task);
#endif

  return PPlus_SUCCESS;
}
void hal_pwrmgr_poweroff(pwroff_cfg_t *pcfg, uint8_t wakeup_pin_num) // check
{
    drv_disable_irq();
    subWriteReg(0x4000f01c, 6, 6, 0x00); // disable software control
    //(void)(wakeup_pin_num);

    for (uint8_t i = 0; i < wakeup_pin_num; i++)
    {
        if (pcfg[i].type == POL_FALLING)
            gpio_pull_set(pcfg[i].pin, GPIO_PULL_UP);
        else
            gpio_pull_set(pcfg[i].pin, GPIO_PULL_DOWN);

        gpio_wakeup_set(pcfg[i].pin, pcfg[i].type);
    }
    /**
        config reset casue as RSTC_OFF_MODE
        reset path walkaround dwc
    */
    AP_AON->SLEEP_R[0] = 2;
    enter_sleep_off_mode(SYSTEM_OFF_MODE);

    while (1)
        ;
}
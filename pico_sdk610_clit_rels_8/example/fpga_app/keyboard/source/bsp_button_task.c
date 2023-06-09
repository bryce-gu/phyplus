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
    Filename:       bsp_button_task.c
    Revised:        $Date $
    Revision:       $Revision $
**************************************************************************************************/

/*********************************************************************
    INCLUDES
*/
#include <string.h>
#include "rom_sym_def.h"
#include "OSAL.h"
#include "OSAL_Timers.h"
#include "pwrmgr.h"
#include "bsp_button_task.h"
#include "keyboard.h"	

uint8 Bsp_Btn_TaskID;

bool bsp_btn_timer_flag = FALSE;
bool bsp_btn_gpio_flag = FALSE;
bool bsp_btn_kscan_flag = FALSE;
bsp_btn_callback_t bsp_btn_cb = NULL;
static uint8 enable_latency_flag = 0;
static BTN_T usr_sum_btn_array[BSP_TOTAL_BTN_NUM];

#if ((BSP_BTN_HARDWARE_CONFIG == BSP_BTN_JUST_KSCAN) ||  (BSP_BTN_HARDWARE_CONFIG ==BSP_BTN_GPIO_AND_KSCAN))

uint8 KscanMK_row[KSCAN_ALL_ROW_NUM];
uint8 KscanMK_col[KSCAN_ALL_COL_NUM];

__ATTR_SECTION_SRAM__ static void kscan_evt_handler(kscan_Evt_t* evt)
{
    bool ret;

    for(uint8_t i=0; i<evt->num; i++)
    {
        if(evt->keys[i].type == KEY_PRESSED)
        {
            ret = bsp_set_key_value_by_row_col(NUM_KEY_COLS,KscanMK_row[evt->keys[i].row],KscanMK_col[evt->keys[i].col],TRUE);
            {
                if(ret == TRUE)
                {
                    bsp_btn_timer_flag = TRUE;
                    osal_set_event(Bsp_Btn_TaskID, BSP_BTN_EVT_SYSTICK);
                }
            }
        }
        else
        {
            ret = bsp_set_key_value_by_row_col(NUM_KEY_COLS,KscanMK_row[evt->keys[i].row],KscanMK_col[evt->keys[i].col],FALSE);
        }
    }
}

void kscan_button_init(uint8 task_id)
{
    kscan_Cfg_t cfg;
    cfg.ghost_key_state = IGNORE_GHOST_KEY;
    cfg.key_rows = rows;
    cfg.key_cols = cols;
    cfg.interval = 5;
    cfg.evt_handler = kscan_evt_handler;
    memset(KscanMK_row,0xff,KSCAN_ALL_ROW_NUM);

    for(int i=0; i<NUM_KEY_ROWS; i++)
    {
        KscanMK_row[rows[i]] = i;
    }

    memset(KscanMK_col,0xff,KSCAN_ALL_COL_NUM);

    for(int i=0; i<NUM_KEY_COLS; i++)
    {
        KscanMK_col[cols[i]] = i;
    }

    hal_kscan_init(cfg, task_id, KSCAN_WAKEUP_TIMEOUT_EVT);
}
#endif

#if (BSP_BTN_HARDWARE_CONFIG == BSP_BTN_SOFT_SCAN)
__ATTR_SECTION_SRAM__ static void kscan_evt_handler(uint8 total_col_num, uint8 row, uint8 col, uint8 key_state)
{
	
    bool ret;
    {
        if(key_state == 1)
        {
			
            ret = bsp_set_key_value_by_row_col(total_col_num, row, col,TRUE);

            {
                if(ret == TRUE)
                {
                    bsp_btn_timer_flag = TRUE;
                    osal_set_event(Bsp_Btn_TaskID, BSP_BTN_EVT_SYSTICK);
                }
            }
        }
        else
        {
            ret = bsp_set_key_value_by_row_col(total_col_num, row, col,FALSE);
        }
    }
} 
#endif


#if ((BSP_BTN_HARDWARE_CONFIG == BSP_BTN_JUST_GPIO) || (BSP_BTN_HARDWARE_CONFIG == BSP_BTN_GPIO_AND_KSCAN))
uint8_t Bsp_Btn_Get_Index(gpio_pin_e pin)
{
    uint8_t i;

    if(hal_gpio_btn_get_index(pin,&i) == PPlus_SUCCESS)
    {
        #if  (BSP_BTN_HARDWARE_CONFIG == BSP_BTN_GPIO_AND_KSCAN)
        return (BSP_KSCAN_SINGLE_BTN_NUM + i);
        #else
        return (i);
        #endif
    }

    return 0xFF;
}
#endif

static void Bsp_Btn_Check(uint8_t ucKeyCode)
{
    #if (BSP_BTN_HARDWARE_CONFIG == BSP_BTN_JUST_GPIO)
    hal_gpio_btn_cb(ucKeyCode);
    #else
    bsp_btn_cb(ucKeyCode);
    #endif

    if(bsp_btn_timer_flag == TRUE)
    {
        if(bsp_KeyEmpty() == TRUE)
        {
            osal_stop_timerEx(Bsp_Btn_TaskID,BSP_BTN_EVT_SYSTICK);
            bsp_btn_timer_flag = FALSE;
        }
    }
}

void gpio_btn_pin_event_handler(gpio_pin_e pin,IO_Wakeup_Pol_e type)
{
    #if ((BSP_BTN_HARDWARE_CONFIG == BSP_BTN_JUST_GPIO) ||  (BSP_BTN_HARDWARE_CONFIG == BSP_BTN_GPIO_AND_KSCAN))

    if(((GPIO_SINGLE_BTN_IDLE_LEVEL == 0) && (POL_RISING == type)) || \
            ((GPIO_SINGLE_BTN_IDLE_LEVEL == 1) && (POL_RISING != type)))
    {
        bsp_btn_timer_flag = TRUE;
        osal_start_reload_timer(Bsp_Btn_TaskID,BSP_BTN_EVT_SYSTICK,BTN_SYS_TICK);
        bsp_set_key_value_by_index(Bsp_Btn_Get_Index(pin),1);
    }
    else
    {
        bsp_set_key_value_by_index(Bsp_Btn_Get_Index(pin),0);
    }

    #endif
}

void Bsp_Btn_Init( uint8 task_id )
{
    Bsp_Btn_TaskID = task_id;
	btp_button_init();
}

uint16 Bsp_Btn_ProcessEvent( uint8 task_id, uint16 events )
{
    uint8_t ucKeyCode;

    if(Bsp_Btn_TaskID != task_id)
    {
        return 0;
    }

    #if ((BSP_BTN_HARDWARE_CONFIG == BSP_BTN_JUST_KSCAN) || (BSP_BTN_HARDWARE_CONFIG == BSP_BTN_GPIO_AND_KSCAN))

    if ( events & KSCAN_WAKEUP_TIMEOUT_EVT )
    {
        hal_kscan_timeout_handler();
        return (events ^ KSCAN_WAKEUP_TIMEOUT_EVT);
    }

    #endif
	
    #if ( BSP_BTN_HARDWARE_CONFIG == BSP_BTN_SOFT_SCAN )
	if( events &  BSP_SOFT_POLLING_EVT )
	{
		keyboard_io_read();
		return ( events ^ BSP_SOFT_POLLING_EVT );
	}
	#endif
	
    if ( events & BSP_BTN_EVT_SYSTICK )
    {
        ucKeyCode = bsp_KeyPro();
        osal_start_timerEx(Bsp_Btn_TaskID, BSP_BTN_EVT_SYSTICK, BTN_SYS_TICK);
        if(ucKeyCode != BTN_NONE)
        {
            Bsp_Btn_Check(ucKeyCode);
        }

        return (events ^ BSP_BTN_EVT_SYSTICK);
    }
	
	
	if( events & BSP_BTN_KSCAN_INIT_CONFIG_EVT )
	{
		btp_button_init();
		return ( events ^ BSP_BTN_KSCAN_INIT_CONFIG_EVT );
	}

    return 0;
}


void btp_button_init(void)
{
	#if (BSP_BTN_HARDWARE_CONFIG == BSP_BTN_JUST_GPIO)

    if(bsp_btn_gpio_flag == TRUE)
    {
        ;
    }

    #elif  (BSP_BTN_HARDWARE_CONFIG == BSP_BTN_JUST_KSCAN)
    if(bsp_btn_kscan_flag == TRUE)
    {
        kscan_button_init(Bsp_Btn_TaskID);
    }

    #elif  (BSP_BTN_HARDWARE_CONFIG == BSP_BTN_GPIO_AND_KSCAN)

    if((bsp_btn_gpio_flag == TRUE) && (bsp_btn_kscan_flag == TRUE))
    {
        kscan_button_init(task_id);
    }
    #elif  (BSP_BTN_HARDWARE_CONFIG == BSP_BTN_SOFT_SCAN)
	if(bsp_btn_kscan_flag == TRUE)
	{
		keyboard_register(kscan_evt_handler, Bsp_Btn_TaskID, BSP_SOFT_POLLING_EVT);
	}
    #endif
    else
    {
        LOG("btn config error %d %d %d\n",__LINE__,bsp_btn_gpio_flag,bsp_btn_kscan_flag);
        return;
    }
	#if 0
	// !config user key support status
    for(int i = 0; i < BSP_TOTAL_BTN_NUM; i++)
    {
		if( i == BSP_COMBINE_KEY_ID_0 || i == BSP_COMBINE_KEY_ID_1 || 
			i == BSP_COMBINE_KEY_ID_2 || i == BSP_IR_LEARN_KEY_ID  )
		{
			usr_sum_btn_array[i].KeyConfig = (BSP_BTN_PD_CFG | BSP_BTN_UP_CFG | BSP_BTN_LPS_CFG | BSP_BTN_LPK_CFG);
		}
		else
		{
			usr_sum_btn_array[i].KeyConfig = (BSP_BTN_PD_CFG | BSP_BTN_UP_CFG );
		}
        
    }
	#endif
	
    #if (BSP_COMBINE_BTN_NUM > 0)

    if(PPlus_SUCCESS != bsp_InitBtn(usr_sum_btn_array,BSP_TOTAL_BTN_NUM,BSP_SINGLE_BTN_NUM,usr_combine_btn_array))
    #else
    if(PPlus_SUCCESS != bsp_InitBtn(usr_sum_btn_array,BSP_TOTAL_BTN_NUM,0,NULL))
    #endif
    {
        LOG("bsp button init error\n");
    }

}

void clean_latency_flag(void)
{	
	enable_latency_flag = 0;
}

void bsp_button_init_config_handle(void)
{
	osal_set_event(Bsp_Btn_TaskID, BSP_BTN_KSCAN_INIT_CONFIG_EVT);
}

uint8 get_latency_flag(void)
{
	return enable_latency_flag;
}


void kscan_enter_power_off_gpio_option( void )
{
	#if( BSP_BTN_HARDWARE_CONFIG ==  BSP_BTN_SOFT_SCAN )
	keyboard_deep_sleep_handler();
	#elif ( BSP_BTN_HARDWARE_CONFIG ==  BSP_BTN_JUST_KSCAN )
	kscan_sleep_handler();
	#endif
}









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

/******************************************************************************


 *****************************************************************************/


/*********************************************************************
    INCLUDES
*/
#include "rom_sym_def.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "gatt.h"
#include "hci.h"
#include "gapgattserver.h"
#include "gattservapp.h"
#include "gatt_profile_uuid.h"
#include "linkdb.h"
#include "peripheral.h"
#include "gapbondmgr.h"
#include "peripheral.h"
#include "hidkbdservice.h"
#include "hiddev.h"
#include "global_config.h"
#include "hidkbd.h"
#include "hci_tl.h"
#include "ll.h"
#include "ll_common.h"
#include "ll_def.h"
#include "OSAL_Memory.h"
#include "log.h"
#include "sensor.h"
#include "arry_queu.h"
#include "user_appl.h"

/*********************************************************************
    MACROS
*/
// Selected HID keycodes
#define KEY_RIGHT_ARROW             0x4F
#define KEY_LEFT_ARROW              0x50
#define KEY_NONE                    0x00

// Selected HID LED bitmaps
#define LED_NUM_LOCK                0x01
#define LED_CAPS_LOCK               0x02

// Selected HID mouse button values
#define MOUSE_BUTTON_1              0x01
#define MOUSE_BUTTON_NONE           0x00

// HID keyboard input report length
#define HID_KEYBOARD_IN_RPT_LEN     8

// HID LED output report length
#define HID_LED_OUT_RPT_LEN         1

// HID mouse input report length
#define HID_MOUSE_IN_RPT_LEN        4

#if EN_CONSUMER_MODE
    // HID consumer control input report length
    #define HID_CC_IN_RPT_LEN                     1//2//1
#endif

#define CFGS_USES_APPL_TXQU         (0)

/*********************************************************************
    CONSTANTS
*/

// HID idle timeout in msec; set to zero to disable timeout
#define DEFAULT_HID_IDLE_TIMEOUT              0

// Battery level is critical when it is less than this %
#define DEFAULT_BATT_CRITICAL_LEVEL           6

/*********************************************************************
    TYPEDEFS
*/

/*********************************************************************
    GLOBAL VARIABLES
*/

// Task ID
uint8 application_TaskID;
extern uint16 gapRole_ConnectionHandle ;

/*********************************************************************
    EXTERNAL VARIABLES
*/

/*********************************************************************
    EXTERNAL FUNCTIONS
*/

/*********************************************************************
    LOCAL VARIABLES
*/


#if( defined( EN_KEYBOARD_INPUT ) && EN_KEYBOARD_INPUT)
// HID Dev configuration
static hidDevCfg_t hidKbdCfg =
{
    DEFAULT_HID_IDLE_TIMEOUT,   // Idle timeout
    HID_KBD_FLAGS               // HID feature flags
};
#endif

#if (1 == CFGS_USES_APPL_TXQU)
arry_queu_ini_queu(tx, 10);
#endif
/*********************************************************************
    LOCAL FUNCTIONS
*/
static void hidKbd_ProcessOSALMsg( osal_event_hdr_t* pMsg );
static void hidKbd_ProcessGattMsg( gattMsgEvent_t* pMsg );
#if( defined( EN_KEYBOARD_INPUT ) && EN_KEYBOARD_INPUT)
void hidKbdSendReport( uint8 keycode );
static uint8 hidKbdRcvReport( uint8 len, uint8* pData );
static uint8 hidKbdRptCB( uint8 id, uint8 type, uint16 uuid,
                          uint8 oper, uint16* pLen, uint8* pData );
static void hidKbdEvtCB( uint8 evt );
#endif
#if( defined( EN_CONSUMER_MODE ) && EN_CONSUMER_MODE)
    static void hidCCSendReport( uint8 cmd, bool keyPressed, uint8 keyRepeated );
    void hidCCSendReportKey( uint8 cmd, bool keyPressed);
#endif

//void hidKbdSendMouseReport_test( /*uint8 buttons,*/uint8 x,uint8 y/*,int8 wheel*/);
void  hidKbdSendMouseReport( uint8 buttons,uint8 x,uint8 y,uint8 wheel);



/*********************************************************************
    PROFILE CALLBACKS
*/

/*********************************************************************
    PUBLIC FUNCTIONS
*/

/*********************************************************************
    @fn      HidEmuKbd_Init

    @brief   Initialization function for the HidEmuKbd App Task.
            This is called during initialization and should contain
            any application specific initialization (ie. hardware
            initialization/setup, table initialization, power up
            notificaiton ... ).

    @param   task_id - the ID assigned by OSAL.  This ID should be
                      used to send messages and set timers.

    @return  none
*/
void HidKbd_Init( uint8 task_id )
{
    // int rslt;

    application_TaskID = task_id;

    #if (1 == CFGS_USES_APPL_TXQU)
    // LOG_ERROR("TX_Q size:%d\r\n", arry_queu_get_size(tx));
    #endif

    #ifdef _PHY_DEBUG
        LOG("%s,%s,Line %d\n",__FILE__,__func__,__LINE__);
    #endif
    if( user_appl_get_mode() ) {
        // Set up HID keyboard service
        HidKbd_AddService();
    } else {
        HCI_GAPTaskRegister(task_id);
        HCI_L2CAPTaskRegister(task_id);
    }
    osal_set_event( application_TaskID, START_DEVICE_EVT );
//	GAP_RegisterForHCIMsgs(application_TaskID);
}

int HidKbd_chk_conn(void)
{
    return ( gapRole_ConnectionHandle != INVALID_CONNHANDLE );
}

int HidKbd_chk_bond(void)
{
    return ( user_appl_get_mode() ? (1 == HIDMouse_bond_ready) : (1));
}

// uint32 cur_tick;
extern void ble_conn_param_disable_latency(void);

void hidKbdSendMouseReport( uint8 buttons,uint8 x, uint8 y,uint8 wheel)
{
#if 0
   static uint8 no_change_flag=0;

     if((hidMouse_data.wheel)&&(get_systick() - cur_tick < 15)) {/// 10ms
          return;
      }

      cur_tick = get_systick();

      if(buttons==0&&x==0&&y==0&&wheel==0)
      {
        no_change_flag++;
        if(no_change_flag>1)
        {
         no_change_flag=2;
          return;
        }
      }
      else
      no_change_flag=0;
#endif

    if ( !HidKbd_chk_conn() ) return;   // con'd
    if (buttons==0&&x==0&&y==0&&wheel==0) return;

    uint8 rprt[HID_MOUSE_IN_RPT_LEN];
    osal_memset(rprt, 0, HID_MOUSE_IN_RPT_LEN);

        rprt[0] = hidMouse_data.button;//hidMouse_data.button;   // Buttons
    if ( x || y ) {
        rprt[1] = hidMouse_data.x;
        rprt[2] = hidMouse_data.y;
    }
    if ( wheel ) {
        rprt[3] = hidMouse_data.wheel;
    }

    if( 1 == user_appl_get_mode() ) {
        if ( 1 == HidKbd_chk_bond() ) {
        ble_conn_param_disable_latency();
        HidDev_Report(HID_RPT_ID_MOUSE_IN, HID_REPORT_TYPE_INPUT, HID_MOUSE_IN_RPT_LEN, rprt);
        }
    } else {
        uint8  rslt;
        uint8 *data = (uint8*)HCI_bm_alloc(10);
        if ( data ) {
            LL_PLUS_DisableSlaveLatency(0);
            osal_memset(data, 0, 10);
            // data[0] = 0;
            // data[1] = 0;
            data[2] = 2;
            // data[3] = 0;
            osal_memcpy(&data[4], &rprt[0], 4);
            // data[8] = 0x0;

            #if (1 == CFGS_USES_APPL_TXQU)
            /* QUEU UPON APPL QUEU */
            if ( arry_queu_chk_empt(tx) ) {
                osal_set_event(application_TaskID, APPL_TXQU_PULL_EVT);
            }
            if ( arry_queu_chk_full(tx) ) {
                osal_bm_free(data);
            } else {
                arry_queu_psh_node(tx, data);
            }
            #else
            // LOG_ERROR("TX:-");
            rslt = HCI_SendDataPkt(0,0,10,(int8 *)data);
            if ( rslt != SUCCESS ) {
                osal_bm_free(data);
                LOG_ERROR("2.4G TX rslt:%d\n",rslt);
            }
            #endif
        } else {
            LOG_ERROR("HEAP:FULL\r\n");
        }
    }
}

/*********************************************************************
    @fn      HidEmuKbd_ProcessEvent

    @brief   HidEmuKbd Application Task event processor.  This function
            is called to process all events for the task.  Events
            include timers, messages and any other user defined events.

    @param   task_id  - The OSAL assigned task ID.
    @param   events - events to process.  This is a bit map and can
                     contain more than one event.

    @return  events not processed
*/
uint16 HidKbd_ProcessEvent( uint8 task_id, uint16 events )
{
    VOID task_id; // OSAL required parameter that isn't used in this function
//    #ifdef _PHY_DEBUG
//		LOG("%s,%s,Line %d,events 0x%X\n",__FILE__,__func__,__LINE__,events);
//	#endif

    if ( events & SYS_EVENT_MSG )
    {
        uint8* pMsg;

        if ( (pMsg = osal_msg_receive( application_TaskID )) != NULL )
        {
            hidKbd_ProcessOSALMsg( (osal_event_hdr_t*)pMsg );
            // Release the OSAL message
            osal_msg_deallocate( pMsg );
        }

        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }

    if ( events & START_DEVICE_EVT )
    {
        if ( 1 == user_appl_get_mode() ) {
        //osal_start_reload_timer(application_TaskID,HID_TEST_EVT,2000);
        } else {
            HCI_LE_SetAdvParamCmd( 40, 40, 0, 0, 0, NULL, 0x07, 0);
            uint8 adv_data[]={0x02,0x01,0x06};
            uint8 scanRspData[] ={	0x0C, 0x09,'P','I','C','O','-','s','p','p','1','0','0'};
            HCI_LE_SetAdvDataCmd( sizeof(adv_data), adv_data );
            HCI_LE_SetScanRspDataCmd( sizeof(scanRspData),scanRspData);
            HCI_LE_SetAdvEnableCmd(TRUE);
        }
        return ( events ^ START_DEVICE_EVT );
    }
#if 0
    if ( events & HID_TEST_EVT )
    {
        if( app_mode == APP_BLE_MODE)
        {
            LOG_DEBUG("m-->\n");
        #if 0//( defined(EN_CONSUMER_MODE) && EN_CONSUMER_MODE )
//	    	#ifdef _PHY_DEBUG
//				LOG("HID_TEST_EVT\n");
//			#endif
            hidCCSendReport(0,1,0);
            hidCCSendReport(0,0,0);
        #endif
        if(gapRole_ConnectionHandle != INVALID_CONNHANDLE )
        {
          hidKbdSendMouseReport_test(0xf5,0x08);
          osal_start_timerEx(application_TaskID,HID_TEST_EVT,2000);
        }
        }
        return ( events ^ HID_TEST_EVT );
    }
#endif

#if 0
    if( events & SBP_CONN_ESTA_EVT )
    {
        // LOG_ERROR("ll_TaskEndsOK\n");

        return (  events ^ SBP_CONN_ESTA_EVT);
    }
#endif

    if( events & SBP_BOND_READY_EVT )
    {
        LOG_ERROR("HIDx Bond SUCC\n");
        HIDMouse_bond_ready=1;
        return (  events ^ SBP_BOND_READY_EVT);
    }

    #if (1 == CFGS_USES_APPL_TXQU)
    if( events & APPL_TXQU_PULL_EVT ) {
        // uint8 rslt;
        void* para;

        #if 1
        if ( SUCCESS == arry_queu_pek_node(tx, para) ) {
            if ( SUCCESS == HCI_SendDataPkt(0,0,10,para) ) {
                arry_queu_pll_node(tx, para);
            // LOG_ERROR("TXQU:RTRY:%d\r\n", rslt);
            }
            osal_start_timerEx(application_TaskID, APPL_TXQU_PULL_EVT, 8);
        }
        #else
        while ( SUCCESS == arry_queu_pek_node(tx, para) ) {
            if ( SUCCESS == HCI_SendDataPkt(0,0,10,para) ) {
                arry_queu_pll_node(tx, para);
            // LOG_ERROR("TXQU:RTRY:%d\r\n", rslt);
            } else {
                osal_start_timerEx(application_TaskID, APPL_TXQU_PULL_EVT, 4);
            }
        }
        #endif

        return (  events ^ APPL_TXQU_PULL_EVT);
    }
    #endif

    return 0;
}

/*********************************************************************
    @fn      hidKbd_ProcessOSALMsg

    @brief   Process an incoming task message.

    @param   pMsg - message to process

    @return  none
*/
static void hidKbd_ProcessOSALMsg( osal_event_hdr_t* pMsg )
{
    #ifdef _PHY_DEBUG
        LOG("%s,%s,Line %d,pMsg->event 0x%X\n",__FILE__,__func__,__LINE__,pMsg->event);
    #endif

    switch ( pMsg->event )
    {
    case GATT_MSG_EVENT:
        hidKbd_ProcessGattMsg( (gattMsgEvent_t*)pMsg );
        break;

    case HCI_GAP_EVENT_EVENT:
    {
        switch( pMsg->status )
        {
            case HCI_COMMAND_COMPLETE_EVENT_CODE:
            {
                #ifdef _PHY_DEBUG
                hciEvt_CmdComplete_t* pHciMsg;
                pHciMsg	= (hciEvt_CmdComplete_t*)pMsg;
                LOG("	HCI_COMMAND_COMPLETE_EVENT_CODE: %x\n", pHciMsg->cmdOpcode);
                //safeToDealloc = gapProcessHCICmdCompleteEvt( (hciEvt_CmdComplete_t *)pMsg );
                #endif

            }
            break;
            case HCI_LE_EVENT_CODE:
            {
                if ( 1 != user_appl_get_mode() ) {
                    hciEvt_BLEConnComplete_t *pkt = (hciEvt_BLEConnComplete_t *)pMsg;
                    if( pkt->BLEEventCode == HCI_BLE_CONNECTION_COMPLETE_EVENT ) {
                        LOG_ERROR("! Est'd !\n");
                        // osal_start_reload_timer(application_TaskID, SPP_PERIODIC_EVT, 10);//300);
                        gapRole_ConnectionHandle = pkt->connectionHandle;
                        LL_PLUS_DisableSlaveLatency(0);
                    }
                }
            }
            break;
            case HCI_DISCONNECTION_COMPLETE_EVENT_CODE:
            {
                if ( 1 != user_appl_get_mode() ) {
                    hciEvt_DisconnComplete_t *pkt = (hciEvt_DisconnComplete_t *)pMsg;
                    if( pkt->hdr.status == HCI_DISCONNECTION_COMPLETE_EVENT_CODE ) {
                        LOG_ERROR("! Disc !\n")
                        // osal_stop_timerEx(application_TaskID, SPP_PERIODIC_EVT);
                        gapRole_ConnectionHandle = INVALID_CONNHANDLE;
                        HCI_LE_SetAdvEnableCmd(TRUE);
                    }
                }
            }
            break;

            default:
                LOG_DEBUG("unKnown pMsg->status 0x%X\n",pMsg->status);
                break;
        }
    }
    break;

    case HCI_DATA_EVENT:
    {
        if ( 1 != user_appl_get_mode() ) {
            hciDataEvent_t *msg  = (hciDataEvent_t *)pMsg;
            // LOG_DUMP_BYTE(msg->pData,msg->len);
            osal_bm_free(msg->pData);
        }
    }

    default:
        break;
    }
}


/*********************************************************************
    @fn      hidKbd_ProcessGattMsg

    @brief   Process GATT messages

    @return  none
*/
static void hidKbd_ProcessGattMsg( gattMsgEvent_t* pMsg )
{
    #ifdef _PHY_DEBUG
        LOG("%s,%s,Line %d\n",__FILE__,__func__,__LINE__);
    #endif
}

#if 0
void hidKbdSendMouseReport_test( /*uint8 buttons,*/uint8 x,uint8 y/*,int8 wheel*/)
{

    uint8 buf[HID_MOUSE_IN_RPT_LEN];
    buf[0] = 0;//hidMouse_data.button;   // Buttons
    buf[1] =x;//x;         // X
    buf[2] = y;//y;         // Y
    buf[3] = 0;//hidMouse_data.wheel;         // Wheel


    HidDev_Report( HID_RPT_ID_MOUSE_IN, HID_REPORT_TYPE_INPUT,
                   HID_MOUSE_IN_RPT_LEN, buf );
}
#endif

#if( defined( EN_KEYBOARD_INPUT ) && EN_KEYBOARD_INPUT)
/*********************************************************************
    @fn      hidKbdSendReport

    @brief   Build and send a HID keyboard report.

    @param   keycode - HID keycode.

    @return  none
*/
void hidKbdSendReport( uint8 keycode )
{
    #ifdef _PHY_DEBUG
        LOG("%s,%s,Line %d\n",__FILE__,__func__,__LINE__);
    #endif
    uint8 buf[HID_KEYBOARD_IN_RPT_LEN];
    buf[0] = 0;         // Modifier keys
    buf[1] = 0;         // Reserved
    buf[2] = keycode;   // Keycode 1
    buf[3] = 0;         // Keycode 2
    buf[4] = 0;         // Keycode 3
    buf[5] = 0;         // Keycode 4
    buf[6] = 0;         // Keycode 5
    buf[7] = 0;         // Keycode 6
    HidDev_Report( HID_RPT_ID_KEY_IN, HID_REPORT_TYPE_INPUT,
                   HID_KEYBOARD_IN_RPT_LEN, buf );
}

/*********************************************************************
    @fn      hidKbdSendMouseReport

    @brief   Build and send a HID mouse report.

    @param   buttons - Mouse button code

    @return  none
*/
void hidKbdSendMouseReport( uint8 buttons,uint8 x,uint8 y )
{
    #ifdef _PHY_DEBUG
        LOG("%s,%s,Line %d\n",__FILE__,__func__,__LINE__);
    #endif
    uint8 buf[HID_MOUSE_IN_RPT_LEN];
    buf[0] = buttons;   // Buttons
    buf[1] = x;         // X
    buf[2] = y;         // Y
    buf[3] = 0;         // Wheel
    buf[4] = 0;         // AC Pan
    HidDev_Report( HID_RPT_ID_MOUSE_IN, HID_REPORT_TYPE_INPUT,
                   HID_MOUSE_IN_RPT_LEN, buf );
}

uint8 hidKbdSendVoiceCMDtReport( uint8 keycode )
{
    #ifdef _PHY_DEBUG
        LOG("%s,%s,Line %d\n",__FILE__,__func__,__LINE__);
    #endif
    uint8 status;
    return status;
}


/*********************************************************************
    @fn      hidKbdRcvReport

    @brief   Process an incoming HID keyboard report.

    @param   len - Length of report.
    @param   pData - Report data.

    @return  status
*/
static uint8 hidKbdRcvReport( uint8 len, uint8* pData )
{
    // verify data length
    if ( len == HID_LED_OUT_RPT_LEN )
    {
        // set keyfob LEDs
        //HalLedSet( HAL_LED_1, ((*pData & LED_CAPS_LOCK) == LED_CAPS_LOCK) );
        //HalLedSet( HAL_LED_2, ((*pData & LED_NUM_LOCK) == LED_NUM_LOCK) );
        return SUCCESS;
    }
    else
    {
        return ATT_ERR_INVALID_VALUE_SIZE;
    }
}

/*********************************************************************
    @fn      hidKbdRptCB

    @brief   HID Dev report callback.

    @param   id - HID report ID.
    @param   type - HID report type.
    @param   uuid - attribute uuid.
    @param   oper - operation:  read, write, etc.
    @param   len - Length of report.
    @param   pData - Report data.

    @return  GATT status code.
*/
static uint8 hidKbdRptCB( uint8 id, uint8 type, uint16 uuid,
                          uint8 oper, uint16* pLen, uint8* pData )
{
    uint8 status = SUCCESS;
    #ifdef _PHY_DEBUG
        LOG("%s,%s,Line %d\n",__FILE__,__func__,__LINE__);
    #endif

    return status;
}

/*********************************************************************
    @fn      hidKbdEvtCB

    @brief   HID Dev event callback.

    @param   evt - event ID.

    @return  HID response code.
*/
static void hidKbdEvtCB( uint8 evt )
{
    #ifdef _PHY_DEBUG
        LOG("%s,%s,Line %d\n",__FILE__,__func__,__LINE__);
    #endif
    // process enter/exit suspend or enter/exit boot mode
    return;
}
#endif

#if( defined( EN_CONSUMER_MODE ) && EN_CONSUMER_MODE)
/*********************************************************************
  @fn	   hidCCSendReport

  @brief   Build and send a HID Consumer Control report.

  @param   cmd - bitmap of left/middle/right mouse button states
  @param   keyPressed - amount of mouse movement along X-axis in units of Mickeys (1/200 of an inch)
  @param   keyRepeated - amount of mouse movement along Y-axis

  @return  none
*/
static void hidCCSendReport( uint8 cmd, bool keyPressed, uint8 keyRepeated )
{
    // Only send the report if something meaningful to report
    if ( keyRepeated == 0 )
    {
        uint8 buf[HID_CC_IN_RPT_LEN] = {0};

        // No need to include Report Id
        if ( keyPressed )
        {
            buf[0]=0X20;

        }
        else
        {
            buf[0]=0X00;

        }

        HidDev_Report( HID_RPT_ID_CC_IN, HID_REPORT_TYPE_INPUT,
                        HID_CC_IN_RPT_LEN, buf );
    }
}
void hidCCSendReportKey( uint8 cmd, bool keyPressed)
{
    uint8 buf[1] = {0};

    if(keyPressed==1)
    {
        buf[0]=cmd;
    }
    else
        buf[0]=0;

    HidDev_Report( HID_RPT_ID_CC_IN, HID_REPORT_TYPE_INPUT, 1, buf );
}
#endif
/*********************************************************************

*********************************************************************/

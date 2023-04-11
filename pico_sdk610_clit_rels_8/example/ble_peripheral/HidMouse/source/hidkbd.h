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

#ifndef HIDEMUKBD_H
#define HIDEMUKBD_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
    INCLUDES
*/

/*********************************************************************
    CONSTANTS
*/

// Task Events
#define START_DEVICE_EVT                              0x0001
#define SPP_DISABLE_LATENCY_EVT                       0x0002
#define SPP_PERIODIC_EVT                              0x0004

#define APPL_TXQU_PULL_EVT                            0x0010

#define HID_TEST_EVT                                  0x0100

#define SBP_CONN_ESTA_EVT                               (0x2000)
#define SBP_BOND_READY_EVT                            0x4000

/*********************************************************************
    MACROS
*/
#define APP_BLE_MODE TRUE
#define APP_2P4G_MODE FALSE

/*********************************************************************
    FUNCTIONS
*/
#if 0
#define arry_queu_ini_queu(queu, size)  int arry_##queu##_queu[size]; uint8 arry_##queu##_free = size; uint8 arry_##queu##_push = 0; uint8 arry_##queu##_pull = 0;
#define arry_queu_get_size(queu)        ( sizeof(arry_##queu##_queu)/sizeof(arry_##queu##_queu[0]) )
#define arry_queu_chk_empt(queu)        ( arry_##queu##_free == arry_queu_get_size(queu) )
#define arry_queu_chk_full(queu)        ( arry_##queu##_free == 0 )
#define arry_queu_psh_node(queu, para)                                  \
    do {                                                                \
        if ( !arry_queu_chk_full(queu) ) {                              \
            arry_##queu##_free -= 1;                                    \
            arry_##queu##_queu[arry_##queu##_push] = para;              \
            arry_##queu##_push = ((arry_##queu##_push + 1) % arry_queu_get_size(queu)); \
        }                                                               \
    } while ( 0 )
#define arry_queu_pek_node(queu, para)                                  \
    do {                                                                \
        para = 0;                                                       \
        if ( !arry_queu_chk_empt(queu) ) {                              \
            para = arry_##queu##_queu[arry_##queu##_pull];              \
        }                                                               \
    } while ( 0 )
#define arry_queu_pll_node(queu, para)                                  \
    do {                                                                \
        if ( !arry_queu_chk_empt(queu) ) {                              \
        arry_queu_pek_node(queu, para);                                 \
        /* if ( para ) {                               */\
            arry_##queu##_pull  = ((arry_##queu##_pull + 1) % arry_queu_get_size(queu)); \
            arry_##queu##_free += 1;                                    \
        }                                                               \
    } while ( 0 )
#endif
/*********************************************************************
    GLOBAL VARIABLES
*/

extern uint8 application_TaskID;
extern uint8 g_instant_cnt;

/*
    Task Initialization for the BLE Application
*/
extern void HidKbd_Init( uint8 task_id );

int
HidKbd_chk_conn(void);

int
HidKbd_chk_bond(void);
/*
    Task Event Processor for the BLE Application
*/
extern uint16 HidKbd_ProcessEvent( uint8 task_id, uint16 events );
extern uint8 hidKbdSendVoiceCMDtReport( uint8 keycode );
extern void hidCCSendReportKey( uint8 cmd, bool keyPressed);
//extern void hidKbdSendMouseReport_test( uint8 x,uint8 y);
extern void hidKbdSendMouseReport( uint8 buttons,uint8 x,uint8 y,uint8 wheel);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /*HIDEMUKBD_H */

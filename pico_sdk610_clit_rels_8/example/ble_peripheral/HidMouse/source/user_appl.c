/*
 ******************************************************************************
 * Name:        user_devs.c
 * description: implements:
 *                  TODO:
 * History:     Date;           Author;         Description
 *              21 Dec. 2018;   Chen, George;   file creation.
 ******************************************************************************
 */


/*
 ******************************************************************************
 * Includes
 ******************************************************************************
 */
#include "user_appl.h"
#include "pcba_cfgs.h"
#include "otp.h"
#include "rom_sym_def.h"
#include "OSAL.h"
#include "l2cap.h"
#include "gap.h"
#include "gpio.h"
#include "pwrmgr.h"
#include "spi_lite.h"
#include "gpio_debounce_key.h"
#include "log.h"
#include "sensor.h"
#include "snsr_shub.h"
#include "arry_queu.h"
#include "hidkbd.h"


/*
 ******************************************************************************
 * Definition
 ******************************************************************************
 */
// #define USER_DEVS_CFGS_LOGS_ENAB
#define USER_APPL_KEY_ENABLE    1
#define USER_APPL_WHEEL_ENABLE  1


#ifdef  USER_DEVS_CFGS_LOGS_ENAB
#define USER_DEVS_CFGS_LOGS_TAGS        "USER_APPL"
/* ERROR */
#define logs_err(fmt, ...)                                                \
    logs_logs_err(USER_DEVS_CFGS_LOGS_TAGS, fmt, ##__VA_ARGS__)

/* WARNING */
#define logs_war(fmt, ...)                                                \
    logs_logs_war(USER_DEVS_CFGS_LOGS_TAGS, fmt, ##__VA_ARGS__)

/* INFORMATION */
#define logs_inf(fmt, ...)                                                \
    LOG_USR0("[INF][%s]",USER_DEVS_CFGS_LOGS_TAGS);LOG_USR0(fmt,##__VA_ARGS__)

/* VERB */
#define logs_ver(fmt, ...)                                                \
    logs_logs_ver(USER_DEVS_CFGS_LOGS_TAGS, fmt, ##__VA_ARGS__)

/* Function entry */
#define logs_ent(fmt, ...)                                                \
    LOG_USR0("[%s]",USER_DEVS_CFGS_LOGS_TAGS);LOG_USR0("%s(",__func__);LOG_USR0(fmt,##__VA_ARGS__);LOG_USR0(")\r\n")

/* function exit */
#define logs_exi(fmt, ...)                                                \
    logs_logs_exi(USER_DEVS_CFGS_LOGS_TAGS, fmt, ##__VA_ARGS__)

#else
/* ERROR */
#define logs_err(fmt, ...)

/* WARNING */
#define logs_war(fmt, ...)

/* INFORMATION */
#define logs_inf(fmt, ...)

/* VERB */
#define logs_ver(fmt, ...)

/* Function entry */
#define logs_ent(fmt, ...)

/* function exit */
#define logs_exi(fmt, ...)

#endif /* USER_DEVS_CFGS_LOGS_TAG */

#define USER_APPL_CFGS_TEST_SUIT        (0)
#define USER_APPL_CFGS_USES_AVRG        (1)

#define USER_APPL_CFGS_BTLE_INTV        (8)
// #define USER_APPL_CFGS_2_4G_INTV        (1)
// #define USER_APPL_CFGS_2_4G_INTV        (2)
#define USER_APPL_CFGS_2_4G_INTV        (8)

#define USER_APPL_CFGS_FLIP_X           PCAB_CFGS_CFGS_FLIP_X
#define USER_APPL_CFGS_FLIP_Y           PCAB_CFGS_CFGS_FLIP_Y
#define USER_APPL_CFGS_SWAP_XY          PCAB_CFGS_CFGS_SWAP_XY

#define USER_APPL_CFGS_IDL0_INTV        (user_appl_get_mode() ? USER_APPL_CFGS_BTLE_INTV : USER_APPL_CFGS_2_4G_INTV)
#define USER_APPL_CFGS_IDL0_TOUT        ((1*5*1000)/USER_APPL_CFGS_IDL0_INTV)
#if defined(PCAB_CFGS_CFGS_MOTI_PINN)
#define USER_APPL_CFGS_IDL1_INTV        (5000)
#else
#define USER_APPL_CFGS_IDL1_INTV        (100)
#endif
#define USER_APPL_CFGS_IDL1_TOUT        ((2*60*1000)/USER_APPL_CFGS_IDL1_INTV + USER_APPL_CFGS_IDL0_TOUT)
#define USER_APPL_CFGS_IDL2_INTV        (1000)
#define USER_APPL_CFGS_IDL2_TOUT        ((2*60*1000)/USER_APPL_CFGS_IDL2_INTV + USER_APPL_CFGS_IDL0_TOUT)

#define USER_APPL_CFGS_USES_DPIS        (1)
#define USER_APPL_CFGS_DPIS_BGNA        (0x1FFFBFC0)    // OPT BGNS ADDR
#define USER_APPL_CFGS_DPIS_ENDA        (0x1FFFC000)    // OPT ENDS ADDR
#define USER_APPL_CFGS_DPIS_ITMX        (200)   // MAXI ENTRY
#define USER_APPL_CFGS_DPIS_ITSZ        (2)     // ENTRY SIZE IN BIT
#define USER_APPL_CFGS_DPIS_MASK        (~((-1U) << USER_APPL_CFGS_DPIS_ITSZ))   // ENTRY MASK
#define USER_APPL_CFGS_DPIS_RMSZ        (USER_APPL_CFGS_DPIS_ITMX*USER_APPL_CFGS_DPIS_ITSZ/8)    // ROOM SIZE IN BYTE
#if ((USER_APPL_CFGS_DPIS_BGNA + USER_APPL_CFGS_DPIS_RMSZ) >= USER_APPL_CFGS_DPIS_ENDA)
#error "!! DPIS ROOM EXCESS LIMIT !!"
#endif

/*
 ******************************************************************************
 * private variable definitions
 ******************************************************************************
 */
static uint8 user_appl_task = 0xff;
static uint8 user_appl_btle = 1;
static uint32 user_appl_dpis_addr = 0;
static uint32 user_appl_dpis_shft = 0;
static uint32 user_appl_dpis_valu = 0;

#if (1 == USER_APPL_CFGS_TEST_SUIT)
// static uint32 user_appl_test = BIT(0);  // force test mode 0
static uint32 user_appl_test = 0x00;
#endif

HIDMOUSE_DATA hidMouse_data;

static gpioin_t gpio_intr[16];  // 16 should be large enough

#if (defined(PCAB_CFGS_CFGS_USES_GDKS) && PCAB_CFGS_CFGS_USES_GDKS)
static gdkey_t keys_cfgs[] = {
    {PCAB_CFGS_CFGS_KEYL_PINN, 1, 0, GPIO_PULL_UP},  // left
    {PCAB_CFGS_CFGS_KEYR_PINN, 1, 0, GPIO_PULL_UP},  // right
    {PCAB_CFGS_CFGS_KEYM_PINN, 1, 0, GPIO_PULL_UP},  // middle

    {PCAB_CFGS_CFGS_DPI_PINN,  1, 0, GPIO_PULL_UP},   // DPI
};

#else
static uint8_t  const keys_cfgs[] = {
    PCAB_CFGS_CFGS_KEYL_PINN,  // left
    PCAB_CFGS_CFGS_KEYR_PINN,  // right
    PCAB_CFGS_CFGS_KEYM_PINN,  // middle
    // PCAB_CFGS_CFGS_DPI_PINN,   // DPI   // NONE-HID KEYS
    // PCAB_CFGS_CFGS_4KEY_PINN,  // key4
    // PCAB_CFGS_CFGS_5KEY_PINN,  // key5
};
#endif

#if (1 == USER_APPL_CFGS_USES_AVRG)
arry_queu_ini_queu(avgx, 2);
arry_queu_ini_queu(avgy, 2);

// arry_queu_ini_queu(avgx, 3);
// arry_queu_ini_queu(avgy, 3);
#endif


/*
 ******************************************************************************
 * private function implementations
 ******************************************************************************
 */
// static volatile int8 wheel_counter = 0;
// static volatile int8 wheel_send = 0;
// static uint8 wheel_wait = 0;
// static int8 wheel_last = 0;
static uint8 pre_low_pin = GPIO_DUMMY;

static void user_appl_wheel_send_handler(void)
{    
    // if (hidMouse_data.wheel == 1)
    // {
    //     LOG_ERROR("wheel send +\n");
    // }
    // else
    // {
    //     LOG_ERROR("wheel send -\n");
    // }

    hidKbdSendMouseReport(0, 0, 0, 1);
}

static void user_appl_whls_hdlr(gpio_pin_e pinn, gpio_polarity_e type)
{
    // LOG_ERROR("pinn:%d, evnt:%d\n", pinn, type);
    _HAL_CS_ALLOC_(); HAL_ENTER_CRITICAL_SECTION();
    if (type)
    {
        if ( pre_low_pin == pinn)
        {
            pwrmgr_lock(MOD_GPIO);
            if (gpio_read(PCAB_CFGS_CFGS_WHLA_PINN) && gpio_read(PCAB_CFGS_CFGS_WHLB_PINN))
            {
                if (PCAB_CFGS_CFGS_WHLA_PINN == pinn)
                {
                    hidMouse_data.wheel = 1;
                    // LOG_ERROR("wheel send +\n");
                }
                else
                {
                    hidMouse_data.wheel = -1;
                // LOG_ERROR("wheel send -\n");
                }
                osal_start_timerEx(user_appl_task, USER_APPL_WHEEL_SEND_EVT, 15);
            } 
            pwrmgr_unlock(MOD_GPIO);
        }
    }
    else
    {
        pwrmgr_lock(MOD_GPIO);
        if (PCAB_CFGS_CFGS_WHLA_PINN == pinn)
        {
            if (!gpio_read(PCAB_CFGS_CFGS_WHLB_PINN))
            {
                pre_low_pin = pinn;
            }
        }
        else
        {
            if (!gpio_read(PCAB_CFGS_CFGS_WHLA_PINN))
            {
                pre_low_pin = pinn;
            }
        }
        pwrmgr_unlock(MOD_GPIO);
    }
    HAL_EXIT_CRITICAL_SECTION();
}

#if defined(PCAB_CFGS_CFGS_MOTI_PINN)
static void
user_appl_moti_hdlr(gpio_pin_e pinn, gpio_polarity_e type)
{
    // logs_ent("pinn:%d,type:%d", pinn,type);

    /* might miss irqs */
    if ( HidKbd_chk_conn() && HidKbd_chk_bond() )
    if ( PCAB_CFGS_CFGS_MOTI_PINN == pinn && POL_ACT_LOW == type ) {
        // osal_stop_timerEx(user_appl_task, USER_CHCK_MOTI_EVNT);
                if ( idle >= USER_APPL_CFGS_IDL0_TOUT )
                if ( idle < USER_APPL_CFGS_IDL1_TOUT ) {

        osal_set_event(user_appl_task, USER_CHCK_MOTI_EVNT);
                    }    }
}
#endif

#if defined(PCAB_CFGS_CFGS_PCBA_6230SC_MOUSE_YJX) || defined(PCAB_CFGS_CFGS_PCBA_6230SC_MOUSE_WK303)

static void user_appl_key_scan_start(void)
{
    pwrmgr_lock(MOD_GPIO);
    for ( uint8_t itr0 = 0; itr0 < 3; itr0 ++ )
    {
        gpioin_unregister( keys_cfgs[itr0]);
        // gpio_interrupt_disable(keys_cfgs[itr0]);
    }
    gpio_pull_set(PCAB_CFGS_CFGS_MULTI_KEY_PINN, GPIO_PULL_UP);
    gpio_pull_set(PCAB_CFGS_CFGS_BOND_KEY1_PINN, GPIO_PULL_UP);
    gpio_pull_set(PCAB_CFGS_CFGS_BOND_KEY2_PINN, GPIO_PULL_UP);

    osal_start_timerEx(user_appl_task, USER_APPL_KEY_SCAN_EVT, 5);
}

static void user_appl_keys_hdlr(gpio_pin_e pinn, gdkey_evt_e evt)
{
    // LOG_ERROR("pinn:%d, evnt:%d\n", pinn, evt);
    _HAL_CS_ALLOC_(); HAL_ENTER_CRITICAL_SECTION();
    user_appl_key_scan_start();
    HAL_EXIT_CRITICAL_SECTION();
}

static void user_appl_key_scan_end(void)
{
    gpio_pull_set(PCAB_CFGS_CFGS_MULTI_KEY_PINN, GPIO_PULL_DOWN);
    gpio_pull_set(PCAB_CFGS_CFGS_BOND_KEY1_PINN, GPIO_PULL_DOWN);
    gpio_pull_set(PCAB_CFGS_CFGS_BOND_KEY2_PINN, GPIO_PULL_DOWN);

    gpioin_register(PCAB_CFGS_CFGS_KEYL_PINN, NULL, (gpioin_Hdl_t)user_appl_keys_hdlr);
    gpioin_register(PCAB_CFGS_CFGS_KEYR_PINN, NULL, (gpioin_Hdl_t)user_appl_keys_hdlr);
    gpioin_register(PCAB_CFGS_CFGS_KEYM_PINN, NULL, (gpioin_Hdl_t)user_appl_keys_hdlr);
    pwrmgr_unlock(MOD_GPIO);
}

static void user_appl_key_scan_handler(void)
{
    static uint8_t keys_curr = 0;
    // const  uint8_t keys_numb = sizeof(keys_cfgs) / sizeof(keys_cfgs[0]);
    uint8_t keys_pre = keys_curr;
    uint8_t keys_rep;

    for ( uint8_t i = 0; i < 3; i ++ )
    {
        gpio_pull_set(keys_cfgs[i], GPIO_PULL_UP);
        gpio_dir(keys_cfgs[i], GPIO_INPUT);

        if (gpio_read(keys_cfgs[i]))
        {//key release
            gpio_pull_set(keys_cfgs[i], GPIO_PULL_DOWN);
            gpio_dir(keys_cfgs[i], GPIO_INPUT);

            if (gpio_read(keys_cfgs[i]))
            {//mul key press
                // LOG_ERROR("key %d press\n",i + 3);
                keys_curr |= (1 << (i + 3));
            }
            else
            {//all key release
                // LOG_ERROR("key release\n");
                keys_curr &= (~(9 << i));
            }
            gpio_pull_set(keys_cfgs[i], GPIO_PULL_UP);
        }
        else
        {//key press
            // LOG_ERROR("key %d press\n",i);
            keys_curr |= (1 << i);

            if (gpio_read(PCAB_CFGS_CFGS_MULTI_KEY_PINN))
            {
                // LOG_ERROR("key %d release\n",i + 3);
                keys_curr &= (~(1 << (i + 3)));
            }
            else
            {
                // LOG_ERROR("key %d press\n",i + 3);
                keys_curr |= (1 << (i + 3));
            }
        }
    }

    if (keys_curr || keys_pre)
    {
        if (keys_pre != keys_curr)
        {
            //send data
            keys_rep = (keys_curr & 0x07) | ((keys_curr & 0x30) >> 1);
            if (keys_rep != hidMouse_data.button)
            {
                LOG_ERROR("key send\n");
                hidMouse_data.button = keys_rep;
                hidKbdSendMouseReport(1, 0, 0, 0);
            }
            else
            {
                if (keys_curr & 0x08)
                {//dpi press
                    LOG_ERROR("dpi change\n");
                    osal_set_event(user_appl_task, USER_CFGS_CPIS_EVNT);
                }
                else
                {//dpi release

                }
            }
        }
        osal_start_timerEx(user_appl_task, USER_APPL_KEY_SCAN_EVT, 20);
    }
    else
    {
        user_appl_key_scan_end();
    }
}
#else

static void
user_appl_keys_hdlr(gpio_pin_e pinn, gdkey_evt_e evt)
{
    // LOG_ERROR("pinn:%d, evnt:%d\n", pinn, evt);
    static volatile uint8_t keys_curr = 0xff;
    const  uint8_t keys_numb = sizeof(keys_cfgs) / sizeof(keys_cfgs[0]);

#if   defined(PCAB_CFGS_CFGS_PCBA_6230SD_MOUSE_V1_3)
#else
    /* DPI */
    if ( PCAB_CFGS_CFGS_DPI_PINN == pinn ) {
    #if (1 == USER_APPL_CFGS_TEST_SUIT)
        static uint32 tick = 0;
        if ( 0 == evt ) {
            tick = get_systick();
        } else
    #endif
        if ( POSEDGE == evt ) {   // release
    #if (1 == USER_APPL_CFGS_TEST_SUIT)
            if ( 5000 <= get_ms_intv(tick) ) {
            // extern uint32 pIrqCnt;
            // extern uint32 pRtoCnt;
            // extern uint32 pCerrCnt;
            // extern uint32 pCokCnt;
            // extern uint32 linkCnt;
            // logs_inf("linkCnt:%d,pIrqCnt:%d,pRtoCnt:%d,pCerrCnt:%d,pCokCnt:%d, Rtio:%d/%d\r\n",
            //           linkCnt,   pIrqCnt,   pRtoCnt,   pCerrCnt,   pCokCnt,   pCokCnt,pIrqCnt);
            // linkCnt = pIrqCnt = pRtoCnt = pCerrCnt = pCokCnt = 0;

            user_appl_test ^= BIT(0);
            logs_inf("TEST: %x \r\n", user_appl_test);
            osal_set_event(user_appl_task, USER_CHCK_MOTI_EVNT);
            } else
    #endif
            osal_set_event(user_appl_task, USER_CFGS_CPIS_EVNT);
        }
        return;
    }

    for ( uint8_t itr0 = 0; itr0 < keys_numb; itr0 ++ ) {
    #if (defined(PCAB_CFGS_CFGS_USES_GDKS) && PCAB_CFGS_CFGS_USES_GDKS)
        if ( pinn == keys_cfgs[itr0].pin ) {
    #else
        if ( pinn == keys_cfgs[itr0] ) {
    #endif
            keys_curr = (keys_curr & ~(1 << itr0)) | (evt << itr0);
            hidMouse_data.button = ~keys_curr;
            // logs_inf("keys: %x\n", hidMouse_data.button);
            // hidMouse_data.x = hidMouse_data.y = 0;
            osal_set_event(user_appl_task, USER_CHCK_KEYS_EVNT);
            return;
        }
    }
#endif
}
#endif

#if  !defined(PCAB_CFGS_CFGS_PCBA_6230SC_MOUSE_YJX) && !defined(PCAB_CFGS_CFGS_PCBA_6230SC_MOUSE_WK303)
#if 1   // rset keys status, in case of missing keys
static uint8_t
user_appl_chk_keys(void* cfgs, uint8 numb)
{
    uint8_t keys_curr = 0xff;

    for ( uint8_t itr0 = 0; itr0 < numb; itr0 ++ ) {
    #if (defined(PCAB_CFGS_CFGS_USES_GDKS) && PCAB_CFGS_CFGS_USES_GDKS)
            keys_curr = (keys_curr & ~(1 << itr0)) | (gpio_read(((gdkey_t*)cfgs)[itr0].pin) << itr0);
    #else
            keys_curr = (keys_curr & ~(1 << itr0)) | (gpio_read(((uint8_t*)cfgs)[itr0]) << itr0);
    #endif
    }

    return ( keys_curr );
}
#endif

static void
user_appl_dpis_hdlr(gpio_pin_e pinn, gpio_polarity_e type)
{
    // LOG_ERROR("pinn:%d, evnt:%d\n", pinn, type);
    /* DPI */
    if ( PCAB_CFGS_CFGS_DPI_PINN == pinn ) {
    #if (1 == USER_APPL_CFGS_TEST_SUIT)
        static uint32 tick = 0;
        if ( 0 == type ) {
            tick = get_systick();
        } else
    #endif
        if ( POSEDGE == type ) {   // release
    #if (1 == USER_APPL_CFGS_TEST_SUIT)
            if ( 5000 <= get_ms_intv(tick) ) {
            // extern uint32 pIrqCnt;
            // extern uint32 pRtoCnt;
            // extern uint32 pCerrCnt;
            // extern uint32 pCokCnt;
            // extern uint32 linkCnt;
            // logs_inf("linkCnt:%d,pIrqCnt:%d,pRtoCnt:%d,pCerrCnt:%d,pCokCnt:%d, Rtio:%d/%d\r\n",
            //           linkCnt,   pIrqCnt,   pRtoCnt,   pCerrCnt,   pCokCnt,   pCokCnt,pIrqCnt);
            // linkCnt = pIrqCnt = pRtoCnt = pCerrCnt = pCokCnt = 0;

            user_appl_test ^= BIT(0);
            logs_inf("TEST: %x \r\n", user_appl_test);
            osal_set_event(user_appl_task, USER_CHCK_MOTI_EVNT);
            } else
    #endif
            osal_set_event(user_appl_task, USER_CFGS_CPIS_EVNT);
        }
    }
}
#endif

#if 0
static uint8
user_appl_chk_moti(void)
{
    uint8 rslt = 1;
    uint8 keys = 0;
    uint8 moti;
    int32 dlta;
    int32 dltx;
    int32 dlty;

    #if (1 == USER_APPL_CFGS_TEST_SUIT)
        if ( user_appl_test & BIT(0) )    // RPRT RATE TEST
        {
            // LOG_ERROR("> ");
            static uint32 itr0 = 0;
            itr0 = (itr0 + 1) % 2;
            hidMouse_data.x = hidMouse_data.y = (itr0 ? +10 : -10);
            hidKbdSendMouseReport(0, 1, 1, 0);
            osal_start_timerEx(user_appl_task, USER_CHCK_MOTI_EVNT, user_appl_get_mode() ? USER_APPL_CFGS_BTLE_INTV : USER_APPL_CFGS_2_4G_INTV);
            return ( evnt ^ USER_CHCK_MOTI_EVNT );
        }
    #endif

    if ( HidKbd_chk_conn() && HidKbd_chk_bond() ) {
        // rset keys status, in case of missing keys
        #if  !defined(PCAB_CFGS_CFGS_PCBA_6230SC_MOUSE_YJX) && !defined(PCAB_CFGS_CFGS_PCBA_6230SC_MOUSE_WK303)
        keys = (hidMouse_data.button ^ ~user_appl_chk_keys((void *)keys_cfgs, sizeof(keys_cfgs)/sizeof(keys_cfgs[0]) - 1));
        #endif
        dltx = dlty = 0;
        moti = snsr_shub_chk_moti();
        // logs_inf("moti:[%02x]\r\n", moti);
        if ( (moti & BIT(7)) ) {
            dltx = snsr_shub_get_dltx();
            dlty = snsr_shub_get_dlty();
            // logs_inf("dlta :[%d,%d]\r\n",dltx,dlty);

            if ( (moti & (BIT(4)|BIT(3))) ) {
            dltx = dlty = 0;
            }
        }

        if ( (keys) || (0 != dltx) || (0 != dlty) ) {
            rslt = 0;
            // if ( USER_APPL_CFGS_IDL0_INTV < get_ms_intv(osal_get_timeoutEx(user_appl_task, USER_CHCK_IDLE_EVNT)) ) {
            // if ( USER_APPL_CFGS_IDL0_TOUT <  user_appl_idle ) {
            //     // osal_set_event(user_appl_task, USER_CHCK_IDLE_EVNT);
            //     osal_start_timerEx(user_appl_task, USER_CHCK_IDLE_EVNT, USER_APPL_CFGS_IDL0_INTV);
            // }
            // user_appl_moti = 0;
            // user_appl_idle = 0;

        #if (1 == USER_APPL_CFGS_USES_AVRG)
            // if ( 0 != dltx /* && 0 != dlty */ )
            {
            arry_queu_pll_node(avgx, dlta);
            arry_queu_psh_node(avgx, dltx);
            dltx = arry_queu_fnd_aavg(avgx);
            }
        #endif

        #if (1 == USER_APPL_CFGS_USES_AVRG)
            // if ( /* 0 != dltx && */ 0 != dlty )
            {
            arry_queu_pll_node(avgy, dlta);
            arry_queu_psh_node(avgy, dlty);
            dlty = arry_queu_fnd_aavg(avgy);
            }
        #endif
            // logs_inf("dlta` :[%d,%d]\r\n",dltx,dlty);

            hidMouse_data.x =
            (uint8)
            #if (1 == USER_APPL_CFGS_FLIP_X)
            -
            #endif
            #if (1 == USER_APPL_CFGS_SWAP_XY)
            (dlty);
            #else
            (dltx);
            #endif

            hidMouse_data.y =
            (uint8)
            #if (1 == USER_APPL_CFGS_FLIP_Y)
            -
            #endif
            #if (1 == USER_APPL_CFGS_SWAP_XY)
            (dltx);
            #else
            (dlty);
            #endif

            hidKbdSendMouseReport(keys, dltx, dlty, 0);
        }
    }

    return ( rslt );
}
#endif

/*
 ******************************************************************************
 * public function implementations
 ******************************************************************************
 */
// if ( events & PARAM_UPDATE_EVT )
// 	{
// 		if( gapRole_ConnectionHandle != INVALID_CONNHANDLE )
// 		{
// 			l2capParamUpdateReq_t updateReq;
// //			uint16 timeout = GAP_GetParamValue( TGAP_CONN_PARAM_TIMEOUT );
// 			updateReq.intervalMin = GAP_DEFAULT_DESIRED_MIN_CONN_INTERVAL;
// 			updateReq.intervalMax = GAP_DEFAULT_DESIRED_MAX_CONN_INTERVAL;
// 			updateReq.slaveLatency = GAP_DEFAULT_DESIRED_SLAVE_LATENCY;
// 			updateReq.timeoutMultiplier = GAP_DEFAULT_DESIRED_CONN_TIMEOUT;
// 			L2CAP_ConnParamUpdateReq( gapRole_ConnectionHandle, &updateReq, gapRole_TaskID );
// 		}
// 		return (events ^ PARAM_UPDATE_EVT);
// 	}
void
user_appl_ini(uint8 task)
{
    user_appl_task = task;

    /* int snsr, less powr */
    snsr_shub_ini_snsr( );
    // snsr_shub_pwr_snsr(1);

#if (1 == USER_APPL_CFGS_USES_AVRG)
    arry_queu_rst_queu(avgx);
    arry_queu_rst_queu(avgy);
#endif

#if (1  == USER_APPL_CFGS_USES_DPIS)
    AP_WDT_FEED;    // wdog feed
    uint32 regv = 0;
    for ( user_appl_dpis_addr = USER_APPL_CFGS_DPIS_BGNA; (user_appl_dpis_addr < USER_APPL_CFGS_DPIS_BGNA+USER_APPL_CFGS_DPIS_RMSZ); user_appl_dpis_addr += sizeof(regv) ) {
        otp_read_data(user_appl_dpis_addr, &regv, 1, OTP_USER_READ_MODE);
        if ( 0U != regv ) break;
    }

    if ( user_appl_dpis_addr < USER_APPL_CFGS_DPIS_BGNA+USER_APPL_CFGS_DPIS_RMSZ ) {
        uint32 mask = USER_APPL_CFGS_DPIS_MASK;
        for ( user_appl_dpis_shft = (32-USER_APPL_CFGS_DPIS_ITSZ); 0 <= user_appl_dpis_shft; user_appl_dpis_shft -= USER_APPL_CFGS_DPIS_ITSZ ) {
            user_appl_dpis_valu = ((regv >> user_appl_dpis_shft) & mask);
            if ( 0U != user_appl_dpis_valu ) {
                break;
            }
        }
        logs_inf("dpis:[addr:%08x,shft:%08x,valu:%08x,regv:%08x] \r\n", user_appl_dpis_addr, user_appl_dpis_shft, user_appl_dpis_valu, regv);
    } else {
        logs_inf("dpis_addr full:[%08x] \r\n", user_appl_dpis_addr);
    }
    /* set dips with cfgs */
    snsr_shub_set_cpis(user_appl_dpis_valu);
#endif

    gpioin_init(gpio_intr, sizeof(gpio_intr)/sizeof(gpio_intr[0]));

#if (defined(USER_APPL_KEY_ENABLE) && USER_APPL_KEY_ENABLE)
#if (defined(PCAB_CFGS_CFGS_USES_GDKS) && PCAB_CFGS_CFGS_USES_GDKS)
    gdkey_register(keys_cfgs, sizeof(keys_cfgs)/sizeof(keys_cfgs[0]), user_appl_keys_hdlr);
    // LOG_ERROR("gdkey_regi rslt:%d\n", rslt);
#else
    gpio_dir(PCAB_CFGS_CFGS_KEYL_PINN, GPIO_INPUT);
    gpio_dir(PCAB_CFGS_CFGS_KEYR_PINN, GPIO_INPUT);
    gpio_dir(PCAB_CFGS_CFGS_KEYM_PINN, GPIO_INPUT);
    gpio_pull_set(PCAB_CFGS_CFGS_KEYL_PINN, GPIO_PULL_UP);
    gpio_pull_set(PCAB_CFGS_CFGS_KEYR_PINN, GPIO_PULL_UP);
    gpio_pull_set(PCAB_CFGS_CFGS_KEYM_PINN, GPIO_PULL_UP);

#if defined(PCAB_CFGS_CFGS_PCBA_6230SC_MOUSE_YJX) || defined(PCAB_CFGS_CFGS_PCBA_6230SC_MOUSE_WK303)
    gpioin_register(PCAB_CFGS_CFGS_KEYL_PINN, NULL, (gpioin_Hdl_t)user_appl_keys_hdlr);
    gpioin_register(PCAB_CFGS_CFGS_KEYR_PINN, NULL, (gpioin_Hdl_t)user_appl_keys_hdlr);
    gpioin_register(PCAB_CFGS_CFGS_KEYM_PINN, NULL, (gpioin_Hdl_t)user_appl_keys_hdlr);
#else
    gpioin_register(PCAB_CFGS_CFGS_KEYL_PINN, (gpioin_Hdl_t)user_appl_keys_hdlr, (gpioin_Hdl_t)user_appl_keys_hdlr);
    gpioin_register(PCAB_CFGS_CFGS_KEYR_PINN, (gpioin_Hdl_t)user_appl_keys_hdlr, (gpioin_Hdl_t)user_appl_keys_hdlr);
    gpioin_register(PCAB_CFGS_CFGS_KEYM_PINN, (gpioin_Hdl_t)user_appl_keys_hdlr, (gpioin_Hdl_t)user_appl_keys_hdlr);
#endif

    #if   defined(PCAB_CFGS_CFGS_PCBA_6230SD_MOUSE_V1_3)

    gpio_dir(PCAB_CFGS_CFGS_KEY0_PINN, GPIO_INPUT);
    gpio_pull_set(PCAB_CFGS_CFGS_KEY0_PINN, GPIO_PULL_DOWN);
    gpioin_register(PCAB_CFGS_CFGS_KEY0_PINN, (gpioin_Hdl_t)user_appl_keys_hdlr, (gpioin_Hdl_t)user_appl_keys_hdlr);

    #elif     defined(PCAB_CFGS_CFGS_PCBA_6230SC_MOUSE_YJX) || defined(PCAB_CFGS_CFGS_PCBA_6230SC_MOUSE_WK303)

    gpio_dir(PCAB_CFGS_CFGS_MULTI_KEY_PINN, GPIO_INPUT);
    gpio_dir(PCAB_CFGS_CFGS_BOND_KEY1_PINN, GPIO_INPUT);
    gpio_dir(PCAB_CFGS_CFGS_BOND_KEY2_PINN, GPIO_INPUT);

    gpio_pull_set(PCAB_CFGS_CFGS_MULTI_KEY_PINN, GPIO_PULL_DOWN);
    gpio_pull_set(PCAB_CFGS_CFGS_BOND_KEY1_PINN, GPIO_PULL_DOWN);
    gpio_pull_set(PCAB_CFGS_CFGS_BOND_KEY2_PINN, GPIO_PULL_DOWN);

    #else

    /* DPI */
    #if defined(PCAB_CFGS_CFGS_DPI_PINN)
    gpio_pull_set(PCAB_CFGS_CFGS_DPI_PINN, GPIO_PULL_UP);
    gpio_dir(PCAB_CFGS_CFGS_DPI_PINN, GPIO_INPUT);
    gpioin_register(PCAB_CFGS_CFGS_DPI_PINN, (gpioin_Hdl_t)user_appl_dpis_hdlr, (gpioin_Hdl_t)user_appl_dpis_hdlr);
    #endif
    #endif
#endif
#endif

#if (defined(USER_APPL_WHEEL_ENABLE) && USER_APPL_WHEEL_ENABLE)
#if 1
    /* wheel, init GPIO_PULL_DOWN less powr csum */
    gpio_pull_set(PCAB_CFGS_CFGS_WHLA_PINN, GPIO_PULL_UP);
    gpio_pull_set(PCAB_CFGS_CFGS_WHLB_PINN, GPIO_PULL_UP);
    gpio_dir(PCAB_CFGS_CFGS_WHLA_PINN, GPIO_INPUT);
    gpio_dir(PCAB_CFGS_CFGS_WHLB_PINN, GPIO_INPUT);

    gpioin_register(PCAB_CFGS_CFGS_WHLA_PINN, (gpioin_Hdl_t)user_appl_whls_hdlr, (gpioin_Hdl_t)user_appl_whls_hdlr);
    gpioin_register(PCAB_CFGS_CFGS_WHLB_PINN, (gpioin_Hdl_t)user_appl_whls_hdlr, (gpioin_Hdl_t)user_appl_whls_hdlr);
#endif
#endif

#if defined(PCAB_CFGS_CFGS_MOTI_PINN)
    gpio_pull_set(PCAB_CFGS_CFGS_MOTI_PINN, GPIO_PULL_UP);
    gpio_dir(PCAB_CFGS_CFGS_MOTI_PINN, GPIO_INPUT);
    gpioin_register(PCAB_CFGS_CFGS_MOTI_PINN, (gpioin_Hdl_t)user_appl_moti_hdlr, (gpioin_Hdl_t)user_appl_moti_hdlr);
#endif

#if defined(PCAB_CFGS_CFGS_PCBA_6230_MOUSE_BYKC_V1_1)
    gpio_pull_set(PCAB_CFGS_CFGS_LED0_PINN, GPIO_PULL_UP);
    gpio_write(PCAB_CFGS_CFGS_LED0_PINN, 1);
    gpio_retention(PCAB_CFGS_CFGS_LED0_PINN, TRUE);
    gpio_pull_set(PCAB_CFGS_CFGS_LED1_PINN, GPIO_PULL_DOWN);
    gpio_write(PCAB_CFGS_CFGS_LED1_PINN, 0);
    gpio_retention(PCAB_CFGS_CFGS_LED1_PINN, TRUE);
#endif

    osal_set_event(user_appl_task, USER_CHCK_MOTI_EVNT);
    // osal_start_timerEx(user_appl_task, USER_CHCK_MOTI_EVNT, 10);
    // osal_start_timerEx(user_appl_task, USER_POWR_OFFS_EVNT, 10*1000);
}

void
user_appl_chk_mode(void)
{
    /* OS_PINN =  GPIO_P06 */
    /* init GPIO_PULL_DOWN less powr csum */
    gpio_pull_set(PCAB_CFGS_CFGS_MODE_PINN, GPIO_PULL_DOWN);
    gpio_dir(PCAB_CFGS_CFGS_MODE_PINN, GPIO_INPUT);
    user_appl_btle = gpio_read(PCAB_CFGS_CFGS_MODE_PINN);

#if 1
    user_appl_btle = 0;
    // user_appl_btle = 1;

    logs_inf("FIXED: %s", user_appl_btle ? "BtLe\r\n" : "2.4G\r\n");
#endif

    logs_inf(user_appl_btle ? "BtLe\r\n" : "2.4G\r\n");
}

uint8
user_appl_get_mode(void)
{
    return ( user_appl_btle );
}

bStatus_t ble_conn_param_update(    uint16 intervalMin,       // Minimum Interval
                                    uint16 intervalMax,       // Maximum Interval
                                    uint16 slaveLatency,      // Slave Latency
                                    uint16 timeoutMultiplier // Timeout Multiplier
                                )
{
    l2capParamUpdateReq_t updateReq;
    // reqs.intervalMin = GAP_GetParamValue(TGAP_CONN_EST_INT_MIN);
    // reqs.intervalMax = GAP_GetParamValue(TGAP_CONN_EST_INT_MAX);
    // reqs.slaveLatency = GAP_GetParamValue(TGAP_CONN_EST_LATENCY);
    // reqs.timeoutMultiplier = GAP_GetParamValue(TGAP_CONN_EST_SUPERV_TIMEOUT);
    // logs_inf("intv:[%d,%d],slat:%d,tout:%d\n", reqs.intervalMin,reqs.intervalMax,reqs.slaveLatency,reqs.timeoutMultiplier);
    updateReq.intervalMin = intervalMin;
    updateReq.intervalMax = intervalMax;
    updateReq.slaveLatency = slaveLatency;
    updateReq.timeoutMultiplier = timeoutMultiplier;

    extern uint8 gapRole_TaskID;
    extern uint16 gapRole_ConnectionHandle;

    return L2CAP_ConnParamUpdateReq( gapRole_ConnectionHandle, &updateReq, gapRole_TaskID );
}

static uint8_t latency_is_enable = 0;

void ble_conn_param_enable_latency(void)
{
    if (latency_is_enable)
    {
        return;
    }
    latency_is_enable = 1;
    // ble_conn_param_update(78,78,4,600);
    // ble_conn_param_update(36,36,4,500);
    ble_conn_param_update(9,9,60,500);
}

void ble_conn_param_disable_latency(void)
{
    if (!latency_is_enable)
    {
        return;
    }
    latency_is_enable = 0;
    ble_conn_param_update(9,9,0,500);
}

uint16
user_appl_evnt_hdlr(uint8 task, uint16 evnt)
{
    // if ( task != user_appl_task ) {
    //     return 0;
    // }
#if defined(PCAB_CFGS_CFGS_PCBA_6230SC_MOUSE_YJX) || defined(PCAB_CFGS_CFGS_PCBA_6230SC_MOUSE_WK303)
    if ( evnt & USER_APPL_KEY_SCAN_EVT )
    {
        user_appl_key_scan_handler();
        return ( evnt ^ USER_APPL_KEY_SCAN_EVT );
    }

    if ( evnt & USER_APPL_WHEEL_SEND_EVT )
    {
        // if (1 == hidMouse_data.wheel)
        // {
        //     LOG_ERROR("wheel send +\n");
        // }
        // else
        // {
        //     LOG_ERROR("wheel send -\n");
        // }
        user_appl_wheel_send_handler();
        // hidKbdSendMouseReport(0, 0, 0, 1);
        return ( evnt ^ USER_APPL_WHEEL_SEND_EVT );
    }
#endif

    if ( evnt & USER_CHCK_MOTI_EVNT ) {
        uint8 keys = 0;
        uint8 moti;
        int32 dlta;
        int32 dltx;
        int32 dlty;
        static int16 idle = -3;
        static int16 disc = 0;

    #if (1 == USER_APPL_CFGS_TEST_SUIT)
        if ( user_appl_test & BIT(0) )    // RPRT RATE TEST
        {
            // LOG_ERROR("> ");
            static uint32 itr0 = 0;
            itr0 = (itr0 + 1) % 2;
            hidMouse_data.x = hidMouse_data.y = (itr0 ? +10 : -10);
            hidKbdSendMouseReport(0, 1, 1, 0);
            osal_start_timerEx(user_appl_task, USER_CHCK_MOTI_EVNT, user_appl_get_mode() ? USER_APPL_CFGS_BTLE_INTV : USER_APPL_CFGS_2_4G_INTV);
            return ( evnt ^ USER_CHCK_MOTI_EVNT );
        }
    #endif

    #if 1
        if ( HidKbd_chk_conn() && HidKbd_chk_bond() ) {
            disc = 0;

            osal_stop_timerEx(user_appl_task, USER_POWR_OFFS_EVNT);
            osal_clear_event( user_appl_task, USER_POWR_OFFS_EVNT);

            // rset keys status, in case of missing keys
        #if  !defined(PCAB_CFGS_CFGS_PCBA_6230SC_MOUSE_YJX) && !defined(PCAB_CFGS_CFGS_PCBA_6230SC_MOUSE_WK303)
            keys = hidMouse_data.button;
            hidMouse_data.button = ~user_appl_chk_keys((void *)keys_cfgs, sizeof(keys_cfgs)/sizeof(keys_cfgs[0]));
            keys = keys ^ hidMouse_data.button;
        #endif

        #if 1
            /* powr rset when conn */
            // snsr_shub_pwr_snsr(0);
            // moti = 0x84;
            dltx = dlty = 0;
            // gpio_write(GPIO_P02, 0);
            moti = snsr_shub_chk_moti();
            // gpio_write(GPIO_P02, 1);
            // logs_inf("moti:[%02x]\r\n", moti);
            if ( (moti & BIT(7)) ) {
                // gpio_write(GPIO_P02, 0);
                dltx = snsr_shub_get_dltx();
                dlty = snsr_shub_get_dlty();
                // gpio_write(GPIO_P02, 1);
                // logs_inf("dlta :[%d,%d]\r\n",dltx,dlty);

                /* over flow */
                if ( (moti & (BIT(4) | BIT(3))) ) {
                dltx = dlty = 0;
                } else {
                idle = (idle < USER_APPL_CFGS_IDL0_TOUT ? 0 : -3);
                }
            }

            if ( (0 != keys) || (0 == idle) ) {
            #if (0 != USER_APPL_CFGS_USES_AVRG)
                // if ( 0 != dltx /* && 0 != dlty */ )
                {
                arry_queu_pll_node(avgx, dlta);
                arry_queu_psh_node(avgx, dltx);
                dltx = arry_queu_fnd_aavg(avgx);
                }
            #endif

            #if (0 != USER_APPL_CFGS_USES_AVRG)
                // if ( /* 0 != dltx && */ 0 != dlty )
                {
                arry_queu_pll_node(avgy, dlta);
                arry_queu_psh_node(avgy, dlty);
                dlty = arry_queu_fnd_aavg(avgy);
                }
            #endif
                // logs_inf("dlta` :[%d,%d]\r\n",dltx,dlty);

                hidMouse_data.x =
                (uint8)
                #if (1 == USER_APPL_CFGS_FLIP_X)
                -
                #endif
                #if (1 == USER_APPL_CFGS_SWAP_XY)
                (dlty);
                #else
                (dltx);
                #endif

                hidMouse_data.y =
                (uint8)
                #if (1 == USER_APPL_CFGS_FLIP_Y)
                -
                #endif
                #if (1 == USER_APPL_CFGS_SWAP_XY)
                (dltx);
                #else
                (dlty);
                #endif

                // static uint32 itr0 = 0;
                // itr0 = (itr0 + 1) % 2;
                // hidMouse_data.x = hidMouse_data.y = (itr0 ? +10 : -10);
                // hidKbdSendMouseReport(0, 1, 1, 0);

                hidKbdSendMouseReport(keys, dltx, dlty, 0);
                osal_start_timerEx(user_appl_task, USER_CHCK_MOTI_EVNT, USER_APPL_CFGS_IDL0_INTV);
            } else {
                if ( idle < USER_APPL_CFGS_IDL0_TOUT ) {
                    // hidKbdSendMouseReport(1, 0, 0, 0);
                    osal_start_timerEx(user_appl_task, USER_CHCK_MOTI_EVNT, USER_APPL_CFGS_IDL0_INTV);
                } else
                if ( idle < USER_APPL_CFGS_IDL1_TOUT ) {
                    if ( 1 != user_appl_get_mode() ) {  // 2.4G
                        LL_PLUS_EnableSlaveLatency(0);
                    } else {
                        ble_conn_param_enable_latency();
                    }
                    osal_start_timerEx(user_appl_task, USER_CHCK_MOTI_EVNT, USER_APPL_CFGS_IDL1_INTV);
                }  else {
                    osal_start_timerEx(user_appl_task, USER_POWR_OFFS_EVNT, 5);
                }
            }
            idle += 1;
        #endif
        } else {
            if ( disc < USER_APPL_CFGS_IDL0_TOUT ) {
                osal_start_timerEx(user_appl_task, USER_CHCK_MOTI_EVNT, USER_APPL_CFGS_IDL0_INTV);
            } else
            if ( disc < USER_APPL_CFGS_IDL2_TOUT ) {
                /* powr offs aftr 5sec when disc */
                // if ( 3 == disc ) snsr_shub_pwr_snsr(1);

                osal_start_timerEx(user_appl_task, USER_CHCK_MOTI_EVNT, USER_APPL_CFGS_IDL2_INTV);
            } else {
                osal_start_timerEx(user_appl_task, USER_POWR_OFFS_EVNT, 5);
            }
            idle = -3;
            disc += 1;
        }
    #endif

        return ( evnt ^ USER_CHCK_MOTI_EVNT );
    }

    if ( evnt & USER_CHCK_KEYS_EVNT ) {
        hidKbdSendMouseReport(1, 0, 0, 0);
        return ( evnt ^ USER_CHCK_KEYS_EVNT );
    }

    // if ( evnt & USER_CHCK_WHLS_EVNT ) {
    //     return ( evnt ^ USER_CHCK_WHLS_EVNT );
    // }

    if ( evnt & USER_CFGS_CPIS_EVNT ) {
        uint32 dpis = user_appl_dpis_valu - 1;

        /* round up: mini -> maxi */
        if ( 0 == dpis ) {
            dpis = USER_APPL_CFGS_DPIS_MASK;
        }
        snsr_shub_set_cpis(dpis);

    #if (1  == USER_APPL_CFGS_USES_DPIS)
        /* cant fit, move into next room */
        if ( dpis != (dpis & user_appl_dpis_valu) ) {
            user_appl_dpis_shft -= USER_APPL_CFGS_DPIS_ITSZ;
        }
        user_appl_dpis_valu = dpis;

        /* room full, move into next dwrd */
        if ( user_appl_dpis_shft >= 32 ) {
            dpis = 0;
            otp_prog_data_polling(user_appl_dpis_addr, &dpis, 1, OTP_USER_READ_MODE);
            user_appl_dpis_shft  = (32-USER_APPL_CFGS_DPIS_ITSZ);
            user_appl_dpis_addr += sizeof(uint32);
        }

        /* prog cfgs into otp */
        if ( user_appl_dpis_addr < USER_APPL_CFGS_DPIS_BGNA+USER_APPL_CFGS_DPIS_RMSZ ) {
            uint32 regv = ((user_appl_dpis_valu << user_appl_dpis_shft) | (~((-1U) << user_appl_dpis_shft)));
            int rslt = otp_prog_data_polling(user_appl_dpis_addr, &regv, 1, OTP_USER_READ_MODE);

            logs_inf("dpis:[addr:%08x,shft:%08x,valu:%08x,regv:%08x] rslt:%d\r\n", user_appl_dpis_addr, user_appl_dpis_shft, user_appl_dpis_valu, regv, rslt);
        } else {
            logs_inf("dpis_addr full:[%08x] \r\n", user_appl_dpis_addr);
        }
    #endif
        return ( evnt ^ USER_CFGS_CPIS_EVNT );
    }

    if ( evnt & USER_POWR_OFFS_EVNT ) {
    #if 1
        snsr_shub_pwr_snsr(1);
    #endif

    #if defined(PCAB_CFGS_CFGS_MOTI_PINN)
        gpioin_unregister(PCAB_CFGS_CFGS_MOTI_PINN);
        gpio_pull_set(PCAB_CFGS_CFGS_MOTI_PINN, gpio_read(PCAB_CFGS_CFGS_MOTI_PINN) ? GPIO_PULL_UP : GPIO_PULL_DOWN);
    #endif

    /* DPI */
    #if defined(PCAB_CFGS_CFGS_DPI_PINN)
        gpioin_unregister(PCAB_CFGS_CFGS_DPI_PINN);
        gpio_pull_set(PCAB_CFGS_CFGS_DPI_PINN, GPIO_PULL_DOWN);
    #endif

#if (defined(USER_APPL_WHEEL_ENABLE) && USER_APPL_WHEEL_ENABLE)
    #if 1
        /* wheel */
        gpioin_unregister(PCAB_CFGS_CFGS_WHLA_PINN);
        gpioin_unregister(PCAB_CFGS_CFGS_WHLB_PINN);
        // gpio_pull_set(PCAB_CFGS_CFGS_WHLA_PINN, gpio_read(PCAB_CFGS_CFGS_WHLA_PINN) ? GPIO_PULL_UP : GPIO_PULL_DOWN);
        // gpio_pull_set(PCAB_CFGS_CFGS_WHLB_PINN, gpio_read(PCAB_CFGS_CFGS_WHLB_PINN) ? GPIO_PULL_UP : GPIO_PULL_DOWN);
        gpio_pull_set(PCAB_CFGS_CFGS_WHLA_PINN,GPIO_PULL_DOWN);
        gpio_pull_set(PCAB_CFGS_CFGS_WHLB_PINN,GPIO_PULL_DOWN);
    #endif
#endif
        /* OS_PINN = GPIO_P06 */
    #if defined(PCAB_CFGS_CFGS_MODE_PINN)
        gpio_pull_set(PCAB_CFGS_CFGS_MODE_PINN, gpio_read(PCAB_CFGS_CFGS_MODE_PINN) ? GPIO_PULL_UP : GPIO_PULL_DOWN);
    #endif

    #if defined(PCAB_CFGS_CFGS_PCBA_6230_MOUSE_BYKC_V1_1)
        gpio_write(PCAB_CFGS_CFGS_LED0_PINN, 0);
        gpio_pull_set(PCAB_CFGS_CFGS_LED0_PINN, GPIO_PULL_DOWN);
        gpio_write(PCAB_CFGS_CFGS_LED1_PINN, 1);
        gpio_pull_set(PCAB_CFGS_CFGS_LED0_PINN, GPIO_PULL_UP);
    #endif

    #if 1
        LOG_ERROR("PWDN\n");
        // uint32 dir0 = read_reg(&(AP_GPIO->swporta_ddr));
        // uint32 pul0 = read_reg(&(AP_AON->IOCTL[0]));
        // uint32 pul1 = read_reg(&(AP_AON->IOCTL[1]));
        // logs_inf("dir0:%08x\n",dir0);
        // logs_inf("pul0:%08x\n",pul0);
        // logs_inf("pul1:%08x\n",pul1);

        // pwroff_cfg_t cfgs[] = {
        //     {.pin = PCAB_CFGS_CFGS_KEYL_PINN, .type = POL_ACT_LOW,},
        //     {.pin = PCAB_CFGS_CFGS_KEYM_PINN, .type = POL_ACT_LOW,},
        //     {.pin = PCAB_CFGS_CFGS_KEYR_PINN, .type = POL_ACT_LOW,},
        // };
        extern void
        hal_pwrmgr_poweroff(pwroff_cfg_t *pcfg, uint8_t wakeup_pin_num);
        // hal_pwrmgr_poweroff(cfgs, sizeof(cfgs)/sizeof(cfgs[0]));
        hal_pwrmgr_poweroff(0, 0);
    #endif

        return ( evnt ^ USER_POWR_OFFS_EVNT );
    }

    return 0;
}

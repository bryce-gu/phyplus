#include "rom_sym_def.h"

//#include <stdio.h>
#include "bus_dev.h"
#include "uart.h"

#include "gpio.h"
#include "clock.h"
#include "timer.h"
#include "ll.h"
#include "ll_def.h"
#include "rf_phy_driver.h"
#include "global_config.h"
#include "jump_function.h"

#include "ll_sleep.h"
#include "ll.h"
#include "ll_buf.h"
#include "ll_debug.h"
#include "ll_hw_drv.h"
#include "pwrmgr.h"
#include "mcu.h"
#include "OSAL_Tasks.h"
#include "gpio.h"
#include "log.h"
#include "linkdb.h"
#include "version.h"
#include "watchdog.h"
#include "otp.h"
#include "ctrap.h"
#include "pico_reg_aon.h"

#include "hidkbd.h"
#include "user_appl.h"


extern int  app_main(void);
extern void hal_rom_code_ini(void);
extern void linkDB_InitContext( linkDBItem_t *linkDBs,pfnLinkDBCB_t *linkCB,uint8 cb_maxs);

// uint8 app_mode = APP_BLE_MODE;

// ===================== connection context relate definition

#define   BLE_MAX_ALLOW_CONNECTION              1
#define   BLE_MAX_ALLOW_PKT_PER_EVENT_TX        4
#define   BLE_MAX_ALLOW_PKT_PER_EVENT_RX        2

#define   BLE_PKT_VERSION                       BLE_PKT_VERSION_4_0

#define   BLE_PKT_BUF_SIZE                  (((BLE_PKT_VERSION == BLE_PKT_VERSION_4_0) ? 1 : 0) * BLE_PKT40_LEN \
                                            + (sizeof(struct ll_pkt_desc) - 2))

#define   BLE_MAX_ALLOW_PER_CONNECTION          ( (BLE_MAX_ALLOW_PKT_PER_EVENT_TX * BLE_PKT_BUF_SIZE*2) \
                                                 +(BLE_MAX_ALLOW_PKT_PER_EVENT_RX * BLE_PKT_BUF_SIZE)   \
                                                  +BLE_PKT_BUF_SIZE)

#define   BLE_CONN_BUF_SIZE                 (BLE_MAX_ALLOW_CONNECTION * BLE_MAX_ALLOW_PER_CONNECTION)


uint8     g_pConnectionBuffer[BLE_CONN_BUF_SIZE] __attribute__ ((aligned(4)));
llConnState_t               pConnContext[BLE_MAX_ALLOW_CONNECTION];

#define LINKDB_CBS	5
linkDBItem_t plinkDB[BLE_MAX_ALLOW_CONNECTION];
pfnLinkDBCB_t plinkCBs[LINKDB_CBS];

llScheduleInfo_t         scheduleInfo[BLE_MAX_ALLOW_CONNECTION];


/*********************************************************************
    OSAL LARGE HEAP CONFIG
*/
#define LARGE_HEAP_SIZE  (1024)
uint8   g_largeHeap[LARGE_HEAP_SIZE] __attribute__ ((aligned(4)));


static void hal_rfphy_init(void)
{
    //============config the txPower
    g_rfPhyTxPower  = RF_PHY_TX_POWER_0DBM ;
    //============config BLE_PHY TYPE
    g_rfPhyPktFmt	= PKT_FMT_BLE1M;
    //============config RF Frequency Offset
    g_rfPhyFreqOffSet   = RF_PHY_FREQ_FOFF_00KHZ;// RF_PHY_FREQ_FOFF_N80KHZ;// RF_PHY_FREQ_FOFF_N100KHZ;
    //============config xtal 16M cap
    XTAL16M_CAP_SETTING(0x09);
    XTAL16M_CURRENT_SETTING(0x03);

    hal_rom_code_ini();

    NVIC_SetPriority(BB_IRQn, IRQ_PRIO_REALTIME);
    NVIC_SetPriority((IRQn_Type)TIM1_IRQn,  IRQ_PRIO_HIGH);     //ll_EVT
    NVIC_SetPriority((IRQn_Type)TIM2_IRQn,  IRQ_PRIO_HIGH);     //OSAL_TICK
}

static void hal_init(void)
{
	//========= low currernt setting IO init
	//========= pull all io to gnd by default, except P07,P15(keep floating)
    aon_ioctl0_pack(3,0,3,0,/*p7*/0,0,3,0,3,0,3,0,3,0,3,0,3,0,3,0); /*P0~P9*/
    aon_ioctl1_pack(3,0,3,0,3,0,3,0,/*p15*/0,0,3,0,3,0,3,0,3,0,3,0);/*P10~P19*/

    PMU_HIGH_LDO_ENABLE(1);
    DIG_LDO_CURRENT_SETTING(0x01);
    pwrmgr_RAM_retention_set();
    pwrmgr_LowCurrentLdo_enable();

    hal_pwrmgr_init();
    otp_cache_init();
    LOG_INIT();

    gpio_init_patch();
    hal_adc_init();
}

static void hal_mem_init_config(void)
{
    osal_mem_set_heap((osalMemHdr_t*)g_largeHeap, LARGE_HEAP_SIZE);

    LL_InitConnectContext(pConnContext, scheduleInfo,
                        g_pConnectionBuffer,
                        BLE_MAX_ALLOW_CONNECTION,
                        BLE_MAX_ALLOW_PKT_PER_EVENT_TX,
                        BLE_MAX_ALLOW_PKT_PER_EVENT_RX,
                        BLE_PKT_VERSION);
    linkDB_InitContext( plinkDB,plinkCBs,LINKDB_CBS);
}


static uint8_t bt_main_addr[6]={0x88,0x88,0xff,0x88,0x70,0x70};

extern void ll_set_ble_mac_addr(uint32_t macAddr);
extern void trap_c(uint32_t *regs);
uint32_t trap_stack[76/4];
/////////////////////////////////////////////////////////////////////////////////////////////////////////
int  main(void)
{
    hal_watchdog_config(WDG_2S);
    // g_system_clk = SYS_CLK_XTAL_16M;
    g_system_clk = SYS_CLK_DLL_48M;

    hal_mem_init_config();

    trap_set_stack(trap_stack, sizeof(trap_stack));
    JUMP_FUNCTION_SET(CK802_TRAP_C, (uint32_t)trap_c);

    drv_irq_init();
    extern void init_config(void);
    init_config();

    extern void patch_slave(void);
    patch_slave();

    hal_rfphy_init();
    hal_init();

    /* OS_PINN =  GPIO_P06 */
    user_appl_chk_mode();

#if(_DEF_DTM_BUILD_)
    rf_phy_dtm_ate();
#endif

    // LOG_ERROR("[MAIN] MAC:\n");
    // LOG_DUMP_BYTE(0x1FFF80C0, B_ADDR_LEN);

    osal_memcpy(bt_main_addr+0, 0x1FFF80C0+2, B_ADDR_LEN-2);
    osal_memcpy(bt_main_addr+B_ADDR_LEN-2, 0x1FFF80C0+0, 2);

    if( user_appl_get_mode() ) {
        // BtLe
        // g_rfPhyPktFmt    = PKT_FMT_BLE1M;
        bt_main_addr[0] += 0;
        // ll_set_ble_mac_addr( 0x1FFF80C0 );
    } else {
        // 2.4G
        // g_rfPhyPktFmt    = PKT_FMT_BLE2M;
        bt_main_addr[0] += 1;
        // pGlobal_config[MAC_ADDRESS_LOC] = (uint32_t)bt_addr;
    }
    ll_set_ble_mac_addr( bt_main_addr );

    LOG_ERROR("[MAIN] MAIN MAC:\n");
    LOG_DUMP_BYTE(bt_main_addr, B_ADDR_LEN);

    LOG_DEBUG("SDK Version ID %08x \n",SDK_VER_RELEASE_ID);
    LOG_DEBUG("rfClk %d sysClk %d tpCap[%02x %02x]\n",g_rfPhyClkSel,g_system_clk,g_rfPhyTpCal0,g_rfPhyTpCal1);

    LOG_DEBUG("sizeof(struct ll_pkt_desc) = %d, buf size = %d\n", sizeof(struct ll_pkt_desc), BLE_CONN_BUF_SIZE);
    LOG_DEBUG("g_pConnectionBuffer %p,sizeof(g_pConnectionBuffer) = %d, sizeof(pConnContext) = %d,\n",\
    g_pConnectionBuffer,sizeof(g_pConnectionBuffer), sizeof(pConnContext));
    LOG_DEBUG("[REST CAUSE] %d\n ",g_system_reset_cause);

    app_main();

}


/////////////////////////////////////  end  ///////////////////////////////////////

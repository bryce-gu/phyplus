/**************************************************************************************************
*******
**************************************************************************************************/

/******************************************************************************


 *****************************************************************************/


/*********************************************************************
 * INCLUDES
 */
#include "rom_sym_def.h"

#include "OSAL.h"
#include "OSAL_PwrMgr.h"

#include "log.h"
#include "gpio.h"

#include "clock.h"
#include "spi_lite.h"
#include "sensor.h"
#include "hidkbd.h"

extern uint8 app_mode;
extern uint8 HIDMouse_bond_ready;
extern uint16 gapRole_ConnectionHandle;

#define ID_ERR						0xFF
#define ID_KA8						0x5430
#define SNSR_CFGS_PDID_MX8650A                                  (0x5030)
#define ID_PAW3205DB			0xD030
#define ID_FCT3065				0x0030
#define ID_FCT3065_XYD		0x7031

enum {
    ID_NUM_KA8 = 0,
    SNSR_PDID_MX8650A,
    ID_NUM_PAW3205DB,
    ID_NUM_FCT3065,
    ID_NUM_FCT3065_XYD,
};

#define SENSOR_ID			 		KA8_ID


#define HID_RESET_HIDSENSOR_EVT		0x0100

#define REG_00_PRODCUT_ID       0X00
#define REG_01_PRODCUT_ID       0X01
#define REG_02_Motion_status    0X02
#define REG_03_Delta_X          0X03
#define REG_04_Delta_Y          0X04
#define REG_05_OP_MODE          0X05
#define REG_06_Config           0X06
#define REG_07_IM_Quality       0X07
#define REG_08_OP_State         0X08
#define REG_09_WR_Protect       0X09
#define REG_0A_Sleep1_Set       0X0A
#define REG_0B_Enter_Time       0X0B
#define REG_0C_Sleep2_Set       0X0C
#define REG_0D_IM_Threshold     0X0D
#define REG_0E_IM_Recognition   0X0E

enum {
    SENSOR_SET_POWER_OFF = 0,
    SENSOR_SET_POWER_ON,
    SENSOR_SET_MODE,
    SENSOR_SET_CONFIG,
    SENSOR_SET_IM_QUALITY,
    SENSOR_SET_ENTER_TIME,
    SENSOR_SET_SLEEP1,
    SENSOR_SET_SLEEP2,
    SENSOR_SET_IM_THRE,
    SENSOR_SET_IM_RECO,
};

const uint8 const ID_KA8_REG[][2] = {
    0x85,	0xB8,
    0x86, 0x04,
    0x87,	0x0A,
    0x88,	0x00,
    0x89,	0x5A,
    0x8A,	0x72,
    0x8B,	0x12,	//0x8B,	0x11,
    0x8C,	0x92,	//0x8C,	0xB2,
    0x8D,	0x0A,	//0x8D,	0x10,
    0x8E,	0xC5,	//0x8E,	0xC9,
    0x89,	0x00,
};


spilite_cfg_t const s_spi_cfg = {
    .sclk_pin = {P16,3},
    .ssn_pin  = {GPIO_DUMMY, 0},
    .MOSI     = {P17, 3},
    .MISO     = {P18, 3},

    .baudrate = 1000000,
    .spi_scmod = SPI_MODE3,
};

SENSOR HidMouseSensor;

HIDMOUSE_DATA hidMouse_data;
uint32 lantency_delay_cnt;
uint32 dot_delay_cnt;

uint8 msensor_TaskID = 0;


uint8 sensorReadByte(uint8 addr)
{
    uint8_t txbuf[2];
    uint8_t rxbuf[2];

    gpio_pull_set((gpio_pin_e)(s_spi_cfg.MOSI.pin), GPIO_PULL_UP);
    gpio_fmux_set((gpio_pin_e)(s_spi_cfg.MOSI.pin), 3);
    //read ID
    txbuf[0] = addr;
    spi_transmit_s(txbuf, NULL, 1);

    gpio_fmux_set((gpio_pin_e)(s_spi_cfg.MOSI.pin), 0);
    spi_transmit_s(NULL, rxbuf, 1);

    return rxbuf[0];
}

void sensorWriteByte(uint8 addr,uint8 data)
{
    uint8_t txbuf[2];
    txbuf[0] = addr;
    txbuf[1] = data;
    gpio_fmux_set((gpio_pin_e)(s_spi_cfg.MOSI.pin), 3);
    spi_transmit_s(txbuf, NULL, 2);
}

uint16
snsr_get_pdid(void)
{
    uint16 pdid = ((((uint16)sensorReadByte(0x01))<<8) | (uint16)sensorReadByte(0x00));
    LOG_ERROR("PDID:%4x\r\n",pdid);

    if ( pdid == ID_KA8 ) {
    //  LOG_DEBUG(" ID_KA8");
        HidMouseSensor.id = ID_NUM_KA8;
    } else
    if ( SNSR_CFGS_PDID_MX8650A == (pdid & 0xF0FF) ) {
        HidMouseSensor.id = SNSR_PDID_MX8650A;
        LOG_ERROR("SNSR_PDID_MX8650A");
    }

    return ( HidMouseSensor.id );
}

uint8 sensor_powerOnOff(uint8 powerFlag)
{
    HidMouseSensor.status = powerFlag;
    //LOG("\n -------------sensor_powerOnOff %d\n",HidMouseSensor.status);
    osal_start_timerEx(msensor_TaskID, HID_RESET_HIDSENSOR_EVT, 100);
    return 0;
}

uint32 get_cur_sensor_data(void)
{
    if(HidMouseSensor.status == 0) {
        return 0;
    }

    if ( snsr_get_pdid()==ID_ERR ) {
        sensor_powerOnOff(1);
        return 0;
    }

    uint8 mode=sensorReadByte(0x06);

    uint8 data1=sensorReadByte(0x02);
    uint8 dataX=sensorReadByte(0x03);
    uint8 dataY=sensorReadByte(0x04);
    //if(dataX || dataY)
       // LOG("x %d y %d\n", (int8_t)dataX, (int8_t)dataY);
    return (((uint32)data1<<16)|(dataX<<8)|(dataY));
}



uint16 snsr_ka8_ini_regs(void)
{
    // LOG("\n -------------init Reg num = %d",sizeof(ID_KA8_REG));
    for(uint8 reg_cnt=0; reg_cnt<(sizeof(ID_KA8_REG)/2); reg_cnt++) {
        sensorWriteByte(ID_KA8_REG[reg_cnt][0],ID_KA8_REG[reg_cnt][1]);
    }
    return 0;
}

void snsr_mx8650a_ini_regs(void)
{
    sensorWriteByte(0xff, 0x27);
    sensorWriteByte(0xab, 0x40);
    sensorWriteByte(0xb3, 0x56);
    sensorWriteByte(0x9b, 0xe6);
    sensorWriteByte(0xa8, 0x12);
    sensorWriteByte(0xa9, 0xc0);
    sensorWriteByte(0xbe, 0xe6);
    sensorWriteByte(0xa5, 0xaf);
    sensorWriteByte(0xa1, 0x02);
    sensorWriteByte(0xa0, 0x21);
    sensorWriteByte(0xb8, 0x10);
    sensorWriteByte(0xb9, 0x0b);
    sensorWriteByte(0xa7, 0xff);
    sensorWriteByte(0xb5, 0xd0);
    sensorWriteByte(0xb6, 0x00);
    sensorWriteByte(0xff, 0x00);
}


uint8 sensor_set_reg_init(uint16 sensor_id)
{
    snsr_ka8_ini_regs();
    //sensor_cpi_op(SENSOR_CPI_READ);

    snsr_mx8650a_ini_regs();
    return 0;
}



uint8 sensor_set_sensor_cpi(uint16 sensor_id)
{
    sensorWriteByte(0x86,HidMouseSensor.cur_DPI);
    return 0;
}

uint8 sensor_set_power_off(uint16 sensor_id)
{
    sensorWriteByte(0x86,0x08);
    return 0;
}

uint8 sensor_set_enable(void)
{
    uint16 cur_sensor_id = snsr_get_pdid();
    if(cur_sensor_id == ID_ERR) {
        gpio_write((gpio_pin_e)(s_spi_cfg.sclk_pin.pin), 1);
        WaitMs(30);
        gpio_write((gpio_pin_e)(s_spi_cfg.sclk_pin.pin), 0);
        WaitMs(20);
        gpio_write((gpio_pin_e)(s_spi_cfg.sclk_pin.pin), 1);
        WaitMs(30);
        gpio_fmux_set((gpio_pin_e)(s_spi_cfg.sclk_pin.pin), 3);
        return 1;
    }
    if(HidMouseSensor.status) {
        if(HidMouseSensor.init_flag == 0) {
        HidMouseSensor.init_flag =1;
        sensor_set_reg_init(cur_sensor_id);
        }
        else {
        sensor_set_sensor_cpi(cur_sensor_id);
        }
        // LOG("\n >>> sensor_set_enable succ %d -cpi: %d\n",HidMouseSensor.status,HidMouseSensor.cur_DPI);
        osal_start_timerEx(msensor_TaskID, MOUSE_MOTION_STATUS_CHECK_LOOP_EVT, 30);
    }
    else {
    sensor_set_power_off(cur_sensor_id);
    osal_stop_timerEx(msensor_TaskID, MOUSE_MOTION_STATUS_CHECK_LOOP_EVT);
    //gpio_write((gpio_pin_e)(s_spi_cfg.sclk_pin.pin), 0);
    gpio_fmux_set((gpio_pin_e)(s_spi_cfg.sclk_pin.pin), 3);
    }

    return 0;
}

uint8 sensor_set_param(uint8 Type, uint8 data)
{
    return 0;
}

uint16 sensor_reg_init(void)
{
    uint8 init_count = 20;	/// timeout 100times
    uint16 cur_sensor_id = 0xFFFF;
    HidMouseSensor.init_flag = 0;

    spi_init();
    spi_bus_init(&s_spi_cfg);

    while(init_count) {
        init_count--;
        cur_sensor_id = snsr_get_pdid();

        if(cur_sensor_id != ID_ERR) {
            HidMouseSensor.init_flag = 1;
            init_count = 0;
            sensor_set_reg_init(cur_sensor_id);
        }
        else {
            gpio_write((gpio_pin_e)(s_spi_cfg.sclk_pin.pin), 1);
            WaitMs(30);
            gpio_write((gpio_pin_e)(s_spi_cfg.sclk_pin.pin), 0);
            WaitMs(20);
            gpio_write((gpio_pin_e)(s_spi_cfg.sclk_pin.pin), 1);
            WaitMs(30);
            gpio_fmux_set((gpio_pin_e)(s_spi_cfg.sclk_pin.pin), 3);
            // LOG_DEBUG("\n                ******sync spi******            ");
        }
    }

    return cur_sensor_id;
}


/*
uint8 sensor_cpi_op(uint8 op)
{
    uint8 vm_sensor_cpi = 0;		/// rang  1 ~ sizeof(sensor_cpi_tab)+1
    //osal_snv_read(VM_ID_SENSOR_CPI,1,&vm_sensor_cpi);
    //if((vm_sensor_cpi == 0)||(vm_sensor_cpi > SENSOR_CPI_TOTAL_NUM)) {
    //    vm_sensor_cpi = SENSOR_CPI_DEFAULT;
    //		osal_snv_write(VM_ID_SENSOR_CPI,1,&vm_sensor_cpi);
    //}
    if(op == SENSOR_CPI_READ){
      HidMouseSensor.cur_DPI = sensor_cpi_tab[HidMouseSensor.id][vm_sensor_cpi-1];
      LOG("\n ==============HidMouseSensor.cur_DPI =  %d",HidMouseSensor.cur_DPI);
      return HidMouseSensor.cur_DPI;
    }
    if(op == SENSOR_CPI_ADD) {	// cpi+
        vm_sensor_cpi++;
        if(vm_sensor_cpi> SENSOR_CPI_TOTAL_NUM) {
            vm_sensor_cpi = 1;
        }
    }
    if(op == SENSOR_CPI_MIN) {// cpi-
        if(vm_sensor_cpi>=2) {
            vm_sensor_cpi--;
        }
        else {
            vm_sensor_cpi = SENSOR_CPI_TOTAL_NUM;
        }
    }
    if(op == SENSOR_CPI_RST){

    }

    osal_snv_write(VM_ID_SENSOR_CPI,1,&vm_sensor_cpi);

    HidMouseSensor.cur_DPI = sensor_cpi_tab[HidMouseSensor.id][vm_sensor_cpi-1];
    LOG("\n ========3======HidMouseSensor.cur_DPI =  %d",HidMouseSensor.cur_DPI);
    sensor_powerOnOff(1);

    return HidMouseSensor.cur_DPI;
}
*/


//void sensor_set_change_cpi()
//{
//    if(hidDevGapState!=GAPROLE_CONNECTED)
//        return;
//    uint8 sensor_vm_cpi_val = sensor_get_cpi_vm_val();

//    sensor_vm_cpi_val++;
//    if(sensor_vm_cpi_val > sizeof(sensor_cpi_tab)) {
//        sensor_vm_cpi_val = 1;
//    }

//    uint8 vm_sensor_cpi;
//    osal_snv_read(VM_ID_SENSOR_CPI,1,&vm_sensor_cpi);
//    if((vm_sensor_cpi == 0)||(vm_sensor_cpi > sizeof(sensor_cpi_tab))) {
//        vm_sensor_cpi = 2;
//    }


//    HidMouseSensor.cur_DPI = sensor_get_cpi(HidMouseSensor.id, vm_sensor_cpi);
////    HidMouseSensor.cur_DPI = sensor_cpi_tab[vm_sensor_cpi-1];

//    LOG("\n vm_sensor_cpi = %d == HidMouseSensor.cur_DPI = %d",vm_sensor_cpi,HidMouseSensor.cur_DPI);
//    osal_snv_write(VM_ID_SENSOR_CPI,1,&vm_sensor_cpi);
//    sensor_powerOnOff(1);
//}

void msensor_init(uint8 task_id)
{
    msensor_TaskID = task_id;

    HidMouseSensor.id = 0xff;
    HidMouseSensor.init_flag = 0x00;
    HidMouseSensor.status = 0x00;
    HidMouseSensor.cur_DPI = 0x04;

    uint16 productID = sensor_reg_init();

    for(uint8 i=0; i<0x0F; i++) {
        LOG_ERROR(">>[%2x] = 0x%2x\n\r",i,sensorReadByte(i));
    }

    sensor_powerOnOff(1);		///default off
    //osal_start_timerEx(msensor_TaskID, MOUSE_MOTION_STATUS_CHECK_LOOP_EVT, 100);
    return productID;
}

uint16 msensor_process_event(uint8 task_id, uint16 events )
{
    if(task_id != msensor_TaskID){
        return 0;
    }

    if(events &HID_RESET_HIDSENSOR_EVT) {
        if(sensor_set_enable()) {
            osal_start_timerEx(msensor_TaskID, HID_RESET_HIDSENSOR_EVT, 30);
        }
        return ( events ^ HID_RESET_HIDSENSOR_EVT );
    }

    if ( events & MOUSE_MOTION_STATUS_CHECK_LOOP_EVT )
    {
        uint32 data;
        uint8 Motion_status;
        static int32 dltx = 0;
        static int32 dlty = 0;

        if ( 1 )//((gapRole_ConnectionHandle!=0xFFFF)&& HIDMouse_bond_ready)
        {
            data = get_cur_sensor_data();
        }

        //LOG("cur_data: %x\n", cur_data);
        Motion_status = data>>16;

        if(((gapRole_ConnectionHandle!=0xFFFF)&& HIDMouse_bond_ready&&app_mode == APP_BLE_MODE)||app_mode == APP_2P4G_MODE){
            if(Motion_status == (0x80|(HidMouseSensor.cur_DPI)))
            {
                dot_delay_cnt = 0;

                #if 0
                dltx = (int8)data;
                dlty = (int8)(data>>8);
                #else
                dltx = (dltx * 1 + ((int8)data) * 10) / 2;
                dlty = (dlty * 1 + ((int8)(data>>8)) * 10) / 2;
                #endif

                hidMouse_data.x = (uint8) (dlty/10);
                hidMouse_data.y = (uint8)-(dltx/10);
                // LOG_ERROR(">>[%d,%d]\n",(uint8)dltx,(uint8)dlty);

                if( app_mode == APP_2P4G_MODE ) {
                    gpio_pull_set(P11,GPIO_PULL_UP);
                    gpio_dir(P11, GPIO_INPUT);
                    osal_start_timerEx(msensor_TaskID, MOUSE_MOTION_STATUS_CHECK_LOOP_EVT, (gpio_read(P11) ? 4 : 1));
                    hidKbdSendMouseReport(0,(uint8)dltx,(uint8)dlty,0);
                } else
                if( app_mode == APP_BLE_MODE )
                {
                    osal_start_timerEx(msensor_TaskID, MOUSE_MOTION_STATUS_CHECK_LOOP_EVT, 8);
                    hidKbdSendMouseReport(0,(uint8)dltx,(uint8)dlty,0);
                }
            } else {
                //hidMouse_Enable_Slave_Latency();
                if(dot_delay_cnt< 50) {
                    dot_delay_cnt++;
                    osal_start_timerEx(msensor_TaskID, MOUSE_MOTION_STATUS_CHECK_LOOP_EVT, 10);
//                  osal_start_timerEx(msensor_TaskID, MOUSE_MOTION_STATUS_CHECK_LOOP_EVT, 100);
//                  LOG("\nMotion_status = %d, _x=%d, _y=%d\n\r",Motion_status,d_x,d_y);
                }
                else {
  ////                    LOG("\nMotion_status = %d, _x=%d, _y=%d\n\r",Motion_status,d_x,d_y);
  //
  ////                    LOG("\nMotion_status = %d, _x=%d, _y=%d\n\r",Motion_status,d_x,d_y);
  //                    static uint32 rst_Sensor_cnt;
  //                    rst_Sensor_cnt++;
  //
  //                    if((rst_Sensor_cnt % 20) == 19) {
  //                      if(Motion_status != HidMouseSensor.cur_DPI) {
  //                          LOG("\n-------------------------------------------------\n\r");
  //                          sensor_cpi_op(SENSOR_CPI_RST);
  //                      }
  //                    }
                    osal_start_timerEx(msensor_TaskID, MOUSE_MOTION_STATUS_CHECK_LOOP_EVT, 100);
                }
            }
        }
        else
        {
            osal_start_timerEx(msensor_TaskID, MOUSE_MOTION_STATUS_CHECK_LOOP_EVT, 500);
        }
        return ( events ^ MOUSE_MOTION_STATUS_CHECK_LOOP_EVT );
    }

    return 0;
}


/*********************************************************************
 * INCLUDES
 */
#include "rom_sym_def.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "log.h"
#include "spi_lite.h"
#include "snsr_mx8650a.h"
#include "snsr_shub.h"

const uint8 const
snsr_mx8650a_regs_valu[][2] = {
#if 0
	{ 0xff, 0x27, },
    { 0xab, 0x40, },
    { 0xb3, 0x56, },
    { 0x9b, 0xe6, },
    { 0xa8, 0x12, },
    { 0xa9, 0xc0, },
    { 0xbe, 0xe6, },
    { 0xa5, 0xaf, },
    { 0xa1, 0x02, },
    { 0xa0, 0x21, },
    { 0xb8, 0x10, },
    { 0xb9, 0x0b, },
    { 0xa7, 0xff, },
    { 0xb5, 0xd0, },
    { 0xb6, 0x00, },
    { 0xff, 0x00, },
#else
    { 0xff, 0x27, },
    { 0x89, 0x5a, },
    { 0xa0, 0xf0, },
    { 0xa1, 0x10, },
    { 0xb5, 0xc8, },
    { 0xa5, 0xaf, },
    { 0xac, 0x78, },
    { 0xad, 0x64, },
    { 0xae, 0x1a, },
    { 0xaf, 0x5e, },
    { 0xb0, 0x11, },
    { 0xb1, 0x94, },
    { 0xb6, 0x00, },
#endif
};

extern uint16
snsr_mx8650a_get_pdid(void);
extern void
snsr_mx8650a_rst_chip(void);
extern void
snsr_mx8650a_pwr_chip(uint8 flag);
extern void
snsr_mx8650a_ini_regs(void);
extern uint8
snsr_mx8650a_get_moti(void);
extern uint8
snsr_mx8650a_get_dltx(void);
extern uint8
snsr_mx8650a_get_dlty(void);
extern uint8
snsr_mx8650a_set_cpis(uint8 valu);
extern uint8
snsr_mx8650a_get_para(uint8 addr);
extern void
snsr_mx8650a_set_para(uint8 addr, uint8 valu);
snsr_shub_algo_t const snsr_mx8650a_algo = {
    .pdid     =SNSR_MX8650A_CFGS_PDID,
    .get_pdid = snsr_mx8650a_get_pdid,
    .rst_chip = snsr_mx8650a_rst_chip,
    .pwr_chip = snsr_mx8650a_pwr_chip,
    .ini_regs = snsr_mx8650a_ini_regs,
    .get_moti = snsr_mx8650a_get_moti,
    .get_dltx = snsr_mx8650a_get_dltx,
    .get_dlty = snsr_mx8650a_get_dlty,
    .set_cpis = snsr_mx8650a_set_cpis,
    .get_para = snsr_mx8650a_get_para,
    .set_para = snsr_mx8650a_set_para,
};

uint16
snsr_mx8650a_get_pdid(void)
{
    // LOG_ERROR("PDID:0x%04x \r\n", ( ((((uint16)snsr_mx8650a_get_para(0x01))<<8) | (uint16)snsr_mx8650a_get_para(0x00)) & 0xFFFF ));

    return ( ((((uint16)snsr_mx8650a_get_para(0x01))<<8) | (uint16)snsr_mx8650a_get_para(0x00)) & 0xFFFF );
}

/*
Motion_Status (0x02, Read Only)
Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
MOTION Reserved Reserved DYOVF DXOVF 1 RES1 RES0
0 x x 0 0 1 0 0
Motion_Status register allows the user to determine if motion has occurred since the last time it was read. If
so, then the user should read Delta_X and Delta_Y registers to get the accumulated motion. It also tells if the
motion buffers have overflowed since the last reading. The current resolution is also shown.
 */
uint8
snsr_mx8650a_get_moti(void)
{
    return ( snsr_mx8650a_get_para(0x02) );
}

/*
Delta_X (0x03, Read Only)
Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
X7 X6 X5 X4 X3 X2 X1 X0
0 0 0 0 0 0 0 0
X movement is counted since last report. Absolute value is determined by resolution. Reading
clears the register. Report range –128 ~ +127.
 */
uint8
snsr_mx8650a_get_dltx(void)
{
    return ( snsr_mx8650a_get_para(0x03) );
}

/*
Delta_Y (0x04, Read Only)
Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0
0 0 0 0 0 0 0 0
Y movement is counted since last report. Absolute value is determined by resolution. Reading
clears the register. Report range –128 ~ +127.
 */
uint8
snsr_mx8650a_get_dlty(void)
{
    return ( snsr_mx8650a_get_para(0x04) );
}

/*
Configuration (0x06, Read/Write)
Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
RESET MOTSWK 0 0 PD_EN 1 CPI1 CPI0
0 0 0 0 0 1 0 0
The Configuration register allows the user to change the configuration of the sensor. Shown
below are the bits, their default values, and optional values.
 */
void
snsr_mx8650a_rst_chip(void)
{
    snsr_mx8650a_set_para(0x06, 0x80);
    WaitMs(10);
}

void
snsr_mx8650a_pwr_chip(uint8 flag)
{
    // uint8 valu;

    // valu = snsr_mx8650a_get_para(0x06);
    // flag = ((flag & 0x01) << 3);

    // if ( (valu & 0x08) ^ flag ) {
        // snsr_mx8650a_set_para(0x06, ((valu & ~0x08) | flag));
    //     WaitMs(10);
    // }

    snsr_mx8650a_set_para(0x06, ((snsr_mx8650a_get_para(0x06) & ~0x08) | ((flag & 0x01) << 3)));
}

uint8
snsr_mx8650a_set_cpis(uint8 cpis)
{
    snsr_mx8650a_set_para(0x06, ((snsr_mx8650a_get_para(0x06) & ~0x03) | (cpis&0x03)));

    return ( snsr_mx8650a_get_para(0x06) & 0x03 );
}

void snsr_mx8650a_ini_regs(void)
{
    snsr_mx8650a_rst_chip();

    /* init para */
    for ( uint8 itr0 = 0; itr0 < (sizeof(snsr_mx8650a_regs_valu) / 2); itr0 += 1 ) {
        snsr_mx8650a_set_para(snsr_mx8650a_regs_valu[itr0][0], snsr_mx8650a_regs_valu[itr0][1]);
        // LOG_ERROR("REG[%02x]:%02x ?= %02x\r\n", snsr_mx8650a_regs_valu[itr0][0], snsr_shub_pul_byte(0x00, snsr_mx8650a_regs_valu[itr0][0] & 0x7f), snsr_mx8650a_regs_valu[itr0][1]);
    }

    /* init cpis */
    // snsr_mx8650a_set_cpis(0x00);

    /* MOTSWK edge-sensitive */
    // snsr_mx8650a_set_para(0x06, ((snsr_mx8650a_get_para(0x06) & ~0x40) | (0x40)));

    // for ( uint8 itr0 = 0; itr0 < 0x0F; itr0 += 1 ) {
    //     LOG_ERROR("REG[%02x]:0x%02x\r\n",itr0,snsr_shub_pul_byte(0x00, itr0));
    // }
}

uint8
snsr_mx8650a_get_para(uint8 addr)
{
    return ( snsr_shub_pul_byte(0x00, addr) );
}

void
snsr_mx8650a_set_para(uint8 addr, uint8 valu)
{
    snsr_shub_psh_byte(0x00, (addr | 0x80), valu);
}

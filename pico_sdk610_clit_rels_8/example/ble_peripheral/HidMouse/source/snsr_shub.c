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
#include "rom_sym_def.h"
#include "snsr_shub.h"
#include "pcba_cfgs.h"
#include "log.h"
#include "spi_lite.h"
#include "gpio.h"

#include "snsr_ka8.h"
#include "snsr_mx8650a.h"


/*
 ******************************************************************************
 * Definition
 ******************************************************************************
 */
#define USER_DEVS_CFGS_LOGS_ENAB

#ifdef  USER_DEVS_CFGS_LOGS_ENAB
#define USER_DEVS_CFGS_LOGS_TAGS        "SNSR_SHUB"
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
    logs_logs_ent(USER_DEVS_CFGS_LOGS_TAGS, fmt, ##__VA_ARGS__)

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


#define SNSR_SHUB_CFGS_USES_KA8         (0)
#define SNSR_SHUB_CFGS_USES_MX8650A     (1)


/*
 ******************************************************************************
 * private variable definitions
 ******************************************************************************
 */

static spilite_cfg_t const snsr_spix = {
    .sclk_pin   = {PCAB_CFGS_CFGS_SCLK_PINN, PCAB_CFGS_CFGS_SCLK_FMUX},
    .ssn_pin    = {PCAB_CFGS_CFGS_CHPS_PINN, PCAB_CFGS_CFGS_CHPS_FMUX},
    .MOSI       = {PCAB_CFGS_CFGS_MOSI_PINN, PCAB_CFGS_CFGS_MOSI_FMUX},
    .MISO       = {PCAB_CFGS_CFGS_MISO_PINN, PCAB_CFGS_CFGS_MISO_FMUX},
    .baudrate   = 2000000,
    .spi_scmod  = SPI_MODE3,
};

static const snsr_shub_algo_t* const snsr_list[] = {
    #if     (1 == SNSR_SHUB_CFGS_USES_KA8)
    &snsr_ka8_algo,
    #endif
    #if     (1 == SNSR_SHUB_CFGS_USES_MX8650A)
    &snsr_mx8650a_algo,
    #endif
};

static snsr_shub_algo_t*    snsr_algo/*  = 0 */;
static uint8                snsr_stat = 0;
/*
 ******************************************************************************
 * private function implementations
 ******************************************************************************
 */


/*
 ******************************************************************************
 * public function implementations
 ******************************************************************************
 */
uint8
snsr_shub_pul_byte(spilite_cfg_t* spix, uint8 addr)
{
    uint8_t txbuf[2];
    uint8_t rxbuf[2];

    // gpio_write(GPIO_P08, 1);

    spix = (spilite_cfg_t*)&snsr_spix;

	gpio_pull_set((gpio_pin_e)(spix->MOSI.pin), GPIO_PULL_UP);
    gpio_fmux_set((gpio_pin_e)(spix->MOSI.pin), (gpio_fmux_e)spix->MOSI.pmux);
    //read ID
    txbuf[0] = addr;
    spi_transmit_s(txbuf, NULL, 1);

    gpio_fmux_set((gpio_pin_e)(spix->MOSI.pin), FMUX_GPIO);
    spi_transmit_s(NULL, rxbuf, 1);

    // gpio_write(GPIO_P08, 0);

    return rxbuf[0];
}

void
snsr_shub_psh_byte(spilite_cfg_t* spix, uint8 addr, uint8 data)
{
    uint8_t txbuf[2];

    // gpio_write(GPIO_P08, 1);

    spix = (spilite_cfg_t*)&snsr_spix;

	gpio_pull_set((gpio_pin_e)(spix->MOSI.pin), GPIO_PULL_UP);
    gpio_fmux_set((gpio_pin_e)(spix->MOSI.pin), (gpio_fmux_e)spix->MOSI.pmux);

    txbuf[0] = addr;
    txbuf[1] = data;
    spi_transmit_s(txbuf, NULL, 2);

    // gpio_write(GPIO_P08, 0);
}

/*
 * # Name:  user_devs_ini
 * # Desc:  init devs tree before load from dev or write for new
 * # Para:  none
 * # return:none
 */
void
snsr_shub_ini_snsr(void)
{
    spi_init();
    spi_bus_init(&snsr_spix);

    unsigned int        trys = 3;
    snsr_shub_algo_t*   algo = 0;

#if 0
    while ( (trys --) && (0 == algo) ) {
        AP_WDT_FEED;
        for ( int itr0 = 0; itr0 < sizeof(snsr_list)/sizeof(snsr_list[0]); itr0 += 1 ) {
            algo = snsr_list[itr0];

            if ( 0 != algo ) {
                /* spi bus re-sync */
                gpio_write((gpio_pin_e)(snsr_spix.sclk_pin.pin), 1);
                WaitMs(30);
                gpio_write((gpio_pin_e)(snsr_spix.sclk_pin.pin), 0);
                WaitMs(5);
                gpio_write((gpio_pin_e)(snsr_spix.sclk_pin.pin), 1);
                WaitMs(30);
                gpio_fmux_set((gpio_pin_e)(snsr_spix.sclk_pin.pin), (gpio_fmux_e)snsr_spix.sclk_pin.pmux);

                logs_inf("PDID:#H:%04x <-> #H:%04x \r\n", algo->pdid, algo->get_pdid());
                if ( algo->pdid == (algo->get_pdid ? algo->get_pdid() : (0xFFFF)) ) {
                    break;
                } else {
                    algo = 0;
                }
            }
        }
    }
#else
    /* spi bus re-sync */
    gpio_write((gpio_pin_e)(snsr_spix.sclk_pin.pin), 1);
    WaitMs(30);
    gpio_write((gpio_pin_e)(snsr_spix.sclk_pin.pin), 0);
    WaitMs(5);
    gpio_write((gpio_pin_e)(snsr_spix.sclk_pin.pin), 1);
    WaitMs(30);
    gpio_fmux_set((gpio_pin_e)(snsr_spix.sclk_pin.pin), (gpio_fmux_e)snsr_spix.sclk_pin.pmux);

    snsr_algo = algo = snsr_list[0];
#endif

    if ( 0 != algo ) {
        logs_inf("PDID:#H:%04x <-> #H:%04x \r\n", algo->pdid, algo->get_pdid());

        snsr_algo = algo;

        if ( algo->ini_regs ) algo->ini_regs();
        // if ( algo->set_cpis ) algo->set_cpis(0x00);
        // if ( algo->set_para && algo->get_para ) algo->set_para(0x06, ((algo->get_para(0x06) & ~0x40) | (0x40)));
    } else {
        logs_inf("!! NONE SNSR !!\r\n");
        /*
        temp solv:
            read KA8 pdid fail, aftr powr trig up.
         */
        system_soft_reset();
    }
}

void
snsr_shub_pwr_snsr(unsigned char flag)
{
    snsr_shub_algo_t*   algo = snsr_algo;

    if ( 0 != algo ) {
        algo->pwr_chip ? algo->pwr_chip(flag) : (0x00);
    }

    spi_bus_deinit(&snsr_spix);
}

/*
 * # Name:  user_devs_ini
 * # Desc:  init devs tree before load from dev or write for new
 * # Para:  none
 * # return:none
 */
int
snsr_shub_chk_moti(void)
{
    int rslt = 0;
    snsr_shub_algo_t*   algo = snsr_algo;

    if ( 0 != algo ) {
        rslt = algo->get_moti ? algo->get_moti() : (0x00);
    }

    return ( rslt );
}

int
snsr_shub_get_dltx(void)
{
    int rslt = 0;
    snsr_shub_algo_t*   algo = snsr_algo;

    if ( 0 != algo ) {
        rslt = (signed char)(algo->get_dltx ? algo->get_dltx() : (0x00));
    }

    return ( rslt );
}

int
snsr_shub_get_dlty(void)
{
    int rslt = 0;
    snsr_shub_algo_t*   algo = snsr_algo;

    if ( 0 != algo ) {
        rslt = (signed char)(algo->get_dlty ? algo->get_dlty() : (0x00));
    }

    return ( rslt );
}

int
snsr_shub_set_cpis(unsigned char cpis)
{
    int rslt = 0;
    snsr_shub_algo_t*   algo = snsr_algo;
    unsigned char vmap[] = { 0x00, 0x03, 0x02, };

    if ( 0 != algo ) {
        rslt = algo->set_cpis ? algo->set_cpis(vmap[(cpis&0x03) - 1]) : (0x00);
    }

    logs_inf("CPIS:%02x\r\n", rslt);
    return ( rslt );
}

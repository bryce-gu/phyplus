/*
 ******************************************************************************
 * Name:        user_dhub.h
 *
 * description: defines:
 * 				implementation of CATE device plug & unplug detection using pulling.
 *
 *  History:     Date;           Author;         Description
 *              18 Mar. 2016;   Chen, George;   file creation.
 ******************************************************************************
 */


#ifndef __SNSR_SHUB_H__
#define __SNSR_SHUB_H__


/*
 ******************************************************************************
 * Includes
 ******************************************************************************
 */
// #include "tbox_cfgs.h"
// #include "comn_cfgs.h"
// #include "core_cfgs.h"
// #include "devs_cfgs.h"
#include "spi_lite.h"


#ifdef __cplusplus
extern "C" {
#endif

/*
 ******************************************************************************
 * defines
 ******************************************************************************
 */

typedef struct snsr_shub_stru_algo_def {
    unsigned short  pdid;
    unsigned short  (*get_pdid)(void);
    void            (*rst_chip)(void);
    void            (*pwr_chip)(unsigned char);
    void            (*ini_regs)(void);
    unsigned char   (*get_moti)(void);
    unsigned char   (*get_dltx)(void);
    unsigned char   (*get_dlty)(void);
    unsigned char   (*set_cpis)(unsigned char);
    unsigned char   (*get_para)(unsigned char);
    void            (*set_para)(unsigned char, unsigned char);
} snsr_shub_algo_t;

/*
 ******************************************************************************
 * public variables
 ******************************************************************************
 */


/*
 ******************************************************************************
 * public functions
 ******************************************************************************
 */
unsigned char
snsr_shub_pul_byte(spilite_cfg_t* spix, unsigned char addr);
void
snsr_shub_psh_byte(spilite_cfg_t* spix, unsigned char addr, unsigned char data);
void
snsr_shub_ini_snsr(void);
void
snsr_shub_rst_snsr(void);
void
snsr_shub_pwr_snsr(unsigned char flag);
int
snsr_shub_chk_moti(void);
int
snsr_shub_get_dltx(void);
int
snsr_shub_get_dlty(void);
int
snsr_shub_set_cpis(unsigned char cpis);


#ifdef __cplusplus
}
#endif

#endif /* __SNSR_SHUB_H__ */

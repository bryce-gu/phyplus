/*
 ******************************************************************************
 * Name:        arry_queu.h
 *
 * description: defines:
 *
 *  History:     Date;           Author;         Description
 *              18 Mar. 2016;   Chen, George;   file creation.
 ******************************************************************************
 */


#ifndef __ARRY_QUEU_H__
#define __ARRY_QUEU_H__


/*
 ******************************************************************************
 * Includes
 ******************************************************************************
 */
#include "OSAL.h"


#ifdef __cplusplus
extern "C" {
#endif

/*
 ******************************************************************************
 * defines
 ******************************************************************************
 */

typedef struct arry_queu_stru_queu_def {
    int *arry;
    int  size;
    int  free;
    int  push;
    int  pull;
} arry_queu_t;

#define arry_queu_ini_queu(queu, aysz)                                      \
    int arry_##queu##_arry[aysz];                                           \
    arry_queu_t arry_##queu##_queu = {                                      \
        .arry = arry_##queu##_arry,                                         \
        .size = aysz,                                                       \
        .free = aysz,                                                       \
        .push = 0,                                                          \
        .pull = 0,                                                          \
    };

#define arry_queu_get_refs(queu)                                            \
    ( &arry_##queu##_queu )

#define arry_queu_rst_queu(queu)                                            \
    osal_memset(arry_##queu##_arry, 0, sizeof(arry_##queu##_arry))

#define arry_queu_chk_empt(queu)                                            \
    arry_queu_chk_empt_intl(arry_queu_get_refs(queu))

#define arry_queu_chk_full(queu)                                            \
    arry_queu_chk_full_intl(arry_queu_get_refs(queu))

#define arry_queu_psh_node(queu, para)                                      \
    arry_queu_psh_node_intl(arry_queu_get_refs(queu), (para))

#define arry_queu_pek_node(queu, para)                                      \
    arry_queu_pek_node_intl(arry_queu_get_refs(queu), &(para))

#define arry_queu_pll_node(queu, para)                                      \
    arry_queu_pul_node_intl(arry_queu_get_refs(queu), &(para))

#define arry_queu_fnd_maxi(queu)                                            \
    arry_queu_fnd_maxi_intl(arry_queu_get_refs(queu))

#define arry_queu_fnd_mini(queu)                                            \
    arry_queu_fnd_mini_intl(arry_queu_get_refs(queu))

#define arry_queu_fnd_aavg(queu)                                            \
    arry_queu_fnd_aavg_intl(arry_queu_get_refs(queu))

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
int
arry_queu_chk_empt_intl(arry_queu_t* queu);

int
arry_queu_chk_full_intl(arry_queu_t* queu);

void
arry_queu_psh_node_intl(arry_queu_t* queu, int para);

int
arry_queu_pek_node_intl(arry_queu_t* queu, int* para);

int
arry_queu_pul_node_intl(arry_queu_t* queu, int* para);

int
arry_queu_fnd_maxi_intl(arry_queu_t* queu);

int
arry_queu_fnd_mini_intl(arry_queu_t* queu);

int
arry_queu_fnd_mavg_intl(arry_queu_t* queu);


#ifdef __cplusplus
}
#endif

#endif /* __ARRY_QUEU_H__ */

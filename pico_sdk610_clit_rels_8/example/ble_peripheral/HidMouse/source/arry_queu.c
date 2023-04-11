/*
 ******************************************************************************
 * Name:        arry_queu.c
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
#include "arry_queu.h"
#include "log.h"


/*
 ******************************************************************************
 * Definition
 ******************************************************************************
 */
#define ARRY_QUEU_CFGS_LOGS_ENAB

#ifdef  ARRY_QUEU_CFGS_LOGS_ENAB
#define ARRY_QUEU_CFGS_LOGS_TAGS        "ARRY_QUEU"
/* ERROR */
#define logs_err(fmt, ...)                                                \
    logs_logs_err(ARRY_QUEU_CFGS_LOGS_TAGS, fmt, ##__VA_ARGS__)

/* WARNING */
#define logs_war(fmt, ...)                                                \
    logs_logs_war(ARRY_QUEU_CFGS_LOGS_TAGS, fmt, ##__VA_ARGS__)

/* INFORMATION */
#define logs_inf(fmt, ...)                                                \
    LOG_USR0("[INF][%s]",ARRY_QUEU_CFGS_LOGS_TAGS);LOG_USR0(fmt,##__VA_ARGS__)

/* VERB */
#define logs_ver(fmt, ...)                                                \
    logs_logs_ver(ARRY_QUEU_CFGS_LOGS_TAGS, fmt, ##__VA_ARGS__)

/* Function entry */
#define logs_ent(fmt, ...)                                                \
    logs_logs_ent(ARRY_QUEU_CFGS_LOGS_TAGS, fmt, ##__VA_ARGS__)

/* function exit */
#define logs_exi(fmt, ...)                                                \
    logs_logs_exi(ARRY_QUEU_CFGS_LOGS_TAGS, fmt, ##__VA_ARGS__)

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

#endif /* ARRY_QUEU_CFGS_LOGS_ENAB */


/*
 ******************************************************************************
 * private variable definitions
 ******************************************************************************
 */


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
__INLINE int
arry_queu_chk_empt_intl(arry_queu_t* queu)
{
    return ( queu->free == queu->size );
}

__INLINE int
arry_queu_chk_full_intl(arry_queu_t* queu)
{
    return ( queu->free == 0 );
}

void
arry_queu_psh_node_intl(arry_queu_t* queu, int para)
{
    if ( !arry_queu_chk_full_intl(queu) ) {
        queu->free -= 1;
        queu->arry[queu->push] = para;
        queu->push = ((queu->push + 1) % queu->size);
    }
}

int
arry_queu_pek_node_intl(arry_queu_t* queu, int* para)
{
    int rslt = arry_queu_chk_empt_intl(queu);
    if ( !rslt ) {
        *para = queu->arry[queu->pull];
    }
    return ( rslt);
}

int
arry_queu_pul_node_intl(arry_queu_t* queu, int* para)
{
    int rslt = arry_queu_pek_node_intl(queu, para);
    if ( !rslt ) {
        queu->pull  = ((queu->pull + 1) % queu->size);
        queu->free += 1;
    }
    return ( rslt );
}

int
arry_queu_fnd_maxi_intl(arry_queu_t* queu)
{
    int maxi = 0;
    for (int itr0 = 1; itr0 < queu->size; itr0 += 1 ) {
        if ( queu->arry[maxi] < queu->arry[itr0] ) {
            maxi = itr0;
        }
    }
    return ( maxi );
}

int
arry_queu_fnd_mini_intl(arry_queu_t* queu)
{
    int mini = 0;
    for (int itr0 = 1; itr0 < queu->size; itr0 += 1 ) {
        if ( queu->arry[mini] > queu->arry[itr0] ) {
            mini = itr0;
        }
    }
    return ( mini );
}

int
arry_queu_fnd_aavg_intl(arry_queu_t* queu)
{
    // int maxi = arry_queu_fnd_maxi_intl(queu);
    // int mini = arry_queu_fnd_mini_intl(queu);
    // int avrg = 0, coun = 0;
    int avrg = 0;
    for (int itr0 = 0; itr0 < queu->size; itr0 += 1 ) {
        // if (maxi != itr0 && mini != itr0 ) {
            avrg += queu->arry[itr0];
            // coun += 1;
        // }
    }
    return ( avrg / queu->size );
}

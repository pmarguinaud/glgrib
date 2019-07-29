/**** *lfi_dumm.c* - Implementation of dummy LFI routines
 *
 *    Author. 
 *    ------- 
 *     Philippe Marguinaud *METEO-FRANCE*
 *     Original : 12-08-2013
 *
 */


#include "lfi_dumm.h"

/*
 * We define two macros to define dummy LFI routines
 */

#define lfi_dumm(lfixxx, LFIXXX) \
void lfixxx##_dumm (LFIXXX##_ARGS_DECL) \
{                                       \
}

#define lfi_dumm_krep(lfixxx, LFIXXX) \
void lfixxx##_dumm (LFIXXX##_ARGS_DECL) \
{                                       \
  *KREP = 0;                            \
}


lfi_dumm_krep (lfiouv, LFIOUV)
lfi_dumm_krep (lficas, LFICAS)
lfi_dumm_krep (lfiecr, LFIECR)
lfi_dumm_krep (lfifer, LFIFER)
lfi_dumm_krep (lfilec, LFILEC)
lfi_dumm_krep (lfinfo, LFINFO)
lfi_dumm_krep (lfipos, LFIPOS)
lfi_dumm (lfiver, LFIVER)
lfi_dumm_krep (lfiofm, LFIOFM)
lfi_dumm (lfineg, LFINEG)
lfi_dumm_krep (lfilaf, LFILAF)
lfi_dumm (lfiosg, LFIOSG)
lfi_dumm (lfinum, LFINUM)
lfi_dumm_krep (lfisup, LFISUP)
lfi_dumm_krep (lfiopt, LFIOPT)
lfi_dumm (lfinmg, LFINMG)
lfi_dumm_krep (lficap, LFICAP)
lfi_dumm (lfifra, LFIFRA)
lfi_dumm (lficfg, LFICFG)
lfi_dumm_krep (lfierf, LFIERF)
lfi_dumm_krep (lfilas, LFILAS)
lfi_dumm_krep (lfiren, LFIREN)
lfi_dumm (lfiini, LFIINI)
lfi_dumm_krep (lfipxf, LFIPXF)
lfi_dumm (lfioeg, LFIOEG)
lfi_dumm_krep (lfinaf, LFINAF)
lfi_dumm (lfiofd, LFIOFD)
lfi_dumm_krep (lfiomf, LFIOMF)
lfi_dumm_krep (lfiafm, LFIAFM)
lfi_dumm_krep (lfista, LFISTA)
lfi_dumm_krep (lfiosf, LFIOSF)
lfi_dumm_krep (lfilap, LFILAP)
lfi_dumm_krep (lfioef, LFIOEF)
lfi_dumm_krep (lfimst, LFIMST)
lfi_dumm_krep (lfinim, LFINIM)
lfi_dumm_krep (lfisfm, LFISFM)
lfi_dumm (lfinsg, LFINSG)
lfi_dumm (lfideb, LFIDEB)
lfi_dumm (lfiomg, LFIOMG)
lfi_dumm (lfifmd, LFIFMD)

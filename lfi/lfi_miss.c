/**** *lfi_miss.c* - Definition of \`missing' LFI routines; these routines print a message and abort
 *
 *    Author. 
 *    ------- 
 *     Philippe Marguinaud *METEO-FRANCE*
 *     Original : 12-08-2013
 *
 */


#include <stdio.h>
#include "lfi_miss.h"
#include "lfi_abor.h"

typedef struct magic_t
{
  char cmagic[8];
}
magic_t;

/* We define helper functions and macros here */

static void not_implemented (void * LFI, const char * name)
{
  magic_t * m = LFI;
  lfi_abor ("`%s' is not implemented by `%8.8s'", name, m->cmagic);
}

#define lfi_miss(lfixxx, LFIXXX) \
void lfixxx##_miss (LFIXXX##_ARGS_DECL)  \
{                                        \
  not_implemented (LFI, #lfixxx);        \
}

lfi_miss (lfiouv, LFIOUV)
lfi_miss (lfican, LFICAN)
lfi_miss (lficas, LFICAS)
lfi_miss (lfiecr, LFIECR)
lfi_miss (lfifer, LFIFER)
lfi_miss (lfilec, LFILEC)
lfi_miss (lfinfo, LFINFO)
lfi_miss (lfinff, LFINFF)
lfi_miss (lfipos, LFIPOS)
lfi_miss (lfiver, LFIVER)
lfi_miss (lfiofm, LFIOFM)
lfi_miss (lfineg, LFINEG)
lfi_miss (lfilaf, LFILAF)
lfi_miss (lfiosg, LFIOSG)
lfi_miss (lfinum, LFINUM)
lfi_miss (lfisup, LFISUP)
lfi_miss (lfiopt, LFIOPT)
lfi_miss (lfinmg, LFINMG)
lfi_miss (lficap, LFICAP)
lfi_miss (lfifra, LFIFRA)
lfi_miss (lficfg, LFICFG)
lfi_miss (lfierf, LFIERF)
lfi_miss (lfilas, LFILAS)
lfi_miss (lfiren, LFIREN)
lfi_miss (lfiini, LFIINI)
lfi_miss (lfipxf, LFIPXF)
lfi_miss (lfioeg, LFIOEG)
lfi_miss (lfinaf, LFINAF)
lfi_miss (lfiofd, LFIOFD)
lfi_miss (lfiomf, LFIOMF)
lfi_miss (lfiafm, LFIAFM)
lfi_miss (lfista, LFISTA)
lfi_miss (lfiosf, LFIOSF)
lfi_miss (lfilap, LFILAP)
lfi_miss (lfioef, LFIOEF)
lfi_miss (lfimst, LFIMST)
lfi_miss (lfinim, LFINIM)
lfi_miss (lfisfm, LFISFM)
lfi_miss (lfinsg, LFINSG)
lfi_miss (lfideb, LFIDEB)
lfi_miss (lfiomg, LFIOMG)
lfi_miss (lfifmd, LFIFMD)

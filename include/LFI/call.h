#ifndef _LFI_CALL_H
#define _LFI_CALL_H

/**** *lfi_call.h* - Define LFI library interface in a callback structure
 *
 *    Author. 
 *    ------- 
 *     Philippe Marguinaud *METEO-FRANCE*
 *     Original : 12-08-2013
 *
 * Description :
 * All LFI functions are recorded here; in principle it is enough to implement
 * these functions to implement a LFI-like library.
 */


#include "LFI/args.h"

typedef struct lficb_t
{
  void (*lfiouv) (LFIOUV_ARGS_DECL);
  void (*lfican) (LFICAN_ARGS_DECL);
  void (*lficas) (LFICAS_ARGS_DECL);
  void (*lfiecr) (LFIECR_ARGS_DECL);
  void (*lfifer) (LFIFER_ARGS_DECL);
  void (*lfilec) (LFILEC_ARGS_DECL);
  void (*lfinfo) (LFINFO_ARGS_DECL);
  void (*lfinff) (LFINFF_ARGS_DECL);
  void (*lfipos) (LFIPOS_ARGS_DECL);
  void (*lfiver) (LFIVER_ARGS_DECL);
  void (*lfiofm) (LFIOFM_ARGS_DECL);
  void (*lfineg) (LFINEG_ARGS_DECL);
  void (*lfilaf) (LFILAF_ARGS_DECL);
  void (*lfiosg) (LFIOSG_ARGS_DECL);
  void (*lfinum) (LFINUM_ARGS_DECL);
  void (*lfisup) (LFISUP_ARGS_DECL);
  void (*lfiopt) (LFIOPT_ARGS_DECL);
  void (*lfinmg) (LFINMG_ARGS_DECL);
  void (*lficap) (LFICAP_ARGS_DECL);
  void (*lfifra) (LFIFRA_ARGS_DECL);
  void (*lficfg) (LFICFG_ARGS_DECL);
  void (*lfierf) (LFIERF_ARGS_DECL);
  void (*lfilas) (LFILAS_ARGS_DECL);
  void (*lfiren) (LFIREN_ARGS_DECL);
  void (*lfiini) (LFIINI_ARGS_DECL);
  void (*lfipxf) (LFIPXF_ARGS_DECL);
  void (*lfioeg) (LFIOEG_ARGS_DECL);
  void (*lfinaf) (LFINAF_ARGS_DECL);
  void (*lfiofd) (LFIOFD_ARGS_DECL);
  void (*lfiomf) (LFIOMF_ARGS_DECL);
  void (*lfiafm) (LFIAFM_ARGS_DECL);
  void (*lfista) (LFISTA_ARGS_DECL);
  void (*lfiosf) (LFIOSF_ARGS_DECL);
  void (*lfilap) (LFILAP_ARGS_DECL);
  void (*lfioef) (LFIOEF_ARGS_DECL);
  void (*lfimst) (LFIMST_ARGS_DECL);
  void (*lfinim) (LFINIM_ARGS_DECL);
  void (*lfisfm) (LFISFM_ARGS_DECL);
  void (*lfinsg) (LFINSG_ARGS_DECL);
  void (*lfideb) (LFIDEB_ARGS_DECL);
  void (*lfiomg) (LFIOMG_ARGS_DECL);
  void (*lfifmd) (LFIFMD_ARGS_DECL);
}
lficb_t;

#endif

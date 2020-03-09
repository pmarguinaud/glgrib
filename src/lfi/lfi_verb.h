#ifndef _LFI_VERB_H
#define _LFI_VERB_H
/**** *lfi_verb.h* - Routines for handling LFI messages
 *
 *    Author. 
 *    ------- 
 *     Philippe Marguinaud *METEO-FRANCE*
 *     Original : 12-08-2013
 *
 * Description :
 * All routines have the same interfaces as their LFI counterparts,
 * except that an extra argument is pre-pended; this argument tells
 * whether we are entering or leaving the routine.
 * When the LFI routine is about to be entered, then IN arguments are
 * printed; when the LFI routine has exited, OUT arguments are printed
 */


#include "lfi_args.h"

struct lfi_hndl_t;

typedef struct lficb_verb_t
{
  void (*lfiouv) (int, struct lfi_hndl_t *, LFIOUV_ARGS_DECL);
  void (*lfican) (int, struct lfi_hndl_t *, LFICAN_ARGS_DECL);
  void (*lficas) (int, struct lfi_hndl_t *, LFICAS_ARGS_DECL);
  void (*lfiecr) (int, struct lfi_hndl_t *, LFIECR_ARGS_DECL);
  void (*lfifer) (int, struct lfi_hndl_t *, LFIFER_ARGS_DECL);
  void (*lfilec) (int, struct lfi_hndl_t *, LFILEC_ARGS_DECL);
  void (*lfinfo) (int, struct lfi_hndl_t *, LFINFO_ARGS_DECL);
  void (*lfinff) (int, struct lfi_hndl_t *, LFINFF_ARGS_DECL);
  void (*lfipos) (int, struct lfi_hndl_t *, LFIPOS_ARGS_DECL);
  void (*lfiver) (int, struct lfi_hndl_t *, LFIVER_ARGS_DECL);
  void (*lfiofm) (int, struct lfi_hndl_t *, LFIOFM_ARGS_DECL);
  void (*lfineg) (int, struct lfi_hndl_t *, LFINEG_ARGS_DECL);
  void (*lfilaf) (int, struct lfi_hndl_t *, LFILAF_ARGS_DECL);
  void (*lfiosg) (int, struct lfi_hndl_t *, LFIOSG_ARGS_DECL);
  void (*lfinum) (int, struct lfi_hndl_t *, LFINUM_ARGS_DECL);
  void (*lfisup) (int, struct lfi_hndl_t *, LFISUP_ARGS_DECL);
  void (*lfiopt) (int, struct lfi_hndl_t *, LFIOPT_ARGS_DECL);
  void (*lfinmg) (int, struct lfi_hndl_t *, LFINMG_ARGS_DECL);
  void (*lficap) (int, struct lfi_hndl_t *, LFICAP_ARGS_DECL);
  void (*lfifra) (int, struct lfi_hndl_t *, LFIFRA_ARGS_DECL);
  void (*lficfg) (int, struct lfi_hndl_t *, LFICFG_ARGS_DECL);
  void (*lfierf) (int, struct lfi_hndl_t *, LFIERF_ARGS_DECL);
  void (*lfilas) (int, struct lfi_hndl_t *, LFILAS_ARGS_DECL);
  void (*lfiren) (int, struct lfi_hndl_t *, LFIREN_ARGS_DECL);
  void (*lfiini) (int, struct lfi_hndl_t *, LFIINI_ARGS_DECL);
  void (*lfipxf) (int, struct lfi_hndl_t *, LFIPXF_ARGS_DECL);
  void (*lfioeg) (int, struct lfi_hndl_t *, LFIOEG_ARGS_DECL);
  void (*lfinaf) (int, struct lfi_hndl_t *, LFINAF_ARGS_DECL);
  void (*lfiofd) (int, struct lfi_hndl_t *, LFIOFD_ARGS_DECL);
  void (*lfiomf) (int, struct lfi_hndl_t *, LFIOMF_ARGS_DECL);
  void (*lfiafm) (int, struct lfi_hndl_t *, LFIAFM_ARGS_DECL);
  void (*lfista) (int, struct lfi_hndl_t *, LFISTA_ARGS_DECL);
  void (*lfiosf) (int, struct lfi_hndl_t *, LFIOSF_ARGS_DECL);
  void (*lfilap) (int, struct lfi_hndl_t *, LFILAP_ARGS_DECL);
  void (*lfioef) (int, struct lfi_hndl_t *, LFIOEF_ARGS_DECL);
  void (*lfimst) (int, struct lfi_hndl_t *, LFIMST_ARGS_DECL);
  void (*lfinim) (int, struct lfi_hndl_t *, LFINIM_ARGS_DECL);
  void (*lfisfm) (int, struct lfi_hndl_t *, LFISFM_ARGS_DECL);
  void (*lfinsg) (int, struct lfi_hndl_t *, LFINSG_ARGS_DECL);
  void (*lfideb) (int, struct lfi_hndl_t *, LFIDEB_ARGS_DECL);
  void (*lfiomg) (int, struct lfi_hndl_t *, LFIOMG_ARGS_DECL);
  void (*lfifmd) (int, struct lfi_hndl_t *, LFIFMD_ARGS_DECL);
}
lficb_verb_t;

extern lficb_verb_t lficb_verb;

extern void lfi_verb (struct lfi_hndl_t * lfi, const char * func, ...);
extern void lfi_mess_ (const integer64 *, character *, character_len);

#endif

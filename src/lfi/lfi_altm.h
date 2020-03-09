#ifndef _LFI_ALTM_H
#define _LFI_ALTM_H
/**** *lfi_altm.h* - LFI multi-file library
 *
 *    Author. 
 *    ------- 
 *     Philippe Marguinaud *METEO-FRANCE*
 *     Original : 12-08-2013
 *
 * Description :
 */

#include "lfi_args.h"
#include "lfi_call.h"
#include "lfi_hndl.h"

extern lficb_t lficb_altm;

extern lfi_hndl_t * lfi_get_altm_hndl (void *);

extern void lfi_altm_index_ (integer64 *, character *, integer64 *, logical *, character_len);
extern void lfi_altm_merge_ (integer64 *, character *, integer64 *, logical *, character_len);
extern void lfi_altm_copy_ (integer64 *, character *, character *, logical *, character_len, character_len);
extern void lfi_altm_remove_ (integer64 *, character *, logical *, character_len);

#endif


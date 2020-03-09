#ifndef _LFI_ALTS_H
#define _LFI_ALTS_H
/**** *lfi_alts.h* - Plain LFI file library
 *
 *    Author. 
 *    ------- 
 *     Philippe Marguinaud *METEO-FRANCE*
 *     Original : 12-08-2013
 *
 */

#include "lfi_args.h"
#include "lfi_call.h"
#include "lfi_hndl.h"

extern lficb_t lficb_alts;

extern lfi_hndl_t * lfi_get_alts_hndl (void *);

extern void lfi_alts_pack_ (integer64 *, character *, character *, 
                            character_len, character_len);

#endif


#ifndef _LFI_FMUL_H
#define _LFI_FMUL_H
/**** *lfi_fmul.h* - Management of "facteurs multiplicatifs"
 *
 *    Author. 
 *    ------- 
 *     Philippe Marguinaud *METEO-FRANCE*
 *     Original : 12-08-2013
 *
 */

#include "lfi_type.h"

typedef struct lfi_fmul_t
{
  struct lfi_fmul_t * next;
  integer64 mult;
  integer64 unit;
}
lfi_fmul_t;

extern lfi_fmul_t * lfi_fmul_set (lfi_fmul_t **, const integer64 * KNUMER, const integer64 * KFACTM);
extern lfi_fmul_t * lfi_fmul_get (lfi_fmul_t *, const integer64 * KNUMER, integer64 * KFACTM);
extern integer64 lfi_fmul_del (lfi_fmul_t **, const integer64 * KNUMER);
extern void lfi_fmul_free (lfi_fmul_t **);

#endif


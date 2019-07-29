#ifndef _LFI_GROK_H
#define _LFI_GROK_H
/**** *lfi_grok.h* - Find LFI file type
 *
 *    Author. 
 *    ------- 
 *     Philippe Marguinaud *METEO-FRANCE*
 *     Original : 12-08-2013
 *
 */

#include "lfi_type.h"

typedef enum
{
  LFI_NONE = 0,
  LFI_PURE = 1,
  LFI_ALTM = 2,
  LFI_UNKN = 3
}
lfi_grok_t;

extern lfi_grok_t lfi_grok (const character *, character_len);
extern int lfi_unum (integer64 *);

#endif


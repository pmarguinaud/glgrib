/**** *lfi_hndl.c* - LFI handle
 *
 *    Author. 
 *    ------- 
 *     Philippe Marguinaud *METEO-FRANCE*
 *     Original : 12-08-2013
 *
 */
#include <stdlib.h>
#include <string.h>
#include "lfi_type.h"
#include "lfi_hndl.h"
#include "lfi_fort.h"
#include "lfi_alts.h"
#include "lfi_altm.h"
#include "lfi_netw.h"

/* Build the list of possible LFI libraries */
lfi_hndl_t * lfi_hndl_list (void * data)
{
  lficom_t * lficom = data;

  if (strncmp (lficom->cmagic, "LFI_FORT", 8))
    abort ();

  if (lficom->lfihl == NULL)
    {
      lfi_hndl_t * lfi_alts = lfi_get_alts_hndl (data);
      lfi_hndl_t * lfi_altm = lfi_get_altm_hndl (data);
      lfi_hndl_t * lfi_netw = lfi_get_netw_hndl (data);
      lfi_altm->next = lfi_netw;
      lfi_alts->next = lfi_altm;
      lficom->lfihl = lfi_alts;
    }

  return lficom->lfihl;
}

/* Find out which LFI library opened a given unit (KNUMER) */
lfi_hndl_t * lfi_hndl (void * data, integer64 * KNUMER)
{
  lfi_hndl_t * lfi;
  for (lfi = lfi_hndl_list (data); lfi; lfi = lfi->next)
    if (lfi->is_open (lfi, KNUMER))
      break;
  return lfi;
}

/* Free all LFI libraries; 
 * this routine should be called by FREE_LFI in lfimod.F90
 */
void lfi_hndl_free_ (void * data)
{
  lficom_t * lficom = data;
  lfi_hndl_t * lfi;

  if (strncmp (lficom->cmagic, "LFI_FORT", 8))
    abort ();

  for (lfi = lficom->lfihl; lfi; )
    {
      lfi_hndl_t * lfj = lfi->next;
      lfi->destroy (lfi);
      lfi = lfj;
    }
}





/**** *lfi_fmul.c* - Handle a list of KNUMER/KFACTM
 *
 *    Author. 
 *    ------- 
 *     Philippe Marguinaud *METEO-FRANCE*
 *     Original : 12-08-2013
 *
 * Description :
 * Implement an associative array KNUMER => KFACTM
 */
#include <stdlib.h>
#include "lfi_fmul.h"

/* Insert a new pair KNUMER/KFACTM; if KNUMER already exists, then it is updated with the new KFACTM */
lfi_fmul_t * lfi_fmul_set (lfi_fmul_t ** fmul, const integer64 * KNUMER, const integer64 * KFACTM)
{
  lfi_fmul_t * fm;
  integer64 IFACTM;

  fm = lfi_fmul_get (*fmul, KNUMER, &IFACTM);

  if (fm == NULL)
    {
      fm = (lfi_fmul_t *)malloc (sizeof (lfi_fmul_t));
      fm->next = *fmul;
      *fmul = fm;
    }

  fm->mult = *KFACTM;
  fm->unit = *KNUMER;
  
  return fm;
}

/* Get a KFACTM given a KNUMER */
lfi_fmul_t * lfi_fmul_get (lfi_fmul_t * fmul, const integer64 * KNUMER, integer64 * KFACTM)
{
  lfi_fmul_t * fm;
  for (fm = fmul; fm; fm = fm->next)
    if (fm->unit == *KNUMER)
      {
        *KFACTM = fm->mult;
        return fm;
      }
  return NULL;
}

/* Delete a KNUMER entry */
integer64 lfi_fmul_del (lfi_fmul_t ** fmul, const integer64 * KNUMER)
{
  
  lfi_fmul_t * fm, * fn;
  for (fn = NULL, fm = *fmul; fm; fn = fm, fm = fm->next)
    if (fm->unit == *KNUMER)
      {
        integer64 fmult = fm->mult;
        if (fn != NULL)
          fn->next = fm->next;
        else
          *fmul = fm->next;
        free (fm);
        return fmult;
      }
  return -1;
}

/* Free a whole list */
void lfi_fmul_free (lfi_fmul_t ** fmul)
{
  lfi_fmul_t * fm;
  for (fm = *fmul; fm; )
    {
      lfi_fmul_t * fn = fm->next;
      free (fm);
      fm = fn;
    }
  *fmul = NULL;
}


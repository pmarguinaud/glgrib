#ifndef _LFI_HNDL_H
#define _LFI_HNDL_H
/**** *lfi_hndl.h* - LFI handle definition
 *
 *    Author. 
 *    ------- 
 *     Philippe Marguinaud *METEO-FRANCE*
 *     Original : 12-08-2013
 *
 */

#include "lfi_type.h"
#include "lfi_args.h"
#include "lfi_call.h"
#include "lfi_verb.h"

typedef struct lfi_hndl_t
{
  lficb_t * cb;                                       /* LFI routines                           */
  lficb_verb_t * cb_verb;                             /* LFI message routines                   */
  void * data;                                        /* LFI library data                       */
  struct lfi_hndl_t * next;                           /* Next LFI library                       */
  void (*destroy) (struct lfi_hndl_t *);              /* Destroy LFI library                    */
  int (*is_verb) (struct lfi_hndl_t *, integer64 *);  /* Check if unit has verbose mode on      */
  int (*is_open) (struct lfi_hndl_t *, integer64 *);  /* Check if unit is opened                */
  int (*is_fatl) (struct lfi_hndl_t *, integer64 *);  /* Check if unit has fatal errors enabled */
}
lfi_hndl_t;

/* Retrieve LFI library associated with a given KNUMER */
extern lfi_hndl_t * lfi_hndl (void *, integer64 *);

/* Get LFI libraries list */
extern lfi_hndl_t * lfi_hndl_list (void *);

#endif


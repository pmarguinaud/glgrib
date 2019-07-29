#ifndef _LFI_FORT_H
#define _LFI_FORT_H

/* This structure is the head of LFICOM (see lfimod.F90) */

typedef struct lficom_t
{
  character cmagic[8];  /* "LFI_FORT"                */
  void * lfihl;         /* Linked list of lfi_hndl_t */
}
lficom_t;

#endif

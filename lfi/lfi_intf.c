/**** *lfi_intf.c* - Interface to different LFI libraries
 *
 *    Author. 
 *    ------- 
 *     Philippe Marguinaud *METEO-FRANCE*
 *     Original : 12-08-2013
 *
 * Description :
 * When lfiouv is called a choice is made on which LFI library will handle this unit;
 * all subsequent calls will probe all LFI libraries to check which one has opened this unit.
 * This is the basic principle; some routines have no KNUMER argument; these routines
 * set the general message level, etc... and have to be handled by the lfi_intf macro.
 * However, some other routines set the "facteur multiplicatif" of unopened units and 
 * have to be handled in a special manner.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lfi_args.h"
#include "lfi_hndl.h"
#include "lfi_verb.h"
#include "lfi_abor.h"
#include "lfi_grok.h"
#include "lfi_util.h"

static void missing_handler (integer64 * KNUMER)
{
  lfi_abor ("No handler was found for unit %lld\n", *KNUMER);
}

static void fatal_error (const char * func, integer64 * KREP)
{
  lfi_abor ("*** %s, KREP=%lld", func, *KREP);
}

/* This macro handles functions with a KNUMER and a KREP argument */

#define lfi_intf_knumer_krep(lfixxx, LFIXXX) \
void lfixxx##_mt64_ (LFIXXX##_ARGS_DECL)                      \
{                                                             \
  lfi_hndl_t * lfi = lfi_hndl (LFI, KNUMER);                  \
  if (lfi != NULL)                                            \
    {                                                         \
      void * LFI = lfi->data;                                 \
      int verb = lfi->is_verb (lfi, KNUMER);                  \
      int fatl = lfi->is_fatl (lfi, KNUMER);                  \
                                                              \
      if (verb)                                               \
        lfi->cb_verb->lfixxx (0, lfi, LFIXXX##_ARGS_LIST);    \
                                                              \
      lfi->cb->lfixxx (LFIXXX##_ARGS_LIST);                   \
                                                              \
      if (verb)                                               \
        lfi->cb_verb->lfixxx (1, lfi, LFIXXX##_ARGS_LIST);    \
                                                              \
      if ((*KREP != 0) && fatl)                               \
        fatal_error (#LFIXXX, KREP);                          \
                                                              \
    }                                                         \
  else                                                        \
    {                                                         \
      missing_handler (KNUMER);                               \
    }                                                         \
}

/* This macro handles functions without a KNUMER and a KREP argument */

#define lfi_intf(lfixxx, LFIXXX) \
void lfixxx##_mt64_ (LFIXXX##_ARGS_DECL)                      \
{                                                             \
  lfi_hndl_t * lfi, * lfi_head = lfi_hndl_list (LFI);         \
  integer64 INIVAU, IULOUT;                                   \
  lfi_head->cb->lfiomg (lfi_head->data, &INIVAU, &IULOUT);    \
  for (lfi = lfi_head; lfi; lfi = lfi->next)                  \
    {                                                         \
      void * LFI = lfi->data;                                 \
      lfi->cb->lfixxx (LFIXXX##_ARGS_LIST);                   \
    }                                                         \
  if (INIVAU == 2)                                            \
    {                                                         \
      lficb_verb.lfixxx (0, lfi_head, LFIXXX##_ARGS_LIST);    \
      lficb_verb.lfixxx (1, lfi_head, LFIXXX##_ARGS_LIST);    \
    }                                                         \
}

/* This macro handles functions with a KNUMER argument, but without a KREP argument */

#define lfi_intf_knumer(lfixxx, LFIXXX) \
void lfixxx##_mt64_ (LFIXXX##_ARGS_DECL)                      \
{                                                             \
  lfi_hndl_t * lfi = lfi_hndl (LFI, KNUMER);                  \
  if (lfi != NULL)                                            \
    {                                                         \
      void * LFI = lfi->data;                                 \
      int verb = lfi->is_verb (lfi, KNUMER);                  \
                                                              \
      if (verb)                                               \
        lfi->cb_verb->lfixxx (0, lfi, LFIXXX##_ARGS_LIST);    \
                                                              \
      lfi->cb->lfixxx (LFIXXX##_ARGS_LIST);                   \
                                                              \
      if (verb)                                               \
        lfi->cb_verb->lfixxx (1, lfi, LFIXXX##_ARGS_LIST);    \
                                                              \
    }                                                         \
  else                                                        \
    {                                                         \
      missing_handler (KNUMER);                               \
    }                                                         \
}



/* lfiouv has to be coded explicitely; 
 * the choice of the LFI library is made here
 */

void lfiouv_mt64_ (LFIOUV_ARGS_DECL)
{
  lfi_hndl_t * lfi = lfi_hndl_list (LFI);
  int verb;
  int unum = lfi_unum (KNUMER);
  lfi_grok_t lg;
  lficb_verb_t * vcb = &lficb_verb;
  LFI_CSTR (cstto, CDSTTO);

  if (istrue (*LDNOMM))
    {
      lg = lfi_grok (CDNOMF, CDNOMF_len);
    }
  else
    {
      character CLNOMF[32];
      character_len CLNOMF_len = sprintf (CLNOMF, "fort.%lld", *KNUMER);
      lg = lfi_grok (CLNOMF, CLNOMF_len);
    }

  switch (lg)
    {
      case LFI_PURE:
        break;
      case LFI_ALTM:
        unum = 1;
        break;
      case LFI_NONE:
        if ((strcmp (cstto, "NEW") != 0) && (strcmp (cstto, "UNKNOWN") != 0))
          {
            *KREP = -9;
            lfi = NULL;
            goto done;
          }
        break;
      case LFI_NETW:
        unum = 2;
        break;
      case LFI_UNKN:
        *KREP = -10;
        lfi = NULL;
        goto done;
    }

  
  if ((lg != LFI_NONE) && (strcmp (cstto, "OLD") != 0) && (strcmp (cstto, "UNKNOWN") != 0))
    {
      *KREP = -9;
      lfi = NULL;
      goto done;
    }

  for (; unum; unum--)
    lfi = lfi->next;

  LFI = lfi->data;
  lfi->cb->lfiouv (LFIOUV_ARGS_LIST);

done:

  if (lfi != NULL)
    vcb = lfi->cb_verb;

  verb = (*KREP != 0) && (vcb != NULL);

  if (! verb)
    verb = lfi->is_verb (lfi, KNUMER);

  if (verb)
    {
      vcb->lfiouv (0, lfi, LFIOUV_ARGS_LIST);
      vcb->lfiouv (1, lfi, LFIOUV_ARGS_LIST);
    }

  if ((*KREP != 0) && (istrue (*LDERFA)))
    fatal_error ("LFIOUV", KREP);


}

lfi_intf_knumer_krep (lfican, LFICAN)

lfi_intf_knumer_krep (lficas, LFICAS)

lfi_intf_knumer_krep (lfiecr, LFIECR)

lfi_intf_knumer_krep (lfifer, LFIFER)

lfi_intf_knumer_krep (lfilec, LFILEC)

lfi_intf_knumer_krep (lfinfo, LFINFO)

lfi_intf_knumer_krep (lfinff, LFINFF)

lfi_intf_knumer_krep (lfipos, LFIPOS)

lfi_intf (lfiver, LFIVER)


/* lfiofm has to be coded explicitely, because it operates on possibly unopened units */

void lfiofm_mt64_ (LFIOFM_ARGS_DECL)
{
  lfi_hndl_t * lfi = lfi_hndl (LFI, KNUMER);
  if (lfi != NULL)
    {
      void * LFI = lfi->data;
      int verb = lfi->is_verb (lfi, KNUMER);

      int fatl = lfi->is_fatl (lfi, KNUMER);


      if (verb)
        lfi->cb_verb->lfiofm (0, lfi, LFIOFM_ARGS_LIST);

      lfi->cb->lfiofm (LFIOFM_ARGS_LIST);

      if (verb)
        lfi->cb_verb->lfiofm (1, lfi, LFIOFM_ARGS_LIST);

      if ((*KREP != 0) && fatl)
        fatal_error ("LFIOFM", KREP);

    }
  else
    {
      lfi_hndl_t * lfi, * lfi_head = lfi_hndl_list (LFI);
      for (lfi = lfi_head; lfi; lfi = lfi->next)
        {
          void * LFI = lfi->data;
          lfi->cb->lfiofm (LFIOFM_ARGS_LIST);
        }
      lficb_verb.lfiofm (0, lfi_head, LFIOFM_ARGS_LIST);
      lficb_verb.lfiofm (1, lfi_head, LFIOFM_ARGS_LIST);
    }
}

lfi_intf (lfineg, LFINEG)

lfi_intf_knumer_krep (lfilaf, LFILAF)

lfi_intf (lfiosg, LFIOSG)

lfi_intf_knumer (lfinum, LFINUM)

lfi_intf_knumer_krep (lfisup, LFISUP)

lfi_intf_knumer_krep (lfiopt, LFIOPT)

lfi_intf (lfinmg, LFINMG)

lfi_intf_knumer_krep (lficap, LFICAP)

lfi_intf (lfifra, LFIFRA)

lfi_intf (lficfg, LFICFG)

lfi_intf_knumer_krep (lfierf, LFIERF)

lfi_intf_knumer_krep (lfilas, LFILAS)

lfi_intf_knumer_krep (lfiren, LFIREN)

lfi_intf (lfiini, LFIINI)

lfi_intf_knumer_krep (lfipxf, LFIPXF)

lfi_intf (lfioeg, LFIOEG)

lfi_intf_knumer_krep (lfinaf, LFINAF)

lfi_intf (lfiofd, LFIOFD)

lfi_intf_knumer_krep (lfiomf, LFIOMF)


/* lfiafm has to be coded explicitely, because it operates on unopened units */

void lfiafm_mt64_ (LFIAFM_ARGS_DECL)
{
  lfi_hndl_t * lfi = lfi_hndl (LFI, KNUMER);
  if (lfi != NULL)
    {
      *KREP = -5;
    }
  else
    {
      lfi_hndl_t * lfi, * lfi_head = lfi_hndl_list (LFI);
      for (lfi = lfi_head; lfi; lfi = lfi->next)
        {
          void * LFI = lfi->data;
          lfi->cb->lfiafm (LFIAFM_ARGS_LIST);
        }
      lficb_verb.lfiafm (0, lfi_head, LFIAFM_ARGS_LIST);
      lficb_verb.lfiafm (1, lfi_head, LFIAFM_ARGS_LIST);
    }
}

lfi_intf_knumer_krep (lfista, LFISTA)

lfi_intf_knumer_krep (lfiosf, LFIOSF)

lfi_intf_knumer_krep (lfilap, LFILAP)

lfi_intf_knumer_krep (lfioef, LFIOEF)

lfi_intf_knumer_krep (lfimst, LFIMST)

lfi_intf_knumer_krep (lfinim, LFINIM)


/* lfisfm has to be coded explicitely, because it operates on unopened units */

void lfisfm_mt64_ (LFISFM_ARGS_DECL)
{
  lfi_hndl_t * lfi = lfi_hndl (LFI, KNUMER);
  if (lfi != NULL)
    {
      *KREP = -5;
    }
  else
    {
      lfi_hndl_t * lfi, * lfi_head = lfi_hndl_list (LFI);
      for (lfi = lfi_head; lfi; lfi = lfi->next)
        {
          void * LFI = lfi->data;
          lfi->cb->lfisfm (LFISFM_ARGS_LIST);
        }
      lficb_verb.lfisfm (0, lfi_head, LFISFM_ARGS_LIST);
      lficb_verb.lfisfm (1, lfi_head, LFISFM_ARGS_LIST);
    }
}

lfi_intf (lfinsg, LFINSG)

lfi_intf (lfideb, LFIDEB)

lfi_intf (lfiomg, LFIOMG)

lfi_intf (lfifmd, LFIFMD)

/* lfipro has to be coded explicitely, because it operates on unopened units */

void lfipro_mt64_ (LFIPRO_ARGS_DECL)
{
  lfi_hndl_t * lfi, * lfi_head = lfi_hndl_list (LFI);
  *LDOPEN = fort_FALSE;
  for (lfi = lfi_head; lfi; lfi = lfi->next)
    {   
      if (lfi->is_open (lfi, KNUMER))
        {   
          *LDOPEN = fort_TRUE;
          break;
        }   
    }   
}




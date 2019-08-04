/**** *lfi_altm.c* - Multi-file LFI layer.
 *
 *    Author. 
 *    ------- 
 *     Philippe Marguinaud *METEO-FRANCE*
 *     Original : 12-08-2013
 *
 * Description :
 * This LFI library is based on lfi_alts and is able to handle multiple files.
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "lfi_type.h"
#include "lfi_altm.h"
#include "lfi_dumm.h"
#include "lfi_miss.h"
#include "lfi_abor.h"
#include "lfi_grok.h"
#include "lfi_hndl.h"
#include "lfi_alts.h"
#include "lfi_verb.h"
#include "lfi_fmul.h"
#include "lfi_util.h"

#include "drhook.h"

/* Ancillary macros; the article name length is defined here */

#define ARTNLEN 16
#define minARTN(x) ((x) > ARTNLEN ? ARTNLEN : (x))

#define eqan(a,b) (strncmp ((a), (b), ARTNLEN) == 0)
#define neqan(a,b) (!eqan(a,b))

/* Name of a hole in the index */
static const char *
  blank_index = "                "
;

/* Sub-file descriptor */
typedef struct lfi_altm_fh_fidx_t
{
  lfi_hndl_t * als;              /* LFI handle associated to this file        */
  const char * cnomf;            /* File name                                 */
  integer64 inumer;              /* File unit                                 */
  int       dead;                /* File is empty (and may have been removed) */
}
lfi_altm_fh_fidx_t;

/* Article descriptor */
typedef struct lfi_altm_fh_aidx_t
{
  character name[ARTNLEN];       /* Article name                            */
  character namf[ARTNLEN];       /* Article name in LFI file                */
  integer64 ifh;                 /* File rank; refers to a lfi_altm_fh_fidx_t 
                                  * struct in the file index                */
}
lfi_altm_fh_aidx_t;

/* Open unit descriptor */
typedef struct lfi_altm_fh_t
{
  struct lfi_altm_fh_t * next;

  integer64 inumer;              /* File unit                                */
  const char * cnomf;            /* File name                                */
  const char * cstto;            /* File status (Fortran)                    */
  integer64 inimes;              /* Message level                            */
  logical llerfa;                /* All errors are fatal                     */
  int nfidx;                     /* Number of LFI files opened               */
  lfi_altm_fh_fidx_t * fidx;     /* LFI handles                              */
  int naidx;                     /* Number of articles                       */
  lfi_altm_fh_aidx_t * aidx;     /* Articles descriptors                     */
  int iart;                      /* Current article rank                     */
  int modified;                  /* File was modified                        */
  integer64 ifh_w;               /* File index to write to                   */
  integer64 fmult;               /* Facteur multiplicatif                    */
  int unlink;                    /* Unlink unreferenced sub-files            */
  int rmdir;                     /* Remove directory of sub-files when empty */
}  
lfi_altm_fh_t;

/* LFI library descriptor */
typedef struct lfi_altm_t
{
  char cmagic[8];
  lfi_altm_fh_t * fh;            /* File descriptors list                      */
  integer64 fmult;               /* Facteur multiplicatif                      */
  lfi_fmul_t * fmult_list;       /* Predefined "facteurs multiplicatifs"       */
  int maxartals;                 /* Maximum number of articles per sub-file    */
  int nerfag;                    /* Erreurs fatales                            */
  int inivau;                    /* Niveau global des messages                 */
  int iulout;                    /* Unite Fortran pour impression des messages */
}
lfi_altm_t;

/* Find a suitable name for a sub-file; this name is based on main file name */
static const char * getfname (const char * cnomf)
{
  int cnomf_len = strlen (cnomf);
  char cnomfy[cnomf_len+20];
  int i;

  /* Look for a suitable name */

  strcpy (cnomfy, cnomf);
  strcat (cnomfy, ".d");

  if (lfi_mkdir (cnomfy) < 0)
    lfi_abor ("Cannot mkdir `%s'", cnomfy);

  for (i = 0; ; i++)
    {
      struct stat st;
      sprintf (cnomfy+cnomf_len+2, "/%6.6d", i);
      if (stat (cnomfy, &st) < 0)
        break;
    }

  return strdup (cnomfy);
}

/* Cast and check opaque void * in a lfi_altm_t pointer */
static lfi_altm_t * lookup_alm (void * LFI)
{
  lfi_altm_t * alm = LFI; 
  if (strncmp (alm->cmagic, "lfi_altm", 8))
    lfi_abor ("Corrupted descriptor");
  return alm;
}

/* Find unit descriptor by unit number */
static lfi_altm_fh_t * lookup_fh (lfi_altm_t * alm, integer64 * KNUMER, int fatal)
{
  lfi_altm_fh_t * fh;
  for (fh = alm->fh; fh; fh = fh->next)
    if (fh->inumer == *KNUMER)
      return fh;
  if (fatal)
    lfi_abor ("File number `%lld' is not opened", *KNUMER); 
  return NULL;
}

/* Look for next/previous article in the index */
static int seek_rc (lfi_altm_fh_t * fh, int d) 
{
  int iart = fh->iart;
  d = d >= 0 ? +1 : -1;

  if ((d < 0) && (iart < 0))
    iart = fh->naidx;

  while (1)
    {
      iart = iart + d;
      if ((iart >= fh->naidx) || (iart < 0))
        goto ko;
      if (neqan (fh->aidx[iart].name, blank_index))
        goto ok;
    }

  return 0;
ko: 
  return -1;
ok:
  return iart;
}

/* Check article name length */
static int check_an_len (const char * CDNOMA, const character_len CDNOMA_len)
{
  int len = lfi_fstrlen (CDNOMA, CDNOMA_len);
  return len > ARTNLEN ? 0 : 1;
}

/* Check whether article name is valid */
static int check_an (const char * CDNOMA, const character_len CDNOMA_len)
{
  int len;
  if (! check_an_len (CDNOMA, CDNOMA_len))
    return 0;
  len = minARTN (CDNOMA_len);
  return strncmp (CDNOMA, blank_index, len);
}

/* Search article by name */
static int lookup_rc (lfi_altm_fh_t * fh, character * name, character_len name_len)
{
  char _name[ARTNLEN];
  int iart;

  if (! check_an_len (name, name_len))
    return -15;

  memset (_name, ' ', ARTNLEN);
  memcpy (_name, name, minARTN (name_len));

  /* Look around current article */
  if (fh->iart >= 0)
    {    
      const int da = 1; 
      int iart1 = fh->iart-da > 0 ? fh->iart-da : 0; 
      int iart2 = fh->iart+da < fh->naidx ? fh->iart+da : fh->naidx;
      for (iart = iart1; iart < iart2; iart++)
        if (eqan (_name, fh->aidx[iart].name))
          return iart;
    }    

  for (iart = 0; iart < fh->naidx; iart++)
    if (eqan (_name, fh->aidx[iart].name))
      return iart;

  return -20;
}


/* Write file header; this is an ASCII file */
static void fh_write_hdr (lfi_altm_fh_t * fh, integer64 * KREP)
{
  integer64 ifh, len;
  integer64 iart, nart;
  integer64 fh2na[fh->nfidx]; /* Number of articles per file */
  integer64 fh2ra[fh->nfidx]; /* File reranking map          */
  integer64 rank;
  FILE * fp;

  errno = 0;
  fp = fopen (fh->cnomf, "w");
  
  if (fp == NULL)
    lfi_abor ("Cannot open `%s' for writing", fh->cnomf);
  
  /* File name max len */
  
  for (len = 0, ifh = 0; ifh < fh->nfidx; ifh++)
    len = len < strlen (fh->fidx[ifh].cnomf) ? strlen (fh->fidx[ifh].cnomf) : len;
  
  /* Number of articles per file */
  
  for (ifh = 0; ifh < fh->nfidx; ifh++)
    fh2na[ifh] = 0;
  
  for (iart = 0; iart < fh->naidx; iart++)
    if (neqan (blank_index, fh->aidx[iart].name))
      fh2na[fh->aidx[iart].ifh]++;
  
  /* Compute files ranks */
  
  for (rank = 0, ifh = 0; ifh < fh->nfidx; ifh++)
    if (fh2na[ifh] > 0)
      fh2ra[ifh] = rank++;
    else
      fh2ra[ifh] = -1;
  
  if (fprintf (fp, "LFI_ALTM\n") != 9)
    goto write_err;
  
  /* Number of files, file name max length */
  fprintf (fp, "%lld %lld\n", rank, len);
  
  /* Print file list */
  for (ifh = 0; ifh < fh->nfidx; ifh++)
    /* Skip files when no article is referenced */
    if (fh2ra[ifh] >= 0)
      {
        const char * cnomf = fh->fidx[ifh].cnomf;
        if (fprintf (fp, "%s\n", cnomf) != strlen (cnomf) + 1)
          goto write_err;
      }
    else if (fh->unlink && (! fh->fidx[ifh].dead))
    /* Unlink unreferenced files */
      {
        const char * cnomf = fh->fidx[ifh].cnomf;
        if (unlink (cnomf) < 0)
          lfi_abor ("Cannot unlink `%s'", cnomf);
        if (fh->rmdir)
          {
            const char * dir = lfi_dirname (cnomf);
            lfi_rmdir (dir);
            free ((void *)dir);
          }
        fh->fidx[ifh].dead = 1;
      }
  
  for (nart = 0, iart = 0; iart < fh->naidx; iart++)
    if (neqan (blank_index, fh->aidx[iart].name))
      nart++;
  
  /* Number of articles, article name length */
  fprintf (fp, "%lld %d\n", nart, ARTNLEN);
  
  /* Print article names */
  for (iart = 0; iart < fh->naidx; iart++)
    {
      if (eqan (fh->aidx[iart].name, blank_index))
        continue;

      ifh = fh->aidx[iart].ifh;

      fprintf (fp, "%lld ", fh2ra[ifh]);

      /* Article name is the same in sub-file */
      if (eqan (fh->aidx[iart].name, fh->aidx[iart].namf))
        {
          if (fprintf (fp, "%-16.16s ", fh->aidx[iart].name) != 17)
            goto write_err;
        }
      /* Article name is different in sub-file; write its name in sub-file */
      else
        {
          if (fprintf (fp, "%-16.16s.%-16.16s ", fh->aidx[iart].name, fh->aidx[iart].namf) != 34)
            goto write_err;
        }
      if (fh2ra[ifh] < 0)
        lfi_abor ("Internal error; unexpected negative file rank");
      if (fprintf (fp, "\n") != 1)
        goto write_err;
    }
  
  if (fclose (fp) != 0)
    goto close_err;

  *KREP = 0;

  return;

write_err:

  lfi_abor ("Cannot write to `%s'", fh->cnomf);

close_err:

  lfi_abor ("Cannot close `%s'", fh->cnomf);

}

/* Mark file as modified */
static void fh_modified (lfi_altm_fh_t * fh)
{
  integer64 IREP;

  if (fh->modified)
    return;

  fh->modified = 1;

  fh_write_hdr (fh, &IREP);
}

/* Get a new sub-file handle for writing; 
 * this sub-file descriptor is returned by value, and be used in a limited scope */
static lfi_altm_fh_fidx_t getfhw (lfi_altm_fh_t * fh, int new)
{
  lfi_altm_fh_fidx_t fhw;

  /* If a file is already open, then return it */
  if ((! new) && (fh->ifh_w >= 0))
    {
      fhw = fh->fidx[fh->ifh_w];
      return fhw;
    }
  
  /* Reallocate file handle index */ 

  fh->fidx = (lfi_altm_fh_fidx_t *)realloc (fh->fidx, (fh->nfidx + 1) * sizeof (lfi_altm_fh_fidx_t));
  fh->ifh_w = fh->nfidx;
  fh->nfidx = fh->nfidx + 1;


  /* Initialize & open new file handle */

  fh->fidx[fh->ifh_w].als    = lfi_get_alts_hndl (NULL);
  fh->fidx[fh->ifh_w].inumer = fh->inumer;
  fh->fidx[fh->ifh_w].cnomf  = getfname (fh->cnomf);
  fh->fidx[fh->ifh_w].dead   = 0;

  fhw = fh->fidx[fh->ifh_w];

  /* Open sub-file */
  {
    logical LLNOMM = fort_TRUE, LLERFA = fort_FALSE, LLIMST = fort_FALSE;
    integer64 INIMES = 0, INBARP = 0, INBARI = 0;
    integer64 INUMER = fhw.inumer;
    integer64 IREP;
    character * CLSTTO = (character *)"NEW";
    character * CLNOMF = (character *)fhw.cnomf;

    fhw.als->cb->lfifmd (fhw.als->data, &fh->fmult);

    fhw.als->cb->lfiouv (fhw.als->data, &IREP, &INUMER, &LLNOMM, CLNOMF, CLSTTO, &LLERFA, &LLIMST, &INIMES, &INBARP, &INBARI,
                         strlen (CLNOMF), strlen (CLSTTO));
    if (IREP != 0)
      lfi_abor ("Failed to open `%s' for writing", CLNOMF);
  }

  return fhw;
}

static void fh_set_filename (lfi_altm_fh_t * fh, character * CDNOMF, character_len CDNOMF_len)
{
  if (fh->cnomf != NULL)
    free ((void *)fh->cnomf);

  fh->cnomf = lfi_fstrdup (CDNOMF, CDNOMF_len, NULL);
}

static const char * resolve_filename (const char * base, const char * filename, int filename_len, char * path)
{
  LFI_CSTR (_file, filename);
  int base_len = strlen (base);

  if (path == NULL)
    path = (char *)malloc (base_len + filename_len + 10);

  if (_file[0] != '/')
    {
      int i;
      strcpy (path, base);
      for (i = base_len-1; i >= 0; i--)
        if (path[i] == '/')
          break;
      path[i+1] = '\0';
      strcat (path, _file);
    }
  else
    {
      strcpy (path, _file);
    }
  
  lfi_cleanup_path (path);

  return path;
}


static lfi_hndl_t * _getfhr (const char * base, character * CDNOMF, integer64 * KNUMER, character_len CDNOMF_len)
{
  integer64 IREP, INIMES = 0, INBARP = 0, INBARI = 0;
  logical LLNOMM = fort_TRUE, LLERFA = fort_FALSE, LLIMST = fort_FALSE;
  character * CLSTTO = (character *)"OLD";
  lfi_hndl_t * als = lfi_get_alts_hndl (NULL);
  const char * cnomf = resolve_filename (base, CDNOMF, CDNOMF_len, NULL);

  als->cb->lfiouv (als->data, &IREP, KNUMER, &LLNOMM, (character *)cnomf, CLSTTO, &LLERFA, &LLIMST, &INIMES, &INBARP, &INBARI,
                   strlen (cnomf), strlen (CLSTTO));
  
  if (IREP != 0)
    {
      lfi_abor ("Failed to open `%s' for reading", cnomf);
    }

  free ((void *)cnomf);

  return als;
}

/* Get a file descriptor for file of rank ifh; 
 * the object is returned by value and should be used in a limited scope 
 * this routine has to be THREAD-SAFE */
static lfi_altm_fh_fidx_t getfhr (lfi_altm_fh_t * fh, int ifh)
{
  lfi_altm_fh_fidx_t fhr;

  if (ifh < 0)
    {
      memset (&fhr, 0, sizeof (fhr));
      return fhr;
    }

  /* Open sub-file */
  if (fh->fidx[ifh].als == NULL)
    {
      character * CLNOMF = (character *)fh->fidx[ifh].cnomf;
      character_len CLNOMF_len = strlen (CLNOMF);
      integer64 * INUMER = &fh->fidx[ifh].inumer;
      fh->fidx[ifh].als = _getfhr (fh->cnomf, CLNOMF, INUMER, CLNOMF_len);
    }

  return fh->fidx[ifh];
}

#define ALM_DECL \
  lfi_altm_t * alm = lookup_alm (LFI);
#define FH_DECL(fatal) \
  lfi_altm_fh_t * fh = lookup_fh (alm, KNUMER, fatal);
#define ART_DECL \
  int iart = lookup_rc (fh, CDNOMA, CDNOMA_len);                     \
  integer64 ifh = iart < 0 ? -1 : fh->aidx[iart].ifh;                \
  lfi_altm_fh_fidx_t fhr = getfhr (fh, ifh);

/* Allocate and initialize a new file descriptor */
static lfi_altm_fh_t * fh_new (LFIOUV_ARGS_DECL)
{
  ALM_DECL;
  lfi_altm_fh_t * fh;

  *KREP = 0;
  *KNBARI = 0;
  
  if (*KNUMER == 0)
    {
      integer64 inumer = -2000000;
      lfi_altm_fh_t * fh;
again:
      for (fh = alm->fh; fh; fh = fh->next)
        if (fh->inumer == inumer)
          {
            inumer--;
            goto again;
          }
      *KNUMER = inumer;
    }

  fh = (lfi_altm_fh_t *)malloc (sizeof (lfi_altm_fh_t));
  memset (fh, '\0', sizeof (lfi_altm_fh_t));
  fh->next      = NULL;
  fh->inumer    = *KNUMER;
  fh->inimes    = *KNIMES;
  fh->llerfa    = *LDERFA;
  fh->cstto     = lfi_fstrdup (CDSTTO, CDSTTO_len, NULL);
  fh->iart      = -1;
  fh->ifh_w     = -1;
  fh->fmult     = alm->fmult;
  fh->unlink    = 1;
  fh->rmdir     = 1;
  fh_set_filename (fh, CDNOMF, CDNOMF_len);
  lfi_fmul_get (alm->fmult_list, KNUMER, &fh->fmult);
  

  return fh;
}

/* Open a LFI_ALTM file; do not register it in the LFI handle */
static lfi_altm_fh_t * lfiouv_mult (LFIOUV_ARGS_DECL)
{
  integer64 ifh, len;
  integer64 iart, artnlen;
  FILE * fp = NULL;
  char cmagic[9];
  lfi_altm_fh_t * fh = fh_new (LFIOUV_ARGS_LIST);

  errno = 0;

  fp = fopen (fh->cnomf, "r");

  if (fp == NULL)
    lfi_abor ("Cannot open `%s' for reading", fh->cnomf);

  if (fscanf (fp, "%8s\n", cmagic) < 0)
    goto read_err;

  if (fscanf (fp, "%d %lld\n", &fh->nfidx, &len) != 2)
    goto read_err;

  fh->fidx = (lfi_altm_fh_fidx_t *)calloc (sizeof (lfi_altm_fh_fidx_t), fh->nfidx);

  for (ifh = 0; ifh < fh->nfidx; ifh++)
    {
      fh->fidx[ifh].cnomf  = (const char *)malloc (len+1);
      fh->fidx[ifh].inumer = *KNUMER;
      if (fscanf (fp, "%s\n", (char *)fh->fidx[ifh].cnomf) != 1)
        goto read_err;
    }

  if (fscanf (fp, "%d %lld\n", &fh->naidx, &artnlen) != 2)
    goto read_err;

  if (artnlen != ARTNLEN)
    lfi_abor ("Unexpected article length in unit %lld, `%s'", artnlen, fh->cnomf);

  fh->aidx = (lfi_altm_fh_aidx_t *)malloc (sizeof (lfi_altm_fh_aidx_t) * fh->naidx);

  for (iart = 0; iart < fh->naidx; iart++)
    {
      char c;

      if (fscanf (fp, "%lld ", &fh->aidx[iart].ifh) != 1)
        goto read_err;

      if (fread (fh->aidx[iart].name, 1, ARTNLEN, fp) != ARTNLEN)
        goto read_err;

      if (fread (&c, 1, 1, fp) != 1)
        goto read_err;

      if (c != ' ')
        {
          if (fread (fh->aidx[iart].namf, 1, ARTNLEN, fp) != ARTNLEN)
            goto read_err;
        }
      else
        {
          memcpy (fh->aidx[iart].namf, fh->aidx[iart].name, ARTNLEN);
        }

    }

  if (fclose (fp) != 0)
    goto close_err;

  *KNBARI = fh->naidx;
  *KREP = 0;

  return fh;

read_err:

      lfi_abor ("Cannot read from `%s'", fh->cnomf);

close_err:

      lfi_abor ("Cannot close `%s'", fh->cnomf);

  return NULL;
}

static void lfifer_mult (lfi_altm_fh_t * fh)
{
  int ifh;

  /* Free all data */

  for (ifh = 0; ifh < fh->nfidx; ifh++)
    {
      if (fh->fidx[ifh].als != NULL)
        lfi_abor ("Attempt to free LFI_ALTM handle while sub-files are still opened");
      free ((void *)fh->fidx[ifh].cnomf);
    }

  free (fh->fidx);
  free (fh->aidx);
  free ((void *)fh->cnomf); /* Avoid warning with (void *) */
  free ((void *)fh->cstto); /* Avoid warning with (void *) */
  free (fh);

}


static int remove_duplicates (lfi_altm_fh_t * fh)
{
  int iart;
  int rmd = 0;

  for (iart = 0; iart < fh->naidx; iart++)
    {
      /* Check if article was read afterwards; if so, blank the last entry */
      int iartx;
      for (iartx = iart+1; iartx < fh->naidx; iartx++)
        if (eqan (fh->aidx[iart].name, fh->aidx[iartx].name))
          {
            memcpy (fh->aidx[iartx].name, blank_index, ARTNLEN);
            rmd++;
          }
    }
 
  return rmd;
}


/* Create a new multi-file object from a list of traditional LFI files ;
 * all arguments have the same meaning as in lfiouv, 
 * except that CDNOMF is a character array of size KNNOMF+1, whose first element is the multi-file name,
 * and remaining elements are LFI files to take into account */
static lfi_altm_fh_t * lfiouv_pure_lfi (void * LFI, integer64 * KREP, integer64 * KNUMER, logical * LDNOMM, character * CDNOMF, 
                                        integer64 * KNNOMF, character * CDSTTO, logical * LDERFA, logical * LDIMST, integer64 * KNIMES, 
                                        integer64 * KNBARP, integer64 * KNBARI, character_len CDNOMF_len, character_len CDSTTO_len,
                                        int fast)
{
  integer64 ILONG, IPOSEX;
  logical LLAVAN = fort_TRUE;
  character CLNOMA[ARTNLEN];
  integer64 CLNOMA_len = ARTNLEN;
  int iart, ifh;
  lfi_altm_fh_t * fh;

  DRHOOK_START ("lfiouv_pure_lfi");

  fh = fh_new (LFIOUV_ARGS_LIST);

  fh->nfidx = *KNNOMF;
  fh->fidx  = (lfi_altm_fh_fidx_t *)malloc (sizeof (lfi_altm_fh_fidx_t) * fh->nfidx);

  /* Open LFI files */
  
  for (ifh = 0; ifh < fh->nfidx; ifh++)
    {
      fh->fidx[ifh].als    = NULL;
      fh->fidx[ifh].cnomf  = lfi_fstrdup (CDNOMF + CDNOMF_len * (ifh + 1), CDNOMF_len, NULL);
      fh->fidx[ifh].inumer = *KNUMER;
      fh->fidx[ifh].dead   = 0;
    }

  if ((fast) && (*KNNOMF > 1))
#pragma omp parallel private (ifh) 
    {
#pragma omp for schedule (dynamic)
      for (ifh = 0; ifh < fh->nfidx; ifh++)
        getfhr (fh, ifh);
    }

  /* Count the number of LFI articles */
  
  for (fh->naidx = 0, ifh = 0; ifh < fh->nfidx; ifh++)
    {
      lfi_altm_fh_fidx_t fhr = getfhr (fh, ifh);
      integer64 INALDO, INTROU, INARES, INAMAX;
      fhr.als->cb->lfinaf (fhr.als->data, KREP, &fhr.inumer, &INALDO, &INTROU, &INARES, &INAMAX);
      fh->naidx += INALDO;
    }

  /* Allocate article index */

  fh->aidx  = (lfi_altm_fh_aidx_t *)malloc (sizeof (lfi_altm_fh_aidx_t) * fh->naidx);

  /* Read LFI article list */

  for (iart = 0, ifh = 0; ifh < fh->nfidx; ifh++)
    {
      lfi_altm_fh_fidx_t fhr = getfhr (fh, ifh);
      fhr.als->cb->lfipos (fhr.als->data, KREP, KNUMER);
      for (; ; iart++) 
        {
          fhr.als->cb->lficas (fhr.als->data, KREP, &fhr.inumer, CLNOMA, &ILONG, &IPOSEX, &LLAVAN, CLNOMA_len);
          if ((ILONG == 0) && eqan (CLNOMA, blank_index))
            break;
          memcpy (fh->aidx[iart].name, CLNOMA, ARTNLEN);
          memcpy (fh->aidx[iart].namf, CLNOMA, ARTNLEN);
          fh->aidx[iart].ifh = ifh;
        }
    }

  *KNBARI = fh->naidx;

  *KNBARI -= remove_duplicates (fh);

  fh_modified (fh);

  DRHOOK_END (0);

  return fh;
}

static lfi_altm_fh_t * lfiouv_mixed_lfi (void * LFI, integer64 * KREP, integer64 * KNUMER, logical * LDNOMM, character * CDNOMF, 
                                         integer64 * KNNOMF, character * CDSTTO, logical * LDERFA, logical * LDIMST, integer64 * KNIMES, 
                                         integer64 * KNBARP, integer64 * KNBARI, logical * LDRELATIVE, 
                                         character_len CDNOMF_len, character_len CDSTTO_len,
                                         int fast, int do_link, int do_copy, int do_unlink)
{
  integer64 ILONG, IPOSEX;
  logical LLAVAN = fort_TRUE;
  character CLNOMA[ARTNLEN];
  integer64 CLNOMA_len = ARTNLEN;
  int iart, ifh;
  lfi_altm_fh_t * fh;
  
  typedef struct 
  {
    int pure;
    char * cnomf;
    character * CLNOMF;
    lfi_hndl_t    * als;
    lfi_altm_fh_t * fh;
    integer64 inumer;
    integer64 irep;
    int _errno;
  } _lfi_mixed_t;

  _lfi_mixed_t * lm;
  int ifi;

  DRHOOK_START ("lfiouv_mixed_lfi");

  fh = fh_new (LFIOUV_ARGS_LIST);

  fh->unlink = do_unlink;
  fh->rmdir  = do_unlink;

  lm = (_lfi_mixed_t *)malloc (sizeof (_lfi_mixed_t) * *KNNOMF);
  memset (lm, 0, sizeof (_lfi_mixed_t) * *KNNOMF);

  /* Open all sub-files */

 /* omp pragmas may crash the Cray compiler ; use O0 in that case. REK 13-Jan-2015 */
#pragma omp parallel if (fast)
  {
    int ifi, err = 0;
#pragma omp for schedule (dynamic)
    for (ifi = 0; ifi < *KNNOMF; ifi++)
      {
        lfi_grok_t grok;
        character * CLNOMF = CDNOMF + CDNOMF_len * (ifi + 1);

        if (err > 0)
          continue;

        grok = lfi_grok (CLNOMF, CDNOMF_len);
        lm[ifi].CLNOMF = CLNOMF;

        switch (grok)
          {
            case LFI_NONE:
              lm[ifi]._errno = ENOENT;
              lm[ifi].irep = 1;
              goto error_omp;
            case LFI_UNKN:
              lm[ifi].irep = -10;
              goto error_omp;
            case LFI_PURE:
              lm[ifi].pure  = 1;
              lm[ifi].cnomf = lfi_fstrdup (CLNOMF, CDNOMF_len, NULL);
              lm[ifi].als   = _getfhr ("", CLNOMF, &lm[ifi].inumer, CDNOMF_len);
              break;
            case LFI_ALTM:
              {
                integer64 INIMES = 0, INBARP = 0, INBARI = 0;
                logical LLNOMM = fort_TRUE, LLERFA = fort_FALSE, LLIMST = fort_FALSE;
                character * CLSTTO = (character *)"OLD";
                lm[ifi].fh = lfiouv_mult (LFI, &lm[ifi].irep, &lm[ifi].inumer, &LLNOMM, CLNOMF, CLSTTO, &LLERFA, 
                                          &LLIMST, &INIMES, &INBARP, &INBARI, CDNOMF_len, strlen (CLSTTO));
                if (lm[ifi].irep != 0)
                  {
                    goto error_omp;
                  }
              }
              break;
            default:
              lm[ifi].irep = -10;
              goto error_omp;
          }

        continue;

error_omp:
        err++;
      }
  }

  /* Check if any errors occured */

  {
    int ifi, err = 0;
    for (ifi = 0; ifi < *KNNOMF; ifi++)
      {
        if (lm[ifi].irep)
          {
            errno = lm[ifi]._errno;
            *KREP = lm[ifi].irep;
            lfi_verb (NULL, "lfiouv_mixed_lfi", "KREP", KREP, "CDNOMF", lm[ifi].CLNOMF, CDNOMF_len, NULL);
            err++;
          }
        if (err)
          goto error;
      }
  }

  /* Reckon the number of sub-files */

  fh->nfidx = 0;

  for (ifi = 0; ifi < *KNNOMF; ifi++)
    if (lm[ifi].pure)
      fh->nfidx++;
    else
      fh->nfidx += lm[ifi].fh->nfidx;

  fh->fidx = (lfi_altm_fh_fidx_t *)malloc (sizeof (lfi_altm_fh_fidx_t) * fh->nfidx);

  /* Set sub files attributes of newly created file & count articles */
  
  fh->naidx = 0;

{
  LFI_CSTR (cnomf, CDNOMF);
  const char * cnoml;

  for (ifi = 0, ifh = 0; ifi < *KNNOMF; ifi++)
    if (lm[ifi].pure)
      {
        integer64 INALDO, INTROU, INARES, INAMAX;
        lfi_hndl_t * als = lm[ifi].als;

        if (do_link)
          {
            cnoml = getfname (cnomf);
      
            /* Link sub-file */
      
            if ((*KREP = lfi_smartcopy (lm[ifi].cnomf, cnoml, do_copy)) != 0)
              goto error;
          }
        else
          {
            cnoml = strdup (lm[ifi].cnomf);
          }


        /* Make path of sub-file relative to path of main file */
        if (istrue (*LDRELATIVE))
          cnoml = lfi_make_relative_path (cnomf, (character *)cnoml);

        fh->fidx[ifh].als    = als;
        fh->fidx[ifh].cnomf  = cnoml;
        fh->fidx[ifh].inumer = lm[ifi].inumer;
        fh->fidx[ifh].dead   = 0;

        free ((void *)lm[ifi].cnomf);

        lm[ifi].als          = NULL;
        lm[ifi].cnomf        = NULL;
        lm[ifi].inumer       = 0;

        als->cb->lfinaf (als->data, KREP, &fh->fidx[ifh].inumer, &INALDO, &INTROU, &INARES, &INAMAX);
        fh->naidx += INALDO;

        ifh++;
      }
    else
      {
        int ifg;
        lfi_altm_fh_t * fg = lm[ifi].fh;

        for (ifg = 0; ifg < fg->nfidx; ifg++, ifh++)
          {
            const char * cnomg = resolve_filename (fg->cnomf, fg->fidx[ifg].cnomf, strlen (fg->fidx[ifg].cnomf), NULL);

            if (do_link)
              {
                 cnoml = getfname (cnomf);

                 /* Link sub-file */

                 if ((*KREP = lfi_smartcopy (cnomg, cnoml, do_copy)) != 0)
                   goto error;
              }
            else
              {
                cnoml = strdup (cnomg);
              }

            /* Make path of sub-file relative to path of main file */
            if (istrue (*LDRELATIVE))
              cnoml = lfi_make_relative_path (cnomf, (character *)cnoml);

            fh->fidx[ifh].als    = NULL;
            fh->fidx[ifh].cnomf  = cnoml;
            fh->fidx[ifh].inumer = 0;

            free ((void *)cnomg);
          }

        fh->naidx += fg->naidx;
      }

}


  /* Allocate article index */

  fh->aidx  = (lfi_altm_fh_aidx_t *)malloc (sizeof (lfi_altm_fh_aidx_t) * fh->naidx);

  for (iart = 0, ifi = 0, ifh = 0; ifi < *KNNOMF; ifi++)
    if (lm[ifi].pure)
      {
        lfi_altm_fh_fidx_t * fhr = &fh->fidx[ifh]; /* Should be open at this point */

        if (fhr->als == NULL)
          lfi_abor ("Internal error: expected opened sub-file");

        fhr->als->cb->lfipos (fhr->als->data, KREP, &fhr->inumer);

        for (; ; iart++) 
          {
            fhr->als->cb->lficas (fhr->als->data, KREP, &fhr->inumer, CLNOMA, &ILONG, &IPOSEX, &LLAVAN, CLNOMA_len);
            if ((ILONG == 0) && eqan (CLNOMA, blank_index))
              break;
            memcpy (fh->aidx[iart].name, CLNOMA, ARTNLEN);
            memcpy (fh->aidx[iart].namf, CLNOMA, ARTNLEN);
            fh->aidx[iart].ifh = ifh;
          }

        ifh++;
      }
    else
      {
        int iartg;
        lfi_altm_fh_t * fg = lm[ifi].fh;

        for (iartg = 0; iartg < fg->naidx; iartg++, iart++)
          {
            memcpy (fh->aidx[iart].name, fg->aidx[iartg].name, ARTNLEN);
            memcpy (fh->aidx[iart].namf, fg->aidx[iartg].namf, ARTNLEN);
            fh->aidx[iart].ifh = ifh + fg->aidx[iartg].ifh;
          }

        ifh += fg->nfidx;
      }

  /* Close LFI_ALTM files */


  for (ifi = 0; ifi < *KNNOMF; ifi++)
    if (! lm[ifi].pure)
      lfifer_mult (lm[ifi].fh);

  *KNBARI = fh->naidx;

  *KNBARI -= remove_duplicates (fh);

  fh_modified (fh);

  DRHOOK_END (0);

  return fh;

error:

  return NULL;
}

static void lfiouv_altm (LFIOUV_ARGS_DECL)
{
  ALM_DECL;
  FH_DECL (0);
  lfi_grok_t islfi;
  character CLNOMF[32];

  DRHOOK_START ("lfiouv_altm");


  if (! istrue (*LDNOMM))
    {
      if (*KNUMER <= 0)
        lfi_abor ("LDNOMM=T is not compatible with KNUMER<=0");
      CDNOMF_len = sprintf (CLNOMF, "fort.%lld", *KNUMER);
      CDNOMF = CLNOMF;
    }

  islfi = lfi_grok (CDNOMF, CDNOMF_len);

  if (fh != NULL)
    {
      *KREP = -13;
      goto end;
    }

  islfi = lfi_grok (CDNOMF, CDNOMF_len);

  /* Check Fortran STATUS */
  if ((islfi == LFI_PURE) || (islfi == LFI_ALTM))
    {
      if (lfi_fstrcmp (CDSTTO, "OLD", CDSTTO_len, 3) && 
          lfi_fstrcmp (CDSTTO, "UNKNOWN", CDSTTO_len, 7))
        {
          *KREP = -9;
          goto end;
        }
    }
  else if (islfi == LFI_NONE)
    {
      if (lfi_fstrcmp (CDSTTO, "NEW", CDSTTO_len, 3) && 
          lfi_fstrcmp (CDSTTO, "UNKNOWN", CDSTTO_len, 7))
        {
          *KREP = -9;
          goto end;
        }
    }
  
  /* Initialize descriptor */
  
  if (islfi == LFI_PURE)
    {
      LFI_CSTR (cnomf1, CDNOMF);
      const char * cnomf2;
      character * CLNOMF;
      integer64 INNOMF = 1;
      integer64 CLNOMF_len;
      integer64 cnomf1_len;
      integer64 cnomf2_len;

      /* We have to rename the sub-file to something else, so we create a name and rename the file */

      cnomf2 = getfname (cnomf1);

      cnomf1_len = strlen (cnomf1);
      cnomf2_len = strlen (cnomf2);

      CLNOMF_len = cnomf1_len > cnomf2_len ? cnomf1_len : cnomf2_len;

      CLNOMF = (character *)malloc (2 * CLNOMF_len);
      memset (CLNOMF, ' ', 2 * CLNOMF_len);

      memcpy (CLNOMF,              cnomf1, cnomf1_len);
      memcpy (CLNOMF + CLNOMF_len, cnomf2, cnomf2_len);

      if (rename (cnomf1, cnomf2) < 0)
        lfi_abor ("Cannot rename `%s' in `%s'", cnomf1, cnomf2);

      /* Invoke lfiouv_pure_lfi */

      fh = lfiouv_pure_lfi (LFI, KREP, KNUMER, LDNOMM, CLNOMF, 
                            &INNOMF, CDSTTO, LDERFA, LDIMST, KNIMES, 
                            KNBARP, KNBARI, CLNOMF_len, CDSTTO_len, 0);

      free ((void *)cnomf1);
      free ((void *)cnomf2);
      free (CLNOMF);

    }
  /* Read LFI index */
  else if (islfi == LFI_ALTM)
    {
      fh = lfiouv_mult (LFIOUV_ARGS_LIST);
    }
  /* New file */
  else if (islfi == LFI_NONE)
    {
      fh = fh_new (LFIOUV_ARGS_LIST);

      if (strcmp (fh->cstto, "OLD") == 0)
        lfi_abor ("File does not exist `%s'", fh->cnomf);

      fh->nfidx = 0;
      fh->fidx  = (lfi_altm_fh_fidx_t *)malloc (sizeof (lfi_altm_fh_fidx_t) * fh->nfidx);
      fh->naidx = 0;
      fh->aidx  = (lfi_altm_fh_aidx_t *)malloc (sizeof (lfi_altm_fh_aidx_t) * fh->naidx);

      fh_modified (fh);

      *KNBARI = fh->naidx;
      *KREP = 0;
    }
  else
    {
      LFI_CSTR (cnomf, CDNOMF);
      lfi_abor ("Cannot open file `%s'", cnomf);
    }

  if (fh != NULL)
    {
      fh->next = alm->fh;
      alm->fh = fh;
    }

end:

  DRHOOK_END (0);
}

static void lfifer_altm (LFIFER_ARGS_DECL)
{
  ALM_DECL;
  lfi_altm_fh_t * fh, * fg;
  integer64 ifh;

  DRHOOK_START ("lfifer_altm");

  for (fh = alm->fh, fg = NULL; fh; fg = fh, fh = fh->next)
    if (fh->inumer == *KNUMER)
      break;

  if (fh == alm->fh)
    alm->fh = fh->next;
  else
    fg->next = fh->next;

  /* Save index to file */

  if (fh->modified)
    {
      fh_write_hdr (fh, KREP);
      if (*KREP != 0)
        goto error;
    }

  /* Close files & free all data */

  for (ifh = 0; ifh < fh->nfidx; ifh++)
    {
      lfi_hndl_t * als = fh->fidx[ifh].als;
      if (als != NULL)
        {
          void * LFI = als->data;
          integer64 * KNUMER = &fh->fidx[ifh].inumer;
          fh->fidx[ifh].als->cb->lfifer (LFIFER_ARGS_LIST);
          fh->fidx[ifh].als->destroy (fh->fidx[ifh].als);
        }
      fh->fidx[ifh].als = NULL;
    }

  lfifer_mult (fh);

  *KREP = 0;

  goto done;

error:

  *KREP = -1;

done:

  DRHOOK_END (0);
}

static void lfican_altm (LFICAN_ARGS_DECL)
{
  ALM_DECL;
  FH_DECL (1);
  int iart = seek_rc (fh, +1);
  DRHOOK_START ("lfican_altm");
  
  *KREP = 0;
  memset (CDNOMA, ' ', CDNOMA_len);
      
  if (iart < 0)
    {
      goto end; 
    } 

  memcpy (CDNOMA, fh->aidx[iart].name, minARTN (CDNOMA_len));

  if (CDNOMA_len < lfi_fstrlen (fh->aidx[iart].name, ARTNLEN))
    {
      *KREP = -24;
      goto end;
    }

  if (istrue (*LDAVAN))
    fh->iart = iart;

end:
  DRHOOK_END (0);
}

static void lficas_altm (LFICAS_ARGS_DECL)
{
  ALM_DECL;
  FH_DECL (1);
  int iart = seek_rc (fh, +1);
  DRHOOK_START ("lficas_altm");
  
  *KREP = 0;
  memset (CDNOMA, ' ', CDNOMA_len);
      
  if (iart < 0)
    {
      *KLONG  = 0;
      *KPOSEX = 0;
      goto end; 
    } 

  /* Look for article len & pos in sub-file */
  {
    integer64 ifh = fh->aidx[iart].ifh;
    lfi_altm_fh_fidx_t fhr = getfhr (fh, ifh);
    /* Argument list */
    character CDNOMA[ARTNLEN];
    integer64 * KNUMER = &fh->fidx[ifh].inumer;
    character_len CDNOMA_len = ARTNLEN;
    void * LFI = fhr.als->data;

    memcpy (CDNOMA, fh->aidx[iart].namf, ARTNLEN);
    fhr.als->cb->lfinfo (LFINFO_ARGS_LIST);
    if ((*KREP != 0) || ((*KLONG == 0) && (*KPOSEX == 0)))
      lfi_abor ("Internal error in unit %lld, `%s'", fh->inumer, fh->cnomf);
  }

  memcpy (CDNOMA, fh->aidx[iart].name, minARTN (CDNOMA_len));

  if (CDNOMA_len < lfi_fstrlen (fh->aidx[iart].name, ARTNLEN))
    {
      *KREP = -24;
      goto end;
    }

  if (istrue (*LDAVAN))
    fh->iart = iart;

end:
  DRHOOK_END (0);
}

static void lfipos_altm (LFIPOS_ARGS_DECL)
{
  ALM_DECL;
  FH_DECL (1);
  DRHOOK_START ("lfipos_altm");

  /* Reset article counter */
  fh->iart = -1;

  *KREP = 0; 

  DRHOOK_END (0); 
}

static void lfinfo_altm (LFINFO_ARGS_DECL)
{
  ALM_DECL;
  FH_DECL (1);
  ART_DECL;
  DRHOOK_START ("lfinfo_altm");

  if (iart < 0)
    {
      *KREP   = 0;
      *KLONG  = 0;
      *KPOSEX = 0;
    }
  else
    {
      void * LFI = fhr.als->data;
      character * CDNOMA = fh->aidx[iart].namf;
      KNUMER = &fh->fidx[ifh].inumer;
      fhr.als->cb->lfinfo (LFINFO_ARGS_LIST);

      /* Article was not found; this is an internal error */
      if ((*KREP != 0) || ((*KLONG == 0) && (*KREP == 0)))
        lfi_abor ("Internal error in unit %lld, `%s'", fh->inumer, fh->cnomf);

      fh->iart = iart;

      *KPOSEX = iart;

    }

  DRHOOK_END (0);
}

static void lfinff_altm (LFINFF_ARGS_DECL)
{
  ALM_DECL;
  FH_DECL (1);
  ART_DECL;
  DRHOOK_START ("lfinff_altm");

  if (iart < 0)
    {
      *KREP   = 0;
      CDNOMA[0] = '\0';
      if (CDNOMF_len > 0)
        CDNOMF[0] = '\0';
    }
  else
    {
      *KREP   = 0;
      strncpy (CDNOMA, fh->aidx[iart].namf, CDNOMA_len);
      strncpy (CDNOMF, fh->fidx[ifh].cnomf, CDNOMF_len);
      fh->iart = iart;
    }

  DRHOOK_END (0);
}

static void lfilaf_altm (LFILAF_ARGS_DECL)
{
  ALM_DECL;
  FH_DECL (1);
  int iart;
  integer64 idonn = 1; 
  DRHOOK_START ("lfilaf_altm");

  *KREP = 0; 

  for (iart = 0; iart < fh->naidx; iart++)
    {    
      if (neqan (blank_index, fh->aidx[iart].name))
        {    
          integer64 ILONG, IPOSEX;
          int ifh = fh->aidx[iart].ifh;
          lfi_altm_fh_fidx_t fhr = getfhr (fh, ifh);
          KNUMER = &fhr.inumer;
          
          fhr.als->cb->lfinfo (fhr.als->data, KREP, KNUMER, fh->aidx[iart].namf, &ILONG, &IPOSEX, ARTNLEN);
          if (*KREP != 0)
            goto end;

          printf (
"%7lld-eme article de donnees: \"%16.16s\", %6lld mots, position %8lld a %8lld, \"%16.16s\" dans fichier '%s'\n",
idonn++, fh->aidx[iart].name, ILONG, IPOSEX, IPOSEX + ILONG-1, fh->aidx[iart].namf, fh->fidx[ifh].cnomf);
        }    
    }    

end:

  DRHOOK_END (0); 
}

static void lfinum_altm (LFINUM_ARGS_DECL)
{
  ALM_DECL;
  FH_DECL (0);

  DRHOOK_START ("lfinum_altm");
  
  *KRANG = fh == NULL ? 0 : 1; 

  DRHOOK_END (0); 
}

static void lfilec_altm (LFILEC_ARGS_DECL)
{
  ALM_DECL;
  FH_DECL (1);
  ART_DECL;
  DRHOOK_START ("lfilec_altm");

  if (iart < 0)
    {
      *KREP = iart;
      goto end;
    }
  else
    {
      fhr.als->cb->lfilec (fhr.als->data, KREP, &fh->fidx[ifh].inumer, fh->aidx[iart].namf, KTAB, KLONG, ARTNLEN);
      if (*KREP != 0)
        goto end;

      fh->iart = iart;
    }

end:
  DRHOOK_END (0);
}

static void lfilas_altm (LFILAS_ARGS_DECL)
{
  ALM_DECL;
  FH_DECL (1);
  int iart = seek_rc (fh, +1); 
  DRHOOK_START ("lfilas_altm");
  
  if (iart < 0) 
    {    
      *KREP = -23;
      goto end; 
    }    
  else 
    {    
      integer64 ifh = fh->aidx[iart].ifh;
      lfi_altm_fh_fidx_t fhr = getfhr (fh, ifh);

      fhr.als->cb->lfilec (fhr.als->data, KREP, &fhr.inumer, fh->aidx[iart].namf, KTAB, KLONG, ARTNLEN);
      if (*KREP != 0)
        goto end;

      memset (CDNOMA, ' ', CDNOMA_len);
      memcpy (CDNOMA, fh->aidx[iart].name, minARTN (CDNOMA_len)); 

      if (CDNOMA_len < lfi_fstrlen (fh->aidx[iart].name, ARTNLEN))
        {
          *KREP = -24;
          goto end;
        }

      fh->iart = iart;
    }    

end:
  DRHOOK_END (0); 
}

static void lfilap_altm (LFILAP_ARGS_DECL)
{
  ALM_DECL;
  FH_DECL (1);
  int iart = seek_rc (fh, -1); 
  DRHOOK_START ("lfilap_altm");

  if (iart < 0)
    {
      *KREP = -26;
      goto end;
    }
  else
    {
      integer64 ifh = fh->aidx[iart].ifh;
      lfi_altm_fh_fidx_t fhr = getfhr (fh, ifh);

      fhr.als->cb->lfilec (fhr.als->data, KREP, &fhr.inumer, fh->aidx[iart].namf, KTAB, KLONG, ARTNLEN);
      if (*KREP != 0)
        goto end;

      memset (CDNOMA, ' ', CDNOMA_len);
      memcpy (CDNOMA, fh->aidx[iart].name, minARTN (CDNOMA_len)); 

      if (CDNOMA_len < lfi_fstrlen (fh->aidx[iart].name, ARTNLEN))
        {
          *KREP = -24;
          goto end;
        }

      fh->iart = iart;
    }

end:
  DRHOOK_END (0);
}

static void lficap_altm (LFICAP_ARGS_DECL)
{
  ALM_DECL;
  FH_DECL (1);
  int iart = seek_rc (fh, -1);
  DRHOOK_START ("lficap_altm");
  
  memset (CDNOMA, ' ', CDNOMA_len);
  *KREP = 0;
      
  if (iart < 0)
    {
      *KLONG = 0;
      *KPOSEX = 0;
      goto end; 
    } 

  {
    int ifh = fh->aidx[iart].ifh;
    lfi_altm_fh_fidx_t fhr = getfhr (fh, ifh);
    integer64 * KNUMER = &fhr.inumer;
    character CDNOMA[ARTNLEN];
    character_len CDNOMA_len = ARTNLEN;
    void * LFI = fhr.als->data;

    memcpy (CDNOMA, fh->aidx[iart].namf, minARTN (CDNOMA_len));
    fhr.als->cb->lfinfo (LFINFO_ARGS_LIST);
    if ((*KREP != 0) || ((*KLONG == 0) && (*KPOSEX == 0)))
      lfi_abor ("Internal error in unit %lld, `%s'", fh->inumer, fh->cnomf);
  }

  memcpy (CDNOMA, fh->aidx[iart].name, minARTN (CDNOMA_len));
  if (CDNOMA_len < lfi_fstrlen (fh->aidx[iart].name, ARTNLEN))
    {
      *KREP = -24;
      goto end;
    }

  if (istrue (*LDRECU))
    fh->iart = iart;

end:
  DRHOOK_END (0);
}

static void lfisup_altm (LFISUP_ARGS_DECL)
{ 
  ALM_DECL;
  FH_DECL (1);
  ART_DECL;
  integer64 IPOSEX;
  DRHOOK_START ("lfisup_altm");

  *KREP = 0;

  if (iart < 0)
    {
      *KREP = iart;
      goto end;
    }

  fhr.als->cb->lfinfo (fhr.als->data, KREP, &fh->fidx[ifh].inumer, &fh->aidx[ifh].namf[0], KLONUT, &IPOSEX, ARTNLEN);
  if (*KREP != 0)
    goto end;

  memcpy (fh->aidx[iart].name, blank_index, ARTNLEN);
  memcpy (fh->aidx[iart].namf, blank_index, ARTNLEN);

  fh_modified (fh);

end:
  DRHOOK_END (0);
}

static void lfinaf_altm (LFINAF_ARGS_DECL)
{
  ALM_DECL;
  FH_DECL (1);
  int iart;

  DRHOOK_START ("lfinaf_altm");

  *KNTROU = 0;
  *KNALDO = 0;
  *KNARES = 0; 
  *KNAMAX = 0; 
  *KREP   = 0; 

  for (iart = 0; iart < fh->naidx; iart++)
    if (neqan (blank_index, fh->aidx[iart].name))
      (*KNALDO)++;

  DRHOOK_END (0); 
}

static void lfiren_altm (LFIREN_ARGS_DECL)
{
  ALM_DECL;
  FH_DECL (1);
  int iart1 = lookup_rc (fh, CDNOM1, CDNOM1_len);
  int iart2 = lookup_rc (fh, CDNOM2, CDNOM2_len);
  DRHOOK_START ("lfiren_altm");  

  if (iart1 < 0) 
    {    
      *KREP = iart1;
      goto end; 
    }    

  if (! check_an (CDNOM2, CDNOM2_len))
    {    
      *KREP = -15;
      goto end; 
    }    

  if (iart2 >= 0)
    {
      *KREP = -25;
      goto end;
    }
  
  fh_modified (fh);

  memcpy (fh->aidx[iart1].name, blank_index, ARTNLEN);
  memcpy (fh->aidx[iart1].name, CDNOM2, minARTN (CDNOM2_len));

  *KREP = 0; 

end:
  DRHOOK_END (0); 
}

static void lfiecr_altm (LFIECR_ARGS_DECL)
{
  ALM_DECL;
  FH_DECL (1);
  int iart = lookup_rc (fh, CDNOMA, CDNOMA_len);
  integer64 ILONG, IPOSEX;
  lfi_altm_fh_fidx_t fhw;

  DRHOOK_START ("lfiecr_altm");

  if (! check_an (CDNOMA, CDNOMA_len))
    {
      *KREP = -15;
      goto end;
    }

  if (iart < 0)
    {
      int naidx;

      for (iart = 0; iart < fh->naidx; iart++)
        if (eqan (blank_index, fh->aidx[iart].name))
          goto found;

      /* Grow article index */

      naidx = 2 * fh->naidx + 1;

      fh->aidx = (lfi_altm_fh_aidx_t *)realloc (fh->aidx, naidx * sizeof (lfi_altm_fh_aidx_t));
      for (iart = fh->naidx; iart < naidx; iart++)
        {
          memcpy (fh->aidx[iart].name, blank_index, ARTNLEN);
          memcpy (fh->aidx[iart].namf, blank_index, ARTNLEN); 
          fh->aidx[iart].ifh = -1;
        }

      iart = fh->naidx;

      fh->naidx = naidx;
    }

found:

  fhw = getfhw (fh, 0); 

  fhw.als->cb->lfinfo (fhw.als->data, KREP, &fhw.inumer, CDNOMA, &ILONG, &IPOSEX, CDNOMA_len);

  /* 
   * Article already exists in LFI file;
   * see if it was renamed in main index
   */
  if ((ILONG != 0) || (IPOSEX != 0))
    {
      character CLNOMA[ARTNLEN];
      memcpy (CLNOMA, blank_index, ARTNLEN);
      memcpy (CLNOMA, CDNOMA, minARTN (CDNOMA_len));
      if (! (eqan (CLNOMA, fh->aidx[iart].name) && eqan (CLNOMA, fh->aidx[iart].namf)))
        fhw = getfhw (fh, 1);
    }
  
  {
    integer64 INALDO, INTROU, INARES, INAMAX;
    fhw.als->cb->lfinaf (fhw.als->data, KREP, &fhw.inumer, &INALDO, &INTROU, &INARES, &INAMAX);
    /* 
     * Many articles were written to this file;
     * switch to a new one
     */
    if ((INALDO + INTROU) > alm->maxartals)
      fhw = getfhw (fh, 1);
  }

  /* Update article index */

  memcpy (fh->aidx[iart].name, blank_index, ARTNLEN);
  memcpy (fh->aidx[iart].name, CDNOMA, minARTN (CDNOMA_len));
  memcpy (fh->aidx[iart].namf, blank_index, ARTNLEN);
  memcpy (fh->aidx[iart].namf, CDNOMA, minARTN (CDNOMA_len));

  fh->aidx[iart].ifh = fh->ifh_w;

  /* Write article */

  {
    void * LFI = fhw.als->data;
    fhw.als->cb->lfiecr (LFIECR_ARGS_LIST);
    if (*KREP != 0)
      goto end;
  }

  fh_modified (fh);

  fh->iart = iart;

end:

  DRHOOK_END (0);
}

static void lfiopt_altm (LFIOPT_ARGS_DECL)
{
  ALM_DECL;
  FH_DECL (0);
  DRHOOK_START ("lfiopt_altm");

  if (fh == NULL)
    {    
      *KREP = -1;
      goto end; 
    }    

  memset (CDNOMF, ' ', CDNOMF_len);
  memset (CDSTTO, ' ', CDSTTO_len);

  memcpy (CDNOMF, fh->cnomf, CDNOMF_len > strlen (fh->cnomf) ? strlen (fh->cnomf) : CDNOMF_len);
  memcpy (CDSTTO, fh->cstto, CDSTTO_len > strlen (fh->cstto) ? strlen (fh->cstto) : CDSTTO_len);

  *LDNOMM = fort_TRUE;
  *LDIMST = fort_FALSE;
  *LDERFA = fh->llerfa;
  *KNIMES = fh->inimes;
  *KREP = 0; 

end:
  DRHOOK_END (0); 

}

static void lfinim_altm (LFINIM_ARGS_DECL)
{
  ALM_DECL;
  FH_DECL (1);
 
  DRHOOK_START ("lfinaf_altm");

  *KREP = 0;

  if ((*KNIMES > 2) || (*KNIMES < 0))
    {
      *KREP = -2;
     goto end;
    }

  fh->inimes = *KNIMES;

end:

  DRHOOK_END (0);
}

static void lfierf_altm (LFIERF_ARGS_DECL)
{
  ALM_DECL;
  FH_DECL (1);

  DRHOOK_START ("lfierf_altm");

  fh->llerfa = *LDERFA;
  *KREP = 0;

  DRHOOK_END (0);
}

static void lfioef_altm (LFIOEF_ARGS_DECL)
{
  ALM_DECL;
  FH_DECL (1);

  DRHOOK_START ("lfioef_altm");

  *LDERFA = fh->llerfa;
  *KREP = 0;

  DRHOOK_END (0);
}

static void lfifmd_altm (LFIFMD_ARGS_DECL)
{
  ALM_DECL;
  DRHOOK_START ("lfifmd_altm");

  if (*KFACMD > 0)
    alm->fmult = *KFACMD;

  DRHOOK_END (0);
}

static void lfiofd_altm (LFIOFD_ARGS_DECL)
{
  ALM_DECL;

  DRHOOK_START ("lfiofd_altm"); 

  *KFACMD = alm->fmult;

  DRHOOK_END (0);
}

static void lfiofm_altm (LFIOFM_ARGS_DECL)
{
  ALM_DECL;
  FH_DECL (0);
  DRHOOK_START ("lfiofm_altm");

  *KREP = 0;
  if (fh) 
    {
      *KFACTM = fh->fmult;
      *LDOUVR = fort_TRUE;
    }
  else
    {
      *KFACTM = alm->fmult;
      lfi_fmul_get (alm->fmult_list, KNUMER, KFACTM);
      *LDOUVR = fort_FALSE;
    }

  DRHOOK_END (0);
}

static void lfiafm_altm (LFIAFM_ARGS_DECL)
{
  ALM_DECL;
  FH_DECL (0);
  DRHOOK_START ("lfiafm_altm");

  *KREP = 0;
  if (fh) 
    {
      *KREP = -5;
    }
  else if (*KFACTM < 0)
    {
      *KREP = -14;
    }
  else
    {
      lfi_fmul_set (&alm->fmult_list, KNUMER, KFACTM);
    }

  DRHOOK_END (0);
}

static void lfisfm_altm (LFISFM_ARGS_DECL)
{
  
  ALM_DECL;
  FH_DECL (0);
  DRHOOK_START ("lfisfm_altm");

  *KREP = 0;
  if (fh) 
    {
      *KREP = -5;
    }
  else
    {
      if (lfi_fmul_del (&alm->fmult_list, KNUMER) < 0)
        *KREP = -31;
    }

  DRHOOK_END (0);
}

static void lfineg_altm (LFINEG_ARGS_DECL)
{
  ALM_DECL;
  DRHOOK_START ("lfineg_altm");
  if ((*KNIVAU >= 0) && (*KNIVAU <= 2))
    alm->nerfag = *KNIVAU;
  DRHOOK_END (0);
}

static void lfioeg_altm (LFIOEG_ARGS_DECL)
{
  ALM_DECL;
  DRHOOK_START ("lfioeg_altm");
  *KNIVAU = alm->nerfag;
  DRHOOK_END (0);
}

static void lfiomg_altm (LFIOMG_ARGS_DECL)
{
  ALM_DECL;
  DRHOOK_START ("lfiomg_altm");
  *KNIVAU = alm->inivau;
  *KULOUT = alm->iulout;
  DRHOOK_END (0);
}

static void lfinmg_altm (LFINMG_ARGS_DECL)
{
  ALM_DECL;
  DRHOOK_START ("lfinmg_altm");
  alm->inivau = *KNIVAU;
  alm->iulout = *KULOUT;
  DRHOOK_END (0);
}

#undef ALM_DECL

lficb_t lficb_altm = {
  lfiouv_altm,        /*        Ouverture fichier                                        */
  lfican_altm,        /* KNUMER Nom de l'article suivant                                 */
  lficas_altm,        /* KNUMER Caracteristiques de l'article suivant                    */
  lfiecr_altm,        /* KNUMER Ecriture                                                 */
  lfifer_altm,        /* KNUMER Fermeture                                                */
  lfilec_altm,        /* KNUMER Lecture                                                  */
  lfinfo_altm,        /* KNUMER Caracteristiques d'un article nomme                      */
  lfinff_altm,        /* KNUMER Get real file & record name                              */
  lfipos_altm,        /* KNUMER Remise a zero du pointeur de fichier                     */
  lfiver_dumm,        /* KNUMER Verrouillage d'un fichier                                */
  lfiofm_altm,        /* KNUMER Obtention du facteur multiplicatif                       */
  lfineg_altm,        /*        Niveau global d'erreur                                   */
  lfilaf_altm,        /* KNUMER Liste des articles                                       */
  lfiosg_dumm,        /*        Obtention du niveau d'impression des statistiques        */
  lfinum_altm,        /* KNUMER Rang de l'unite logique KNUMER                           */
  lfisup_altm,        /* KNUMER Suppression d'un article                                 */
  lfiopt_altm,        /* KNUMER Obtention des options d'ouverture d'un fichier           */
  lfinmg_altm,        /*        Niveau global d'erreur                                   */
  lficap_altm,        /* KNUMER Caracteristiques de l'article precedent                  */
  lfifra_dumm,        /*        Messages en Francais                                     */
  lficfg_dumm,        /*        Impression des parametres de base de LFI                 */
  lfierf_altm,        /* KNUMER Erreur fatale                                            */
  lfilas_altm,        /* KNUMER Lecture de l'article de donnees suivant                  */
  lfiren_altm,        /* KNUMER Renommer un article                                      */
  lfiini_dumm,        /*        Initialisation de LFI                                    */
  lfipxf_miss,        /* KNUMER Export d'un fichier LFI                                  */
  lfioeg_altm,        /*        Obtention du niveau global de traitement des erreurs     */
  lfinaf_altm,        /* KNUMER Nombre d'articles divers                                 */
  lfiofd_altm,        /*        Facteur multiplicatif courant                            */
  lfiomf_dumm,        /* KNUMER Obtention du niveau de messagerie                        */
  lfiafm_altm,        /* KNUMER Attribution d'un facteur multiplicatif a une unite       */
  lfista_dumm,        /* KNUMER Impression des statistiques d'utilisation                */
  lfiosf_miss,        /* KNUMER Obtention de l'option d'impression des statistiques      */
  lfilap_altm,        /* KNUMER Lecture de l'article precedent                           */
  lfioef_altm,        /* KNUMER Obtention de l'option courante de traitement des erreurs */
  lfimst_dumm,        /* KNUMER Activation de l'option d'impression de statistiques      */
  lfinim_altm,        /* KNUMER Ajustement du niveau de messagerie                       */
  lfisfm_altm,        /* KNUMER Suppression d'un facteur multiplicatif                   */
  lfinsg_dumm,        /*        Niveau global d'impression de statistiques               */
  lfideb_dumm,        /*        Mode mise au point (debug)                               */
  lfiomg_altm,        /*        Obtention du niveau global des messages LFI              */
  lfifmd_altm,        /*        Facteur multiplicatif par defaut                         */
};

#define ALM_DECL \
  lfi_altm_t * alm = lookup_alm (lfi->data);

static void lfi_del_altm_hndl (lfi_hndl_t * lfi)
{
  ALM_DECL;

  if (alm->fh)
    lfi_abor ("Attempt to release lfi handler with opened files");

  free (alm);
  free (lfi);
}

static int lfi_opn_altm_hndl (lfi_hndl_t * lfi, integer64 * KNUMER)
{
  ALM_DECL;
  FH_DECL (0);
  return fh == NULL ? 0 : 1;
}

static int lfi_vrb_altm_hndl (lfi_hndl_t * lfi, integer64 * KNUMER)
{
  ALM_DECL;
  FH_DECL (1);
  return fh->inimes == 2 ? 1 : 0;
}

static int lfi_fat_altm_hndl (lfi_hndl_t * lfi, integer64 * KNUMER)
{
  ALM_DECL;
  FH_DECL (1);
  return (alm->nerfag == 0) || ((alm->nerfag == 1) && istrue (fh->llerfa));
}

#undef ALM_DECL

/* Create the LFI handler */

lfi_hndl_t * lfi_get_altm_hndl (void * data)
{
  lfi_hndl_t * lfi = (lfi_hndl_t *)malloc (sizeof (lfi_hndl_t));
  lfi_altm_t * alm = (lfi_altm_t *)malloc (sizeof (lfi_altm_t));

  memset (alm, 0, sizeof (lfi_altm_t));
  memcpy (alm->cmagic, "lfi_altm", 8); 

  alm->fmult      = 6;
  alm->fmult_list = NULL;
  alm->maxartals  = 3000;
  alm->nerfag     = 1;
  alm->inivau     = 0;
  alm->iulout     = 0;

  lfi->cb      = &lficb_altm;
  lfi->cb_verb = &lficb_verb;
  lfi->data    = alm;
  lfi->destroy = lfi_del_altm_hndl;
  lfi->is_open = lfi_opn_altm_hndl;
  lfi->is_verb = lfi_vrb_altm_hndl;
  lfi->is_fatl = lfi_fat_altm_hndl;
  lfi->next    = NULL;

  return lfi;
}

void lfi_altm_merge_ (integer64 * KREP, character * CDNOMF, integer64 * KNNOMF, logical * LDRELATIVE, character_len CDNOMF_len)
{
  lfi_hndl_t * lfi = lfi_get_altm_hndl (NULL);
  lfi_altm_t * alm = lfi->data;
  integer64 INUMER = 1;

  *KREP = 0;

  /* Open file */
  {
    integer64 INIMES = 2, INBARP = 0, INBARI = 0;
    logical LLNOMM = fort_TRUE, LLERFA = fort_TRUE, LLIMST = fort_TRUE;
    character * CLSTTO = (character *)"OLD";
    character_len CLSTTO_len = 3;
    
    alm->fh = lfiouv_mixed_lfi (
                                 alm, KREP, &INUMER, &LLNOMM, CDNOMF, 
                                 KNNOMF, CLSTTO, &LLERFA, &LLIMST, &INIMES, 
                                 &INBARP, &INBARI, LDRELATIVE, CDNOMF_len, CLSTTO_len, 
                                 1, 1, 1, 1 /* Fast, link, copy, unlink */ 
                               );

    if (*KREP != 0)
      goto end;

  }

  /* Close file */
  {
    character * CLSTTO = (character *)"KEEP";
    character_len CLSTTO_len = 4;
    lfi->cb->lfifer (alm, KREP, &INUMER, CLSTTO, CLSTTO_len);

    if (*KREP != 0)
      goto end;

  }

  lfi_del_altm_hndl (lfi);

end:

  if (*KREP != 0)
    {
      lfi_verb (NULL, "lfi_altm_merge", "KREP", KREP, NULL);
      lfi_abor ("lfi_altm_merge");
    }

  return;
}

/* Create an index from several native LFI files */

void lfi_altm_index_ (integer64 * KREP, character * CDNOMF, integer64 * KNNOMF, logical * LDRELATIVE, character_len CDNOMF_len)
{
  lfi_hndl_t * lfi = lfi_get_altm_hndl (NULL);
  lfi_altm_t * alm = lfi->data;
  integer64 INUMER = 1;

  *KREP = 0;

  /* Open file */
  {
    integer64 INIMES = 2, INBARP = 0, INBARI = 0;
    logical LLNOMM = fort_TRUE, LLERFA = fort_TRUE, LLIMST = fort_TRUE;
    character * CLSTTO = (character *)"OLD";
    character_len CLSTTO_len = 3;
    
    alm->fh = lfiouv_mixed_lfi (
                                  alm, KREP, &INUMER, &LLNOMM, CDNOMF, 
                                  KNNOMF, CLSTTO, &LLERFA, &LLIMST, &INIMES, 
                                  &INBARP, &INBARI, LDRELATIVE, CDNOMF_len, CLSTTO_len, 
                                  1, 0, 0, 0 /* Fast, no link, no copy, no unlink */
                                );

    if (*KREP != 0)
      goto end;

  }

  /* Close file */
  {
    character * CLSTTO = (character *)"KEEP";
    character_len CLSTTO_len = 4;
    lfi->cb->lfifer (alm, KREP, &INUMER, CLSTTO, CLSTTO_len);

    if (*KREP != 0)
      goto end;

  }

  lfi_del_altm_hndl (lfi);

end:

  if (*KREP != 0)
    {
      lfi_verb (NULL, "lfi_altm_index", "KREP", KREP, NULL);
      lfi_abor ("lfi_altm_index");
    }

  return;
}



/*
 * Copy a lfi_altm file using hard links
 */
void lfi_altm_copy_ (integer64 * KREP, character * CDNOMF1, character * CDNOMF2, logical * LDRELATIVE,
                     character_len CDNOMF1_len, character_len CDNOMF2_len)
{
  lfi_grok_t islfi = lfi_grok (CDNOMF1, CDNOMF1_len);
  lfi_hndl_t * lfi = lfi_get_altm_hndl (NULL);
  lfi_altm_t * alm = lfi->data;

  *KREP = 0;

  if (islfi == LFI_PURE)
    {
      LFI_CSTR (cnomf2, CDNOMF2);
      /* Link or copy source file */
      if ((*KREP = lfi_fsmartcopy (CDNOMF1, CDNOMF2, 1, CDNOMF1_len, CDNOMF2_len)) != 0)
        goto clean_alts;
      goto end;
clean_alts:
      {
         int errno_save = errno;
         unlink (cnomf2);
         errno = errno_save;
         goto end;
      }
    }
  else if (islfi == LFI_ALTM)
    {
      integer64 INUMER = 0;
      lfi_altm_fh_t * fh;
      int ifh;

      /* Open source file */
      {
        integer64 INIMES = 2, INBARP = 0, INBARI = 0;
        logical LLNOMM = fort_TRUE, LLERFA = fort_TRUE, LLIMST = fort_TRUE;
        character * CLSTTO = "OLD";
        character_len CLSTTO_len = 3;

        lfi->cb->lfiouv (alm, KREP, &INUMER, &LLNOMM, CDNOMF1, CLSTTO, &LLERFA, &LLIMST, 
                         &INIMES, &INBARP, &INBARI, CDNOMF1_len, CLSTTO_len);

        if (*KREP != 0)
          goto end;

      }

      /* Get filehandle */
      fh = alm->fh;

      {
        LFI_CSTR (cnomf2, CDNOMF2);
        int j;

        /* Link sub-files */

        for (ifh = 0; ifh < fh->nfidx; ifh++)
          {
            const char * cnoml1 = resolve_filename (fh->cnomf, fh->fidx[ifh].cnomf, 
                                                    strlen (fh->fidx[ifh].cnomf),
                                                    NULL); /* Source file */
            const char * cnoml2;                           /* Target file */

            cnoml2 = getfname (cnomf2);

            /* Link sub-file */

            *KREP = lfi_smartcopy (cnoml1, cnoml2, 1);

            free ((void *)cnoml1);

            /* Update sub-file name */
            free ((void *)fh->fidx[ifh].cnomf);

            /* Make path of sub-file relative to path of main file */
            if (istrue (*LDRELATIVE))
              cnoml2 = lfi_make_relative_path (cnomf2, (character *)cnoml2);

            fh->fidx[ifh].cnomf = cnoml2;

            if (*KREP != 0)
              goto clean_altm;

          }

      }

      /* Replace source file name with destination name */
      fh_set_filename (fh, CDNOMF2, CDNOMF2_len);

      /* Write headr */
      fh_write_hdr (fh, KREP);

      /* Close */
      {
        character * CLSTTC = "KEEP";
        character_len CLSTTC_len = 4;

        lfi->cb->lfifer (alm, KREP, &INUMER, CLSTTC, CLSTTC_len);

        if (*KREP != 0)
          goto clean_altm;


        goto end;

clean_altm:
        {
          int errno_save = errno;
          int ifh1;
          for (ifh1 = 0; ifh1 < ifh; ifh1++)
            {
              const char *  dir;
              dir = lfi_dirname (fh->fidx[ifh1].cnomf);
              unlink (fh->fidx[ifh1].cnomf);
              rmdir (dir);
              free ((void *)dir);
            }
          errno = errno_save;
          goto end;
        }

      }

    }
  else if (islfi == LFI_NONE)
    {
      errno = ENOENT;
      *KREP = 1;
      goto end;
    }
  else if (islfi == LFI_UNKN)
    {
      *KREP = -10;
     goto end;
    }
  else
    {
      lfi_abor ("Unknown return code from lfi_grok");
    }

  lfi_del_altm_hndl (lfi);

end:

  if (*KREP != 0)
    {
      lfi_verb (NULL, "lfi_altm_copy", "KREP", KREP, NULL);
      lfi_abor ("lfi_altm_copy");
    }

  return;
}

/*
 * Remove a lfi_altm file 
 */
void lfi_altm_remove_ (integer64 * KREP, character * CDNOMF, logical * LDFORCE, character_len CDNOMF_len)
{
  lfi_grok_t islfi = lfi_grok (CDNOMF, CDNOMF_len);
  lfi_hndl_t * lfi = lfi_get_altm_hndl (NULL);
  lfi_altm_t * alm = lfi->data;

  *KREP = 0;

  if (islfi == LFI_PURE)
    {
      LFI_CSTR (cnomf, CDNOMF);
      errno = 0;
      if (unlink (cnomf))
        {
          *KREP = errno;
          goto end;
        }
    }
  else if (islfi == LFI_ALTM)
    {
      integer64 INUMER = 0;
      lfi_altm_fh_t * fh;

      /* Open source file */
      {
        integer64 INIMES = 2, INBARP = 0, INBARI = 0;
        logical LLNOMM = fort_TRUE, LLERFA = fort_TRUE, LLIMST = fort_TRUE;
        character * CLSTTO = "OLD";
        character_len CLSTTO_len = 3;

        lfi->cb->lfiouv (alm, KREP, &INUMER, &LLNOMM, CDNOMF, CLSTTO, &LLERFA, &LLIMST, 
                         &INIMES, &INBARP, &INBARI, CDNOMF_len, CLSTTO_len);

        if (*KREP != 0)
          goto end;

      }

      /* Get filehandle */
      fh = alm->fh;

      {
        /* Unlink sub-files */
        int ifh;

        for (ifh = 0; ifh < fh->nfidx; ifh++)
          {
            const char * cnoml = resolve_filename (fh->cnomf, fh->fidx[ifh].cnomf, 
                                                   strlen (fh->fidx[ifh].cnomf), NULL); /* Source file */
            const char * dirnl = lfi_dirname (cnoml);


            errno = 0;
            if (unlink (cnoml) != 0)
              {
                if (istrue (*LDFORCE))
                  {
                    errno = 0;
                  }
                else
                  {
                    *KREP = errno;
                    goto end;
                  }
              }

            lfi_rmdir (dirnl); 

            free ((void *)cnoml);
            free ((void *)dirnl);
          }

      }

      /* Close */
      {
        character * CLSTTC = "KEEP";
        character_len CLSTTC_len = 4;

        lfi->cb->lfifer (alm, KREP, &INUMER, CLSTTC, CLSTTC_len);

        if (*KREP != 0)
          goto end;
      }

      /* Unlink main file */
      {
        LFI_CSTR (cnomf, CDNOMF)
        errno = 0;
        if (unlink (cnomf))
          {
            *KREP = errno;
            goto end;
          }
      }

    }
  else if (islfi == LFI_NONE)
    {
      errno = ENOENT;
      *KREP = 1;
      goto end;
    }
  else if (islfi == LFI_UNKN)
    {
      *KREP = -10;
     goto end;
    }
  else
    {
      lfi_abor ("Unknown return code from lfi_grok");
    }

  lfi_del_altm_hndl (lfi);

end:

  if (*KREP != 0)
    {
      lfi_verb (NULL, "lfi_altm_remove", "KREP", KREP, "CDNOMF", CDNOMF, CDNOMF_len, NULL);
      lfi_abor ("lfi_altm_remove");
    }

  return;
}


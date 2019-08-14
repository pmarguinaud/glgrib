/**** *lfi_alts.c* - LFI library
 *
 *    Author. 
 *    ------- 
 *     Philippe Marguinaud *METEO-FRANCE*
 *     Original : 12-08-2013
 *     Modified : 08-12-2014 Portability on Mac OSX
 *
 * Description :
 * This LFI library can handle native LFI files
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#if defined(DARWIN)
#include <limits.h>
#define MAXINT INT_MAX
#define MAXLONG LONG_MAX
#elif defined(MACOSX)
#include <limits.h>
#define MAXLONG LONG_MAX
#else
#include <values.h>
#endif

#include "lfi_type.h"
#include "lfi_alts.h"
#include "lfi_dumm.h"
#include "lfi_miss.h"
#include "lfi_abor.h"
#include "lfi_misc.h"
#include "lfi_verb.h"
#include "lfi_fmul.h"
#include "lfi_altm.h"
#include "lfi_util.h"
#include "lfi_grok.h"

#include "drhook.h"

/* Ancillary macros */

#define ARTNLEN 16
#define minARTN(x) ((x) > ARTNLEN ? ARTNLEN : (x))

#define eqan(a,b) (strncmp ((a), (b), ARTNLEN) == 0)
#define neqan(a,b) (!eqan(a,b))


/* Reserved article names */
static const char 
  * fin_d_index = "**FIN D'INDEX** ", /* End of article index                 */
  * trou__index = "                ", /* Hole in the index                    */
  * page__index = "****************"  /* Placeholder for extra index sections */
 ;

/* File header */
typedef struct lfi_alts_fh_hdr_t
{
  integer64  size;         /* Size in bytes of a physical article                 */
  integer64  ilpar;        /* Taille d'un article physique en mots de 8 octets    */
  integer64  ilmna;        /* Taille maximale des noms d'articles                 */
  integer64  ifeam;        /* Flag de fermeture correcte de fichier               */
  integer64  illdo;        /* Taille de l'entete                                  */
  integer64  inaph;        /* Nombre d'articles physiques                         */
  integer64  inalo;        /* Nombre d'articles logiques                          */
  integer64  ilnal;        /* Taille minimale d'un article logique                */
  integer64  ilxal;        /* Taille maximale d'un article logique                */
  integer64  iltal;        /* Taille totale des donnees                           */
  integer64  inres;        /* Nombre de reecritures avec la meme taille           */
  integer64  inrec;        /* Nombre de reecritures avec des donnees plus courtes */
  integer64  inrel;        /* Nombre de reecritures avec des donnees plus longues */
  integer64  ixapi;        /*                                                     */
  integer64  idcre;        /* Date de creation                                    */
  integer64  ihcre;        /* Heure de creation                                   */
  integer64  iddmg;        /* Date de derniere modification                       */
  integer64  ihdmg;        /* Heure de derniere modification                      */
  integer64  idmng;        /* Date de premiere modification                       */
  integer64  ihmng;        /* Heure de premiere modification                      */
  integer64  inpir;        /* ?                                                   */
  integer64  intru;        /* Nombre de trous                                     */
  integer64  iaxpd;        /* Nombre maximum d'articles physiques                 */
  integer64  ioffi[1];     /* Extra index position                                */
} 
lfi_alts_fh_hdr_t;

/* Article descriptor */
typedef struct lfi_alts_fh_idx_t
{
  integer64 len;           /* Article length; length is reckoned in 8b words          */
  integer64 pos;           /* Article position in the LFI file; position is reckoned  */
                           /* in 8b words, starting from one                          */
  character name[ARTNLEN]; /* Article name                                            */
} lfi_alts_fh_idx_t;

typedef enum lfi_alts_fh_kind_t
{
  FH_KIND_REGULAR = 0,
  FH_KIND_STREAM  = 1,
  FH_KIND_RECORD  = 2
} lfi_alts_fh_kind_t;

/* Open file descriptor */
typedef struct lfi_alts_fh_t
{
  lfi_alts_fh_kind_t kind;
  FILE * fp;                         /* stdio file descriptor for this file */
  int fd;                            /* file descriptor                     */
  long offset;                       /* current offset                      */
  char * buf;
  integer64 inumer;                  /* Unit number                         */
  char * cnomf;                      /* File name                           */
  char * cstto;                      /* Fortran status                      */
  integer64 inimes;                  /* Message level                       */
  logical llerfa;                    /* All errors are fatal                */
  int lswap;                         /* File requires byte-swap             */
  integer64 fmult;                   /* Facteur multiplicatif               */
  integer64 nart;                    /* Maximum number of articles          */
  integer64 iart;                    /* Current article                     */
  int modified;                      /* File was modified                   */
  lfi_alts_fh_hdr_t * hdr;           /* File header                         */
  lfi_alts_fh_idx_t * idx;           /* File article index                  */
  struct lfi_alts_fh_t * next;       /* Next file                           */
} 
lfi_alts_fh_t;

/* LFI library data */
typedef struct lfi_alts_t
{
  char cmagic[8];
  lfi_alts_fh_t * fh;      /* File descriptor list                              */
  int fmult;               /* Default for the "facteur multiplicatif"           */
  lfi_fmul_t * fmult_list; /* Predefined units and "facteurs multiplicatifs"    */
  int nerfag;              /* Erreurs fatales                                   */
  int inivau;              /* Niveau global des messages                        */
  int iulout;              /* Unite Fortran pour impression des messages        */
}
lfi_alts_t;


static const int use_fd = 0;
static const int buf_sz = 0;

/* Open file; try various modes depending on Fortran status */
static void fh_open (lfi_alts_fh_t * fh, char * cnomf, char * cstto, lfi_alts_fh_kind_t kind, integer64 * KREP)
{
  FILE * fp = NULL;  

  errno = 0;

  *KREP = 0;

  if (kind == FH_KIND_RECORD)
    {
      fp = NULL;
    }
  else if (kind == FH_KIND_STREAM)
    {  
      if ((cnomf[0] == '-') && (cnomf[1] == '\0'))
        fp = stdout;
      else
        fp = fopen (cnomf, "w");
    }
  else if (strcmp (cstto, "NEW") == 0)
    {
      fp = fopen (cnomf, "r");
      if (fp != NULL)
        {
          fclose (fp);
          fp = NULL;
          goto end;
        }
      fp = fopen (cnomf, "w+");
    }
  else if (strcmp (cstto, "OLD") == 0)
    {
      fp = fopen (cnomf, "r+");
      if (fp == NULL)
        fp = fopen (cnomf, "r");
    }
  else
    {
      fp = fopen (cnomf, "r+");
      if (fp == NULL)
        fp = fopen (cnomf, "w+");
      if (fp == NULL)
        fp = fopen (cnomf, "r");
    }


end:

  fh->fp = fp;
  fh->kind = kind;

  if (kind == FH_KIND_RECORD)
    {
      fh->fd = -1;
    }
  else if (fp == NULL)
    {
      *KREP = -9;
      fh->fd = -1;
    }
  else
    {
      fh->fd = fileno (fp);
      fh->buf = NULL;
      if (buf_sz > 0)
        {
          fh->buf = (char *)malloc (buf_sz);
          setvbuf (fh->fp, fh->buf, _IOFBF, buf_sz);
        }
    }

  return;
}

static void fh_close (lfi_alts_fh_t * fh, integer64 * KREP)
{
  if (fh->kind != FH_KIND_RECORD)
    {
      errno = 0;
      *KREP = fclose (fh->fp);
      if (fh->buf != NULL)
        free ((void *)fh->buf);
      fh->fp = NULL;
    }
}

static integer64 fh_seek (lfi_alts_fh_t * fh, integer64 pos, int whence)
{
  if (fh->kind != FH_KIND_REGULAR)
    lfi_abor ("Cannot fseek unit %lld, `%s'", fh->inumer, fh->cnomf);

  if (use_fd)
    {    
      fh->offset = lseek (fh->fd, pos, whence);
    }    
  else 
    {    
      if (fseek (fh->fp, pos, whence) == 0)
        fh->offset = ftell (fh->fp);
      else 
        return -1;
    }    
  return fh->offset;
}

static void fh_flush (lfi_alts_fh_t * fh)
{
  if (fh->kind == FH_KIND_RECORD)
    return;
  if (! use_fd)
    fflush (fh->fp);
}

static void fh_fstat (lfi_alts_fh_t * fh, struct stat * st)
{
  if (fh->kind == FH_KIND_RECORD)
    memset (st, 0, sizeof (struct stat));
  fstat (fh->fd, st);
}

/* Read a chunk of data; pos and len are in bytes, starting from zero */
static void fh_read (lfi_alts_fh_t * fh, void * ptr, integer64 pos, integer64 len, int lswap)
{
  integer64 lenr = len;
  char * _ptr = (char *)ptr;

  if (fh->kind != FH_KIND_REGULAR)
    lfi_abor ("Cannot fseek unit %lld, `%s'", fh->inumer, fh->cnomf);

  errno = 0;

  if (fh->offset != pos)
    if (fh_seek (fh, pos, SEEK_SET) < 0)
      lfi_abor ("Cannot fseek unit %lld, `%s'", fh->inumer, fh->cnomf);
  
  while (lenr > 0)
    {
      integer64 nr = use_fd
                   ? read (fh->fd, &_ptr[len-lenr], lenr)
                   : fread (&_ptr[len-lenr], 1, lenr, fh->fp);
      if (nr == 0)
        break;
      lenr -= nr;
    }

  if (lenr > 0)
    lfi_abor ("Reading %lld bytes at offset %lld, from unit %lld failed",
              len, pos, fh->inumer);

  fh->offset += len;

  if (fh->lswap && lswap)
    {
      int t = 8, n = len / 8;
      jswap_ (ptr, ptr, &t, &n);
    }
}

/* Write a chunk of data; pos and len are in bytes, starting from zero */
static void fh_write (lfi_alts_fh_t * fh, void * ptr, integer64 pos, integer64 len, int lswap)
{
  char * _ptr;
  integer64 lenw = len;

  if (fh->kind == FH_KIND_RECORD)
    {
      if (fh->offset != pos)
        lfi_abor ("Cannot fseek unit %lld, `%s'", fh->inumer, fh->cnomf);
      fh->offset += len;
      return;
    }

  errno = 0;

  if (fh->lswap && lswap)
    {
      int t = 8, n = len / 8;
      _ptr = (char *)malloc (len);
      jswap_ (_ptr, ptr, &t, &n);
    }
  else
    {
      _ptr = (char *)ptr;
    }
  
  if (fh->offset != pos)
    if (fh_seek (fh, pos, SEEK_SET) < 0)
      lfi_abor ("Cannot fseek unit %lld, `%s'", fh->inumer, fh->cnomf);

  while (lenw > 0)
    {
      integer64 nw = use_fd
                   ? write (fh->fd, &_ptr[len-lenw], lenw)
                   : fwrite (&_ptr[len-lenw], 1, lenw, fh->fp);
      if (nw == 0)
        break;
      lenw -= nw;
    }

  if (lenw > 0)
    lfi_abor ("Writing %lld bytes at offset %lld, in unit %lld failed",
              len, pos, fh->inumer);

  fh->offset += len;

  if (fh->lswap && lswap)
    free ((void *)_ptr);
}

/* Retrieve current date */
static void xgetdate (integer64 * YYYYMMDD, integer64 * HHmmss)
{
  time_t T = time (NULL);
  struct tm TM;

  gmtime_r (&T, &TM);

  *YYYYMMDD = (1900 + TM.tm_year) * 10000 + (1 + TM.tm_mon) * 100 + TM.tm_mday;
  *HHmmss   = TM.tm_hour * 10000 + TM.tm_min * 100 + TM.tm_sec;
  
}

/* Write file header; a side effect is that some statistics are updated.
 * if kind is true, then it is assumed that these statistics are correct,
 * and the file header is written without doing any fseeks */
static void fh_write_hdr (lfi_alts_fh_t * fh, integer64 * KREP)
{
  int isize = fh->hdr->size;
  int ioffib = isize / 8 - fh->hdr->illdo;
  character csect2[isize];
  integer64 isect3[isize/8];
  int iart, iartx;
  int iidx;
  
  *KREP = 0;

  integer64 YYYYMMDD, HHmmss;
  xgetdate (&YYYYMMDD, &HHmmss);
  
  fh->hdr->iddmg = YYYYMMDD;
  fh->hdr->ihdmg =   HHmmss;

  if (fh->kind == FH_KIND_STREAM || fh->kind == FH_KIND_RECORD)
    {
      fh_write (fh, &fh->hdr->ilpar, 0*isize, isize, 1);
    }
  else
    {
      fh->hdr->ilnal = +MAXLONG;
      fh->hdr->ilxal = 0;
    }
  
  
  for (iartx = 0, iidx = 0; ; iidx++)
    {
      integer64 offsetx = iidx ? (fh->hdr->ioffi[ioffib-iidx]-1) * isize : isize;
  
      if (offsetx < 0)
        break;
  
      for (iart = 0; iart < isize / ARTNLEN; iart++, iartx++)
        {
          while (eqan (fh->idx[iartx].name, page__index))
            iartx++;
          isect3[iart*2]   = fh->idx[iartx].len;
          isect3[iart*2+1] = fh->idx[iartx].pos;
          memcpy (&csect2[iart*ARTNLEN], fh->idx[iartx].name, ARTNLEN);

          if ((fh->kind == FH_KIND_REGULAR) && neqan (fh->idx[iartx].name, trou__index) && neqan (fh->idx[iartx].name, fin_d_index))
            {
              if (fh->idx[iartx].len < fh->hdr->ilnal)
                fh->hdr->ilnal = fh->idx[iartx].len;
              if (fh->idx[iartx].len > fh->hdr->ilxal)
                fh->hdr->ilxal = fh->idx[iartx].len;
            }
        }
  
      fh_write (fh, csect2, offsetx,         isize, 0);
      fh_write (fh, isect3, offsetx + isize, isize, 1);
    }

  if (fh->kind == FH_KIND_REGULAR)
    fh_write (fh, &fh->hdr->ilpar, 0*isize, isize, 1);
  
  return;
}

/* If file size is not a multiple of physical article size, 
 * then pad file with zeroes */
static void fh_write_pad (lfi_alts_fh_t * fh, integer64 * KREP)
{
  integer64 nr;
  integer64 sz;

  *KREP = 0;

  if (fh->kind == FH_KIND_STREAM || fh->kind == FH_KIND_RECORD)
  /* Assume we are already at EOF
   * we need this option in case we are writing to a fifo
   */
    {
      sz = fh->offset;
    }
  else
    {
      if ((sz = fh_seek (fh, 0, SEEK_END)) < 0)
        lfi_abor ("Cannot fseek unit %lld, `%s'", fh->inumer, fh->cnomf);
    }

  if ((nr = sz % fh->hdr->size) != 0)
    {
      integer64 nw = fh->hdr->size - nr;
      char * buf = (char *)malloc (nw);
      memset (buf, 0, nw);
      fh_write (fh, buf, sz, nw, 0);
      free (buf);
    }

  return;
}


/* Set modified status */
static void fh_modified (lfi_alts_fh_t * fh)
{
  integer64 flag = 1;
  integer64 IREP = 0;


  if (fh->kind != FH_KIND_REGULAR)
    return;

  if (fh->modified)
    return;

  fh_write_hdr (fh, &IREP);
  fh_write (fh, &flag, 16, 8, 1);
  fh_flush (fh);

  fh->modified = 1;
}

/* Cast void * to a lfi_alts_t pointer and make a check */
static lfi_alts_t * lookup_als (void * LFI)
{
  lfi_alts_t * als = LFI; 
  if (strncmp (als->cmagic, "lfi_alts", 8))  
    lfi_abor ("Corrupted descriptor"); 
  return als;
}

/* Search for open file by unit number (KNUMER) */
static lfi_alts_fh_t * lookup_fh (lfi_alts_t * als, integer64 * KNUMER, int fatal)
{
  lfi_alts_fh_t * fh;
  for (fh = als->fh; fh; fh = fh->next)
    if (fh->inumer == *KNUMER)
      return fh;
  if (fatal)
    lfi_abor ("File number `%lld' is not opened", *KNUMER);
  return NULL;
}

/* Check article name length */
static int check_an_len (const char * CDNOMA, const character_len CDNOMA_len)
{
  int len = lfi_fstrlen (CDNOMA, CDNOMA_len);
  return len > ARTNLEN ? 0 : 1;
}

/* Check article name */
static int check_an (const char * CDNOMA, const character_len CDNOMA_len)
{
  int len;
  if (! check_an_len (CDNOMA, CDNOMA_len))
    return 0;
  len = minARTN (CDNOMA_len);
  return strncmp (CDNOMA, fin_d_index, len)
      && strncmp (CDNOMA, trou__index, len)
      && strncmp (CDNOMA, page__index, len);
}

/* Search for an article given its name */
static int lookup_rc (lfi_alts_fh_t * fh, character * name, character_len name_len)
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
      int iart2 = fh->iart+da < fh->nart ? fh->iart+da : fh->nart;
      for (iart = iart1; iart < iart2; iart++)
        if (eqan (_name, fh->idx[iart].name))
          return iart;
    }

  for (iart = 0; iart < fh->nart; iart++)
    if (eqan (_name, fh->idx[iart].name))
      return iart;

  return -20;
}

/* Search for next/previous article */
static int seek_rc (lfi_alts_fh_t * fh, int d)
{
  int iart = fh->iart;

  d = d >= 0 ? +1 : -1;
  
  if ((d < 0) && (iart < 0))
    iart = fh->nart;

  while (1)
    {
      iart = iart + d;
      if ((iart >= fh->nart) || (iart < 0))
        goto ko;
      if (eqan (fh->idx[iart].name, fin_d_index) && (d > 0))
        goto ko;
      if (neqan (fh->idx[iart].name, trou__index) && neqan (fh->idx[iart].name, page__index))
        goto ok;
    }

  return 0;
ko:
  return -1;
ok:
  return iart;
}

/* Assuming index is full, grow index and return a pointer on first empty slot */
static void fh_grow_idx (lfi_alts_fh_t * fh, integer64 alen, int * iartx)
{
  int ioffib = fh->hdr->size / 8 - fh->hdr->illdo;
  int nart = fh->nart + fh->hdr->size / ARTNLEN + 1; /* The 1 is here to store the new index section position */
  int iart;
  int iidx;
  int iart_idx;
  int iart_art;
  integer64 pos, pose;

  /* Look for an empty index slot in header; we need that to mark the extra index section position in the article index */
  for (iidx = 0; fh->hdr->ioffi[ioffib-1-iidx]; iidx++);
    if (ioffib-1-iidx == fh->hdr->illdo)
      {
        *iartx = -1;
        return;
      }

  fh->idx = (lfi_alts_fh_idx_t *)realloc (fh->idx, sizeof (lfi_alts_fh_idx_t) * nart);

  /* Fill new entries with empty slot values */

  for (iart = fh->nart; iart < nart; iart++)
    {
      fh->idx[iart].len = 0;
      fh->idx[iart].pos = 0;
      memcpy (fh->idx[iart].name, fin_d_index, ARTNLEN);
    }

  /* For some reason, the legacy LFI prefers that the article to be written be
   * inserted before the new index entry 
   */

  iart_art = fh->nart;
  iart_idx = fh->nart + 1;
  
  /* Compute len and pos of index section and add it in the index */

  fh->idx[iart_idx].len = 2 * fh->hdr->size / 8;
  pos = fh->idx[iart_idx-2].pos + fh->idx[iart_idx-2].len + alen;

  pose = (pos-1) % (fh->hdr->size / 8);

  if (pose > 0)
    pos += (fh->hdr->size / 8) - pose;

  fh->idx[iart_idx].pos = pos;

  memcpy (fh->idx[iart_idx].name, page__index, ARTNLEN);

  /* Update header */

  if (ioffib-1-iidx < 0)
    lfi_abor ("Internal error: index is full");

  fh->hdr->ioffi[ioffib-1-iidx] = 1 + ((pos-1) * 8) / fh->hdr->size;

  *iartx = iart_art;
  fh->nart = nart;

}

#define ALS_DECL \
  lfi_alts_t * als = lookup_als (LFI);
#define FH_DECL(fatal) \
  lfi_alts_fh_t * fh = lookup_fh (als, KNUMER, fatal);  
#define ART_DECL \
  int iart = lookup_rc (fh, CDNOMA, CDNOMA_len);
  
/* Extended lfiouv; the extra kind argument tells that we are writing to a fifo or a socket */
static void lfiouv_alts_ext (LFIOUV_ARGS_DECL, lfi_alts_fh_kind_t kind)
{
  ALS_DECL;
  FH_DECL (0);
  char * clendi;
  char * cnomf, * cstto;
  int iislen, llrqle;
  struct stat st;
  int old;
  int isize;
  character CLNOMF[32];

  DRHOOK_START ("lfiouv_alts_ext");

  if (fh != NULL)
    {
      *KREP = -13;
     goto end;
    }

  if (! istrue (*LDNOMM))
    {
      if (*KNUMER <= 0)
        lfi_abor ("LDNOMM=T is not compatible with KNUMER<=0");
      CDNOMF_len = sprintf (CLNOMF, "fort.%lld", *KNUMER);
      CDNOMF = CLNOMF;
    }

  cnomf = lfi_fstrdup (CDNOMF, CDNOMF_len, NULL);
  cstto = lfi_fstrdup (CDSTTO, CDSTTO_len, NULL);

  fh = (lfi_alts_fh_t *)malloc (sizeof (lfi_alts_fh_t));
  memset (fh, 0, sizeof (*fh));

  fh_open (fh, cnomf, cstto, kind, KREP);

  if (*KREP != 0)
    {
      free (cnomf);
      free (cstto);
      free (fh);
      goto end;
    }

  if (*KNUMER == 0)
    {
      /* Allocate a unit number for this file */
      integer64 inumer = -1000000;
      lfi_alts_fh_t * fh;
again:
      for (fh = als->fh; fh; fh = fh->next)
        if (fh->inumer == inumer)
          {
            inumer--;
            goto again;
          }
      *KNUMER = inumer;
    }

  fh->cnomf    = cnomf;
  fh->cstto    = cstto;
  fh->inumer   = *KNUMER;
  fh->inimes   = *KNIMES;
  fh->llerfa   = *LDERFA;
  fh->offset   =  0;
  fh->iart     = -1;
  fh->modified =  0;

  /* Facteur multiplicatif */

  fh->fmult    = als->fmult;
  lfi_fmul_get (als->fmult_list, KNUMER, &fh->fmult);

  /* Choose endian mode */

  clendi = getenv ("LFI_BYTE_ORDER");
  iswap_isle_ (&iislen);

  if (clendi && strcmp (clendi, "LITTLE_ENDIAN") == 0)
    llrqle = 1;
  else if (clendi && strcmp (clendi, "BIG_ENDIAN") == 0)
    llrqle = 0;
  else if (clendi && strcmp (clendi, "NATIVE_ENDIAN") == 0)
    llrqle = iislen;
  else
    llrqle = 0;

  fh->lswap = iislen != llrqle;

  fh_fstat (fh, &st);
  
  old = st.st_size > 0;

  /* File already exists; grok its endianess */
  if (old)
    {
      integer64 i_lfi_s1_2[2], i_lfi_s1_1;
      DRHOOK_START ("lfiouv_alts_ext:read1");
      fh_read (fh, i_lfi_s1_2, 0, 16, 0);
      DRHOOK_END (0);
      i_lfi_s1_1 = i_lfi_s1_2[0];
      fh->lswap = i_lfi_s1_2[1] > 128;
      if (fh->lswap)
        {
          int t = 8, n = 1;
          jswap_ (&i_lfi_s1_1, &i_lfi_s1_1, &t, &n);
        }
      fh->fmult = i_lfi_s1_1 / 512;
    }

  isize = sizeof (integer64) * 512 * fh->fmult;

  fh->nart = isize / ARTNLEN;
  fh->hdr = (lfi_alts_fh_hdr_t *)malloc (sizeof (lfi_alts_fh_hdr_t) + sizeof (integer64) * isize);
  fh->hdr->size = isize;

  /* File already exists; read header and index */

  if (old)
    {
      character csect2[isize];
      integer64 isect3[isize/8];
      int iidx, iidy, nidx;
      int iart;
      int ioffib;

      DRHOOK_START ("lfiouv_alts_ext:read");

      fh_read (fh, &fh->hdr->ilpar, 0*isize, isize, 1);

      /* Count indexes that may be scattered inside the file */

      ioffib = isize / 8 - fh->hdr->illdo;

      for (nidx = 0; fh->hdr->ioffi[ioffib-1-nidx]; nidx++);

      /* Update maximum number of articles */
     
      fh->nart = (isize / ARTNLEN) * (nidx + 1) + nidx;

      /* Allocate index struct; up to maximum number of articles */

      fh->idx = (lfi_alts_fh_idx_t *)malloc (sizeof (lfi_alts_fh_idx_t) * fh->nart);

      /* Read all index data */
      for (iart = 0, iidx = 0, iidy = 1; iidx < nidx + 1; iidx++)
        {
          integer64 offsetx = iidx ? (fh->hdr->ioffi[ioffib-iidx]-1) * isize : isize;
          int iartx;

          fh_read (fh, csect2, offsetx +     0, isize, 0);
          fh_read (fh, isect3, offsetx + isize, isize, 1);

          for (iartx = 0; iartx < isize / ARTNLEN; iartx++, iart++)
            {
              integer64 len = isect3[iartx*2], 
                        pos = isect3[iartx*2+1];

              /* Add a dummy article for each index that is embedded within data */

              if (fh->hdr->ioffi[ioffib-iidy])
                {
                  int ioffi;
                  while ((ioffi = fh->hdr->ioffi[ioffib-iidy]))
                    { 
                      integer64 offsety = (ioffi-1) * isize;
                      if ((pos-1)*8 < offsety)
                        break;
                      fh->idx[iart].len = 2 * isize / 8;
                      fh->idx[iart].pos = 1 + offsety / 8;
                      memcpy (fh->idx[iart].name, page__index, ARTNLEN);
                      iart++;
                      iidy++;
                    }
                }

              fh->idx[iart].len = len;
              fh->idx[iart].pos = pos;
              memcpy (fh->idx[iart].name, &csect2[iartx*ARTNLEN], ARTNLEN);
            }

        }

      /* End of index */
      for (iart = fh->nart-1; iart >= 0; iart--)
        if (eqan (fh->idx[iart].name, trou__index))
          memcpy (fh->idx[iart].name, fin_d_index, ARTNLEN);
        else
          break;
 
      DRHOOK_END (0);
    }
  else
    {
      /* New file; create a header and an index */
     
      int iart;
      integer64 YYYYMMDD, HHmmss;
     
      fh->idx = (lfi_alts_fh_idx_t *)malloc (sizeof (lfi_alts_fh_idx_t) * fh->nart);
     
      xgetdate (&YYYYMMDD, &HHmmss);
      memset (&fh->hdr->ilpar, 0, fh->hdr->size);
      for (iart = 0; iart < fh->nart; iart++)
        {
          fh->idx[iart].len = 0;
          fh->idx[iart].pos = 0;
          memcpy (fh->idx[iart].name, fin_d_index, ARTNLEN);
        }
      fh->hdr->inaph =           3;
      fh->hdr->ilpar =  isize /  8;
      fh->hdr->ixapi =  isize / 16;
      fh->hdr->ilmna =     ARTNLEN;
      fh->hdr->ifeam =           0;
      fh->hdr->illdo =          22;
      fh->hdr->idcre =    YYYYMMDD;
      fh->hdr->iddmg =    YYYYMMDD;
      fh->hdr->idmng =    YYYYMMDD;
      fh->hdr->ihcre =      HHmmss;
      fh->hdr->ihdmg =      HHmmss;
      fh->hdr->ihmng =      HHmmss;
      fh->hdr->inpir =           1;

      fh_modified (fh);
    }


  fh->next = als->fh;
  als->fh = fh;


  *KNBARI = fh->hdr->inalo - fh->hdr->intru;

  if (fh->hdr->ifeam != 0)
    *KREP = -11;
  else
    *KREP = 0;

end:
  DRHOOK_END (0);
  return;
}

static void lfiouv_alts (LFIOUV_ARGS_DECL)
{
  DRHOOK_START ("lfiouv_alts");
  lfiouv_alts_ext (LFIOUV_ARGS_LIST, FH_KIND_REGULAR);
  DRHOOK_END (0);
}

static void lfifer_alts (LFIFER_ARGS_DECL)
{
  ALS_DECL;
  lfi_alts_fh_t * fh, * fg;
  DRHOOK_START ("lfifer_alts");

  *KREP = 0;

  /* We search the file handle by hand, because we need to update the list of opened file handles */

  for (fh = als->fh, fg = NULL; fh; fg = fh, fh = fh->next)
    if (fh->inumer == *KNUMER)
      break;

  if (fh == als->fh)
    als->fh = fh->next;
  else
    fg->next = fh->next;

  if (fh->modified)
    {
      fh_write_hdr (fh, KREP);
      if (*KREP != 0)
        goto end;
      fh_write_pad (fh, KREP);
      if (*KREP != 0)
        goto end;
    }

  if (*KREP == 0)
    fh_close (fh, KREP);

end:

  free (fh->cnomf);
  free (fh->cstto);
  free (fh->idx);
  free (fh->hdr);

  free (fh);

  DRHOOK_END (0);

}

static void lfinum_alts (LFINUM_ARGS_DECL)
{
  ALS_DECL;
  FH_DECL (0);

  DRHOOK_START ("lfinum_alts");
  
  *KRANG = fh == NULL ? 0 : 1;

  DRHOOK_END (0);
}

static void lfinfo_alts (LFINFO_ARGS_DECL)
{
  ALS_DECL;
  FH_DECL (1);
  ART_DECL;
  DRHOOK_START ("lfinfo_alts");

  if (iart < 0)
    {
      *KREP   = 0;
      *KLONG  = 0;
      *KPOSEX = 0;
    }
  else
    {
      fh->iart = iart;
      *KLONG  = fh->idx[iart].len;
      *KPOSEX = fh->idx[iart].pos;
      *KREP   = 0;
    }

  DRHOOK_END (0);
}

static void lfinff_alts (LFINFF_ARGS_DECL)
{
  ALS_DECL;
  FH_DECL (1);
  ART_DECL;
  DRHOOK_START ("lfinff_alts");

  if (iart < 0)
    {
      *KREP   = 0;
      CDNOMA[0] = '\0';
      if (CDNOMF_len > 0)
        CDNOMF[0] = '\0';
    }
  else
    {
      fh->iart = iart;
      *KREP   = 0;
      strncpy (CDNOMF, fh->cnomf, CDNOMF_len);
    }

  DRHOOK_END (0);
}

static void lfilec_alts (LFILEC_ARGS_DECL)
{
  ALS_DECL;
  FH_DECL (1);
  ART_DECL;
  DRHOOK_START ("lfilec_alts");

  *KREP = 0;

  if (*KLONG < 0)
    {
      *KREP = -14;
      goto end;
    }
  
  if (iart < 0)
    {
      *KREP = iart;
      goto end;
    }
  else
    {
      integer64 len = fh->idx[iart].len > *KLONG ? *KLONG : fh->idx[iart].len;
      integer64 pos = fh->idx[iart].pos;

      fh_read (fh, KTAB, (pos-1) * 8, len * 8, 1);

      if (*KLONG < fh->idx[iart].len)
        {
          *KREP = -21;
          goto end;
        }

      fh->iart = iart;

    }

end:
  DRHOOK_END (0);
}

static void lfilas_alts (LFILAS_ARGS_DECL)
{
  ALS_DECL;
  FH_DECL (1);
  int iart = seek_rc (fh, +1);
  DRHOOK_START ("lfilas_alts");
  
  if (iart < 0)
    {
      *KREP = -23;
      goto end;
    }
  else
    {
      integer64 len = fh->idx[iart].len > *KLONG ? *KLONG : fh->idx[iart].len;
      integer64 pos = fh->idx[iart].pos;

      fh_read (fh, KTAB, (pos-1) * 8, len * 8, 1);

      *KREP = 0;

      if (*KLONG < fh->idx[iart].len)
        {
          *KREP = -21;
          goto end;
        }

      memset (CDNOMA, ' ', CDNOMA_len);
      memcpy (CDNOMA, fh->idx[iart].name, minARTN (CDNOMA_len)); 

      if (CDNOMA_len < lfi_fstrlen (fh->idx[iart].name, ARTNLEN))
        {
          *KREP = -24;
          goto end;
        }

      fh->iart = iart;
    }

end:
  DRHOOK_END (0);
}

static void lfilap_alts (LFILAP_ARGS_DECL)
{
  ALS_DECL;
  FH_DECL (1);
  int iart = seek_rc (fh, -1);
  DRHOOK_START ("lfilap_alts");
  
  if (iart < 0)
    {
      *KREP = -26;
      goto end;
    }
  else
    {
      integer64 len = fh->idx[iart].len > *KLONG ? *KLONG : fh->idx[iart].len;
      integer64 pos = fh->idx[iart].pos;

      fh_read (fh, KTAB, (pos-1) * 8, len * 8, 1);
 
      *KREP = 0;
     
      if (*KLONG < fh->idx[iart].len)
        {
          *KREP = -21;
          goto end;
        }

      memset (CDNOMA, ' ', CDNOMA_len);
      memcpy (CDNOMA, fh->idx[iart].name, minARTN (CDNOMA_len)); 

      if (CDNOMA_len < lfi_fstrlen (fh->idx[iart].name, ARTNLEN))
        {
          *KREP = -24;
          goto end;
        }

      fh->iart = iart;
    }

end:
  DRHOOK_END (0);
}

static void lfican_alts (LFICAN_ARGS_DECL)
{
  ALS_DECL;
  FH_DECL (1);
  int iart = seek_rc (fh, +1);
  DRHOOK_START ("lfican_alts");

  *KREP   = 0;
  memset (CDNOMA, ' ', CDNOMA_len);

  if (iart < 0)
    {
      goto end;
    }
 
  memcpy (CDNOMA, fh->idx[iart].name, minARTN (CDNOMA_len)); 

  if (CDNOMA_len < lfi_fstrlen (fh->idx[iart].name, ARTNLEN))
    {
      *KREP = -24;
      goto end;
    }

  if (istrue (*LDAVAN))
    fh->iart = iart;

end:
  DRHOOK_END (0);
}

static void lficas_alts (LFICAS_ARGS_DECL)
{
  ALS_DECL;
  FH_DECL (1);
  int iart = seek_rc (fh, +1);
  DRHOOK_START ("lficas_alts");

  *KREP   = 0;
  memset (CDNOMA, ' ', CDNOMA_len);

  if (iart < 0)
    {
      *KLONG  = 0;
      *KPOSEX = 0;
      goto end;
    }
 
  *KLONG  = fh->idx[iart].len;
  *KPOSEX = fh->idx[iart].pos;
  
  memcpy (CDNOMA, fh->idx[iart].name, minARTN (CDNOMA_len)); 

  if (CDNOMA_len < lfi_fstrlen (fh->idx[iart].name, ARTNLEN))
    {
      *KREP = -24;
      goto end;
    }

  if (istrue (*LDAVAN))
    fh->iart = iart;

end:
  DRHOOK_END (0);
}

static void lfipos_alts (LFIPOS_ARGS_DECL)
{
  ALS_DECL;
  FH_DECL (1);
  DRHOOK_START ("lfipos_alts");

  fh->iart = -1;

  *KREP = 0;

  DRHOOK_END (0);
}

static void lficap_alts (LFICAP_ARGS_DECL)
{
  ALS_DECL;
  FH_DECL (1);
  int iart = seek_rc (fh, -1);
  DRHOOK_START ("lficap_alts");

  *KREP  = 0;
  memset (CDNOMA, ' ', CDNOMA_len);

  if (iart < 0)
    {
      *KLONG  = 0;
      *KPOSEX = 0;
      goto end;
    }
 
  *KLONG  = fh->idx[iart].len;
  *KPOSEX = fh->idx[iart].pos;
  
  if (CDNOMA_len < lfi_fstrlen (fh->idx[iart].name, ARTNLEN))
    {
      *KREP = -24;
      goto end;
    }

  if (istrue (*LDRECU))
    fh->iart = iart;

end:
  DRHOOK_END (0);
}

static void lfilaf_alts (LFILAF_ARGS_DECL)
{
  ALS_DECL;
  FH_DECL (1);
  int iart;
  integer64 itrou = 1;
  integer64 idonn = 1;
  DRHOOK_START ("lfilaf_alts");

  *KREP = 0;

  for (iart = 0; iart < fh->nart; iart++)
    {
      if (eqan (fin_d_index, fh->idx[iart].name))
        break;
      else if (eqan (page__index, fh->idx[iart].name))
        continue;
      else if (eqan (trou__index, fh->idx[iart].name))
        {
          printf (
" =====> %7lld-eme TROU repertorie dans l'index, longueur reutilisable: %6lld mots, position %8lld a %8lld",
itrou++, fh->idx[iart].len, fh->idx[iart].pos, fh->idx[iart].pos + fh->idx[iart].len-1);
        }
      else
        {
          printf (
"%7lld-eme article de donnees: \"%16.16s\", %6lld mots, position %8lld a %8lld",
idonn++, fh->idx[iart].name, fh->idx[iart].len, fh->idx[iart].pos, fh->idx[iart].pos + fh->idx[iart].len-1);
        }
      if (iart < fh->nart-1)
        if (neqan (fin_d_index, fh->idx[iart+1].name))
          {
            integer64 d = fh->idx[iart+1].pos - (fh->idx[iart].pos + fh->idx[iart].len);
            if (d > 0)
              printf (" < %+7lld >", d);
          }

      printf ("\n");
    }

  DRHOOK_END (0);
}

static void lfisup_alts (LFISUP_ARGS_DECL)
{
  ALS_DECL;
  FH_DECL (1);
  ART_DECL;

  DRHOOK_START ("lfisup_alts");

  if (iart < 0)
    {
      *KREP = iart;
      goto end;
    }

  memcpy (fh->idx[iart].name, trou__index, ARTNLEN);
  *KLONUT = fh->idx[iart].len;
  *KREP   = 0;
  
  fh_modified (fh);
  fh->hdr->intru++;
  fh->hdr->iltal -= fh->idx[iart].len;

  if (iart < fh->nart-1)
    if (neqan (fin_d_index, fh->idx[iart+1].name))
      fh->idx[iart].len = fh->idx[iart+1].pos - fh->idx[iart].pos;

  /* If the hole is at eof, then round its end limit on a physical page boundary */
  {
    integer64 pose = fh->idx[iart].pos - 1 + fh->idx[iart].len;
    int t = iart == fh->nart-1;
    if (! t)
      t = eqan (fin_d_index, fh->idx[iart+1].name);
    if (t)
      {
        integer64 pags = fh->hdr->size / 8;
        integer64 post = pose % pags;
        if (post > 0)
          pose = pose + pags - post;
      }
    fh->idx[iart].len = 1 + pose - fh->idx[iart].pos;
  }

  fh->iart = -1;

end:
  DRHOOK_END (0);
}

static void lfiren_alts (LFIREN_ARGS_DECL)
{
  ALS_DECL;
  FH_DECL (1);
  int iart1 = lookup_rc (fh, CDNOM1, CDNOM1_len);
  int iart2 = lookup_rc (fh, CDNOM2, CDNOM2_len);
  integer64 ILONUT;
  DRHOOK_START ("lfiren_alts");  

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

  lfisup_alts (LFI, KREP, KNUMER, CDNOM2, &ILONUT, CDNOM2_len);

  memcpy (fh->idx[iart1].name, trou__index, ARTNLEN);
  memcpy (fh->idx[iart1].name, CDNOM2, minARTN (CDNOM2_len));

  *KREP = 0;

end:
  DRHOOK_END (0);
}

static void lfiecr_alts (LFIECR_ARGS_DECL)
{
  ALS_DECL;
  FH_DECL (1);
  int iart = lookup_rc (fh, CDNOMA, CDNOMA_len);
  int iartx = -1; /* Index of article with the most appropriate size */
  int append;
  DRHOOK_START ("lfiecr_alts");

  if (*KLONG < 0)
    {
      *KREP = -14;
      goto end;
    }

  if (iart >= 0)
    {
      integer64 ILONUT;
      lfisup_alts (LFI, KREP, KNUMER, CDNOMA, &ILONUT, CDNOMA_len);
    }
  /* Check article name validity */
  else if (! check_an (CDNOMA, CDNOMA_len))
    {
      *KREP = -15;
      goto end;
    }

  /* 
   * Try original slot
   */

  if (iart >= 0)
    {
      if (fh->idx[iart].len >= *KLONG)
        {
          iartx = iart;
        }
      else if (iart < fh->nart-1)
        {
          if (eqan (trou__index, fh->idx[iart+1].name) && ((fh->idx[iart].len + fh->idx[iart+1].len) >= *KLONG))
            {
              integer64 d = *KLONG - fh->idx[iart].len;
              fh->idx[iart+1].pos = fh->idx[iart+1].pos + d;
              fh->idx[iart+1].len = fh->idx[iart+1].len - d;
              fh->idx[iart].len = *KLONG;
              iartx = iart;
            }
          else if (eqan (fin_d_index, fh->idx[iart+1].name))
            {
              fh->idx[iart].len = *KLONG;
              iartx = iart;
            }
        }
    }

  /* 
   * Look for a large enough empty slot;
   * try to select the most appropriate slot : 
   * the hole with the smallest size if possible
   */
   
  if (iartx < 0)
    for (iart = 0; iart < fh->nart; iart++)
      {
        if (eqan (page__index, fh->idx[iart].name))
          continue;
        else if (eqan (fin_d_index, fh->idx[iart].name))
          {
            if (iartx < 0)
              iartx = iart;
            break;
          }
        /* Hole is big enough */
        else if (eqan (trou__index, fh->idx[iart].name) && (fh->idx[iart].len >= *KLONG))
          {
            if (iartx < 0)
              iartx = iart;
            else if (fh->idx[iart].len < fh->idx[iartx].len)
              iartx = iart;
          }
        /* If last hole is at EOF, we can enlarge it */
        else if (0 && eqan (trou__index, fh->idx[iart].name) && (iart < fh->nart-1) && (iartx < 0))
          {
            if (eqan (fin_d_index, fh->idx[iart+1].name))
              {
                iartx = iart;
                /* Make this look like we appended data at EOF */
                memcpy (fh->idx[iart].name, fin_d_index, ARTNLEN);
                fh->hdr->intru--;
                fh->hdr->inalo--;
                break;
              }
          }
      }

  /* Index is full; try to grow index */

  if (iartx < 0)
    {
      fh_grow_idx (fh, *KLONG, &iartx);
      if (iartx < 0)
        {
          *KREP = -1;
          goto end;
        }
    }

  /* Everything is OK; mark file as modified */

  fh_modified (fh);

  append = eqan (fh->idx[iartx].name, fin_d_index);

  /* If article is appended, then compute its position */
  if (append)
    {
      if (iartx > 0)
        fh->idx[iartx].pos = fh->idx[iartx-1].pos + fh->idx[iartx-1].len;
      else
        fh->idx[iartx].pos = fh->hdr->inaph * fh->hdr->ilpar + 1;
    }

  /* Finish to setup index */
  fh->idx[iartx].len = *KLONG;
  memcpy (fh->idx[iartx].name, trou__index, ARTNLEN);
  memcpy (fh->idx[iartx].name, CDNOMA, minARTN (CDNOMA_len));

  /* Write data */
  fh_write (fh, KTAB, (fh->idx[iartx].pos - 1) * 8, fh->idx[iartx].len * 8, 1);

  /* Update header parameters */

  fh->hdr->iltal += fh->idx[iartx].len;

  if (append)
    {
      fh->hdr->inalo++;
     
      /* Update number of physical articles */
      {
        integer64 size = fh->idx[iartx].pos + fh->idx[iartx].len - 8;
        fh->hdr->inaph = size / fh->hdr->ilpar;
        if (size % (fh->hdr->inaph * fh->hdr->ilpar))
          fh->hdr->inaph++;
      }
     
      fh->hdr->iaxpd = fh->hdr->inaph;
    }
  else
    {
      /* We reused a hole; decrement hole count */
      fh->hdr->intru--;
    }

  fh->iart = iartx;

  *KREP = 0;

end:

  DRHOOK_END (0);

}

static void lfiopt_alts (LFIOPT_ARGS_DECL)
{
  ALS_DECL;
  FH_DECL (0);
  DRHOOK_START ("lfiopt_alts");

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

static void lfinaf_alts (LFINAF_ARGS_DECL)
{
  ALS_DECL;
  FH_DECL (1);
  DRHOOK_START ("lfinaf_alts");

  *KNTROU = fh->hdr->intru;
  *KNALDO = fh->hdr->inalo - fh->hdr->intru;
  *KNARES = 0;
  *KNAMAX = 0;
  *KREP   = 0;

  DRHOOK_END (0);

}

static void lfinim_alts (LFINIM_ARGS_DECL)
{
  ALS_DECL;
  FH_DECL (1);

  DRHOOK_START ("lfinim_alts");

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

static void lfierf_alts (LFIERF_ARGS_DECL)
{
  ALS_DECL;
  FH_DECL (1);
  DRHOOK_START ("lfierf_alts");

  fh->llerfa = *LDERFA;
  *KREP = 0;

  DRHOOK_END (0);
}

static void lfioef_alts (LFIOEF_ARGS_DECL)
{
  ALS_DECL;
  FH_DECL (1);

  DRHOOK_START ("lfioef_alts");

  *LDERFA = fh->llerfa;
  *KREP = 0;

  DRHOOK_END (0);
}

static void lfifmd_alts (LFIFMD_ARGS_DECL)
{
  ALS_DECL;

  DRHOOK_START ("lfifmd_alts");

  if (*KFACMD > 0)
    als->fmult = *KFACMD;

  DRHOOK_END (0);
}

static void lfiofd_alts (LFIOFD_ARGS_DECL)
{
  ALS_DECL;
  DRHOOK_START ("lfiofd_alts");
  *KFACMD = als->fmult;
  DRHOOK_END (0);
}

static void lfiofm_alts (LFIOFM_ARGS_DECL)
{
  ALS_DECL;
  FH_DECL (0);
  DRHOOK_START ("lfiofm_alts");

  if (fh) 
    {
      *KFACTM = fh->fmult;
      *LDOUVR = fort_TRUE;
      *KREP = 0;
    }
  else
    {
      *KFACTM = als->fmult;
      lfi_fmul_get (als->fmult_list, KNUMER, KFACTM);
      *LDOUVR = fort_FALSE;
      *KREP = 0;
    }

  DRHOOK_END (0);
}

static void lfiafm_alts (LFIAFM_ARGS_DECL)
{
  ALS_DECL;
  FH_DECL (0);
  DRHOOK_START ("lfiafm_alts");

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
      lfi_fmul_set (&als->fmult_list, KNUMER, KFACTM);
      *KREP = 0;
    }

  DRHOOK_END (0);
}

static void lfisfm_alts (LFISFM_ARGS_DECL)
{
  
  ALS_DECL;
  FH_DECL (0);
  DRHOOK_START ("lfisfm_alts");

  *KREP = 0;
  if (fh) 
    {
      *KREP = -5;
    }
  else
    {
      if (lfi_fmul_del (&als->fmult_list, KNUMER) < 0)
        *KREP = -31;
    }

  DRHOOK_END (0);
}

static void lfineg_alts (LFINEG_ARGS_DECL)
{
  ALS_DECL;
  DRHOOK_START ("lfineg_alts");
  if ((*KNIVAU >= 0) && (*KNIVAU <= 2))
    als->nerfag = *KNIVAU;
  DRHOOK_END (0);
}

static void lfioeg_alts (LFIOEG_ARGS_DECL)
{
  ALS_DECL;
  DRHOOK_START ("lfioeg_alts");
  *KNIVAU = als->nerfag;
  DRHOOK_END (0);
}

static void lfiomg_alts (LFIOMG_ARGS_DECL)
{
  ALS_DECL;
  DRHOOK_START ("lfiomg_alts");
  *KNIVAU = als->inivau;
  *KULOUT = als->iulout;
  DRHOOK_END (0);
}

static void lfinmg_alts (LFINMG_ARGS_DECL)
{
  ALS_DECL;
  DRHOOK_START ("lfinmg_alts");
  als->inivau = *KNIVAU;
  als->iulout = *KULOUT;
  DRHOOK_END (0);
}

#undef ALS_DECL

lficb_t lficb_alts = {
  lfiouv_alts,        /*        Ouverture fichier                                        */
  lfican_alts,        /* KNUMER Caracteristiques de l'article suivant                    */
  lficas_alts,        /* KNUMER Caracteristiques de l'article suivant                    */
  lfiecr_alts,        /* KNUMER Ecriture                                                 */
  lfifer_alts,        /* KNUMER Fermeture                                                */
  lfilec_alts,        /* KNUMER Lecture                                                  */
  lfinfo_alts,        /* KNUMER Caracteristiques d'un article nomme                      */
  lfinff_alts,        /* KNUMER Get real file & record name                              */
  lfipos_alts,        /* KNUMER Remise a zero du pointeur de fichier                     */
  lfiver_dumm,        /* KNUMER Verrouillage d'un fichier                                */
  lfiofm_alts,        /* KNUMER Obtention du facteur multiplicatif                       */
  lfineg_alts,        /*        Niveau global d'erreur                                   */
  lfilaf_alts,        /* KNUMER Liste des articles                                       */
  lfiosg_dumm,        /*        Obtention du niveau d'impression des statistiques        */
  lfinum_alts,        /* KNUMER Rang de l'unite logique KNUMER                           */
  lfisup_alts,        /* KNUMER Suppression d'un article                                 */
  lfiopt_alts,        /* KNUMER Obtention des options d'ouverture d'un fichier           */
  lfinmg_alts,        /*        Niveau global d'erreur                                   */
  lficap_alts,        /* KNUMER Caracteristiques de l'article precedent                  */
  lfifra_dumm,        /*        Messages en Francais                                     */
  lficfg_dumm,        /*        Impression des parametres de base de LFI                 */
  lfierf_alts,        /* KNUMER Erreur fatale                                            */
  lfilas_alts,        /* KNUMER Lecture de l'article de donnees suivant                  */
  lfiren_alts,        /* KNUMER Renommer un article                                      */
  lfiini_dumm,        /*        Initialisation de LFI                                    */
  lfipxf_miss,        /* KNUMER Export d'un fichier LFI                                  */
  lfioeg_alts,        /*        Obtention du niveau global de traitement des erreurs     */
  lfinaf_alts,        /* KNUMER Nombre d'articles divers                                 */
  lfiofd_alts,        /*        Facteur multiplicatif courant                            */
  lfiomf_dumm,        /* KNUMER Obtention du niveau de messagerie                        */
  lfiafm_alts,        /* KNUMER Attribution d'un facteur multiplicatif a une unite       */
  lfista_dumm,        /* KNUMER Impression des statistiques d'utilisation                */
  lfiosf_miss,        /* KNUMER Obtention de l'option d'impression des statistiques      */
  lfilap_alts,        /* KNUMER Lecture de l'article precedent                           */
  lfioef_alts,        /* KNUMER Obtention de l'option courante de traitement des erreurs */
  lfimst_dumm,        /* KNUMER Activation de l'option d'impression de statistiques      */
  lfinim_alts,        /* KNUMER Ajustement du niveau de messagerie                       */
  lfisfm_alts,        /* KNUMER Suppression d'un facteur multiplicatif                   */
  lfinsg_dumm,        /*        Niveau global d'impression de statistiques               */
  lfideb_dumm,        /*        Mode mise au point (debug)                               */
  lfiomg_alts,        /*        Obtention du niveau global des messages LFI              */
  lfifmd_alts,        /*        Facteur multiplicatif par defaut                         */
};

#define ALS_DECL \
  lfi_alts_t * als = lookup_als (lfi->data); 

static void lfi_del_alts_hndl (lfi_hndl_t * lfi)
{
  ALS_DECL;

  if (als->fh)
    lfi_abor ("Attempt to release lfi handler with opened files");

  lfi_fmul_free (&als->fmult_list);

  free (als);
  free (lfi);
}

static int lfi_opn_alts_hndl (lfi_hndl_t * lfi, integer64 * KNUMER)
{
  ALS_DECL;
  FH_DECL (0);
  return fh == NULL ? 0 : 1;
}

static int lfi_vrb_alts_hndl (lfi_hndl_t * lfi, integer64 * KNUMER)
{
  ALS_DECL;
  FH_DECL (1);
  return fh->inimes == 2 ? 1 : 0;
}

static int lfi_fat_alts_hndl (lfi_hndl_t * lfi, integer64 * KNUMER)
{
  ALS_DECL;
  FH_DECL (1);
  return (als->nerfag == 0) || ((als->nerfag == 1) && istrue (fh->llerfa));
}

#undef ALS_DECL

lfi_hndl_t * lfi_get_alts_hndl (void * data)
{
  lfi_hndl_t * lfi = (lfi_hndl_t *)malloc (sizeof (lfi_hndl_t));
  lfi_alts_t * als = (lfi_alts_t *)malloc (sizeof (lfi_alts_t));

  memset (als, 0, sizeof (lfi_alts_t));
  memcpy (als->cmagic, "lfi_alts", 8); 
  als->fmult      = 6;
  als->fmult_list = NULL;
  als->nerfag     = 1;
  als->inivau     = 0;
  als->iulout     = 0;

  lfi->cb = &lficb_alts;
  lfi->cb_verb = &lficb_verb;
  lfi->data = als;
  lfi->destroy = lfi_del_alts_hndl;
  lfi->is_open = lfi_opn_alts_hndl;
  lfi->is_verb = lfi_vrb_alts_hndl;
  lfi->is_fatl = lfi_fat_alts_hndl;
  lfi->next = NULL;
  return lfi;
}

static void lfi_alts_pack (integer64 * KREP, character * CDNOMF1, character * CDNOMF2, 
                           character_len CDNOMF1_len, character_len CDNOMF2_len, 
                           lfi_alts_fh_kind_t kind, long * offset)
{
  lfi_grok_t islfi = lfi_grok (CDNOMF1, CDNOMF1_len);
  /* Multi-file is opened with lfi_altm */
  lfi_hndl_t * lfm;
  /* Target file is opened with lfi_alts */
  lfi_hndl_t * lfs;
  integer64 INUMER = 1;
  integer64 INALDO;

  *KREP = 0;
  *offset = 0;

  if (islfi != LFI_ALTM)
    {
      struct stat st;
      char path[CDNOMF1_len+1];

      if (kind != FH_KIND_RECORD)
        if ((*KREP = lfi_fsmartcopy (CDNOMF1, CDNOMF2, 1, CDNOMF1_len, CDNOMF2_len)) != 0)
          goto error;

      lfi_fstrdup (CDNOMF1, CDNOMF1_len, path);

      if (stat (path, &st) == 0)
        *offset = st.st_size;

      goto end;
    }


  lfm = lfi_get_altm_hndl (NULL);
  lfs = lfi_get_alts_hndl (NULL);
  


  {
    integer64 INTROU, INARES, INAMAX;
    integer64 INIMES = 2, INBARP = 0, INBARI = 0;
    logical LLNOMM = fort_TRUE, LLERFA = fort_TRUE, LLIMST = fort_TRUE;
    character * CLSTTO = (character *)"OLD";
    character * CLSTTN = (character *)"NEW";
    character_len CLSTTO_len = 3;
    character_len CLSTTN_len = 3;
    integer64 IFACMD;

    lfm->cb->lfiouv (lfm->data, KREP, &INUMER, &LLNOMM, CDNOMF1, CLSTTO, &LLERFA, 
                     &LLIMST, &INIMES, &INBARP, &INBARI, CDNOMF1_len, CLSTTO_len);

    if (*KREP != 0)
      goto error;

    /* Count articles */
    lfm->cb->lfinaf (lfm->data, KREP, &INUMER, &INALDO, &INTROU, &INARES, &INAMAX);

    if (*KREP != 0)
      goto error;

    /* 
     * Choose a "facteur multiplicatif" so that all articles will fit 
     * in a single header section
     */
    {
      integer64 isize1 = 512 * 8,
                isize2 = ARTNLEN * INALDO,
                isized = isize2 % isize1;
      if (isized > 0)
        isize2 = isize2 + isize1 - isized;
      IFACMD = isize2 / isize1;
      if (IFACMD < 6) 
        IFACMD = 6;
      lfs->cb->lfifmd (lfs->data, &IFACMD);
    }

    /* Open new file */
    lfiouv_alts_ext (lfs->data, KREP, &INUMER, &LLNOMM, CDNOMF2, CLSTTN, &LLERFA,
                     &LLIMST, &INIMES, &INBARP, &INBARI, CDNOMF2_len, CLSTTN_len, 
                     kind);
    
    if (*KREP != 0)
      goto error;

  }


  {
    int iart; 
    lfi_alts_fh_t * fh = lookup_fh (lfs->data, &INUMER, 0);
    integer64 pos = 1 + (fh->hdr->size * 3) / 8;
    integer64 len = 0;

    /* Build index */
    lfm->cb->lfipos (lfm->data, KREP, &INUMER);

    if (*KREP != 0)
      goto error;

    /* Min & max article size */
    fh->hdr->ilnal = +MAXLONG;
    fh->hdr->ilxal = 0;

    for (iart = 0; iart < INALDO; iart++)
      {
        logical LLAVAN = fort_TRUE;
        integer64 IPOSEX;

        lfm->cb->lficas (lfm->data, KREP, &INUMER, fh->idx[iart].name, &fh->idx[iart].len, &IPOSEX, &LLAVAN, ARTNLEN);

        if (*KREP != 0)
          goto error;

        fh->idx[iart].pos = pos;
        pos += fh->idx[iart].len;
        len += fh->idx[iart].len;

        if (fh->hdr->ilnal > fh->idx[iart].len)
          fh->hdr->ilnal = fh->idx[iart].len;
        if (fh->hdr->ilxal < fh->idx[iart].len) 
          fh->hdr->ilxal = fh->idx[iart].len;

      }

    /* Update header */

    {
      integer64 size = len * 8 + fh->hdr->size * 3; /* File size in bytes */
      if (size % fh->hdr->size) /* Make size a multiple of fh->hdr->size */
        size += (fh->hdr->size - (size % fh->hdr->size));
      fh->hdr->inaph = size / fh->hdr->size;
      fh->hdr->inalo = INALDO;
      fh->hdr->iltal = len;
      fh->hdr->iaxpd = fh->hdr->inaph;
    }

    /* Write header */

    fh_write_hdr (fh, KREP);
    if (*KREP != 0)
      goto error;

    /* Read and write articles */

    for (iart = 0; iart < INALDO; iart++)
      {
        integer64 KTAB[fh->idx[iart].len];

        if (kind != FH_KIND_RECORD)
          {
            lfm->cb->lfilec (lfm->data, KREP, &INUMER, fh->idx[iart].name, KTAB, &fh->idx[iart].len, ARTNLEN);

            if (*KREP != 0)
              goto error;
          }

        fh_write (fh, KTAB, (fh->idx[iart].pos-1) * 8, fh->idx[iart].len * 8, 1);

      }

    /* Pad file */

    fh_write_pad (fh, KREP);

    if (*KREP != 0)
      goto error;

    *offset = fh->offset;
  }

  /* Close files */
  {
    character * CLSTTK = (character *)"KEEP";
    character_len CLSTTK_len = 4;

    lfm->cb->lfifer (lfm->data, KREP, &INUMER, CLSTTK, CLSTTK_len);

    if (*KREP != 0)
      goto error;

    lfs->cb->lfifer (lfs->data, KREP, &INUMER, CLSTTK, CLSTTK_len);

    if (*KREP != 0)
      goto error;

  }



  lfm->destroy (lfm);
  lfs->destroy (lfs);

end:

  return;

error:

  lfi_verb (NULL, "lfi_altm_pack", "KREP", KREP, NULL);
  lfi_abor ("lfi_altm_pack");

  return;

}

/* Pack a multi-file into a single regular LFI file */
void lfi_alts_pack_ (integer64 * KREP, character * CDNOMF1, character * CDNOMF2, 
                     character_len CDNOMF1_len, character_len CDNOMF2_len)
{
  long offset = 0;
  lfi_alts_pack (KREP, CDNOMF1, CDNOMF2, CDNOMF1_len, CDNOMF2_len, FH_KIND_STREAM, &offset);

}

void lfi_alts_size_ (integer64 * KREP, character * CDNOMF1, integer64 * KSIZE, character_len CDNOMF1_len)
{
  long offset = 0;
  character * CDNOMF2 = "";
  character_len CDNOMF2_len = 0;
  lfi_alts_pack (KREP, CDNOMF1, CDNOMF2, CDNOMF1_len, CDNOMF2_len, FH_KIND_RECORD, &offset);
  *KSIZE = offset;
}




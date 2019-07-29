/**** *lfi_grok.c* - Find out the kind of an LFI file
 *
 *    Author. 
 *    ------- 
 *     Philippe Marguinaud *METEO-FRANCE*
 *     Original : 12-08-2013
 *     Modified : 08-12-2014 Portability on Mac OSX
 *
 */
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#if defined(DARWIN)
#include <limits.h>
#define MAXINT INT_MAX
#define MAXLONG LONG_MAX
#elif defined(MACOSX)
#include <limits.h>
#define MAXINT INT_MAX
#else
#include <values.h>
#endif
#include <errno.h>

#include "lfi_grok.h"
#include "lfi_type.h"
#include "lfi_misc.h"
#include "lfi_abor.h"


#define YYYYMMDD_chk(x) (((x) >= 19000000) && ((x) <= 21000000))
#define HHmmss_chk(x) ((x) <= 999999)

/* Open a LFI file and try to guess its type */
lfi_grok_t lfi_grok (const character * file, character_len file_len)
{
  FILE * fp;
  char f[file_len+1];
  integer64 isect1[22];
  char cmagic[8];
  lfi_grok_t lg;
  int nr;
  int i;

  memcpy (f, file, file_len);
  f[file_len] = '\0';
  for (i = file_len-1; i >= 0; i--)
    if (f[i] == ' ')
      f[i] = '\0';
    else
      break;

  fp = fopen (f, "r");

  if (fp == NULL)
    goto lfi_none;

  nr = fread (isect1, sizeof (integer64), 22, fp);

  if (nr != 22)
    goto lfi_altm;

  if (isect1[3] != 22)
    {
      int t = 8, n = 22;
      jswap_ (isect1, isect1, &t, &n);
    }

  if (isect1[3] != 22)
    goto lfi_altm;

  if (isect1[1] != 16)
    goto lfi_altm;

  if (! YYYYMMDD_chk (isect1[13]))
    goto lfi_altm;
    
  if (! HHmmss_chk (isect1[14]))
    goto lfi_altm;
    
  lg = LFI_PURE;
  goto done;

lfi_altm:

  fseek (fp, 0, SEEK_SET);
  nr = fread (cmagic, 1, 8, fp);
  if (nr != 8)
    goto lfi_unkn;
  if (memcmp ("LFI_ALTM", cmagic, 8))
    goto lfi_unkn;
  lg = LFI_ALTM;

  goto done;

lfi_none:

  lg = LFI_NONE;
  goto done;

lfi_unkn:
  
  lg = LFI_UNKN;
  goto done;

done:

  if (fp != NULL)
    fclose (fp);

  errno = 0;

  return lg;
}


static int iscan (const char ** str, int * val)
{
  int j, k = 1;
  const char * s = *str;

  if (*s == '+')
    s++;
  if (*s == '-')
    {
      s++;
      k = -1;
    }
  if (! isdigit (*s))
    return 0;

  j = 0;
  for ( ; isdigit (*s); s++)
    j = 10 * j + (*s - '0');

  *str = s;
  *val = k * j;

  return 1;
}

/* Find the LFI library to use to open a given LFI unit (KNUMER) 
 * we use the variable LFI_HNDL_SPEC to do that
 */
int lfi_unum (integer64 * KNUMER)
{
  int unit = *KNUMER;
  const char * str = (const char *)getenv ("LFI_HNDL_SPEC");
  int unum = 1;

  if (str == NULL)
    return unum-1;

  while (1)
    {
      int umin = -MAXINT;
      int umax = +MAXINT;
      int u;
      int rmin = iscan (&str, &umin);

      if (strncmp (str, "..", 2) == 0)
        {
          str += 2;
          iscan (&str, &umax);
        }
      else if (rmin)
        {
          umax = umin;
        }

      if (strncmp (str, ":", 1) != 0)
        goto error;
      str++;

      iscan (&str, &u);

      if ((umin <= unit) && (unit <= umax))
        {
          unum = u;
          break;
        }


      if (str[0] == '\0')
        break;

      if (str[0] != ',')
        goto error;

      str++;

    }

  return unum-1;

error:

  lfi_abor ("Could not parse LFI_HNDL_SPEC =`%s'", str);

  return -1;
}





/**** *lfi_util.c* - Ancillary routines
 *
 *    Author. 
 *    ------- 
 *     Philippe Marguinaud *METEO-FRANCE*
 *     Original : 12-08-2013
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "lfi_util.h"

/* Compute the length of a Fortran character string */
character_len lfi_fstrlen (const character * fstr, const character_len len)
{
  character_len i;
  for (i = len; i > 0; i--)
    if (fstr[i-1] != ' ')
      break;
  return i;
}

/* Makes a C character string out of a Fortran string */
char * lfi_fstrdup (const character * fstr, const character_len len, char * cstr)
{
  int i;

  if (cstr == NULL)
    cstr = (char *)malloc (len + 1);

  memcpy (cstr, fstr, len);
  cstr[len] = '\0';

  for (i = len-1; i >= 0; i--)
    if (fstr[i] == ' ')
      cstr[i] = '\0';
    else
      break;

  return cstr;
}

int lfi_fstrcmp (const character * s1, const character * s2, character_len len1, character_len len2)
{
  int i;
  len1 = lfi_fstrlen (s1, len1);
  len2 = lfi_fstrlen (s2, len2);
  if (len1 > len2)
    return +1;
  if (len1 < len2)
    return -1;
  for (i = 0; i < len1; i++)
    if (s1[i] > s2[i])
      return +1;
    if (s1[i] < s2[i])
      return -1;
  return 0;
}

char * lfi_fdirname (const character * fstr, const character_len len)
{
  char * dir;
  int i;

  for (i = len; i > 0; i--)
    if (fstr[i-1] == '/')
      break;
 
  dir = (char *)malloc (i + 1);

  strncpy (dir, fstr, i);
  dir[i] = '\0';

  return dir;
}

int lfi_copy (const char * f1, const char * f2)
{
#define SIZE 4096
  char buf[SIZE];
  FILE * fp1;
  FILE * fp2; 
  int nr;

  errno = 0;

  if ((fp1 = fopen (f1, "r")) == NULL)
    goto error;

  if ((fp2 = fopen (f2, "w")) == NULL)
    goto error;

  while ((nr = fread (buf, 1, SIZE, fp1)) > 0)
    {
      if (fwrite (buf, 1, SIZE, fp2) != nr)
        goto error;

      if (nr < SIZE)
        break;
    }

  if (ferror (fp1))
    goto error;


#undef SIZE

  if (fclose (fp1) != 0)
    goto error;
  if (fclose (fp2) != 0)
    goto error;


error:

  return errno;
}

int lfi_fcopy (const character * cnomf1, const character * cnomf2, character_len cnomf1_len, character_len cnomf2_len)
{
  char f1[cnomf1_len+1];
  char f2[cnomf2_len+1];
  return lfi_copy (lfi_fstrdup (cnomf1, cnomf1_len, f1), lfi_fstrdup (cnomf2, cnomf2_len, f2));
}

int lfi_fsmartcopy (const character * cnomf1, const character * cnomf2, int copy, 
                   character_len cnomf1_len, character_len cnomf2_len)
{
  char f1[cnomf1_len+1];
  char f2[cnomf2_len+1];
  return lfi_smartcopy (lfi_fstrdup (cnomf1, cnomf1_len, f1), lfi_fstrdup (cnomf2, cnomf2_len, f2), copy);
}

int lfi_smartcopy (const char * f1, const char * f2, int copy)
{
  int c = 0;

  errno = 0;

  if (link (f1, f2) != 0)
    {
      if ((errno == EXDEV) && copy)
        c = lfi_copy (f1, f2);
      else
        c = errno;
    }

  return c;
}

int lfi_mkdir (const char * path)
{
  struct stat st;

  if (stat (path, &st) == 0)
    return 0;

  errno = 0;

  return mkdir (path, 0777);
}

const char * lfi_cleanup_path (char * path)
{
  int i, j;

  /* Cleanup; remove double / */

  for (i = 0; path[i]; )
    if ((path[i] == '/') && (path[i+1] == '/'))
      strcpy (&path[i], &path[i+1]);
    else
      i++;

  /* Cleanup path (collapse '/dir/../' in '/') */

again:
  for (i = 0, j = 0; path[i]; i++)
    if (strncmp (&path[i], "/../", 3) == 0)
      {
        if (j > 0)
          {
            strcpy (&path[j], &path[i+4]);
            goto again;
          }
      }
    else if (path[i] == '/')
      {
        j = i + 1;
      }

  return path;
}

/* 
 *  Example : (base = "/home/marguina/tmp/ICMSH0000+0000", 
 *             path = "/home/marguina/tmp/ICMSH0000+0000.d/0000") 
 *  returns "ICMSH0000+0000.d/0000"
 */

char * lfi_make_relative_path (const char * base, char * path)
{
  char * _base_ = strdup (base);
  char * _path_ = strdup (path);
  char * _base = _base_;
  char * _path = _path_;
  int i, j, k;

  lfi_cleanup_path (_base);
  lfi_cleanup_path (_path);

  /* Find out common _path in __path and __base */

  for (i = 0, j = 0, k = 0; _base[i] && _path[i]; i++)
    if (_base[i] != _path[i])
      {
        break;
      }
    else if (_base[i] == '/')
      {
        j = i + 1;
        k++;
      }
 
  /* Remove common part and truncate _base */

  _base = &_base[j];
  _path = &_path[j];

  for (i = strlen (_base); i > 0; i--)
    if (_base[i] == '/')
      break;

  _base[i] = '\0';

  /* Depth count */

  if (_base[0])
    j = 1;
  else
    j = 0;

  for (i = 0; _base[i]; i++)
    if (_base[i] == '/')
      j++;

  free ((void *)path);
  path = (char *)malloc (3 * j + strlen (_path) + 1);
  path[0] = '\0';

  /* Build final path */

  while (j--)
    strcat (path, "../");

  strcat (path, _path);

  free ((void *)_base_);
  free ((void *)_path_);

  lfi_cleanup_path (path);

  return path;
}

int lfi_rmdir (const char * path)
{
  errno = 0;
  return rmdir (path);
}

const char * lfi_dirname (const char * path)
{
  int i;
  char * dir = strdup (path);

  for (i = strlen (dir); i >= 0; i--)
    if (dir[i] == '/')
      {
        dir[i] = '\0';
        return (const char *)dir;
      }

  strcpy (dir, ".");

  return dir;
}



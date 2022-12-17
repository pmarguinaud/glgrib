#ifndef _LFI_UTIL
#define _LFI_UTIL
/**** *lfi_util.h* 
 *
 *    Author. 
 *    ------- 
 *     Philippe Marguinaud *METEO-FRANCE*
 *     Original : 12-08-2013
 *
 */

#include "LFI/type.h"

extern character_len lfi_fstrlen (const character *, const character_len);
extern char * lfi_fstrdup (const character *, const character_len, char *);
extern int lfi_fstrcmp (const character *, const character *, character_len, character_len);
extern char * lfi_fdirname (const character *, const character_len);
extern int lfi_copy (const char *, const char *);
extern int lfi_fcopy (const character *, const character *, character_len, character_len);
extern int lfi_smartcopy (const char *, const char *, int);
extern int lfi_fsmartcopy (const character *, const character *, int, character_len, character_len);
extern int lfi_mkdir (const char *);
extern const char * lfi_cleanup_path (char *);
extern char * lfi_make_relative_path (const char *, char *);
extern int lfi_rmdir (const char *);
extern const char * lfi_dirname (const char *);

#define LFI_CSTR(cstr,fstr) \
  char _lfi_##cstr[fstr##_len+1]; \
  char * cstr = lfi_fstrdup (fstr, fstr##_len, _lfi_##cstr);

#endif


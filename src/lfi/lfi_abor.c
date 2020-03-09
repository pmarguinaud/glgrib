/**** *lfi_abor.c* - LFI abor routine
 *
 *    Author. 
 *    ------- 
 *     Philippe Marguinaud *METEO-FRANCE*
 *     Original : 12-08-2013
 *
 * Description :
 * Print a message and abort.
 */
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "lfi_abor.h"
#include "lfi_misc.h"

/* Variadic function to handle more complex messages */

void lfi_abor (const char * fmt, ...)
{
  va_list ap;
  int err = errno;
  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  va_end (ap);
  if (err > 0)
    {
      char * errstr = strerror (err);
      fprintf (stderr, " : %s\n", errstr);
    }
  else 
    {
      fprintf (stderr, "\n");
    }
    
  abort ();
}



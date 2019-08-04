/**** *lfi_verb.c* - Definition of LFI message functions
 *
 *    Author. 
 *    ------- 
 *     Philippe Marguinaud *METEO-FRANCE*
 *     Original : 12-08-2013
 *
 */


#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "lfi_type.h"
#include "lfi_abor.h"
#include "lfi_verb.h"
#include "lfi_hndl.h"


/* LFI error messages (from lfieng.F90);
 * the rank of each message is -KREP
 */

static const char * mess[] =
{
  NULL,
  "LOGICAL UNIT CURRENTLY NOT OPENED FOR THE SOFTWARE",
  "\"LEVEL\" VALUE OUTSIDE [0-2] RANGE",
  "BAD LOCK OPTION",
  "EXPLICIT CHANGE FOR MULTI-TASKING MODE",
  "LOGICAL UNIT IS CURRENTLY OPENED",
  "NOT ENOUGH SPACE WITHIN TABLES TO OPEN REQUESTED UNIT",
  "INVALID \"STATUS\" FOR FORTRAN INSTRUCTION \"OPEN\"",
  "INCOMPATIBLE VALUES GIVEN FOR \"LDNOMM\" AND \"CDSTTO\"",
  "INCOMPATIBILITY BETWEEN \"STATUS\" 'NEW' OR 'OLD' AND FILE EXISTENCE OR NON-EXISTENCE",
  "THE FILE IS NOT A LFI ONE, OR MAY NOT BE TREATED THROUGH THIS CONFIGURATION OR VERSION OF THE SOFTWARE",
  "FILE NOT CLOSED AFTER A MODIFICATION",
  "FILE HAS A \"STATUS\" 'OLD' BUT AN ERROR OCCURRED WHEN READING THE FIRST PHYSICAL RECORD OF FILE",
  "FILE IS ALREADY OPEN FOR ANOTHER LFI LOGICAL UNIT",
  "INCORRECT VALUE FOR INTEGER ARGUMENT",
  "INCORRECT CHARACTER ARGUMENT",
  "INCOHERENCE IN TABLES, FILE, INTERNAL CALLS, SOFTWARE",
  "TOO MANY LOGICAL RECORDS TO STORE AN EXTRA ONE",
  "A LOGICAL RECORD NAME FORMED ONLY WITH SPACES IS INVALID",
  "FILE OPENED WITH \"STATUS\" SET TO 'SCRATCH', SO MAY NOT BE KEPT AT CLOSE TIME",
  "NO LOGICAL RECORD WITH SUCH NAME FOUND WITHIN LOGICAL UNIT",
  "REQUESTED LOGICAL RECORD IS LONGER (HAS MORE DATA) IN FILE",
  "REQUESTED LOGICAL RECORD SHORTER (HAS LESS DATA) IN FILE",
  "NO OR NO MORE \"NEXT\" RECORD TO READ",
  "THE CHARACTER VARIABLE GIVEN AS ACTUAL OUTPUT ARGUMENT IS TOO SHORt TO STORE THE RECORD NAME",
  "THE NEW NAME OF THE LOGICAL RECORD IS (ALREADY) USED FOR ANOTHER LOGICAL RECORD WITHIN THE FILE",
  "NO OR NO MORE \"PREVIOUS\" LOGICAL RECORD TO READ",
  "INSUFFICIENT CONTIGUOUS SPACE WITHIN TABLES TO TREAT THE \"multiple\" FILE REQUESTED",
  "MULTIPLY FACTOR (OF ELEMENTARY PHYSICAL RECORD LENGTH) TOO BIG FOR THE CURRENT CONFIGURATION OF THE SOFTWARE",
  "NOT ENOUGH SPACE WITHIN TABLES TO STORE THE MULTIPLY FACTOR TO BE ASSOCIATED TO LOGICAL UNIT",
  "LOGICAL UNIT NUMBER INVALID FOR FORTRAN",
  "LOGICAL UNIT HAS NO MULTIPLY FACTOR PREDEFINED",
};

void lfi_mess_ (const integer64 * KUNIT, character * CDMESS, character_len CDMESS_len)
{
  fwrite (CDMESS, CDMESS_len, 1, stderr);
  fprintf (stderr, "\n");
}

static integer64 iulout (lfi_hndl_t * lfi)
{
  integer64 INIVAU, IULOUT = -1;
  if (lfi != NULL)
    lfi->cb->lfiomg (lfi->data, &INIVAU, &IULOUT);
  return IULOUT;
}


/* Variadic function to print LFI arguments */

void lfi_verb (lfi_hndl_t * lfi, const char * func, ...)
{
  typedef union
    {
      integer64 * K;
      logical * L;
      character * C; 
      real64 * P;
    }
  argv_t;
  typedef const char * argn_t;

  int narg = 0;
  argn_t argn;
  argv_t argv;
  character_len len;
  character_len CLMESS_len = 1024;
  character CLMESS[CLMESS_len];
  character * buf = CLMESS;
#define buf_len (CLMESS_len - (buf - CLMESS))
  integer64 IUNIT = iulout (lfi);

  va_list ap;

  va_start (ap, func);  

  while (1)
    {
      argn = va_arg (ap, argn_t);  
      if (argn == NULL)
        break;
      argv = va_arg (ap, argv_t);  

      if (narg == 0)
        {
          if (strcmp (argn, "KREP") == 0)
            if (*argv.K != 0)
              {
                buf += snprintf (buf, buf_len, " */*/* %s - KREP=%lld", func, *argv.K);
                if (*argv.K < 0)
                  {
                    int im = - *argv.K;
                    if ((0 < im) && (im < sizeof (mess)))
                      if (mess[im] != NULL)
                        buf += snprintf (buf, buf_len, ", %s", mess[im]);
                  }
                goto end;
              }
          buf += snprintf (buf, buf_len, " ///// %s - ", func);
        }

      if (narg > 0)
        buf += snprintf (buf, buf_len, ", ");

      buf += snprintf (buf, buf_len, "%s=", argn);

      switch (argn[0])
        {
          case 'C':
            len = va_arg (ap, character_len);
            for (; (len > 0) && (argv.C[len-1] == ' '); len--);
            buf += snprintf (buf, buf_len, "'%*.*s'", (int)len, (int)len, argv.C);
            break;
          case 'L':
            buf += snprintf (buf, buf_len, "%c", istrue (*argv.L) ? 'T' : 'F');
            break;
          case 'P':
            buf += snprintf (buf, buf_len, "%f", *argv.P);
            break;
          case 'K':
            buf += snprintf (buf, buf_len, "%lld", *argv.K);
            break;
          default:
            lfi_abor ("Unknown argument type : %s", argn); 
            break;
        }
      narg++;

      if (buf_len <= 0)
        break;

    }

end:

  if (IUNIT >= 0)
    {
      lfi_mess_ (&IUNIT, CLMESS, CLMESS_len - buf_len);
    }
  else
   {
     fwrite (CLMESS, CLMESS_len - buf_len, 1, stderr);
     fprintf (stderr, "\n");
   }
     
  va_end (ap);        
}



static void lfiouv_verb (int st, struct lfi_hndl_t * lfi, LFIOUV_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFIOUV", "KREP", KREP, "KNUMER", KNUMER, "KNBARI", KNBARI, NULL);
  else
    lfi_verb (lfi, "LFIOUV", "KNUMER", KNUMER, "LDNOMM", LDNOMM, "CDNOMF", CDNOMF, CDNOMF_len, "CDSTTO", CDSTTO, CDSTTO_len, "LDERFA", LDERFA, "LDIMST", LDIMST, "KNIMES", KNIMES, "KNBARP", KNBARP, NULL);
}


static void lfican_verb (int st, struct lfi_hndl_t * lfi, LFICAN_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFICAN", "KREP", KREP, "CDNOMA", CDNOMA, CDNOMA_len, NULL);
  else
    lfi_verb (lfi, "LFICAN", "KNUMER", KNUMER, "LDAVAN", LDAVAN, NULL);
}


static void lficas_verb (int st, struct lfi_hndl_t * lfi, LFICAS_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFICAS", "KREP", KREP, "CDNOMA", CDNOMA, CDNOMA_len, "KLONG", KLONG, "KPOSEX", KPOSEX, NULL);
  else
    lfi_verb (lfi, "LFICAS", "KNUMER", KNUMER, "LDAVAN", LDAVAN, NULL);
}


static void lfiecr_verb (int st, struct lfi_hndl_t * lfi, LFIECR_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFIECR", "KREP", KREP, NULL);
  else
    lfi_verb (lfi, "LFIECR", "KNUMER", KNUMER, "CDNOMA", CDNOMA, CDNOMA_len, "KLONG", KLONG, NULL);
}


static void lfifer_verb (int st, struct lfi_hndl_t * lfi, LFIFER_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFIFER", "KREP", KREP, NULL);
  else
    lfi_verb (lfi, "LFIFER", "KNUMER", KNUMER, "CDSTTC", CDSTTC, CDSTTC_len, NULL);
}


static void lfilec_verb (int st, struct lfi_hndl_t * lfi, LFILEC_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFILEC", "KREP", KREP, NULL);
  else
    lfi_verb (lfi, "LFILEC", "KNUMER", KNUMER, "CDNOMA", CDNOMA, CDNOMA_len, "KLONG", KLONG, NULL);
}


static void lfinfo_verb (int st, struct lfi_hndl_t * lfi, LFINFO_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFINFO", "KREP", KREP, "KLONG", KLONG, "KPOSEX", KPOSEX, NULL);
  else
    lfi_verb (lfi, "LFINFO", "KNUMER", KNUMER, "CDNOMA", CDNOMA, CDNOMA_len, NULL);
}


static void lfinff_verb (int st, struct lfi_hndl_t * lfi, LFINFF_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFINFF", "KREP", KREP, "CDNOMA", CDNOMA, CDNOMA_len, "CDNOMF", CDNOMF, CDNOMF_len, NULL);
  else
    lfi_verb (lfi, "LFINFF", "KNUMER", KNUMER, "CDNOMA", CDNOMA, CDNOMA_len, NULL);
}


static void lfipos_verb (int st, struct lfi_hndl_t * lfi, LFIPOS_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFIPOS", "KREP", KREP, NULL);
  else
    lfi_verb (lfi, "LFIPOS", "KNUMER", KNUMER, NULL);
}


static void lfiver_verb (int st, struct lfi_hndl_t * lfi, LFIVER_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFIVER", NULL);
  else
    lfi_verb (lfi, "LFIVER", "PVEROU", PVEROU, "CDSENS", CDSENS, CDSENS_len, NULL);
}


static void lfiofm_verb (int st, struct lfi_hndl_t * lfi, LFIOFM_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFIOFM", "KREP", KREP, "KFACTM", KFACTM, "LDOUVR", LDOUVR, NULL);
  else
    lfi_verb (lfi, "LFIOFM", "KNUMER", KNUMER, NULL);
}


static void lfineg_verb (int st, struct lfi_hndl_t * lfi, LFINEG_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFINEG", NULL);
  else
    lfi_verb (lfi, "LFINEG", "KNIVAU", KNIVAU, NULL);
}


static void lfilaf_verb (int st, struct lfi_hndl_t * lfi, LFILAF_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFILAF", "KREP", KREP, NULL);
  else
    lfi_verb (lfi, "LFILAF", "KNUMER", KNUMER, "LDTOUT", LDTOUT, NULL);
}


static void lfiosg_verb (int st, struct lfi_hndl_t * lfi, LFIOSG_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFIOSG", "KNIVAU", KNIVAU, NULL);
  else
    lfi_verb (lfi, "LFIOSG", NULL);
}


static void lfinum_verb (int st, struct lfi_hndl_t * lfi, LFINUM_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFINUM", "KRANG", KRANG, NULL);
  else
    lfi_verb (lfi, "LFINUM", "KNUMER", KNUMER, NULL);
}


static void lfisup_verb (int st, struct lfi_hndl_t * lfi, LFISUP_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFISUP", "KREP", KREP, "KLONUT", KLONUT, NULL);
  else
    lfi_verb (lfi, "LFISUP", "KNUMER", KNUMER, "CDNOMA", CDNOMA, CDNOMA_len, NULL);
}


static void lfiopt_verb (int st, struct lfi_hndl_t * lfi, LFIOPT_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFIOPT", "KREP", KREP, "LDNOMM", LDNOMM, "CDNOMF", CDNOMF, CDNOMF_len, "CDSTTO", CDSTTO, CDSTTO_len, "LDERFA", LDERFA, "LDIMST", LDIMST, "KNIMES", KNIMES, NULL);
  else
    lfi_verb (lfi, "LFIOPT", "KNUMER", KNUMER, NULL);
}


static void lfinmg_verb (int st, struct lfi_hndl_t * lfi, LFINMG_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFINMG", NULL);
  else
    lfi_verb (lfi, "LFINMG", "KNIVAU", KNIVAU, "KULOUT", KULOUT, NULL);
}


static void lficap_verb (int st, struct lfi_hndl_t * lfi, LFICAP_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFICAP", "KREP", KREP, "CDNOMA", CDNOMA, CDNOMA_len, "KLONG", KLONG, "KPOSEX", KPOSEX, NULL);
  else
    lfi_verb (lfi, "LFICAP", "KNUMER", KNUMER, "LDRECU", LDRECU, NULL);
}


static void lfifra_verb (int st, struct lfi_hndl_t * lfi, LFIFRA_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFIFRA", NULL);
  else
    lfi_verb (lfi, "LFIFRA", "LDFRAN", LDFRAN, NULL);
}


static void lficfg_verb (int st, struct lfi_hndl_t * lfi, LFICFG_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFICFG", NULL);
  else
    lfi_verb (lfi, "LFICFG", NULL);
}


static void lfierf_verb (int st, struct lfi_hndl_t * lfi, LFIERF_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFIERF", "KREP", KREP, NULL);
  else
    lfi_verb (lfi, "LFIERF", "KNUMER", KNUMER, "LDERFA", LDERFA, NULL);
}


static void lfilas_verb (int st, struct lfi_hndl_t * lfi, LFILAS_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFILAS", "KREP", KREP, "CDNOMA", CDNOMA, CDNOMA_len, NULL);
  else
    lfi_verb (lfi, "LFILAS", "KNUMER", KNUMER, "KLONG", KLONG, NULL);
}


static void lfiren_verb (int st, struct lfi_hndl_t * lfi, LFIREN_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFIREN", "KREP", KREP, NULL);
  else
    lfi_verb (lfi, "LFIREN", "KNUMER", KNUMER, "CDNOM1", CDNOM1, CDNOM1_len, "CDNOM2", CDNOM2, CDNOM2_len, NULL);
}


static void lfiini_verb (int st, struct lfi_hndl_t * lfi, LFIINI_ARGS_DECL)
{
}


static void lfipxf_verb (int st, struct lfi_hndl_t * lfi, LFIPXF_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFIPXF", "KREP", KREP, "CDNOMA", CDNOMA, CDNOMA_len, "KLONG", KLONG, NULL);
  else
    lfi_verb (lfi, "LFIPXF", "KNUMER", KNUMER, "KNUMEX", KNUMEX, "CDCFGX", CDCFGX, CDCFGX_len, "KLAREX", KLAREX, "KXCNEX", KXCNEX, "KFACEX", KFACEX, "KNUTRA", KNUTRA, NULL);
}


static void lfioeg_verb (int st, struct lfi_hndl_t * lfi, LFIOEG_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFIOEG", "KNIVAU", KNIVAU, NULL);
  else
    lfi_verb (lfi, "LFIOEG", NULL);
}


static void lfinaf_verb (int st, struct lfi_hndl_t * lfi, LFINAF_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFINAF", "KREP", KREP, "KNALDO", KNALDO, "KNTROU", KNTROU, "KNARES", KNARES, "KNAMAX", KNAMAX, NULL);
  else
    lfi_verb (lfi, "LFINAF", "KNUMER", KNUMER, NULL);
}


static void lfiofd_verb (int st, struct lfi_hndl_t * lfi, LFIOFD_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFIOFD", "KFACMD", KFACMD, NULL);
  else
    lfi_verb (lfi, "LFIOFD", NULL);
}


static void lfiomf_verb (int st, struct lfi_hndl_t * lfi, LFIOMF_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFIOMF", "KREP", KREP, "KNIMES", KNIMES, NULL);
  else
    lfi_verb (lfi, "LFIOMF", "KNUMER", KNUMER, NULL);
}


static void lfiafm_verb (int st, struct lfi_hndl_t * lfi, LFIAFM_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFIAFM", "KREP", KREP, NULL);
  else
    lfi_verb (lfi, "LFIAFM", "KNUMER", KNUMER, "KFACTM", KFACTM, NULL);
}


static void lfista_verb (int st, struct lfi_hndl_t * lfi, LFISTA_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFISTA", "KREP", KREP, NULL);
  else
    lfi_verb (lfi, "LFISTA", "KNUMER", KNUMER, NULL);
}


static void lfiosf_verb (int st, struct lfi_hndl_t * lfi, LFIOSF_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFIOSF", "KREP", KREP, "LDIMST", LDIMST, NULL);
  else
    lfi_verb (lfi, "LFIOSF", "KNUMER", KNUMER, NULL);
}


static void lfilap_verb (int st, struct lfi_hndl_t * lfi, LFILAP_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFILAP", "KREP", KREP, "CDNOMA", CDNOMA, CDNOMA_len, NULL);
  else
    lfi_verb (lfi, "LFILAP", "KNUMER", KNUMER, "KLONG", KLONG, NULL);
}


static void lfioef_verb (int st, struct lfi_hndl_t * lfi, LFIOEF_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFIOEF", "KREP", KREP, "LDERFA", LDERFA, NULL);
  else
    lfi_verb (lfi, "LFIOEF", "KNUMER", KNUMER, NULL);
}


static void lfimst_verb (int st, struct lfi_hndl_t * lfi, LFIMST_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFIMST", "KREP", KREP, NULL);
  else
    lfi_verb (lfi, "LFIMST", "KNUMER", KNUMER, "LDIMST", LDIMST, NULL);
}


static void lfinim_verb (int st, struct lfi_hndl_t * lfi, LFINIM_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFINIM", "KREP", KREP, NULL);
  else
    lfi_verb (lfi, "LFINIM", "KNUMER", KNUMER, "KNIMES", KNIMES, NULL);
}


static void lfisfm_verb (int st, struct lfi_hndl_t * lfi, LFISFM_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFISFM", "KREP", KREP, NULL);
  else
    lfi_verb (lfi, "LFISFM", "KNUMER", KNUMER, NULL);
}


static void lfinsg_verb (int st, struct lfi_hndl_t * lfi, LFINSG_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFINSG", NULL);
  else
    lfi_verb (lfi, "LFINSG", "KNIVAU", KNIVAU, NULL);
}


static void lfideb_verb (int st, struct lfi_hndl_t * lfi, LFIDEB_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFIDEB", NULL);
  else
    lfi_verb (lfi, "LFIDEB", "LDEBUG", LDEBUG, NULL);
}


static void lfiomg_verb (int st, struct lfi_hndl_t * lfi, LFIOMG_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFIOMG", "KNIVAU", KNIVAU, "KULOUT", KULOUT, NULL);
  else
    lfi_verb (lfi, "LFIOMG", NULL);
}


static void lfifmd_verb (int st, struct lfi_hndl_t * lfi, LFIFMD_ARGS_DECL)
{
  if (st)
    lfi_verb (lfi, "LFIFMD", NULL);
  else
    lfi_verb (lfi, "LFIFMD", "KFACMD", KFACMD, NULL);
}


/* lficb_verb definition */

lficb_verb_t lficb_verb = 
{
  lfiouv_verb,
  lfican_verb,
  lficas_verb,
  lfiecr_verb,
  lfifer_verb,
  lfilec_verb,
  lfinfo_verb,
  lfinff_verb,
  lfipos_verb,
  lfiver_verb,
  lfiofm_verb,
  lfineg_verb,
  lfilaf_verb,
  lfiosg_verb,
  lfinum_verb,
  lfisup_verb,
  lfiopt_verb,
  lfinmg_verb,
  lficap_verb,
  lfifra_verb,
  lficfg_verb,
  lfierf_verb,
  lfilas_verb,
  lfiren_verb,
  lfiini_verb,
  lfipxf_verb,
  lfioeg_verb,
  lfinaf_verb,
  lfiofd_verb,
  lfiomf_verb,
  lfiafm_verb,
  lfista_verb,
  lfiosf_verb,
  lfilap_verb,
  lfioef_verb,
  lfimst_verb,
  lfinim_verb,
  lfisfm_verb,
  lfinsg_verb,
  lfideb_verb,
  lfiomg_verb,
  lfifmd_verb,
};

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "LFI/fort.h"
#include "LFI/type.h"
#include "LFI/args.h"

extern void lfiouv_mt64_ (LFIOUV_ARGS_DECL);
extern void lfifer_mt64_ (LFIFER_ARGS_DECL);
extern void lfilec_mt64_ (LFILEC_ARGS_DECL);
extern void lfinfo_mt64_ (LFINFO_ARGS_DECL);

int main (int argc, char * argv[])
{
  const char * file = argv[1];
  const char * name = argv[2];


// ftp://giedi-prime//home/phi001/3d/glgrib/testdata/fasplit/ZFLD2.fa SFX.ZS

  lficom_t lficomm;
  void * LFI = &lficomm;
  integer64 IREP, INUMER = 77, INIMES = 0, INBARP = 0, INBARI = 0,
            ILONG = 0, IPOSEX = 0;
  logical LLNOMM = fort_TRUE, LLERFA = fort_TRUE, LLIMST = fort_FALSE;
  character * CLNOMF = (character*)file, * CLSTTO = (character*)"OLD", 
            * CLSTTC = (character*)"KEEP", * CLNOMA = (character*)name; 
  character_len CLNOMF_len = strlen (file), CLSTTO_len = 3, CLSTTC_len = 4,  
                CLNOMA_len = strlen (name);
  integer64 * ITAB = NULL;
  

  strncpy (lficomm.cmagic, "LFI_FORT", 8); 
  lficomm.lfihl = NULL;

  lfiouv_mt64_ (LFI, &IREP, &INUMER, &LLNOMM, CLNOMF, CLSTTO, &LLERFA, &LLIMST, 
                &INIMES, &INBARP, &INBARI, CLNOMF_len, CLSTTO_len);

  lfinfo_mt64_ (LFI, &IREP, &INUMER, CLNOMA, &ILONG, &IPOSEX, CLNOMA_len);

  printf (" ILONG = %lld\n", ILONG);

  ITAB = (integer64 *)malloc (ILONG * sizeof (integer64));

  lfilec_mt64_ (LFI, &IREP, &INUMER, CLNOMA, ITAB, &ILONG, CLNOMA_len);

  lfifer_mt64_ (LFI, &IREP, &INUMER, CLSTTC, CLSTTC_len);

  return 0;
}



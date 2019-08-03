#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lfi_grok.h"
#include "lfi_type.h"
#include "lfi_args.h"
#include "lfi_fort.h"

extern void lfiouv_mt64_ (LFIOUV_ARGS_DECL);
extern void lfinff_mt64_ (LFINFF_ARGS_DECL);
extern void lfifer_mt64_ (LFIFER_ARGS_DECL);


int main (int argc, char * argv[])
{
  char * file = argv[1];
  char * name = argv[2];

  lfi_grok_t type = lfi_grok (file, strlen (file));

  switch (type)
    {
      case LFI_NONE:
      case LFI_UNKN:
        return 1;
    }

  lficom_t lficomm;
  void * LFI = &lficomm;
  integer64 IREP, INUMER = 77, INIMES = 0, INBARP = 0, INBARI = 0;
  logical LLNOMM = fort_TRUE, LLERFA = fort_TRUE, LLIMST = fort_FALSE;
  character * CLNOMF = (character*)file, * CLSTTO = (character*)"OLD", 
            * CLSTTC = (character*)"KEEP";
  character_len CLNOMF_len = strlen (file), CLSTTO_len = 3, CLSTTC_len = 4;

  character_len CLNOMA_len = 16;
  character_len CLNOMG_len = 512;
  character CLNOMG[CLNOMG_len+1];
  character CLNOMA[CLNOMA_len+1];

  strncpy (CLNOMA, name, CLNOMA_len);
  CLNOMA_len = strlen (CLNOMA);

  printf (" CLNOMA = %s\n", CLNOMA);
  
  strncpy (lficomm.cmagic, "LFI_FORT", 8);
  lficomm.lfihl = NULL;
  
  lfiouv_mt64_ (LFI, &IREP, &INUMER, &LLNOMM, CLNOMF, CLSTTO, &LLERFA, &LLIMST, 
        	&INIMES, &INBARP, &INBARI, CLNOMF_len, CLSTTO_len);
  
  printf (" IREP = %lld\n", IREP);

  lfinff_mt64_ (LFI, &IREP, &INUMER, CLNOMA, CLNOMG, CLNOMA_len, CLNOMG_len);

  printf (" IREP = %lld, CLNOMA = %s, CLNOMG = %s\n", IREP, CLNOMA, CLNOMG);
  
  lfifer_mt64_ (LFI, &IREP, &INUMER, CLSTTC, CLSTTC_len);

  printf (" IREP = %lld\n", IREP);

  return 0;
}


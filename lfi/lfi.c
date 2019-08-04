#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "lfi_grok.h"
#include "lfi_type.h"
#include "lfi_args.h"
#include "lfi_fort.h"

extern void lfiouv_mt64_ (LFIOUV_ARGS_DECL);
extern void lfinff_mt64_ (LFINFF_ARGS_DECL);
extern void lfipos_mt64_ (LFIPOS_ARGS_DECL);
extern void lficas_mt64_ (LFICAS_ARGS_DECL);
extern void lfican_mt64_ (LFICAS_ARGS_DECL);
extern void lfinaf_mt64_ (LFINAF_ARGS_DECL);
extern void lfifer_mt64_ (LFIFER_ARGS_DECL);


int test_lfinff (int argc, char * argv[])
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

typedef struct write_hdr_t
{
  FILE * fp;
  integer64 dat[22];
  int off;
} write_hdr_t;


static size_t write_hdr (char * ptr, size_t size, size_t nmemb, void * userdata)
{
  write_hdr_t * hdr = userdata;
  fwrite (ptr, size, nmemb, hdr->fp);
  memcpy (&hdr->dat[hdr->off], ptr, size * nmemb);
  hdr->off += size * nmemb;
  return size * nmemb;
}

static size_t write_idx (char * ptr, size_t size, size_t nmemb, void * userdata)
{
  FILE * fp = userdata;
  fwrite (ptr, size, nmemb, fp);
  return size * nmemb;
}

int test_curl (const char * url)
{
  CURLcode ret;

  // Create CURL handle
  CURL * hnd = curl_easy_init();
  curl_easy_setopt (hnd, CURLOPT_BUFFERSIZE, 102400L);
  curl_easy_setopt (hnd, CURLOPT_URL, url);
  curl_easy_setopt (hnd, CURLOPT_NETRC, (long)CURL_NETRC_REQUIRED);
  curl_easy_setopt (hnd, CURLOPT_USERAGENT, "curl/7.58.0");
  curl_easy_setopt (hnd, CURLOPT_MAXREDIRS, 50L);
  curl_easy_setopt (hnd, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
  curl_easy_setopt (hnd, CURLOPT_TCP_KEEPALIVE, 1L);

  // Get first 22*8 characters of remote file
  
  const char * file = "header";
  write_hdr_t hdr;
  memset (&hdr, 0, sizeof (hdr));
  hdr.fp = fopen (file, "w");

  curl_easy_setopt (hnd, CURLOPT_RANGE, "0-175");
  curl_easy_setopt (hnd, CURLOPT_WRITEFUNCTION, write_hdr);
  curl_easy_setopt (hnd, CURLOPT_WRITEDATA, &hdr);

  ret = curl_easy_perform (hnd);

  fclose (hdr.fp);

  if (ret != 0)
    goto end;


  lfi_grok_t type = lfi_grok (file, strlen (file));
  
  printf (" type = %d\n", type);
  switch (type)
    {
      case LFI_NONE:
      case LFI_UNKN:
      case LFI_ALTM:
        goto end;
    }

  // Index size is written in first 8-byte word
  {
    int t = 8, n = 1;
    iswap_ (&hdr.dat[0], &hdr.dat[0], &t, &n);
  }

  // Get full index
  char range[32];
  sprintf (range, "0-%lld", 3*8*hdr.dat[0]-1);
  curl_easy_setopt (hnd, CURLOPT_RANGE, range);
  curl_easy_setopt (hnd, CURLOPT_WRITEFUNCTION, write_idx);

  FILE * fp = fopen ("header", "w");
  curl_easy_setopt (hnd, CURLOPT_WRITEDATA, fp);
  ret = curl_easy_perform (hnd);
  fclose (fp);

  if (ret != 0)
    goto end;


  {
    lficom_t lficomm;
    void * LFI = &lficomm;
    integer64 IREP, INUMER = 77, INIMES = 0, INBARP = 0, INBARI = 0,
              INALDO, INTROU, INARES, INAMAX;
    logical LLNOMM = fort_TRUE, LLERFA = fort_TRUE, LLIMST = fort_FALSE;
    character * CLNOMF = (character*)file, * CLSTTO = (character*)"OLD", 
              * CLSTTC = (character*)"KEEP";
    character_len CLNOMF_len = strlen (file), CLSTTO_len = 3, CLSTTC_len = 4;
   
    character_len CLNOMA_len = 16;
    character CLNOMA[CLNOMA_len+1];
   
    strncpy (lficomm.cmagic, "LFI_FORT", 8);
    lficomm.lfihl = NULL;
    
    lfiouv_mt64_ (LFI, &IREP, &INUMER, &LLNOMM, CLNOMF, CLSTTO, &LLERFA, &LLIMST, 
          	&INIMES, &INBARP, &INBARI, CLNOMF_len, CLSTTO_len);
    
    lfinaf_mt64_ (LFI, &IREP, &INUMER, &INALDO, &INTROU, &INARES, &INAMAX);

    lfipos_mt64_ (LFI, &IREP, &INUMER);
    for (int i = 0; i < INALDO; i++)
      {
        integer64 ILONGD, IPOSEX; 
        logical LLAVAN = fort_TRUE;
        lficas_mt64_ (LFI, &IREP, &INUMER, CLNOMA, &ILONGD, &IPOSEX, &LLAVAN, CLNOMA_len);
        CLNOMA[CLNOMA_len] = '\0';
        printf (" IPOSEX = %10lld, ILONGD = %10lld, CLNOMA = >%s<\n", IPOSEX, ILONGD, CLNOMA);
      }

   
    lfifer_mt64_ (LFI, &IREP, &INUMER, CLSTTC, CLSTTC_len);
  }

end:
  if (hnd != NULL)
    curl_easy_cleanup (hnd);
  hnd = NULL;
  printf (" ret = %d\n", ret);
  return (int)ret;
}

int main (int argc, char * argv[])
{
  test_curl (argv[1]);
  return 0;
}



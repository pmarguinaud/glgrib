#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <sqlite3.h>

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

time_t filetime (const char * url)
{
  time_t t = 0;
  long filetime = -1;
  CURL * curl = curl_easy_init ();
  curl_easy_setopt (curl, CURLOPT_URL, url);
  curl_easy_setopt (curl, CURLOPT_FILETIME, 1L);
  curl_easy_setopt (curl, CURLOPT_NOBODY, 1L);
  curl_easy_setopt (curl, CURLOPT_NETRC, (long)CURL_NETRC_REQUIRED);
  curl_easy_setopt (curl, CURLOPT_BUFFERSIZE, 102400L);
  curl_easy_setopt (curl, CURLOPT_TCP_KEEPALIVE, 1L);
  curl_easy_setopt (curl, CURLOPT_HEADER, 0L);
  curl_easy_setopt (curl, CURLOPT_MAXREDIRS, 50L);
  if (curl_easy_perform (curl) == CURLE_OK)
    if ((curl_easy_getinfo (curl, CURLINFO_FILETIME, &filetime) == CURLE_OK ) && (filetime >= 0)) 
      t = (time_t)filetime;
  curl_easy_cleanup (curl);
  return t;
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


  int rc;
  sqlite3 * db = NULL;
  sqlite3_stmt * ins = NULL, * sel = NULL;

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


#define TRY(expr) do { if ((rc = expr) != SQLITE_OK) goto end; } while (0)

    TRY (sqlite3_open ("lfi.db", &db));
    TRY (sqlite3_exec (db, "BEGIN", 0, 0, 0));

    TRY (sqlite3_prepare_v2 (db, "SELECT INUMER FROM URL_TO_NUMER WHERE URL = ?;", -1, &sel, 0));
    TRY (sqlite3_bind_text (sel, 1, url, strlen (url), NULL));

    TRY (sqlite3_prepare_v2 (db, "INSERT INTO URL_TO_NUMER (URL) VALUES (?);", -1, &ins, 0));
    TRY (sqlite3_bind_text (ins, 1, url, strlen (url), NULL));

    int INUM;
    if ((rc = sqlite3_step (sel)) == SQLITE_ROW)
      {
        INUM = sqlite3_column_int (sel, 0);
        sqlite3_stmt * del = NULL;
        TRY (sqlite3_prepare_v2 (db, "DELETE FROM LFI_PURE_INDEX WHERE INUMER = ?;", -1, &del, 0));
        TRY (sqlite3_bind_int (del, 1, INUM));
        if ((rc = sqlite3_step (del)) != SQLITE_DONE)
          {
            printf ("%s", sqlite3_errmsg (db));
            goto end;
          }
        TRY (sqlite3_finalize (del));
        del = NULL;
      } 
    else
      {
        if ((rc = sqlite3_step (ins)) != SQLITE_DONE)
          {
            printf ("%s", sqlite3_errmsg (db));
            goto end;
          }
        if ((rc = sqlite3_step (sel)) == SQLITE_ROW)
          INUM = sqlite3_column_int (sel, 0);
        else
          goto end;
      }

    printf (" INUM = %d\n", INUM);

    TRY (sqlite3_finalize (sel));
    sel = NULL;
    TRY (sqlite3_finalize (ins));
    ins = NULL;


    TRY (sqlite3_prepare_v2 (db, "INSERT INTO LFI_PURE_INDEX VALUES (?, ?, ?, ?);", -1, &ins, 0));

    printf (" INALDO = %lld\n", INALDO);

    for (int i = 0; i < INALDO; i++)
      {
        integer64 ILONGD, IPOSEX; 
        logical LLAVAN = fort_TRUE;
        lficas_mt64_ (LFI, &IREP, &INUMER, CLNOMA, &ILONGD, &IPOSEX, &LLAVAN, CLNOMA_len);
        CLNOMA[CLNOMA_len] = '\0';
        TRY (sqlite3_bind_int  (ins, 1, INUM));
        TRY (sqlite3_bind_text (ins, 2, CLNOMA, strlen (CLNOMA), NULL));
        TRY (sqlite3_bind_int  (ins, 3, IPOSEX));
        TRY (sqlite3_bind_int  (ins, 4, ILONGD));
        if ((rc = sqlite3_step (ins)) != SQLITE_DONE)
          {
            printf ("%s", sqlite3_errmsg (db));
            goto end;
          }
        printf (" IPOSEX = %10lld, ILONGD = %10lld, CLNOMA = >%s<\n", IPOSEX, ILONGD, CLNOMA);
        sqlite3_reset (ins);
      }
    TRY (sqlite3_finalize (ins));
    ins = NULL;

    TRY (sqlite3_exec (db, "COMMIT", 0, 0, 0));

   
    lfifer_mt64_ (LFI, &IREP, &INUMER, CLSTTC, CLSTTC_len);
  }

end:

  if (rc != SQLITE_OK)
    printf ("%s\n", sqlite3_errmsg (db));
  if (ins != NULL)
    sqlite3_finalize (ins);
  if (sel != NULL)
    sqlite3_finalize (sel);
  if (db != NULL)
    sqlite3_close (db);
  if (hnd != NULL)
    curl_easy_cleanup (hnd);
  hnd = NULL;
  printf (" ret = %d\n", ret);
  return (int)ret;
}

int main (int argc, char * argv[])
{
  printf ("%ld\n", filetime (argv[1]));
//test_curl (argv[1]);
  return 0;
}



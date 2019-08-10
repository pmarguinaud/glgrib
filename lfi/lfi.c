#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <sqlite3.h>
#include <errno.h>

#include "lfi_grok.h"
#include "lfi_type.h"
#include "lfi_args.h"
#include "lfi_fort.h"

extern void lfiouv_mt64_ (LFIOUV_ARGS_DECL);
extern void lfinff_mt64_ (LFINFF_ARGS_DECL);
extern void lfipos_mt64_ (LFIPOS_ARGS_DECL);
extern void lficas_mt64_ (LFICAS_ARGS_DECL);
extern void lfican_mt64_ (LFICAN_ARGS_DECL);
extern void lfinaf_mt64_ (LFINAF_ARGS_DECL);
extern void lfifer_mt64_ (LFIFER_ARGS_DECL);
extern void lfinff_mt64_ (LFINFF_ARGS_DECL);
extern void iswap_ (void *, void *, const int *, const int *);

CURL * get_curl (CURL * hnd)
{
  if (hnd != NULL)
    return hnd;
  // Create CURL handle
  hnd = curl_easy_init();
  curl_easy_setopt (hnd, CURLOPT_BUFFERSIZE, 102400L);
  curl_easy_setopt (hnd, CURLOPT_NETRC, (long)CURL_NETRC_REQUIRED);
  curl_easy_setopt (hnd, CURLOPT_USERAGENT, "curl/7.58.0");
  curl_easy_setopt (hnd, CURLOPT_MAXREDIRS, 50L);
  curl_easy_setopt (hnd, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
  curl_easy_setopt (hnd, CURLOPT_TCP_KEEPALIVE, 1L);
  return hnd;
}


typedef struct
{
  sqlite3 * db;
  CURL * hnd;
  int status;
  char mess[1024];
} lfinetw_ctx_t;


char * parse_protohost (const char * url, char * protohost, int protohost_len)
{
  char * ph = protohost;
  int len = protohost_len;

  if (len < 16)
    return NULL;

  if (strncmp ("ftp://", url, 6) == 0)
    {
      strcpy (ph, "ftp://");
      len -= 6; ph += 6; url += 6;
    }
  else if (strncmp ("http://", url, 7) == 0)
    {
      strcpy (ph, "http://");
      len -= 7; ph += 7; url += 7;
    }
  else
    {
      return NULL;
    }

  for (int i = 0; ; i++)
    if (url[i] == '\0')
      return NULL;
    else if (url[i] == '/')
      {
        if (len < i + 2)
          return NULL;
        strncpy (ph, url, i+1);
        ph[i+1] = '\0';
        return protohost;
      }
  
  return NULL;
}

static size_t discard (void * ptr, size_t size, size_t nmemb, void * data)
{
  return (size_t)(size * nmemb);
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
  curl_easy_setopt (curl, CURLOPT_VERBOSE, 0L); 
  curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, discard);
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
  size_t off;
} write_hdr_t;


static size_t write_hdr (char * ptr, size_t size, size_t nmemb, void * userdata)
{
  write_hdr_t * hdr = userdata;
  fwrite (ptr, size, nmemb, hdr->fp);
  memcpy ((char *)hdr->dat + hdr->off, ptr, size * nmemb);
  hdr->off += size * nmemb;
  return size * nmemb;
}

typedef struct write_dat_t
{
  char * ptr;
  size_t off;
} write_dat_t;


static size_t write_dat (char * ptr, size_t size, size_t nmemb, void * userdata)
{
  write_dat_t * dat = userdata;
  memcpy ((char *)dat->ptr + dat->off, ptr, size * nmemb);
  dat->off += size * nmemb;
  return size * nmemb;
}

#define TRY(expr) do { if ((rc = expr) != SQLITE_OK) goto end; } while (0)


int new_LFI_MULT (const char * url, const char * file, lfinetw_ctx_t * ctx,
                  write_hdr_t * hdr, time_t ft, int * pIRANK)
{
  int IRANK = *pIRANK;
  CURLcode ret = 0;
  int rc = SQLITE_OK;

  sqlite3_stmt * sel = NULL, * ins = NULL, * inr = NULL;

  // Get full index
  FILE * fp = fopen (file, "w");
  if (fp == NULL)
    goto end;

  ctx->hnd = get_curl (ctx->hnd);
  curl_easy_setopt (ctx->hnd, CURLOPT_URL, url);
  curl_easy_setopt (ctx->hnd, CURLOPT_WRITEFUNCTION, fwrite);
  curl_easy_setopt (ctx->hnd, CURLOPT_WRITEDATA, fp);
  
  ret = curl_easy_perform (ctx->hnd);

  fclose (fp);
  
  curl_easy_setopt (ctx->hnd, CURLOPT_URL, NULL);
  curl_easy_setopt (ctx->hnd, CURLOPT_WRITEFUNCTION, NULL);
  curl_easy_setopt (ctx->hnd, CURLOPT_WRITEDATA, NULL);
  
  if (ret != 0)
    goto end;
  

  // Parse index & feed database
  {
    lficom_t lficomm;
    void * LFI = &lficomm;
    integer64 IREP, INUMER = 77, INIMES = 0, INBARP = 0, INBARI = 0,
              INALDO, INTROU, INARES, INAMAX;
    logical LLNOMM = fort_TRUE, LLERFA = fort_TRUE, LLIMST = fort_FALSE;
    character * CLNOMF = (character*)file, * CLSTTO = (character*)"OLD", 
              * CLSTTC = (character*)"KEEP";
    character_len CLNOMF_len = strlen (file), CLSTTO_len = 3, CLSTTC_len = 4;
   
    strncpy (lficomm.cmagic, "LFI_FORT", 8);
    lficomm.lfihl = NULL;
    
    lfiouv_mt64_ (LFI, &IREP, &INUMER, &LLNOMM, CLNOMF, CLSTTO, &LLERFA, &LLIMST, 
          	      &INIMES, &INBARP, &INBARI, CLNOMF_len, CLSTTO_len);
    
    lfinaf_mt64_ (LFI, &IREP, &INUMER, &INALDO, &INTROU, &INARES, &INAMAX);
  
    lfipos_mt64_ (LFI, &IREP, &INUMER);

    TRY (sqlite3_exec (ctx->db, "BEGIN", 0, 0, 0));
  
    // Create file
    TRY (sqlite3_prepare_v2 (ctx->db, "INSERT INTO URL_TO_NUMER "
         "(CLURL, ITIME, IKIND) VALUES (?, ?, ?);", -1, &ins, 0));
    TRY (sqlite3_bind_text  (ins, 1, url, strlen (url), NULL));
    TRY (sqlite3_bind_int64 (ins, 2, ft));
    TRY (sqlite3_bind_int   (ins, 3, 2));
  
    if ((rc = sqlite3_step (ins)) != SQLITE_DONE)
      goto end;

    TRY (sqlite3_finalize (ins));
    ins = NULL;
  
    // Get file ID
    TRY (sqlite3_prepare_v2 (ctx->db, "SELECT IRANK FROM URL_TO_NUMER WHERE CLURL = ?;", -1, &sel, 0));
    TRY (sqlite3_bind_text (sel, 1, url, strlen (url), NULL));
  
    if ((rc = sqlite3_step (sel)) == SQLITE_ROW)
      IRANK = sqlite3_column_int (sel, 0);
    else
      goto end;
  
    TRY (sqlite3_finalize (sel));
    sel = NULL;
  
  
    // Create index
    TRY (sqlite3_prepare_v2 (ctx->db, "INSERT INTO URL_TO_NUMER (CLURL, ITIME, IKIND) VALUES (?, 0, ?);", -1, &ins, 0));
    TRY (sqlite3_prepare_v2 (ctx->db, "INSERT INTO LFI_MULT (IRANK, CLNOMA, CLNOMB, JRANK) "
                            "VALUES (?, ?, ?, ?);", -1, &inr, 0));
    TRY (sqlite3_prepare_v2 (ctx->db, "SELECT IRANK FROM URL_TO_NUMER WHERE CLURL = ?;", -1, &sel, 0));
  
    int len = strlen (url);

    character_len PROTOHOST_len = len;
    character PROTOHOST[PROTOHOST_len+1];

    parse_protohost (url, PROTOHOST, PROTOHOST_len);
    PROTOHOST_len = strlen (PROTOHOST);

    character_len CLNOMH_len = 256;
    character CLNOMH[CLNOMH_len+1];
    CLNOMH[0] = '\0';
    int JRANK = -1;

    for (int i = 0; i < INALDO; i++)
      {
        logical LLAVAN = fort_TRUE;
        character_len CLNOMG_len = 256;
        character CLNOMG[CLNOMG_len+1];
        character_len CLNOMA_len = 16;
        character CLNOMA[CLNOMA_len+1];
        character_len CLNOMB_len = 16;
        character CLNOMB[CLNOMB_len+1];
   
        strcpy (CLNOMG, PROTOHOST);
 
        lfican_mt64_ (LFI, &IREP, &INUMER, CLNOMA, &LLAVAN, CLNOMA_len);
        CLNOMA[CLNOMA_len] = '\0';

        strcpy (CLNOMB, CLNOMA);

        lfinff_mt64_ (LFI, &IREP, &INUMER, CLNOMB, &CLNOMG[PROTOHOST_len], CLNOMB_len, CLNOMG_len - PROTOHOST_len);
        CLNOMA[CLNOMB_len] = '\0';
 
        for (int i = CLNOMG_len-1; i >= 0; i--)
          if (CLNOMG[i] == ' ')
            CLNOMG[i] = '\0';
          else
            break;

       // LFI pure file has changed
       if (strcmp (CLNOMG, CLNOMH) != 0)
         {
           strcpy (CLNOMH, CLNOMG);

           while (1)
             {
               // Try to get JRANK
               TRY (sqlite3_bind_text (sel, 1, CLNOMG, strlen (CLNOMG), NULL));
               rc = sqlite3_step (sel);
               if (rc == SQLITE_ROW)
                 {
                   JRANK = sqlite3_column_int (sel, 0);
                   break;
                 }
               else if (rc != SQLITE_DONE)
                 goto end;
               TRY (sqlite3_reset (sel));

               // JRANK was not found; insert CLNOMG in database
               TRY (sqlite3_bind_text (ins, 1, CLNOMG, strlen (CLNOMG), NULL));
               TRY (sqlite3_bind_int  (ins, 2, -1));
               if ((rc = sqlite3_step (ins)) != SQLITE_DONE)
                 goto end;
               
               TRY (sqlite3_reset (ins));

             }
           TRY (sqlite3_reset (sel));
         }

         
         TRY (sqlite3_bind_int  (inr, 1, IRANK));
         TRY (sqlite3_bind_text (inr, 2, CLNOMA, strlen (CLNOMA), NULL));
         TRY (sqlite3_bind_text (inr, 3, CLNOMB, strlen (CLNOMB), NULL));
         TRY (sqlite3_bind_int  (inr, 4, JRANK));
         if ((rc = sqlite3_step (inr)) != SQLITE_DONE)
           goto end;
         TRY (sqlite3_reset (inr));
         

      }

    TRY (sqlite3_finalize (ins));
    ins = NULL;
  
    TRY (sqlite3_finalize (inr));
    inr = NULL;
  
    TRY (sqlite3_finalize (sel));
    sel = NULL;
  
    TRY (sqlite3_exec (ctx->db, "COMMIT", 0, 0, 0));
  
    lfifer_mt64_ (LFI, &IREP, &INUMER, CLSTTC, CLSTTC_len);
  }

end:

  *pIRANK = IRANK;

  int r = 0;

  if (rc != SQLITE_OK)
    {
      strncpy (ctx->mess, sqlite3_errmsg (ctx->db), sizeof (ctx->mess)-1); 
      ctx->status = -1;
    }
  if (ret != 0)
    {
      strncpy (ctx->mess, curl_easy_strerror (ret), sizeof (ctx->mess)-1);
      ctx->status = -1;
    }
  if (inr != NULL)
    sqlite3_finalize (inr);
  if (ins != NULL)
    sqlite3_finalize (ins);
  if (sel != NULL)
    sqlite3_finalize (sel);

  return ctx->status;
}

int new_LFI_PURE (const char * url, const char * file, lfinetw_ctx_t * ctx,
                  write_hdr_t * hdr, time_t ft, int * pIRANK)
{
  int IRANK = *pIRANK;
  CURLcode ret = 0;
  int rc = SQLITE_OK;
  sqlite3_stmt * sel = NULL, * ins = NULL;

  // Index size is written in first 8-byte word
  {
    int t = 8, n = 1;
    iswap_ (&hdr->dat[0], &hdr->dat[0], &t, &n);
  }
  
  // Get full index
  char range[32];
  FILE * fp = fopen (file, "w");
  if (fp == NULL)
    {
      ctx->status = -1;
      goto end;
    }

  sprintf (range, "0-%lld", 3*8*hdr->dat[0]-1);

  ctx->hnd = get_curl (ctx->hnd);
  curl_easy_setopt (ctx->hnd, CURLOPT_URL, url);
  curl_easy_setopt (ctx->hnd, CURLOPT_RANGE, range);
  curl_easy_setopt (ctx->hnd, CURLOPT_WRITEFUNCTION, fwrite);
  curl_easy_setopt (ctx->hnd, CURLOPT_WRITEDATA, fp);
  
  // We should look at the header to see whether file has extra indexes, and get them too
  
  ret = curl_easy_perform (ctx->hnd);

  fclose (fp);
  
  curl_easy_setopt (ctx->hnd, CURLOPT_URL, NULL);
  curl_easy_setopt (ctx->hnd, CURLOPT_RANGE, NULL);
  curl_easy_setopt (ctx->hnd, CURLOPT_WRITEFUNCTION, NULL);
  curl_easy_setopt (ctx->hnd, CURLOPT_WRITEDATA, NULL);
  
  if (ret != 0)
    goto end;
  
  
  // Parse index & feed database
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
  
    TRY (sqlite3_exec (ctx->db, "BEGIN", 0, 0, 0));
  
    // Update or create file
    if (IRANK < 0)
      {
        TRY (sqlite3_prepare_v2 (ctx->db, "INSERT INTO URL_TO_NUMER "
             "(CLURL, ITIME, IKIND) VALUES (?, ?, ?);", -1, &ins, 0));
        TRY (sqlite3_bind_text  (ins, 1, url, strlen (url), NULL));
        TRY (sqlite3_bind_int64 (ins, 2, ft));
        TRY (sqlite3_bind_int   (ins, 3, 1));
      }
    else
      {
        TRY (sqlite3_prepare_v2 (ctx->db, "UPDATE URL_TO_NUMER "
             "SET ITIME = ?, IKIND = ? WHERE IRANK = ?;", -1, &ins, 0));
        TRY (sqlite3_bind_int64 (ins, 1, ft));
        TRY (sqlite3_bind_int   (ins, 2, 1));
        TRY (sqlite3_bind_int   (ins, 3, IRANK));
      }
  
    if ((rc = sqlite3_step (ins)) != SQLITE_DONE)
      goto end;

    TRY (sqlite3_finalize (ins));
    ins = NULL;
  
    // If the file was created, get its id
    if (IRANK < 0)
      {
        TRY (sqlite3_prepare_v2 (ctx->db, "SELECT IRANK FROM URL_TO_NUMER WHERE CLURL = ?;", -1, &sel, 0));
        TRY (sqlite3_bind_text (sel, 1, url, strlen (url), NULL));
      
        if ((rc = sqlite3_step (sel)) == SQLITE_ROW)
          IRANK = sqlite3_column_int (sel, 0);
        else
          goto end;
      
        TRY (sqlite3_finalize (sel));
        sel = NULL;
      }

  
    // Insert positions and sizes
    TRY (sqlite3_prepare_v2 (ctx->db, "INSERT INTO LFI_PURE (IRANK, CLNOMA, IPOSEX, ILONGD) "
                             "VALUES (?, ?, ?, ?);", -1, &ins, 0));
  
    for (int i = 0; i < INALDO; i++)
      {
        integer64 ILONGD, IPOSEX; 
        logical LLAVAN = fort_TRUE;
        lficas_mt64_ (LFI, &IREP, &INUMER, CLNOMA, &ILONGD, &IPOSEX, &LLAVAN, CLNOMA_len);
        CLNOMA[CLNOMA_len] = '\0';
        TRY (sqlite3_bind_int  (ins, 1, IRANK));
        TRY (sqlite3_bind_text (ins, 2, CLNOMA, strlen (CLNOMA), NULL));
        TRY (sqlite3_bind_int64  (ins, 3, IPOSEX));
        TRY (sqlite3_bind_int64  (ins, 4, ILONGD));
        if ((rc = sqlite3_step (ins)) != SQLITE_DONE)
          goto end;
        TRY (sqlite3_reset (ins));
      }
    TRY (sqlite3_finalize (ins));
    ins = NULL;
  
    TRY (sqlite3_exec (ctx->db, "COMMIT", 0, 0, 0));
  
  
    lfifer_mt64_ (LFI, &IREP, &INUMER, CLSTTC, CLSTTC_len);
  }

end:

  *pIRANK = IRANK;

  if (rc != SQLITE_OK)
    {
      strncpy (ctx->mess, sqlite3_errmsg (ctx->db), sizeof (ctx->mess)-1); 
      ctx->status = -1;
    }

  if (ret != 0)
    {
      strncpy (ctx->mess, curl_easy_strerror (ret), sizeof (ctx->mess)-1);
      ctx->status = -1; 
    }

  if (ins != NULL)
    sqlite3_finalize (ins);
  if (sel != NULL)
    sqlite3_finalize (sel);

  return ctx->status;
}


// Remove a LFI PURE file from the database
int purge_LFI_PURE (lfinetw_ctx_t * ctx, int IRANK, int intr)
{
  int rc = SQLITE_OK;
  sqlite3_stmt * del = NULL;

  if (! intr)
    TRY (sqlite3_exec (ctx->db, "BEGIN", 0, 0, 0));

  TRY (sqlite3_prepare_v2 (ctx->db, "DELETE FROM LFI_PURE WHERE IRANK = ?;", -1, &del, 0));
  TRY (sqlite3_bind_int (del, 1, IRANK));
  if ((rc = sqlite3_step (del)) != SQLITE_DONE)
    goto end;
  TRY (sqlite3_finalize (del));
  del = NULL;
  
  TRY (sqlite3_prepare_v2 (ctx->db, "DELETE FROM URL_TO_NUMER WHERE IRANK = ?;", -1, &del, 0));
  TRY (sqlite3_bind_int (del, 1, IRANK));
  if ((rc = sqlite3_step (del)) != SQLITE_DONE)
    goto end;
  TRY (sqlite3_finalize (del));
  del = NULL;
  
  if (! intr)
    TRY (sqlite3_exec (ctx->db, "COMMIT", 0, 0, 0));

end:

  if (rc != SQLITE_OK)
    {
      strncpy (ctx->mess, sqlite3_errmsg (ctx->db), sizeof (ctx->mess)-1); 
      ctx->status = -1;
    }
  if (del != NULL)
    sqlite3_finalize (del);

  return ctx->status;
}

int purge_LFI_MULT (lfinetw_ctx_t * ctx, int IRANK)
{
  int rc = SQLITE_OK;
  sqlite3_stmt * del = NULL, * sel = NULL;

  TRY (sqlite3_exec (ctx->db, "BEGIN", 0, 0, 0));

  TRY (sqlite3_prepare_v2 (ctx->db, "SELECT JRANK FROM LFI_MULT WHERE IRANK = ?;", -1, &sel, 0));
  TRY (sqlite3_bind_int (sel, 1, IRANK));
  while ((rc = sqlite3_step (sel)) == SQLITE_ROW)
    {
      int JRANK = sqlite3_column_int (sel, 0);
      if (purge_LFI_PURE (ctx, JRANK, 1) < 0)
        goto end;
    }
  if (rc != SQLITE_DONE)
    goto end;
  TRY (sqlite3_finalize (sel));
  sel = NULL;
  
  TRY (sqlite3_prepare_v2 (ctx->db, "DELETE FROM LFI_MULT WHERE IRANK = ?;", -1, &del, 0));
  TRY (sqlite3_bind_int (del, 1, IRANK));
  if ((rc = sqlite3_step (del)) != SQLITE_DONE)
    goto end;
  TRY (sqlite3_finalize (del));
  del = NULL;
  
  TRY (sqlite3_prepare_v2 (ctx->db, "DELETE FROM URL_TO_NUMER WHERE IRANK = ?;", -1, &del, 0));
  TRY (sqlite3_bind_int (del, 1, IRANK));
  if ((rc = sqlite3_step (del)) != SQLITE_DONE)
    goto end;
  TRY (sqlite3_finalize (del));
  del = NULL;
  
  TRY (sqlite3_exec (ctx->db, "COMMIT", 0, 0, 0));

end:

  if (rc != SQLITE_OK)
    {
      strncpy (ctx->mess, sqlite3_errmsg (ctx->db), sizeof (ctx->mess)-1); 
      ctx->status = -1;
    }
  if (sel != NULL)
    sqlite3_finalize (sel);
  if (del != NULL)
    sqlite3_finalize (del);

  return ctx->status;
}

int check_url_validity (const char * url, time_t ft, lfinetw_ctx_t * ctx, int * pIRANK, int * pIKIND)
{
  int IRANK = -1;
  int rc = SQLITE_OK;

  sqlite3_stmt * sel = NULL;

  // Check timestamp & purge if outdated
  {
    TRY (sqlite3_prepare_v2 (ctx->db, "SELECT IRANK, ITIME, IKIND FROM URL_TO_NUMER WHERE CLURL = ?;", -1, &sel, 0));
    TRY (sqlite3_bind_text (sel, 1, url, strlen (url), NULL));
    rc = sqlite3_step (sel);
    switch (rc)
      {
        case SQLITE_DONE:
            rc = SQLITE_OK;
          break;
        case SQLITE_ROW:
          {
            rc = SQLITE_OK;
            IRANK      = sqlite3_column_int   (sel, 0);
            time_t ft1 = sqlite3_column_int64 (sel, 1);
            int IKIND  = sqlite3_column_int   (sel, 2);
            *pIKIND = IKIND;
            if (ft != ft1) 
              {
                if (IKIND == 1)
                  {
                    if (purge_LFI_PURE (ctx, IRANK, 0) < 0)
                      goto end;
                  }
                else if (IKIND == 2)
                  {
                    if (purge_LFI_MULT (ctx, IRANK) < 0)
                      goto end;
                  }
                else if (IKIND == -1)
                  {
                    goto end;
                  }
                IRANK = -1;
              }
          }
          break;
        default:
          goto end;
      }
    TRY (sqlite3_finalize (sel));
    sel = NULL;
  }

end:

  if (rc != SQLITE_OK)
    {
      strncpy (ctx->mess, sqlite3_errmsg (ctx->db), sizeof (ctx->mess)-1); 
      ctx->status = -1;
    }
  if (sel != NULL)
    sqlite3_finalize (sel);

  *pIRANK = IRANK;

  return ctx->status;;
}

int lfilec_netw (const char *, const char *, lfinetw_ctx_t *, char *, integer64 *);

int lfilec_netw_MULT (lfinetw_ctx_t * ctx, const char * url, const char * name, 
                      int IRANK, char * pDATA, integer64 * pSIZE)
{
  int rc = SQLITE_OK;

  sqlite3_stmt * sel = NULL;

  character_len CLNOMA_len = 16, CLNOMB_len = 16, CLNOMG_len = 256;
  character CLNOMA[CLNOMA_len+1], CLNOMB[CLNOMB_len+1], CLNOMG[CLNOMG_len+1];

  strncpy (CLNOMA, name, CLNOMA_len);
  CLNOMA[CLNOMA_len] = '\0';
  for (int i = strlen (name); i < CLNOMA_len; i++)
    CLNOMA[i] = ' ';

  TRY (sqlite3_prepare_v2 (ctx->db, "SELECT CLNOMB, URL_TO_NUMER.CLURL FROM LFI_MULT "
                                    "INNER JOIN URL_TO_NUMER WHERE LFI_MULT.IRANK = ? AND "
                                    "LFI_MULT.CLNOMA = ? AND LFI_MULT.JRANK = URL_TO_NUMER.IRANK;"
                           , -1, &sel, 0));

  TRY (sqlite3_bind_int  (sel, 1, IRANK));
  TRY (sqlite3_bind_text (sel, 2, CLNOMA, CLNOMA_len, NULL));

  rc = sqlite3_step (sel);

  if (rc == SQLITE_ROW)
    {
      const char * clnomb = (const char *)sqlite3_column_text (sel, 0);  
      const char * clnomg = (const char *)sqlite3_column_text (sel, 1);  
      strncpy (CLNOMB, clnomb, CLNOMB_len);
      strncpy (CLNOMG, clnomg, CLNOMG_len);
      CLNOMB[CLNOMB_len] = '\0';
      CLNOMG[CLNOMG_len] = '\0';
      if (lfilec_netw (CLNOMG, CLNOMB, ctx, pDATA, pSIZE) < 0)
        goto end;
    }
  else if (rc == SQLITE_DONE)
    {
      ctx->status = -2;
    }
  else
    {
      goto end;
    }

  TRY (sqlite3_finalize (sel));
  sel = NULL;

end:

  if (rc != SQLITE_OK)
    {
      strncpy (ctx->mess, sqlite3_errmsg (ctx->db), sizeof (ctx->mess)-1); 
      ctx->status = -1;
    }
  if (sel != NULL)
    sqlite3_finalize (sel);

  return ctx->status;
}

// Get an article from a LFI PURE file, retrieve it from the remote host if necessary
int lfilec_netw_PURE (lfinetw_ctx_t * ctx, const char * url, const char * name, 
                      int IRANK, char * pDATA, integer64 * pSIZE)
{
  int rc = SQLITE_OK;
  int ret = 0;

  sqlite3_stmt * sel = NULL, * upd = NULL;

  char CLNOMA[17];
  TRY (sqlite3_prepare_v2 (ctx->db, "SELECT IPOSEX, ILONGD, KDATA FROM LFI_PURE "
                           "WHERE IRANK = ? AND CLNOMA = ?;", -1, &sel, 0));
  TRY (sqlite3_bind_int (sel, 1, IRANK));

  memset (CLNOMA, 0, 17);
  strncpy (CLNOMA, name, 16);
  for (int i = strlen (name); i < 16; i++)
    CLNOMA[i] = ' ';
  
  TRY (sqlite3_bind_text (sel, 2, CLNOMA, strlen (CLNOMA), NULL));

  rc = sqlite3_step (sel);

  if (rc == SQLITE_ROW)
    {
      rc = SQLITE_OK;

      ssize_t IPOSEX     = sqlite3_column_int64 (sel, 0);
      ssize_t ILONGD     = sqlite3_column_int64 (sel, 1);
      const void * KDATA = sqlite3_column_blob  (sel, 2);
      int KDATA_size     = sqlite3_column_bytes (sel, 2);

      if (KDATA_size == 0)
        {
          size_t offset = (IPOSEX-1) * 8, length = ILONGD * 8;
          char range[64];
          write_dat_t dat;
          memset (&dat, 0, sizeof (dat));
          dat.ptr = (char *)malloc (length);

          sprintf (range, "%ld-%ld", offset, offset + length - 1);

          ctx->hnd = get_curl (ctx->hnd);

          curl_easy_setopt (ctx->hnd, CURLOPT_URL, url);
          curl_easy_setopt (ctx->hnd, CURLOPT_RANGE, range);
          curl_easy_setopt (ctx->hnd, CURLOPT_WRITEFUNCTION, write_dat);
          curl_easy_setopt (ctx->hnd, CURLOPT_WRITEDATA, &dat);

          
          ret = curl_easy_perform (ctx->hnd);

          curl_easy_setopt (ctx->hnd, CURLOPT_URL, NULL);
          curl_easy_setopt (ctx->hnd, CURLOPT_RANGE, NULL);
          curl_easy_setopt (ctx->hnd, CURLOPT_WRITEFUNCTION, NULL);
          curl_easy_setopt (ctx->hnd, CURLOPT_WRITEDATA, NULL);
           

          if (ret != 0)
            goto end;

          {
            int t = 8, n = ILONGD;
            iswap_ (dat.ptr, dat.ptr, &t, &n);
          }

          TRY (sqlite3_prepare_v2 (ctx->db, "UPDATE LFI_PURE SET KDATA = ? "
                                   "WHERE IRANK = ? AND CLNOMA = ?;", -1, &upd, 0));

          TRY (sqlite3_bind_blob64 (upd, 1, dat.ptr, length, NULL));
          TRY (sqlite3_bind_int (upd, 2, IRANK));
          TRY (sqlite3_bind_text (upd, 3, CLNOMA, strlen (CLNOMA), NULL));

          if ((rc = sqlite3_step (upd)) != SQLITE_DONE)
            goto end;

          TRY (sqlite3_finalize (upd));
          upd = NULL;

          free (dat.ptr);

        }

      if ((pDATA != NULL) && (pSIZE != NULL))
        memcpy (pDATA, KDATA, 8 * (*pSIZE < ILONGD ? *pSIZE : ILONGD));
      else if (pSIZE != NULL)
        *pSIZE = ILONGD;



    }
  else if (rc = SQLITE_DONE)
    {
      rc = SQLITE_OK;
      ctx->status = -2;
    }
  else
    {
      goto end;
    }

end:

  if (rc != SQLITE_OK)
    {
      strncpy (ctx->mess, sqlite3_errmsg (ctx->db), sizeof (ctx->mess)-1); 
      ctx->status = -1;
    }
  if (ret != 0)
    {
      strncpy (ctx->mess, curl_easy_strerror (ret), sizeof (ctx->mess)-1);
      ctx->status = -1;
    }
  if (upd != NULL)
    sqlite3_finalize (upd);
  if (sel != NULL)
    sqlite3_finalize (sel);

  return ctx->status;
}

int lfilec_netw (const char * url, const char * name, lfinetw_ctx_t * ctx, char * pDATA, integer64 * pSIZE)
{
  CURLcode ret = 0;
  int rc = SQLITE_OK;
  time_t ft = filetime (url);

  int IRANK, IKIND;

  if (check_url_validity (url, ft, ctx, &IRANK, &IKIND) < 0)
    goto end;

  if (((IRANK == -1) && (ft > 0)) // File is missing from database but exists on remote server
      || (IKIND == -1))           // Or we have a pure LFI file inserted as a placeholder for
                                  // a MULT file
    {
      // Create CURL handle
      ctx->hnd = get_curl (ctx->hnd);
     
      // Get first 22*8 characters of remote file
      
      write_hdr_t hdr;
      memset (&hdr, 0, sizeof (hdr));
     
      char file[64];
      strcpy (file, "/tmp/lfi.XXXXXX");
      int fd = mkstemp (file);
      if (fd < 0)
        {
          int len = sizeof (ctx->mess), off = 0;
          strncpy (ctx->mess + off, "Cannot create temporary file: ", len-1);
          off += strlen (ctx->mess); len -= strlen (ctx->mess);
          strncpy (ctx->mess + off, strerror (errno), len-1);
          ctx->status = -1;
          goto end;
        }

      hdr.fp = fdopen (fd, "w");

      if (hdr.fp == NULL) 
        {
          sprintf (ctx->mess, "Cannot open file `%s' for writing\n", file);
          ctx->status = -1;
          goto end;
        }
     
      curl_easy_setopt (ctx->hnd, CURLOPT_URL, url);
      curl_easy_setopt (ctx->hnd, CURLOPT_RANGE, "0-175");
      curl_easy_setopt (ctx->hnd, CURLOPT_WRITEFUNCTION, write_hdr);
      curl_easy_setopt (ctx->hnd, CURLOPT_WRITEDATA, &hdr);
     
      ret = curl_easy_perform (ctx->hnd);
     
      fclose (hdr.fp);
     
      curl_easy_setopt (ctx->hnd, CURLOPT_URL, NULL);
      curl_easy_setopt (ctx->hnd, CURLOPT_RANGE, NULL);
      curl_easy_setopt (ctx->hnd, CURLOPT_WRITEFUNCTION, NULL);
      curl_easy_setopt (ctx->hnd, CURLOPT_WRITEDATA, NULL);
     
      if (ret != 0)
        goto end;

      lfi_grok_t type = lfi_grok (file, strlen (file));
      
      switch (type)
        {
          case LFI_NONE:
          case LFI_UNKN:
            goto end;
          case LFI_ALTM:
            if (IRANK >= 0) // Should not happen
              goto end;
            IKIND = 2;
            if (new_LFI_MULT (url, file, ctx, &hdr, ft, &IRANK) < 0)
              goto end;
            break;
          case LFI_PURE:
            IKIND = +1;
            if (new_LFI_PURE (url, file, ctx, &hdr, ft, &IRANK) < 0)
              goto end;
            break;
        }

    }

  switch (IKIND)
    {
      case +1:
        if (lfilec_netw_PURE (ctx, url, name, IRANK, pDATA, pSIZE) < 0)
          goto end;
        break;
      case +2:
        if (lfilec_netw_MULT (ctx, url, name, IRANK, pDATA, pSIZE) < 0)
          goto end;
        break;
    }

end:

  if (rc != SQLITE_OK)
    {
      strncpy (ctx->mess, sqlite3_errmsg (ctx->db), sizeof (ctx->mess)-1); 
      ctx->status = -1;
    }
  if (ret != 0)
    {
      strncpy (ctx->mess, curl_easy_strerror (ret), sizeof (ctx->mess)-1);
      ctx->status = -1;
    }

  return ctx->status;
}

int lfinfo_netw (const char * url, const char * name, integer64 * IPOSEX, integer64 * ILONGD)
{

}

#undef TRY

int main (int argc, char * argv[])
{
  lfinetw_ctx_t _ctx, * ctx = &_ctx;
  memset (ctx, 0, sizeof (*ctx));

  if (sqlite3_open ("lfi.db", &ctx->db) != SQLITE_OK)
    return 1;

  integer64 ILONGD = 0;

  if (lfilec_netw (argv[1], argv[2], ctx, NULL, &ILONGD))
    {
      printf ("%s\n", ctx->mess);
    }

  printf (" ILONGD = %lld\n", ILONGD);

  if (ctx->db != NULL)
    sqlite3_close (ctx->db);
  ctx->db = NULL;
  if (ctx->hnd != NULL)
    curl_easy_cleanup (ctx->hnd);
  ctx->hnd = NULL;

  return -ctx->status;
}



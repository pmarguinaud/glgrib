#include "glgrib_dbase.h"

#include <sqlite3.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>


static std::string trim (const std::string & str)
{
  int i, j;
  for (i = 0; i < str.length (); i++)
    if (str[i] != ' ')
      break;
  for (j = str.length ()-1; j >= 0; j--)
    if (str[j] != ' ')
      break;
  if (i > j)
    return std::string ("");
  return str.substr (i, j-i+1);
}

bool glgrib_dbase::open (const std::string & path)
{
  fp = fopen ((path + ".dbf").c_str (), "r");
  if (fp == NULL)
    return false;
  header.read (fp);
  int n = (header.length - 1)/32 - 1;

  fields.resize (n);

  for (int i = 0; i < n; i++)
    {
      fields[i].read (fp);
      std::string name = std::string ((char *)fields[i].name);
      map.add (trim (name), i);
    }

  fseek (fp, header.length, SEEK_SET);

  return true;
}

bool glgrib_dbase::read (record_t * record)
{
  record->clear ();

  if (fp == NULL)
    return false;

  count++;
  record->map = &map;

  char tmp[header.record_length], * ptr = &tmp[0];
  _read (tmp, header.record_length, fp);
  for (int i = 0; i < fields.size (); i++)
    {
      std::string str (ptr, fields[i].length);
      record->push_back (trim (str));
      ptr += fields[i].length;
    }
  
  if (count == header.count)
    {
      fclose (fp);
      fp = NULL;
      count = 0;
    }

  return true;
}

void glgrib_dbase::convert2sqlite (const std::string & path)
{
  struct stat st;
  if (stat ((path + ".db").c_str (), &st) == 0)
    return;

  open (path);

  char * errmsg;
  record_t record;
  std::string sql;
  sqlite3 * db = NULL;
  sqlite3_stmt * req = NULL;
  int rc;

#define TRY(expr) do { if ((rc = expr) != SQLITE_OK) goto end; } while (0)

  TRY (sqlite3_open ((path + ".db").c_str (), &db));
 
  sql = "CREATE TABLE dbase (";
  for (int i = 0; i < fields.size (); i++)
    {
      char tmp[256];
      sprintf (tmp, "%s VARCHAR (%d)", fields[i].name, fields[i].length);
      sql += tmp;
      if (i != fields.size () - 1)
        sql += ", ";
    }
  sql += ")";

  if (sqlite3_exec (db, sql.c_str (), NULL, NULL, &errmsg) != SQLITE_OK)
    {
      throw std::runtime_error (std::string ("Cannot create table ") + std::string (errmsg));
      goto end;
    }

  TRY (sqlite3_exec (db, "BEGIN", 0, 0, 0));

  sql = "INSERT INTO dbase VALUES (";
  for (int i = 0; i < fields.size (); i++)
    {
      sql += "?";
      if (i != fields.size () - 1)
        sql += ", ";
    }
  sql += ")";

  TRY (sqlite3_prepare_v2 (db, sql.c_str (), -1, &req, 0));

  while (read (&record))
    {
      for (int i = 0; i < fields.size (); i++)
        TRY (sqlite3_bind_text  (req, i+1, record[i].c_str (), record[i].length (), NULL));
      if ((rc = sqlite3_step (req)) != SQLITE_DONE)
        goto end;
      TRY (sqlite3_reset (req));
    }

  TRY (sqlite3_exec (db, "COMMIT", 0, 0, 0));

  close ();

#undef TRY

end:

  if (rc != SQLITE_OK)
    throw std::runtime_error (std::string (sqlite3_errmsg (db)));

  if (req != NULL)
    sqlite3_finalize (req);
  if (db != NULL)
    sqlite3_close (db);

}



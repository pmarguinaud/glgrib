#include "glgrib_dbase.h"
#include "glgrib_sqlite.h"

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
  if (fp == nullptr)
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

  if (fp == nullptr)
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
      fp = nullptr;
      count = 0;
    }

  return true;
}

void glgrib_dbase::convert2sqlite (const std::string & path)
{
  struct stat sta;
  if (stat ((path + ".db").c_str (), &sta) == 0)
    return;

  open (path);

  glgrib_sqlite db (path + ".db");
  std::string sql;
  record_t record;

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

  db.execute (sql);

  db.execute ("BEGIN");

  sql = "INSERT INTO dbase VALUES (";
  for (int i = 0; i < fields.size (); i++)
    {
      sql += "?";
      if (i != fields.size () - 1)
        sql += ", ";
    }
  sql += ")";

  glgrib_sqlite::stmt st = db.prepare (sql);

  while (read (&record))
    {
      for (int i = 0; i < fields.size (); i++)
        st.bind (i, &record[i]);
      st.execute ();
      st.reset ();
    }

  db.execute ("COMMIT");

  close ();

}



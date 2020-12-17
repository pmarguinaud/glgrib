#include "glGribDBase.h"
#include "glGribSQLite.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include <fstream>

namespace glGrib
{

namespace
{
std::string trim (const std::string & str)
{
  size_t i, j;
  if (str.length () == 0)
    return str;
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
}

bool DBase::open (const std::string & path)
{
  fh.open (path + ".dbf", std::ios::in | std::ifstream::binary);
  if (! fh)
    return false;
  header.read (fh);
  int n = (header.length - 1)/32 - 1;

  fields.resize (n);

  for (int i = 0; i < n; i++)
    {
      fields[i].read (fh);
      std::string name = std::string ((char *)fields[i].name);
      map.add (trim (name), i);
    }

  fh.seekg (header.length);

  return true;
}

bool DBase::read (record_t * record)
{
  record->clear ();

  if (! fh.is_open ())
    return false;

  count++;

  record->map = &map;

  char tmp[header.record_length], * ptr = &tmp[0];
  _read (tmp, header.record_length, fh);
  for (size_t i = 0; i < fields.size (); i++)
    {
      std::string str (ptr, fields[i].length);
      record->push_back (trim (str));
      ptr += fields[i].length;
    }
  
  if (static_cast<uint32_t> (count) == header.count)
    {
      fh.close ();
      count = 0;
    }

  return true;
}

void DBase::convert2sqlite (const std::string & path)
{
  struct stat sta;
  if (stat ((path + ".db").c_str (), &sta) == 0)
    return;

  open (path);

  SQLite db (path + ".db");
  std::string sql;
  record_t record;

  sql = "CREATE TABLE dbase (";
  for (size_t i = 0; i < fields.size (); i++)
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
  for (size_t i = 0; i < fields.size (); i++)
    {
      sql += "?";
      if (i != fields.size () - 1)
        sql += ", ";
    }
  sql += ")";

  SQLite::stmt st = db.prepare (sql);

  while (read (&record))
    {
      for (size_t i = 0; i < fields.size (); i++)
        st.bind (i, &record[i]);
      st.execute ();
      st.reset ();
    }

  db.execute ("COMMIT");

  close ();

}

}

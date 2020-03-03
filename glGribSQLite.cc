#include "glGribSQLite.h"

void glGribSQLiteDetail::ok (sqlite3_ptr db, int rc)
{
  if (rc != SQLITE_OK)
    throw std::runtime_error (std::string (sqlite3_errmsg (db->data)));
}

void glGribSQLiteDetail::iset (sqlite3_ptr db, sqlite3_stmt_ptr req, int rank, const long int * t) 
{
  ok (db, sqlite3_bind_int (req->data, 1 + rank, *t));
}

void glGribSQLiteDetail::iset (sqlite3_ptr db, sqlite3_stmt_ptr req, int rank, const int * t) 
{
  ok (db, sqlite3_bind_int (req->data, 1 + rank, *t));
}

void glGribSQLiteDetail::iset (sqlite3_ptr db, sqlite3_stmt_ptr req, int rank, const float * t) 
{
  ok (db, sqlite3_bind_double (req->data, 1 + rank, *t));
}

void glGribSQLiteDetail::iset (sqlite3_ptr db, sqlite3_stmt_ptr req, int rank, const char * t) 
{
  ok (db, sqlite3_bind_text (req->data, 1 + rank, t, strlen (t), nullptr));
}

void glGribSQLiteDetail::iset (sqlite3_ptr db, sqlite3_stmt_ptr req, int rank, const std::string * t) 
{
  ok (db, sqlite3_bind_text (req->data, 1 + rank, t->c_str (), strlen (t->c_str ()), nullptr));
}

void glGribSQLiteDetail::isetList (sqlite3_ptr, sqlite3_stmt_ptr, int) {}
 
void glGribSQLiteDetail::oget (sqlite3_ptr db, sqlite3_stmt_ptr req, int rank, int * t)
{
  *t = sqlite3_column_int (req->data, rank);
}

void glGribSQLiteDetail::oget (sqlite3_ptr db, sqlite3_stmt_ptr req, int rank, float * t)
{
  *t = sqlite3_column_double (req->data, rank);
}

void glGribSQLiteDetail::oget (sqlite3_ptr db, sqlite3_stmt_ptr req, int rank, char * t)
{
  const char * str = (const char *)sqlite3_column_text (req->data, rank);
  strcpy (t, str);
}

void glGribSQLiteDetail::oget (sqlite3_ptr db, sqlite3_stmt_ptr req, int rank, std::string * t)
{
  const char * str = (const char *)sqlite3_column_text (req->data, rank);
  t->clear ();
  t->resize (strlen (str) + 1);
  strcpy (&(*t)[0], str);
}

void glGribSQLiteDetail::ogetList (sqlite3_ptr, sqlite3_stmt_ptr, int) {}



#ifndef _GLGRIB_SQLITE_H
#define _GLGRIB_SQLITE_H

#include <sqlite3.h>
#include <string>
#include <iostream>
#include <list>
#include <string.h>
#include <exception>


namespace glgrib_sqlite_detail
{
  static void ok (sqlite3 * db, int rc)
  {
    std::cout << " rc = " << rc << std::endl;
    if (rc != SQLITE_OK)
      throw std::runtime_error (std::string (sqlite3_errmsg (db)));
  }

  void iset (sqlite3 * db, sqlite3_stmt * req, int rank, int t) 
  {
    ok (db, sqlite3_bind_int (req, 1 + rank, t));
  }

  void iset (sqlite3 * db, sqlite3_stmt * req, int rank, float t) 
  {
    ok (db, sqlite3_bind_double (req, 1 + rank, t));
  }

  void iset (sqlite3 * db, sqlite3_stmt * req, int rank, char * t) 
  {
    const char * s = (const char *)t;
    ok (db, sqlite3_bind_text (req, 1 + rank, s, strlen (s), NULL));
  }

  void iset (sqlite3 * db, sqlite3_stmt * req, int rank, std::string t) 
  {
    std::cout << " t = " << t << std::endl;
    ok (db, sqlite3_bind_text (req, 1 + rank, t.c_str (), strlen (t.c_str ()), NULL));
  }

  static void iset_list (sqlite3 *, sqlite3_stmt *, int) {}
  template <typename T, typename... Types>
  static void iset_list (sqlite3 * db, sqlite3_stmt * req, int rank, T t, Types... args)
  {
    iset (db, req, rank, t);
    iset_list (db, req, rank + 1, args...);
  }

  void oget (sqlite3 * db, sqlite3_stmt * req, int rank, int * t)
  {
    *t = sqlite3_column_int (req, rank);
  }

  void oget (sqlite3 * db, sqlite3_stmt * req, int rank, float * t)
  {
    *t = sqlite3_column_double (req, rank);
  }

  void oget (sqlite3 * db, sqlite3_stmt * req, int rank, char * t)
  {
    const char * str = (const char *)sqlite3_column_text (req, rank);
    strcpy (t, str);
  }

  void oget (sqlite3 * db, sqlite3_stmt * req, int rank, std::string * t)
  {
    const char * str = (const char *)sqlite3_column_text (req, rank);
    std::cout << " str = " << str << std::endl;
    t->resize (strlen (str) + 1);
    strcpy (&(*t)[0], str);
  }

  static void oget_list (sqlite3 *, sqlite3_stmt *, int) {}
  template <typename T, typename... Types>
  static void oget_list (sqlite3 * db, sqlite3_stmt * req, int rank, T t, Types... args)
  {
    if (rank >= sqlite3_column_count (req))
      throw std::runtime_error (std::string ("Column out of bounds"));
    oget (db, req, rank, t);
    oget_list (db, req, rank + 1, args...);
  }

};

class glgrib_sqlite
{
public:

  glgrib_sqlite (const std::string & file)
  {
    int rc = sqlite3_open (file.c_str (), &db);
  }

  class stmt
  {
  public:
    stmt ()
    {
    }
    ~stmt ()
    {
      sqlite3_finalize (req);
    }
  protected:
    sqlite3_stmt * req = NULL;
    void execute (sqlite3 * db)
    {
      std::cout << "void execute ();" << std::endl;
    }
    template <typename... Types>
    void execute (sqlite3 * db, Types... args)
    {
      std::cout << "template <typename... Types> void execute ();" << std::endl;
      glgrib_sqlite_detail::iset_list (db, req, 0, args...);
    }
    template <typename... Types>
    bool fetch_row (sqlite3 * db, Types... args)
    {
      int rc = sqlite3_step (req);
      switch (rc)
        {
          case SQLITE_ROW:
            glgrib_sqlite_detail::oget_list (db, req, 0, args...);
            return true;
	  case SQLITE_DONE:
	    return false;
	}
      return false;
    }
    friend class glgrib_sqlite;
  private:
  };

  void execute (stmt * st)
  {
    st->execute (db);
  }

  template <typename... Types>
  void execute (stmt * st, Types... args)
  {
    st->execute (db, args...);
  }

  template <typename... Types>
  bool fetch_row (stmt * st, Types... args)
  {
    int rc;
    return st->fetch_row (db, args...);
  }

  stmt prepare (const std::string & sql)
  {
    stmt st;
    int rc = sqlite3_prepare_v2 (db, sql.c_str (), -1, &st.req, 0);
    return st;
  }

  ~glgrib_sqlite ()
  {
    close ();
  }

  void close ()
  {
    if (db != NULL)
      sqlite3_close (db);
    db = NULL;
  }

private:
  sqlite3 * db = NULL;
};


#endif

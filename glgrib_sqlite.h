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

  class stmt_ivar_base
  {
  public:
  virtual void bind (sqlite3 *, sqlite3_stmt *, int) = 0;
  };


  template <typename T>
  class stmt_ivar : public stmt_ivar_base
  {
  public:
  stmt_ivar (T _t) : t (_t) { }
  virtual void bind (sqlite3 *, sqlite3_stmt *, int) 
  {  
    std::cout << "template <typename T> void stmt_ivar::bind" << std::endl;
  }
  private:
  T t;
  };

  template <> void stmt_ivar<int*>::bind (sqlite3 * db, sqlite3_stmt * req, int rank) 
  {
    ok (db, sqlite3_bind_int (req, 1 + rank, *t));
  }

  template <> void stmt_ivar<float*>::bind (sqlite3 * db, sqlite3_stmt * req, int rank) 
  {
    ok (db, sqlite3_bind_double (req, 1 + rank, *t));
  }

  template <> void stmt_ivar<char**>::bind (sqlite3 * db, sqlite3_stmt * req, int rank) 
  {
    const char * s = (const char *)(*t);
    ok (db, sqlite3_bind_text (req, 1 + rank, s, strlen (s), NULL));
  }

  template <> void stmt_ivar<std::string*>::bind (sqlite3 * db, sqlite3_stmt * req, int rank) 
  {
    ok (db, sqlite3_bind_text (req, 1 + rank, t->c_str (), strlen (t->c_str ()), NULL));
  }

  static void ibuild_list (std::list<stmt_ivar_base*> *)
  {
  }

  template <typename T, typename... Types>
  static void ibuild_list (std::list<stmt_ivar_base*> * list, T t, Types... args)
  {
    stmt_ivar<T> * st = new stmt_ivar<T> (t);
    list->push_back (st);
    ibuild_list (list, args...);
  }

  class stmt_ovar_base
  {
  public:
  virtual void bind (sqlite3 *, sqlite3_stmt *, int) = 0;
  };


  template <typename T>
  class stmt_ovar : public stmt_ovar_base
  {
  public:
  stmt_ovar (T _t) : t (_t) { }
  virtual void bind (sqlite3 *, sqlite3_stmt *, int) 
  {  
    std::cout << "template <typename T> void stmt_ovar::bind" << std::endl;
  }
  private:
  T t;
  };

  template <> void stmt_ovar<int*>::bind (sqlite3 * db, sqlite3_stmt * req, int rank) 
  {
    ok (db, sqlite3_bind_int (req, 1 + rank, *t));
  }

  template <> void stmt_ovar<float*>::bind (sqlite3 * db, sqlite3_stmt * req, int rank) 
  {
    ok (db, sqlite3_bind_double (req, 1 + rank, *t));
  }

  template <> void stmt_ovar<char**>::bind (sqlite3 * db, sqlite3_stmt * req, int rank) 
  {
    const char * s = (const char *)(*t);
    ok (db, sqlite3_bind_text (req, 1 + rank, s, strlen (s), NULL));
  }

  template <> void stmt_ovar<std::string*>::bind (sqlite3 * db, sqlite3_stmt * req, int rank) 
  {
    ok (db, sqlite3_bind_text (req, 1 + rank, t->c_str (), strlen (t->c_str ()), NULL));
  }

  static void obuild_list (std::list<stmt_ovar_base*> *)
  {
  }

  template <typename T, typename... Types>
  static void obuild_list (std::list<stmt_ovar_base*> * list, T t, Types... args)
  {
    stmt_ovar<T> * st = new stmt_ovar<T> (t);
    list->push_back (st);
    obuild_list (list, args...);
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
    template <typename... Types>
    void ibind (Types... args)
    {
      for (std::list<glgrib_sqlite_detail::stmt_ivar_base*>::iterator it = ilist.begin (); it != ilist.end (); it++)
        delete *it;
      ilist.clear ();
      glgrib_sqlite_detail::ibuild_list (&ilist, args...);
    }
    template <typename... Types>
    void obind (Types... args)
    {
      for (std::list<glgrib_sqlite_detail::stmt_ovar_base*>::iterator it = olist.begin (); it != olist.end (); it++)
        delete *it;
      olist.clear ();
      glgrib_sqlite_detail::obuild_list (&olist, args...);
    }
    ~stmt ()
    {
      for (std::list<glgrib_sqlite_detail::stmt_ivar_base*>::iterator it = ilist.begin (); it != ilist.end (); it++)
        delete *it;
      for (std::list<glgrib_sqlite_detail::stmt_ovar_base*>::iterator it = olist.begin (); it != olist.end (); it++)
        delete *it;
      sqlite3_finalize (req);
    }
  protected:
    sqlite3_stmt * req = NULL;
    void execute (sqlite3 * db)
    {
      int rank;
      rank = 0;
      for (std::list<glgrib_sqlite_detail::stmt_ivar_base*>::iterator it = ilist.begin (); 
           it != ilist.end (); it++, rank++)
        (*it)->bind (db, req, rank);
    }
    bool fetch_row (sqlite3 *db)
    {
      int rc = sqlite3_step (req);
      switch (rc)
        {
          case SQLITE_ROW:
            {
              int rank = 0;
              for (std::list<glgrib_sqlite_detail::stmt_ovar_base*>::iterator it = olist.begin (); 
                   it != olist.end (); it++, rank++)
                (*it)->bind (db, req, rank);
	    }
            return true;
	  case SQLITE_DONE:
	    return false;
	}
      return false;
    }
    friend class glgrib_sqlite;
  private:
    std::list<glgrib_sqlite_detail::stmt_ivar_base*> ilist;
    std::list<glgrib_sqlite_detail::stmt_ovar_base*> olist;
  };

  void execute (stmt * st)
  {
    st->execute (db);
  }

  bool fetch_row (stmt * st)
  {
    int rc;
    return st->fetch_row (db);
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

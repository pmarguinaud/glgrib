#pragma once

#include <sqlite3.h>
#include <string>
#include <iostream>
#include <list>
#include <string.h>
#include <exception>
#include <memory>


namespace glgrib_sqlite_detail
{

  class _sqlite3
  {
  public:
    ~_sqlite3 () 
    {
      if (data != nullptr)
        sqlite3_close (data);
      data = nullptr;
    }
    sqlite3 * data = nullptr;
  };

  typedef std::shared_ptr<_sqlite3> sqlite3_ptr;

  class _sqlite3_stmt
  {
  public:
    ~_sqlite3_stmt () 
    {
      if (data != nullptr)
        sqlite3_finalize (data);
      data = nullptr;
    }
    sqlite3_stmt * data = nullptr;
  };

  typedef std::shared_ptr<_sqlite3_stmt> sqlite3_stmt_ptr;

  void ok (sqlite3_ptr db, int rc);

  void iset (sqlite3_ptr db, sqlite3_stmt_ptr req, int rank, const long int * t);
  void iset (sqlite3_ptr db, sqlite3_stmt_ptr req, int rank, const int * t);
  void iset (sqlite3_ptr db, sqlite3_stmt_ptr req, int rank, const float * t);
  void iset (sqlite3_ptr db, sqlite3_stmt_ptr req, int rank, const char * t);
  void iset (sqlite3_ptr db, sqlite3_stmt_ptr req, int rank, const std::string * t);

  void iset_list (sqlite3_ptr, sqlite3_stmt_ptr, int);
  template <typename T, typename... Types>
  void iset_list (sqlite3_ptr db, sqlite3_stmt_ptr req, int rank, const T & t, Types... args)
  {
    iset (db, req, rank, t);
    iset_list (db, req, rank + 1, args...);
  }

  void oget (sqlite3_ptr db, sqlite3_stmt_ptr req, int rank, int * t);
  void oget (sqlite3_ptr db, sqlite3_stmt_ptr req, int rank, float * t);
  void oget (sqlite3_ptr db, sqlite3_stmt_ptr req, int rank, char * t);
  void oget (sqlite3_ptr db, sqlite3_stmt_ptr req, int rank, std::string * t);

  void oget_list (sqlite3_ptr, sqlite3_stmt_ptr, int);
  template <typename T, typename... Types>
  void oget_list (sqlite3_ptr db, sqlite3_stmt_ptr req, int rank, T t, Types... args)
  {
    if (rank >= sqlite3_column_count (req->data))
      throw std::runtime_error (std::string ("Column out of bounds"));
    oget (db, req, rank, t);
    oget_list (db, req, rank + 1, args...);
  }

};

class glGribSqlite
{
public:

  typedef glgrib_sqlite_detail::sqlite3_ptr sqlite3_ptr;
  typedef glgrib_sqlite_detail::sqlite3_stmt_ptr sqlite3_stmt_ptr;

  glGribSqlite (const std::string & file)
  {
    db = std::make_shared<glgrib_sqlite_detail::_sqlite3>();
    if (sqlite3_open (file.c_str (), &db->data) != SQLITE_OK)
      throw std::runtime_error (std::string ("Cannot open database ") + file);
  }

  class stmt
  {
  public:
    
    stmt ()
    {
      req = std::make_shared<glgrib_sqlite_detail::_sqlite3_stmt>();
    }
    ~stmt ()
    {
    }
    void reset ()
    {
      glgrib_sqlite_detail::ok (db, sqlite3_reset (req->data));
    }
    template <typename... Types>
    void bindall (Types... args)
    {
      glgrib_sqlite_detail::iset_list (db, req, 0, args...);
    }
    template <typename... Types>
    void execute (Types... args)
    {
      bindall (args...);
      sqlite3_step (req->data);
    }
    template <typename... Types>
    bool fetchRow (Types... args)
    {
      if (sqlite3_step (req->data) == SQLITE_ROW)
        {
          glgrib_sqlite_detail::oget_list (db, req, 0, args...);
          return true;
	}
      return false;
    }
    template <typename T>
    void bind (int rank, const T * t)
    {
      if (rank >= sqlite3_bind_parameter_count (req->data))
        throw std::runtime_error (std::string ("Column out of bounds"));
      glgrib_sqlite_detail::iset (db, req, rank, t);
    }
protected:
    sqlite3_stmt_ptr req;
    sqlite3_ptr db;
    friend class glGribSqlite;
  };

  stmt prepare (const std::string & sql)
  {
    stmt st;
    st.db = db;
    glgrib_sqlite_detail::ok (db, sqlite3_prepare_v2 (db->data, sql.c_str (), -1, &st.req->data, 0));
    return st;
  }

  void execute (const std::string & sql)
  {
    glgrib_sqlite_detail::ok (db, sqlite3_exec (db->data, sql.c_str (), 0, 0, 0));
  }

  ~glGribSqlite ()
  {
    close ();
  }

  void close () 
  {
  }

private:
  sqlite3_ptr db;
};



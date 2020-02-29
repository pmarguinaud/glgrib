#pragma once

#include "glgrib_window.h"
#include "glgrib_shell.h"

class glgrib_window_set : public std::set<glgrib_window*> 
{
public:
  virtual void run (glgrib_shell * = nullptr);
  glgrib_window * getWindowById (int);
  glgrib_window * getFirstWindow () 
    { 
      glgrib_window_set::iterator it = begin ();
      if (it != end ())
        return *it;
      else
        return nullptr;
    }
  void close ();
  virtual glgrib_window * create (const glgrib_options &);
  void runShell (glgrib_shell **);
  void handleMasterWindow ();
};


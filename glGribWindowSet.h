#pragma once

#include "glGribWindow.h"
#include "glGribShell.h"

class glGribWindowSet : public std::set<glGribWindow*> 
{
public:
  virtual void run (glGribShell * = nullptr);
  glGribWindow * getWindowById (int);
  glGribWindow * getFirstWindow () 
    { 
      glGribWindowSet::iterator it = begin ();
      if (it != end ())
        return *it;
      else
        return nullptr;
    }
  void close ();
  virtual glGribWindow * create (const glGribOptions &);
  void runShell (glGribShell **);
  void handleMasterWindow ();
};


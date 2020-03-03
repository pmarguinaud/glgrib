#pragma once

#include "glGribWindow.h"
#include "glGribShell.h"
#include "glGribOptions.h"

class glGribWindowSet : public std::set<glGribWindow*> 
{
public:
  glGribWindowSet (const glGribOptions &);
  virtual void run (glGribShell * = nullptr);
  virtual void updateWindows ();
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
protected:
  glGribOptions opts;
};


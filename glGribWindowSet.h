#pragma once

#include "glGribWindow.h"
#include "glGribShell.h"
#include "glGribOptions.h"

namespace glGrib
{

class WindowSet : public std::set<Window*> 
{
public:
  WindowSet (const Options &);
  virtual void run (Shell * = nullptr);
  virtual void updateWindows ();
  Window * getWindowById (int);
  Window * getFirstWindow () 
    { 
      WindowSet::iterator it = begin ();
      if (it != end ())
        return *it;
      else
        return nullptr;
    }
  void close ();
  virtual Window * create (const Options &);
  void runShell (Shell **);
  void handleMasterWindow ();
protected:
  Options opts;
};


}

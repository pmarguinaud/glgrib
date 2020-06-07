#pragma once

#include "glGribWindow.h"
#include "glGribShell.h"
#include "glGribOptions.h"

namespace glGrib
{

class WindowSet : public std::set<Window*> 
{
public:
  WindowSet (const Options &, bool = true);
  virtual ~WindowSet () {}
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
  virtual Window * createWindow (const Options &);
  void runShell (Shell **);
  void handleMasterWindow ();
  static WindowSet * create (const glGrib::Options &);
  Options & getOptions ()
  {
    return opts;
  }
  const Options & getOptions () const
  {
    return opts;
  }
private:
  Options opts;
};


}

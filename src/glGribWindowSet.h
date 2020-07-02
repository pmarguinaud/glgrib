#pragma once

#include "glGribRender.h"
#include "glGribShell.h"
#include "glGribOptions.h"

namespace glGrib
{

class WindowSet : public std::set<Render*> 
{
public:
  explicit WindowSet (const Options &, bool = true);
  virtual ~WindowSet () {}
  virtual void run (Shell * = nullptr);
  virtual void updateWindows ();
  Render * getWindowById (int);
  Render * getFirstWindow () 
    { 
      WindowSet::iterator it = begin ();
      if (it != end ())
        return *it;
      else
        return nullptr;
    }
  void close ();
  virtual Render * createWindow (const Options &);

  void runShell (Shell * shell, bool render = true)
  {
    runShell (&shell, render);
  }
  void runShell (Shell **, bool = true);

  void handleMasterWindow ();
  static WindowSet * create (const glGrib::Options &);
  const Options & getOptions ()
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

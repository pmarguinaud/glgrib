#pragma once

#include "glGribShell.h"
#include "glGribOptions.h"

#include <pthread.h>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <functional>

namespace glGrib
{

class Window;

class ShellInterpreter : public Shell
{
public:
  void setup (const OptionsShell &) override;
  void start (class WindowSet *) override;
  void run () override;

  static ShellInterpreter & getInstance () 
  {
    return shellinterp;
  }

  void runWset ();

  void stop ();

  void start (int, const char * []);
  void start (const std::vector<std::string> &);
  void execute (const std::vector<std::string> &) override;

  const std::vector<std::string> & getList ()
  {
    return listStr;
  }

private:
  ShellInterpreter ();
  ~ShellInterpreter () {}
  ShellInterpreter & operator= (const ShellInterpreter &) { return *this; }
  ShellInterpreter (const ShellInterpreter &) {}
  static ShellInterpreter shellinterp;

  glGrib::Options gopts;
  volatile bool hasstarted = false;
};

}

#pragma once

#include "glGrib/Shell.h"
#include "glGrib/Options.h"

#include <string>
#include <vector>
#include <map>
#include <list>
#include <functional>
#include <thread>
#include <mutex>


namespace glGrib
{

class ShellInterpreter : public Shell
{
public:
  void setup (const OptionsShell &) override;
  void start (class WindowSet *) override;
  void run () override;

  static ShellInterpreter & getInstance () 
  {
    if (shellinterp == nullptr)
      shellinterp = new ShellInterpreter ();
    return *shellinterp;
  }

  void runWset ();

  void stop (const std::vector<std::string> &);

  void start (int, const char * []);
  void start (const std::vector<std::string> &);
  void execute (const std::vector<std::string> &) override;

  void lock () override;
  void unlock () override;
  void wait () override { if (getWindowSet ()) thread.join (); }

private:
  ShellInterpreter ();
  ~ShellInterpreter () {}
  ShellInterpreter & operator= (const ShellInterpreter &) { return *this; }
  ShellInterpreter (const ShellInterpreter &) {}
  static ShellInterpreter * shellinterp;

  glGrib::Options gopts;
  volatile bool hasstarted = false;
  std::thread thread;
  std::mutex mutex;
};

}

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

class ShellPerl : public Shell
{
public:
  void setup (const OptionsShell &) override;
  void start (class WindowSet *) override;
  void run () override;
  static ShellPerl & getInstance () 
  {
    return shellperl;
  }

  const std::vector<std::string> & getList () const
  {
    return list;
  }

  void runWset ();

  void stop ();

  void start (int, const char * []);

private:
  ShellPerl ();
  ~ShellPerl () {}
  ShellPerl & operator= (const ShellPerl &) { return *this; }
  ShellPerl (const ShellPerl &) {}
  static ShellPerl shellperl;
  

  void process_help          (const std::vector<std::string> &, glGrib::Window *) override;
  void process_get           (const std::vector<std::string> &, glGrib::Window *) override;
  void process_close         (const std::vector<std::string> &, glGrib::Window *) override;
  void process_snapshot      (const std::vector<std::string> &, glGrib::Window *) override;
  void process_sleep         (const std::vector<std::string> &, glGrib::Window *) override;
  void process_clone         (const std::vector<std::string> &, glGrib::Window *) override;
  void process_set           (const std::vector<std::string> &, glGrib::Window *) override;
  void process_window        (const std::vector<std::string> &, glGrib::Window *) override;

  std::vector<std::string> list;

  glGrib::Options gopts;
};

}

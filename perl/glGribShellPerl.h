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

  const std::string & getStr () const 
  {
    return str;
  }
  const std::vector<std::string> & getListStr () const
  {
    return listStr;
  }
  const std::vector<int> & getListInt () const
  {
    return listInt;
  }

  void runWset ();

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

  std::string str;
  std::vector<std::string> listStr;
  std::vector<int> listInt;

};

}

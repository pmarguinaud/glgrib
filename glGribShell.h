#pragma once

#include "glGribShellBase.h"
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

class Shell : public ShellBase
{
public:
  Shell ();
  void setup (const OptionsShell &) override;
  void start (class WindowSet *) override;
  void run () override;
  char * optionGenerator (const char *, int);
private:
  std::vector<std::string> tokenize (const std::string &);
  void runInt ();
  void runOff ();

  void do_get           (const std::vector<std::string> &, glGrib::Window *) override;
  void do_window_list   (const std::vector<std::string> &, glGrib::Window *) override;


  std::vector<std::string> getsetoptions;
  struct
  {
    int list_index, text_len;
  } og;
};

extern Shell Shell0;

}

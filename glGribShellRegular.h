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

class ShellRegular : public Shell
{
public:
  ShellRegular ();
  void setup (const OptionsShell &) override;
  void start (class WindowSet *) override;
  void run () override;
  char * optionGenerator (const char *, int);
private:
  std::vector<std::string> tokenize (const std::string &);
  void runInt ();
  void runOff ();

  void process_help          (const std::vector<std::string> &, glGrib::Window *) override;
  void process_get           (const std::vector<std::string> &, glGrib::Window *) override;
  void process_close         (const std::vector<std::string> &, glGrib::Window *) override;
  void process_snapshot      (const std::vector<std::string> &, glGrib::Window *) override;
  void process_sleep         (const std::vector<std::string> &, glGrib::Window *) override;
  void process_clone         (const std::vector<std::string> &, glGrib::Window *) override;
  void process_set           (const std::vector<std::string> &, glGrib::Window *) override;
  void process_window        (const std::vector<std::string> &, glGrib::Window *) override;

  std::vector<std::string> getsetoptions;
  struct
  {
    int list_index, text_len;
  } og;
};

extern ShellRegular Shell0;

}

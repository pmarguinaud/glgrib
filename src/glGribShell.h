#pragma once

#include "glGribOptions.h"

#include <string>
#include <vector>
#include <map>
#include <list>
#include <functional>
#include <stdexcept>

namespace glGrib
{

class Render;

class Shell
{
public:
  virtual void setup (const OptionsShell &)  = 0;
  virtual void start (class WindowSet *)  = 0;
  virtual void run ()  = 0;
  virtual void execute (const std::vector<std::string> &);
  bool closed () { return close; }
  void setClosed () { close = true; }

  virtual void lock ()  = 0;
  virtual void unlock ()  = 0;
  virtual void wait () = 0;

  const OptionsShell & getOptions () const { return opts; }
  void setOptions (const OptionsShell & o) { opts = o; }
  bool started () { return wset != nullptr; }

  // Run command and store result in listStr
  void do_help          (const std::vector<std::string> &, glGrib::Render *);
  void do_get           (const std::vector<std::string> &, glGrib::Render *);
  void do_json          (const std::vector<std::string> &, glGrib::Render *);
  void do_close         (const std::vector<std::string> &, glGrib::Render *);
  void do_snapshot      (const std::vector<std::string> &, glGrib::Render *);
  void do_sleep         (const std::vector<std::string> &, glGrib::Render *);
  void do_clone         (const std::vector<std::string> &, glGrib::Render *);
  void do_set           (const std::vector<std::string> &, glGrib::Render *);
  void do_window        (const std::vector<std::string> &, glGrib::Render *);
  void do_window_select (const std::vector<std::string> &, glGrib::Render *);
  void do_window_list   (const std::vector<std::string> &, glGrib::Render *);
  void do_clear         (const std::vector<std::string> &, glGrib::Render *);
  void do_list          (const std::vector<std::string> &, glGrib::Render *);
  void do_resolve       (const std::vector<std::string> &, glGrib::Render *);

  // Process command output
  virtual void process_help          (const std::vector<std::string> &, glGrib::Render *) {}
  virtual void process_get           (const std::vector<std::string> &, glGrib::Render *) {}
  virtual void process_json          (const std::vector<std::string> &, glGrib::Render *) {}
  virtual void process_close         (const std::vector<std::string> &, glGrib::Render *) {}
  virtual void process_snapshot      (const std::vector<std::string> &, glGrib::Render *) {}
  virtual void process_sleep         (const std::vector<std::string> &, glGrib::Render *) {}
  virtual void process_clone         (const std::vector<std::string> &, glGrib::Render *) {}
  virtual void process_set           (const std::vector<std::string> &, glGrib::Render *) {}
  virtual void process_window        (const std::vector<std::string> &, glGrib::Render *) {}
  virtual void process_clear         (const std::vector<std::string> &, glGrib::Render *) {}
  virtual void process_list          (const std::vector<std::string> &, glGrib::Render *) {}
  virtual void process_resolve       (const std::vector<std::string> &, glGrib::Render *) {}

  const std::vector<std::string> & getList ()
  {
    return listStr;
  }

  void setWindowSet (WindowSet *);

  WindowSet * getWindowSet ();

  void clearWindowSet ();

  int getWindowId () const
  {
    return windowid;
  }

  Render * getWindow ();

  Render * getFirstWindow ();

private:

  int close = 0;
  int windowid = 0;

  OptionsShell opts;
  WindowSet * wset = nullptr;

  std::vector<std::string> listStr;

};

}

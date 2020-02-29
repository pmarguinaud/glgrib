#pragma once

#include "glgrib_window.h"
#include "glgrib_container.h"
#include "glgrib_window_set.h"
#include "glgrib_options.h"

class glgrib_window_diff : public glgrib_window
{
public:
  glgrib_window_diff (const glgrib_options & opts)
    : glgrib_window (opts) {}
  void onkey (int, int, int, int, bool = false) override;
  bool getNext ()
  {
    bool _next = next;
    next = false;
    return _next;
  }
  bool getPrev ()
  {
    bool _prev = prev;
    prev = false;
    return _prev;
  }
private:
  bool next = false;
  bool prev = false;
};

class glgrib_window_diff_set : public glgrib_window_set
{
public:
  glgrib_window_diff_set (const glgrib_options &);
  void run (glgrib_shell *) override;
  glgrib_window_diff * create_diff (const glgrib_options &);
  const std::string getNextExt () const;
  const std::string getPrevExt () const;
private:
  glgrib_options opts;
  glgrib_container * cont1 = nullptr;
  glgrib_container * cont2 = nullptr;
  std::string ext = "";
};

#pragma once

#include "glgrib_window.h"
#include "glgrib_container.h"
#include "glgrib_window_set.h"
#include "glgrib_options.h"

class glgrib_window_diff_set : public glgrib_window_set
{
public:
  glgrib_window_diff_set (const glgrib_options &);
  void run (glgrib_shell *) override;
  const std::string getNextExt () const;
  const std::string getPrevExt () const;
private:
  glgrib_options opts;
  glgrib_container * cont1 = nullptr;
  glgrib_container * cont2 = nullptr;
  std::string ext = "";
};

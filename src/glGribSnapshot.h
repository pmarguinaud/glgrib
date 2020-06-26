#pragma once

#include <string>

namespace glGrib
{
  template <typename T>
  void snapshot (T &, const std::string &);
  template <typename T>
  void framebuffer (T &, const std::string &);
};


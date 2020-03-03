#pragma once

#include "glGribWindow.h"
#include "glGribContainer.h"
#include "glGribWindowSet.h"

class glGribWindowDiffSet : public glGribWindowSet
{
public:
  glGribWindowDiffSet (const glGribOptions &);
  const std::string getNextExt () const;
  const std::string getPrevExt () const;
  void updateWindows () override;
private:
  glGribContainer * cont1 = nullptr;
  glGribContainer * cont2 = nullptr;
  std::string ext = "";
};

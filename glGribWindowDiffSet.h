#pragma once

#include "glGribWindow.h"
#include "glGribContainer.h"
#include "glGribWindowSet.h"
#include "glGribOptions.h"

class glGribWindowDiffSet : public glGribWindowSet
{
public:
  glGribWindowDiffSet (const glGribOptions &);
  void run (glGribShell *) override;
  const std::string getNextExt () const;
  const std::string getPrevExt () const;
private:
  glGribOptions opts;
  glGribContainer * cont1 = nullptr;
  glGribContainer * cont2 = nullptr;
  std::string ext = "";
};

#pragma once

#include "glGribOptionsBase.h"
#include "glGribOptionsGeometry.h"
#include "glGribOptionsFont.h"
#include "glGribOptionsContour.h"
#include "glGribOptionsIsofill.h"
#include "glGribOptionsStream.h"
#include "glGribOptionsVector.h"
#include "glGribOptionsPalette.h"
#include "glGribOptionsScalar.h"
#include "glGribOptionsMpiview.h"
#include "glGribOptionsField.h"
#include "glGribOptionsTicksSide.h"
#include "glGribOptionsTicks.h"
#include "glGribOptionsGrid.h"
#include "glGribOptionsLandLayer.h"
#include "glGribOptionsLand.h"
#include "glGribOptionsLandscapePosition.h"
#include "glGribOptionsLandscape.h"
#include "glGribOptionsLines.h"
#include "glGribOptionsOffscreen.h"
#include "glGribOptionsWindow.h"
#include "glGribOptionsLight.h"
#include "glGribOptionsPosition.h"
#include "glGribOptionsTravelling.h"
#include "glGribOptionsInterpolation.h"
#include "glGribOptionsImage.h"
#include "glGribOptionsText.h"
#include "glGribOptionsDate.h"
#include "glGribOptionsTitle.h"
#include "glGribOptionsScene.h"
#include "glGribOptionsView.h"
#include "glGribOptionsColorbar.h"
#include "glGribOptionsMapscale.h"
#include "glGribOptionsRivers.h"
#include "glGribOptionsBorder.h"
#include "glGribOptionsPoints.h"
#include "glGribOptionsGeoPoints.h"
#include "glGribOptionsCities.h"
#include "glGribOptionsCoast.h"
#include "glGribOptionsDepartements.h"
#include "glGribOptionsShell.h"

namespace glGrib
{

class Options : public OptionsBase
{
public:
  DEFINE
  {
    INCLUDE   (field[0]); INCLUDE_H (field[1]); 
    INCLUDE_H (field[2]); INCLUDE_H (field[3]); 
    INCLUDE_H (field[4]); INCLUDE_H (field[5]); 
    INCLUDE_H (field[6]); INCLUDE_H (field[7]); 
    INCLUDE_H (field[8]); INCLUDE_H (field[9]); 
    INCLUDE (coast);
    INCLUDE (cities);
    INCLUDE (geopoints);
    INCLUDE (border);
    INCLUDE (rivers);
    INCLUDE (window);
    INCLUDE (landscape);
    INCLUDE (grid);
    INCLUDE (ticks);
    INCLUDE (scene);
    INCLUDE (view);
    INCLUDE (colorbar);
    INCLUDE (mapscale);
    INCLUDE (departements);
    INCLUDE (shell);
    INCLUDE (land);
    DESC (review.on, Enable review mode);
    DESC (review.path, File to review);
    DESC (diff.on, Enable difference mode);
    DESC (diff.path, Files to show in diff mode);
  }
  struct
  {
    bool on = false;
    std::string path;
  } review;
  struct
  {
    std::vector<std::string> path;
    bool on = false;
  } diff;
  std::vector<OptionsField> field =
    {OptionsField (), OptionsField (), 
     OptionsField (), OptionsField (), 
     OptionsField (), OptionsField (), 
     OptionsField (), OptionsField (), 
     OptionsField (), OptionsField ()};
  OptionsCoast coast;
  OptionsCities cities;
  OptionsGeoPoints geopoints;
  OptionsBorder border;
  OptionsRivers rivers;
  OptionsColorbar colorbar;
  OptionsMapscale mapscale;
  OptionsDepartements departements;
  OptionsWindow window;
  OptionsLandscape landscape;
  OptionsGrid grid;
  OptionsTicks ticks;
  OptionsScene scene;
  OptionsView view;
  OptionsFont font;
  OptionsShell shell;
  OptionsLand land;
  virtual bool parse (int, const char * [], const std::set<std::string> * = nullptr);
};


}

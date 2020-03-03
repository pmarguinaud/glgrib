#include "glGribWindowDiffSet.h"

#include <iostream>



static
void setDiffOptions (glGribOptionsField & opts1, glGribOptionsField & opts2, 
                     const std::string & path1, const std::string & path2)
{
  opts1.diff.on = true;
  opts1.path.clear ();
  opts1.path.push_back (path1);
  opts1.path.push_back (path2);
  opts1.palette.name = "cold_hot";
  opts1.user_pref.on = false;
  opts2.path.clear ();
  opts2.path.push_back (path2);
}

glGribWindowDiffSet::glGribWindowDiffSet (const glGribOptions & o)
  : glGribWindowSet (o)
{
  glGribOptions opts1 = opts, opts2 = opts;

  if (opts.diff.path.size () != 2)
    throw std::runtime_error (std::string ("Option --diff.path requires two arguments"));


  auto fixOpts = [] (glGribOptions * opts)
  {
    opts->diff.on = false;
    opts->diff.path.clear ();
    for (int i = 0; i < opts->field.size (); i++)
      opts->field[i].path.clear ();
  };

  fixOpts (&opts1);
  fixOpts (&opts2);


  opts1.window.position.x = 0;
  opts1.window.position.y = 0;

  opts2.window.position.x = opts1.window.position.x + opts1.window.width;
  opts2.window.position.y = 0;

  cont1 = glGribContainer::create (opts.diff.path[0], true);
  cont2 = glGribContainer::create (opts.diff.path[1], true);


  cont1->buildIndex ();
  cont2->buildIndex ();

  ext = getNextExt ();

  setDiffOptions (opts1.field[0], opts2.field[0], 
                  opts.diff.path[0] + "%" + ext, 
                  opts.diff.path[1] + "%" + ext);

  glGribWindow * gwindow1 = create (opts1);
  glGribWindow * gwindow2 = gwindow1->clone ();

  gwindow2->setOptions (opts2.window);

  gwindow2->scene.setup (opts2);

  gwindow1->setMaster ();

  insert (gwindow2);
  
}

const std::string glGribWindowDiffSet::getNextExt () const
{
  std::string e = ext;
  while (1)
    {
      e = cont1->getNextExt (e);
      if (e == "") 
        break;
      if (cont2->hasExt (e))
        break;
      e = "";
    }
  return e;
}

const std::string glGribWindowDiffSet::getPrevExt () const
{
  std::string e = ext;
  while (1)
    {
      e = cont1->getPrevExt (e);
      if (e == "") 
        break;
      if (cont2->hasExt (e))
        break;
      e = "";
    }
  return e;
}

void glGribWindowDiffSet::updateWindows ()
{
  glGribWindow * gwindow1 = getWindowById (0);
  glGribWindow * gwindow2 = getWindowById (1);
  if ((gwindow1 == nullptr) || (gwindow2 == nullptr))
    return;

  std::string e;
  
  if (gwindow1->getNext ())
    e = getNextExt ();
  
  if (gwindow1->getPrev ())
    e = getPrevExt ();
  
  
  if (e == "")
    return;

  ext = e;
  
  auto fopts1 = opts.field[0];
  auto fopts2 = opts.field[0];
  
  setDiffOptions (fopts1, fopts2, cont1->getFile () + "%" + ext, cont2->getFile () + "%" + ext);
  
  // First window
  gwindow1->makeCurrent ();
  gwindow1->scene.setFieldOptions (0, fopts1);
  
  // Second window
  gwindow2->makeCurrent ();
  gwindow2->scene.setFieldOptions (0, fopts2);
}


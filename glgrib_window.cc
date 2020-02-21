#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glgrib_window.h"
#include "glgrib_png.h"
#include "glgrib_field_scalar.h"
#include "glgrib_field_vector.h"

#include <iostream>
#include <stdexcept>

static double current_time ()
{
  struct timeval tv;
  struct timezone tz;
  gettimeofday (&tv, &tz);
  return (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
}

static
void APIENTRY debug_callback (unsigned int source, unsigned int type, GLuint id, unsigned int severity, 
                              int length, const char * message, const void * data)
{
  glgrib_window * gwindow = (glgrib_window *)data;
  gwindow->debug (source, type, id, severity, length, message);
}

static 
void cursor_position_callback (GLFWwindow * window, double xpos, double ypos)
{
  glgrib_window * gwindow = (glgrib_window *)glfwGetWindowUserPointer (window);
  gwindow->display_cursor_position (xpos, ypos);
}

static
void mouse_button_callback (GLFWwindow * window, int button, int action, int mods)
{
  glgrib_window * gwindow = (glgrib_window *)glfwGetWindowUserPointer (window);
  gwindow->onclick (button, action, mods);
}

static
void scroll_callback (GLFWwindow * window, double xoffset, double yoffset)
{
  glgrib_window * gwindow = (glgrib_window *)glfwGetWindowUserPointer (window);
  gwindow->scroll (xoffset, yoffset);
}

static 
void resize_callback (GLFWwindow * window, int width, int height)
{
  glgrib_window * gwindow = (glgrib_window *)glfwGetWindowUserPointer (window);
  gwindow->resize (width, height);
}

static 
void key_callback (GLFWwindow * window, int key, int scancode, int action, int mods)
{
  glgrib_window * gwindow = (glgrib_window *)glfwGetWindowUserPointer (window);
  gwindow->onkey (key, scancode, action, mods);
}

static void showHelpItem (const char * mm, const char * k, const char * desc, const char * action)
{
  char line[strlen (desc) + strlen (action) + 64];
  char key[32];
  key[0] = '\0';
  if (strcmp (mm, "NONE") != 0)
    {
      strcpy (key, mm);
      strcat (key, "-");
    }
  strcat (key, k);

  sprintf (line, "%-32s %64s\n", key, strlen (desc) ? desc : action);

  int len = strlen (line);
  for (int i = 0; i < len; i++)
    if (line[i] == ' ')
      line[i] = '.';
  printf ("%s", line);
}

void glgrib_window::onkey (int key, int scancode, int action, int mods, bool help)
{
  enum
  {
    NONE    = 0,
    SHIFT   = GLFW_MOD_SHIFT,
    CONTROL = GLFW_MOD_CONTROL,
    ALT     = GLFW_MOD_ALT
  };

#define if_key(mm, k, desc, action) \
do { \
if (help)                                       \
  {                                             \
    showHelpItem (#mm, #k, #desc, #action);     \
  }                                             \
else if ((key == GLFW_KEY_##k) && (mm == mods)) \
  {                                             \
    action;                                     \
    return;                                     \
  }                                             \
} while (0)


  if ((action == GLFW_PRESS || action == GLFW_REPEAT) || help)
    {
      if_key (NONE,    T     ,  Hide/show location & field value at cursor position  , toggle_cursorpos_display ());
      if_key (NONE,    TAB   ,  Enable/disable earth rotation                        , toggle_rotate            ());
      if_key (CONTROL, TAB   ,  Enable/disable light rotation                        , toggle_rotate_light      ());
      if_key (NONE,    Y     ,  Display landscape or current field as wireframe      , toggle_wireframe         ());
      if_key (NONE,    D     ,  Use a framebuffer and generate a snapshot            , framebuffer              ());
      if_key (NONE,    W     ,  Increase field of view                               , widen_fov                ());
      if_key (NONE,    S     ,  Write a snapshot (PNG format)                        , snapshot                 ()); 
      if_key (NONE,    Q     ,  Decrease field of view                               , shrink_fov               ());
      if_key (NONE,    P     ,  Make earth flat/show orography                       , toggle_flat              ());
      if_key (NONE,    6     ,  Increase size of current field                       , increase_radius          ());
      if_key (NONE,    EQUAL ,  Decrease size of current field                       , decrease_radius          ());
      if_key (NONE,    SPACE ,  Reset view                                           , reset_view               ());
      if_key (NONE,    UP    ,  Move northwards                                      , rotate_north             ());
      if_key (NONE,    DOWN  ,  Move southwards                                      , rotate_south             ());
      if_key (NONE,    LEFT  ,  Move westwards                                       , rotate_west              ());
      if_key (NONE,    RIGHT ,  Move eastwards                                       , rotate_east              ());

      if_key (NONE,    F1    ,  Select field #1                                      , select_field           ( 0));
      if_key (NONE,    F2    ,  Select field #2                                      , select_field           ( 1));
      if_key (NONE,    F3    ,  Select field #3                                      , select_field           ( 2));
      if_key (NONE,    F4    ,  Select field #4                                      , select_field           ( 3));
      if_key (NONE,    F5    ,  Select field #5                                      , select_field           ( 4));
      if_key (NONE,    F6    ,  Select field #6                                      , select_field           ( 5));
      if_key (NONE,    F7    ,  Select field #7                                      , select_field           ( 6));
      if_key (NONE,    F8    ,  Select field #8                                      , select_field           ( 7));
      if_key (NONE,    F9    ,  Select field #9                                      , select_field           ( 8));
      if_key (NONE,    F10   ,  Select field #10                                     , select_field           ( 9));
      if_key (NONE,    F11   ,  Select field #11                                     , select_field           (10));
      if_key (NONE,    F12   ,  Select field #12                                     , select_field           (11));

      if_key (CONTROL, F1    ,  Show only field #1                                   , { hide_all_fields (); select_field ( 0); toggle_hide_field (); });
      if_key (CONTROL, F2    ,  Show only field #2                                   , { hide_all_fields (); select_field ( 1); toggle_hide_field (); });
      if_key (CONTROL, F3    ,  Show only field #3                                   , { hide_all_fields (); select_field ( 2); toggle_hide_field (); });
      if_key (CONTROL, F4    ,  Show only field #4                                   , { hide_all_fields (); select_field ( 3); toggle_hide_field (); });
      if_key (CONTROL, F5    ,  Show only field #5                                   , { hide_all_fields (); select_field ( 4); toggle_hide_field (); });
      if_key (CONTROL, F6    ,  Show only field #6                                   , { hide_all_fields (); select_field ( 5); toggle_hide_field (); });
      if_key (CONTROL, F7    ,  Show only field #7                                   , { hide_all_fields (); select_field ( 6); toggle_hide_field (); });
      if_key (CONTROL, F8    ,  Show only field #8                                   , { hide_all_fields (); select_field ( 7); toggle_hide_field (); });
      if_key (CONTROL, F9    ,  Show only field #9                                   , { hide_all_fields (); select_field ( 8); toggle_hide_field (); });
      if_key (CONTROL, F10   ,  Show only field #10                                  , { hide_all_fields (); select_field ( 9); toggle_hide_field (); });
      if_key (CONTROL, F11   ,  Show only field #11                                  , { hide_all_fields (); select_field (10); toggle_hide_field (); });
      if_key (CONTROL, F12   ,  Show only field #12                                  , { hide_all_fields (); select_field (11); toggle_hide_field (); });
      if_key (CONTROL, H     ,  Show all fields                                      , show_all_fields          ());
      if_key (ALT,     H     ,  Show help                                            , showHelp                 ());

      if_key (NONE,    H     ,  Show/hide selected field                             , toggle_hide_field        ());
      if_key (NONE,    G     ,  Increase size of current field                       , scale_field_up           ());
      if_key (CONTROL, G     ,  Decrease size of current field                       , scale_field_down         ());
      if_key (NONE,    F     ,  Increase palette range                               , scale_palette_up         ());
      if_key (CONTROL, F     ,  Decrease palette range                               , scale_palette_down       ());
      if_key (NONE,    J     ,  Try next palette                                     , next_palette             ());
      if_key (NONE,    L     ,  Turn on/off the light                                , toggle_light             ());
      if_key (CONTROL, L     ,  Make current window master window                    , toggleMaster             ());

      if (opts.fix_landscape.on)
      {
      if_key (CONTROL, UP    ,                                                       , fix_landscape (+1,  0,  0,  0));
      if_key (CONTROL, DOWN  ,                                                       , fix_landscape (-1,  0,  0,  0));
      if_key (CONTROL, LEFT  ,                                                       , fix_landscape ( 0, -1,  0,  0));
      if_key (CONTROL, RIGHT ,                                                       , fix_landscape ( 0, +1,  0,  0));
      if_key (ALT,     UP    ,                                                       , fix_landscape ( 0,  0, +1,  0));
      if_key (ALT,     DOWN  ,                                                       , fix_landscape ( 0,  0, -1,  0));
      if_key (ALT,     LEFT  ,                                                       , fix_landscape ( 0,  0,  0, -1));
      if_key (ALT,     RIGHT ,                                                       , fix_landscape ( 0,  0,  0, +1));
      }
      else
      {
      if_key (CONTROL, UP    ,  Move light northwards                                , rotate_light_north       ());
      if_key (CONTROL, DOWN  ,  Move light southwards                                , rotate_light_south       ());
      if_key (CONTROL, LEFT  ,  Move light westwards                                 , rotate_light_west        ());
      if_key (CONTROL, RIGHT ,  Move light eastwards                                 , rotate_light_east        ());
      }

      if_key (CONTROL, C     ,  Clone current window                                 , duplicate                ());
      if_key (ALT,     C     ,  Show/hide colorbar                                   , toggleColorBar           ());
      if_key (CONTROL, P     ,  Try next projection                                  , next_projection          ());
      if_key (SHIFT,   P     ,  Try next transformation                              , toggle_transform_type    ());
      if_key (CONTROL, S     ,  Save current palette                                 , save_current_palette     ());
      if_key (ALT,     S     ,  Resample current field                               , resample_current_field   ());
      if_key (NONE,    V     ,  Hide/show vector arrows                              , toggle_show_vector       ());
      if_key (CONTROL, V     ,  Hide/show vector norm                                , toggle_show_norm         ());
      if_key (NONE,    U     ,  Start shell                                          , startShell               ());


    }

#undef if_key
}

void glgrib_window::toggle_wireframe () 
{ 
  glgrib_field * f = scene.getCurrentField ();

  if (f == NULL)
    {
      scene.d.landscape.toggle_wireframe (); 
      return;
    }

  f->toggle_wireframe ();

}

void glgrib_window::fix_landscape (float dy, float dx, float sy, float sx)
{
  glgrib_options_landscape_position o = scene.d.landscape.getOptions ().lonlat.position;

  float dlat = o.lat2 - o.lat1;
  float dlon = o.lon2 - o.lon1;

  o.lat1 += dy * 0.01;
  o.lat2 += dy * 0.01;
  o.lon1 += dx * 0.01;
  o.lon2 += dx * 0.01;

  o.lat1 -= sy * 0.01;
  o.lat2 += sy * 0.01;
  o.lon1 -= sx * 0.01;
  o.lon2 += sx * 0.01;

  if (o.lon1 > 180.0f)
    o.lon1 -= 360.0f;
  if (o.lon2 > 180.0f)
    o.lon2 -= 360.0f;

  scene.d.landscape.setPositionOptions (o);
}

void glgrib_window::toggleColorBar ()
{
  scene.toggleColorBar ();
}

void glgrib_window::showHelp () 
{
  onkey (0, 0, 0, 0, true);
}

static glgrib_field_vector * get_vector (glgrib_scene & scene)
{
  glgrib_field * f = scene.getCurrentField ();

  if (f == NULL) 
    return NULL;

  glgrib_field_vector * v = NULL;

  try
    {
      v = dynamic_cast<glgrib_field_vector*>(f);
    }
  catch (const std::bad_cast & e)
    {
      v = NULL;
    }

  return v;
}

void glgrib_window::toggle_show_vector ()
{
  glgrib_field_vector * v = get_vector (scene);
  if (v)
    v->toggleShowVector ();
}


void glgrib_window::toggle_show_norm ()
{
  glgrib_field_vector * v = get_vector (scene);
  if (v)
    v->toggleShowNorm ();
}

void glgrib_window::resample_current_field ()
{
  glgrib_field * f = scene.getCurrentField ();

  if (f == NULL) 
    return;

  glgrib_field_vector * v = NULL;

  try
    {
      v = dynamic_cast<glgrib_field_vector*>(f);
    }
  catch (const std::bad_cast & e)
    {
      return;
    }

  if (v == NULL)
    return;

  v->reSample (scene.d.view);

}

void glgrib_window::save_current_palette ()
{
  glgrib_field * f = scene.getCurrentField ();
  if (f == NULL)
    return;
  f->saveOptions ();
}

void glgrib_window::remove_field (int rank)
{
  glgrib_field * f = NULL;
  if ((rank < 0) || (rank > scene.fieldlist.size ()-1))
    return;
  if (scene.fieldlist[rank] != NULL)
    delete scene.fieldlist[rank];
  scene.fieldlist[rank] = f;
}

void glgrib_window::load_field (const glgrib_options_field & opts, int rank)
{

  if ((rank < 0) || (rank > 11))
    return;

  makeCurrent ();

  glgrib_field * f = new glgrib_field_scalar ();
  f->setup (&scene.ld, opts);

  if (rank > scene.fieldlist.size () - 1)
    scene.fieldlist.push_back (f);
  else
    scene.fieldlist[rank] = f;
  
}

void glgrib_window::toggle_transform_type ()
{
  scene.d.view.toggleTransformType ();
}

void glgrib_window::next_projection ()
{
  scene.d.view.nextProjection ();
}

void glgrib_window::duplicate ()
{
  cloned = true;
}

void glgrib_window::rotate_light_north ()
{
  float x, y;
  scene.getLightPos (&x, &y);
  y += 5.0f;
  scene.setLightPos (x, y);
}

void glgrib_window::rotate_light_south ()
{
  float x, y;
  scene.getLightPos (&x, &y);
  y -= 5.0f;
  scene.setLightPos (x, y);
}

void glgrib_window::rotate_light_west  ()
{
  float x, y;
  scene.getLightPos (&x, &y);
  x -= 5.0f;
  scene.setLightPos (x, y);
}

void glgrib_window::rotate_light_east  ()
{
  float x, y;
  scene.getLightPos (&x, &y);
  x += 5.0f;
  scene.setLightPos (x, y);
}

void glgrib_window::toggle_light ()
{
  if (scene.hasLight ())
    scene.unsetLight ();
  else
    scene.setLight ();
}

void glgrib_window::next_palette ()
{
  glgrib_field * f = scene.getCurrentField ();
  if (f != NULL)
    f->setNextPalette ();
}

void glgrib_window::scale_palette_up ()
{
  glgrib_field * fld  = scene.getCurrentField ();
  if (fld == NULL)
    return;
  fld->scalePaletteUp ();
}

void glgrib_window::scale_palette_down ()
{
  glgrib_field * fld  = scene.getCurrentField ();
  if (fld == NULL)
    return;
  fld->scalePaletteDown ();
}

void glgrib_window::select_field (int ifield)
{
  scene.setCurrentFieldRank (ifield);
}

void glgrib_window::scale_field_down ()
{
  glgrib_field * f = scene.getCurrentField ();
  if (f != NULL)
    {
      const glgrib_options_field & o = f->getOptions ();
      f->setScale (o.scale - 0.01);
    }
}

void glgrib_window::scale_field_up ()
{
  glgrib_field * f = scene.getCurrentField ();
  if (f != NULL)
    {
      const glgrib_options_field & o = f->getOptions ();
      f->setScale (o.scale + 0.01);
    }
}

void glgrib_window::toggle_hide_field ()
{
  glgrib_field * fld = scene.getCurrentField ();
  if (fld == NULL)
    return;
  if (fld->visible ())
    fld->hide ();
  else
    fld->show ();
}

void glgrib_window::hide_all_fields ()
{
  for (auto f : scene.fieldlist)
    if (f != NULL)
      f->hide ();
}

void glgrib_window::show_all_fields ()
{
  for (auto f : scene.fieldlist)
    if (f != NULL)
      f->show ();
}

int glgrib_window::get_latlon_from_cursor (float * lat, float * lon)
{
  double xpos, ypos;

  glfwGetCursorPos (window, &xpos, &ypos);
  ypos = opts.height - ypos;
  
  return scene.d.view.get_latlon_from_screen_coords (xpos, ypos, lat, lon);
}

void glgrib_window::snapshot (const std::string & format)
{
  unsigned char * rgb = new unsigned char[opts.width * opts.height * 4];

  // glReadPixels does not seem to work well with all combinations of width/height
  // when GL_RGB is used; GL_RGBA on the other hand seems to work well
  // So we get it in RGBA mode and throw away the alpha channel
  glReadPixels (0, 0, opts.width, opts.height, GL_RGBA, GL_UNSIGNED_BYTE, rgb);

  for (int i = 0; i < opts.width * opts.height; i++)
    for (int j = 0; j < 3; j++)
      rgb[3*i+j] = rgb[4*i+j];


  // %N  -> snapshot_cnt
  // %D  -> date

  const glgrib_option_date * date = scene.get_date ();
  std::string dstr = date ? date->asString () : "";
  for (int i = 0; i < dstr.size (); i++)
    if ((dstr[i] == ' ') || (dstr[i] == '/'))
      dstr[i] = ':';

  std::string fmt = format;

  for (int i = 0; ; i++)
    {
      size_t pos = fmt.find ("%D", i);
      if (pos == std::string::npos)
        break;
      i = pos + 1;
      fmt.replace (pos, 2, dstr);
    }

  
  std::vector<int> posN;
  for (int i = 0; ; i++)
    {
      size_t pos = fmt.find ("%N", i);
      if (pos == std::string::npos)
        break;
      posN.push_back (pos);
      i = pos + 1;
    }
  
  std::string filename;

  if (posN.size () > 0)
    {
      while (1)
        {
          struct stat st;
          char cnt[16];
      
          sprintf (cnt, "%4.4d", snapshot_cnt);
           
      
          filename = fmt;
      
          for (int i = posN.size () - 1; i >= 0; i--)
            filename.replace (posN[i], 2, std::string (cnt));
      
          if (stat (filename.c_str (), &st) < 0)
            break;
          else
            snapshot_cnt++;
        }
      snapshot_cnt++;
    }
  else
    {
      filename = fmt;
    }

  glgrib_write_png (filename, opts.width, opts.height, rgb);

  delete [] rgb;
}

void glgrib_window::framebuffer (const std::string & format)
{
  if (opts.antialiasing.on)
    {
      unsigned int framebufferMMSA;
      unsigned int texturebufferMMSA;
      unsigned int renderbufferMMSA;
      unsigned int framebufferPOST;
      unsigned int texturebufferPOST;

      glGenFramebuffers (1, &framebufferMMSA);
      glBindFramebuffer (GL_FRAMEBUFFER, framebufferMMSA);

      glGenTextures (1, &texturebufferMMSA);
      glBindTexture (GL_TEXTURE_2D_MULTISAMPLE, texturebufferMMSA);
      glTexImage2DMultisample (GL_TEXTURE_2D_MULTISAMPLE, opts.antialiasing.samples, 
		               GL_RGB, opts.width, opts.height, GL_TRUE);
      glBindTexture (GL_TEXTURE_2D_MULTISAMPLE, 0);
      glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                              GL_TEXTURE_2D_MULTISAMPLE, texturebufferMMSA, 0);

      glGenRenderbuffers (1, &renderbufferMMSA);
      glBindRenderbuffer (GL_RENDERBUFFER, renderbufferMMSA);
      glRenderbufferStorageMultisample (GL_RENDERBUFFER, opts.antialiasing.samples, 
		                        GL_DEPTH24_STENCIL8, opts.width, opts.height);
      glBindRenderbuffer (GL_RENDERBUFFER, 0);
      glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, 
                                 GL_RENDERBUFFER, renderbufferMMSA);

      glBindFramebuffer (GL_FRAMEBUFFER, 0);

      if (glCheckFramebufferStatus (GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error (std::string ("Framebuffer is not complete"));

      glGenFramebuffers (1, &framebufferPOST);
      glBindFramebuffer (GL_FRAMEBUFFER, framebufferPOST);

      glGenTextures (1, &texturebufferPOST);
      glBindTexture (GL_TEXTURE_2D, texturebufferPOST);
      glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, opts.width, opts.height, 
                    0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                              GL_TEXTURE_2D, texturebufferPOST, 0);	

      if (glCheckFramebufferStatus (GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error (std::string ("Framebuffer is not complete"));

      glBindFramebuffer (GL_FRAMEBUFFER, framebufferMMSA);
   
      scene.display ();

      glBindFramebuffer (GL_READ_FRAMEBUFFER, framebufferMMSA);
      glBindFramebuffer (GL_DRAW_FRAMEBUFFER, framebufferPOST);
      glBlitFramebuffer (0, 0, opts.width, opts.height, 0, 0, opts.width, opts.height, 
                         GL_COLOR_BUFFER_BIT, GL_NEAREST);

      glBindFramebuffer (GL_FRAMEBUFFER, framebufferPOST);
   
      snapshot (format);

      glDeleteTextures (1, &texturebufferPOST);
      glDeleteFramebuffers (1, &framebufferPOST);
      glDeleteRenderbuffers (1, &renderbufferMMSA);
      glDeleteTextures (1, &texturebufferMMSA);
      glDeleteFramebuffers (1, &framebufferMMSA);
   
      glBindFramebuffer (GL_FRAMEBUFFER, 0);
    }
  else
    {
      unsigned int framebuffer;
      unsigned int renderbuffer;
      unsigned int texturebuffer;
   
      glGenFramebuffers (1, &framebuffer);
      glBindFramebuffer (GL_FRAMEBUFFER, framebuffer);
     
      glGenTextures (1, &texturebuffer);
      glBindTexture (GL_TEXTURE_2D, texturebuffer);
      glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, opts.width, 
                    opts.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
     
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                              GL_TEXTURE_2D, texturebuffer, 0);
     
      glGenRenderbuffers (1, &renderbuffer);
      glBindRenderbuffer (GL_RENDERBUFFER, renderbuffer);
     
      glRenderbufferStorage (GL_RENDERBUFFER, GL_DEPTH32F_STENCIL8, 
                             opts.width, opts.height); 
      glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, 
                                 GL_RENDERBUFFER, renderbuffer); 
   
      if (glCheckFramebufferStatus (GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error (std::string ("Framebuffer is not complete"));
   
      scene.display ();
   
      snapshot (format);
   
      glDeleteRenderbuffers (1, &renderbuffer); 
      glDeleteTextures (1, &texturebuffer);
      glDeleteFramebuffers (1, &framebuffer);

      glBindFramebuffer (GL_FRAMEBUFFER, 0);
    }

}

void glgrib_window::display_cursor_position (double xpos, double ypos)
{
  float lat, lon;
  if (get_latlon_from_cursor (&lat, &lon))
    {
      std::string title_;
      const glgrib_field * field = scene.getCurrentField ();
      if (field)
        {
          int jglo = field->getGeometry ()->latlon2index (lat, lon);
	  if (jglo >= 0)
            {
              char tmp[128];
              std::vector<float> value = field->getValue (jglo);
              sprintf (tmp, "%6.2f %6.2f %8d", lat, lon, jglo);
              title_ = std::string (tmp);
	      for (auto v : value)
                {
                  sprintf (tmp, " %8.3g", v);
                  title_ = title_ + std::string (tmp);
                }
	    }
        }
      if (title_ == "")
        {
          char tmp[128];
          sprintf (tmp, "%6.2f %6.2f", lat, lon);
          title_ = std::string (tmp);
        }
      scene.setMessage (title_);
      glfwSetWindowTitle (window, title_.c_str ());
      return;
    }
  glfwSetWindowTitle (window, title.c_str ());
}

void glgrib_window::toggle_cursorpos_display ()
{
  if (cursorpos)
    glfwSetCursorPosCallback (window, NULL);
  else
    glfwSetCursorPosCallback (window, cursor_position_callback);
  cursorpos = ! cursorpos;
  scene.setMessage (std::string (""));
  glfwSetWindowTitle (window, title.c_str ());
}

void glgrib_window::onclick (int button, int action, int mods)
{
  enum
  {
    NONE    = 0,
    SHIFT   = GLFW_MOD_SHIFT,
    CONTROL = GLFW_MOD_CONTROL,
    ALT     = GLFW_MOD_ALT
  };

#define if_click(mm, k, action) \
do { \
if ((button == GLFW_MOUSE_BUTTON_##k) && (mm == mods)) \
  {                                                    \
    action;                                            \
    return;                                            \
  }                                                    \
} while (0)

  if (action == GLFW_PRESS) 
    {
      if_click (NONE,    LEFT, centerViewAtCursorPos ());
      if_click (CONTROL, LEFT, centerLightAtCursorPos ());
      if_click (ALT,     LEFT, debugTriangleNumber ());
    }
}

void glgrib_window::centerLightAtCursorPos ()
{
  float lat, lon;
  if (get_latlon_from_cursor (&lat, &lon))
    scene.setLightPos (lon, lat);
}

void glgrib_window::centerViewAtCursorPos ()
{
  glgrib_options_view o = scene.d.view.getOptions ();
  if (get_latlon_from_cursor (&o.lat, &o.lon))
    {
      scene.d.view.setOptions (o);
      float xpos, ypos;
      scene.d.view.get_screen_coords_from_latlon (&xpos, &ypos, o.lat, o.lon);
      glfwSetCursorPos (window, xpos, ypos);
    }
}

void glgrib_window::debugTriangleNumber ()
{
  glgrib_field * f = scene.getCurrentField ();
  float lon, lat;
  if (get_latlon_from_cursor (&lat, &lon) && (f != NULL))
    {
      const_glgrib_geometry_ptr geometry = f->getGeometry ();
      std::cout << " getTriangle = " << geometry->getTriangle (lon, lat) << std::endl;
    }
}

void glgrib_window::scroll (double xoffset, double yoffset)
{
  makeCurrent ();
  glgrib_options_view o = scene.d.view.getOptions ();
  if (yoffset > 0)
    {
      if (o.fov < 1.0f)
        o.fov += 0.1f;
      else
        o.fov += 1.0f;
    }
  else 
    {
      if (o.fov < 1.0f)
        o.fov -= 0.1f;
      else
        o.fov -= 1.0f;
      if (o.fov <= 0.0f)
        o.fov = 0.1f;
    }
  scene.d.view.setOptions (o);
  scene.resize ();
}

void glgrib_window::renderFrame ()
{
  nframes++;

  makeCurrent ();

  scene.update ();

  if (Shell.started ())
    Shell.lock ();

  scene.display (); 

  glfwSwapBuffers (window);

  if (Shell.started ())
    Shell.unlock ();

}

void glgrib_window::run (glgrib_shell * shell)
{
  renderFrame ();
  glfwPollEvents ();
  
  if ((glfwGetKey (window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
   || (glfwWindowShouldClose (window) != 0) || (shell && shell->closed ()))
    closed = true;

}


void glgrib_window::resize (int w, int h)
{
  opts.width = w;
  opts.height = h;
  makeCurrent ();
  glViewport (0, 0, opts.width, opts.height);
  scene.setViewport (opts.width, opts.height);
}

void glgrib_window::setHints ()
{
  if (opts.antialiasing.on)
    glfwWindowHint (GLFW_SAMPLES, opts.antialiasing.samples);
  glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, opts.version_major);
  glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, opts.version_minor);
  glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 
  glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  if (opts.debug.on)
    glfwWindowHint (GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

}

static int idcount = 0;

glgrib_window::glgrib_window ()
{
  id_ = idcount++;
}


glgrib_window::glgrib_window (const glgrib_options & opts)
{
  create (opts);
}

void glgrib_window::create (const glgrib_options & o)
{
  id_ = idcount++;
  opts = o.window;

  if (opts.title == "")
    title = std::string ("Window #") + std::to_string (id_);
  else
    title = opts.title;


  createGFLWwindow (NULL);

  t0 = current_time ();

  if (opts.info.on)
    {
#define PR(x) \
  printf (" %-32s = %s\n", #x, glGetString (x))
      PR (GL_VENDOR);
      PR (GL_RENDERER);
      PR (GL_VERSION);
      PR (GL_EXTENSIONS);
    }

  if ((opts.position.x != -1) && (opts.position.y != -1))
    glfwSetWindowPos (window, opts.position.x, opts.position.y);
}

void glgrib_window::createGFLWwindow (GLFWwindow * context)
{
  setHints ();
  
  window = glfwCreateWindow (opts.width, opts.height, title.c_str (), NULL, context);
  glfwSetWindowUserPointer (window, this);

  if (window == NULL)
    {
      fprintf (stderr, "Failed to open GLFW window. "
                       "If you have an Intel GPU, they are not 3.3 compatible. "
                       "Try the 2.1 version of the tutorials.\n");
      glfwTerminate ();
      return;
    }

  makeCurrent ();
  gl_init ();
  
  glewExperimental = true; // Needed for core profile
  if (glewInit () != GLEW_OK) 
    {
      fprintf (stderr, "Failed to initialize GLEW\n");
      glfwTerminate ();
      return;
    }
  
  if (opts.debug.on)
   {
     GLint flags; 
     glGetIntegerv (GL_CONTEXT_FLAGS, &flags);
     if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
       {
         glEnable (GL_DEBUG_OUTPUT);
         glEnable (GL_DEBUG_OUTPUT_SYNCHRONOUS); 
         glDebugMessageCallback (debug_callback, this);
         glDebugMessageControl (GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
       }
   }

  glfwSetInputMode (window, GLFW_STICKY_KEYS, GL_TRUE);
  glfwSetKeyCallback (window, key_callback);
  glfwSetScrollCallback (window, scroll_callback);
  glfwSetMouseButtonCallback (window, mouse_button_callback);
  glfwSetFramebufferSizeCallback (window, resize_callback);  


}

  
glgrib_window::~glgrib_window ()
{
  if (window)
    glfwDestroyWindow (window);
  if (opts.statistics.on)
    {
      double t1 = current_time ();
      printf ("Window #%d rendered %f frames/sec\n", id_, nframes/(t1 - t0));
    }
}

glgrib_window * glgrib_window::clone ()
{
  glgrib_window * w = new glgrib_window ();

#define COPY(x) do { w->x = x; } while (0)
  COPY (opts);

  if (w->opts.title == "")
    w->title = std::string ("Window");
  else
    w->title = w->opts.title;

  w->title = w->title + " #" + std::to_string (w->id_);

  w->createGFLWwindow (window); // use already existing context

  COPY (scene);                 // copy the scene; invoke operator=

#undef COPY

  cloned = false;

  return w;
}

void glgrib_window_set::run (glgrib_shell * shell)
{
  while (! empty ())
    {
      const glgrib_window * wl = NULL;

      for (auto w : *this)
	if (w->isMaster ())
          {
            if (wl != NULL)
              w->unsetMaster ();
	    else
              wl = w;
	  }
      if (wl != NULL)
        for (auto w : *this)
          w->scene.d.view.setOptions (wl->scene.d.view.getOptions ());
      for (auto w : *this)
        {
	  if ((! shell) && (w->getStartShell ()))
            {
	      Shell.setup (w->scene.getOptions ().shell);
	      Shell.start (this);
              shell = &Shell;
	    }

          w->run (shell);

          if (w->isClosed ())
            {
              erase (w);
    	      delete w;
              break;
            }

          if (w->isCloned ())
            {
              glgrib_window * w1 = w->clone ();
    	      insert (w1);
              break;
	    }

	}
    }
}

glgrib_window * glgrib_window_set::getWindowById (int id)
{
  for (auto w : *this)
    if (w->id () == id)
      return w;
  return NULL;
}

void glgrib_window_set::close ()
{
  for (auto win : *this)
    win->shouldClose ();
}

#define GLMESS(x) case GL_DEBUG_SOURCE_##x: return #x
static const char * debug_source (unsigned int source)
{
  switch (source)
    {
      GLMESS (API); GLMESS (WINDOW_SYSTEM); GLMESS (SHADER_COMPILER);
      GLMESS (THIRD_PARTY); GLMESS (APPLICATION); GLMESS (OTHER);
    }
  return "UNKNOWN";
}
#undef GLMESS

#define GLMESS(x) case GL_DEBUG_TYPE_##x: return #x
static const char * debug_type (unsigned int type)
{
  switch (type)
    {
      GLMESS (ERROR); GLMESS (DEPRECATED_BEHAVIOR); GLMESS (UNDEFINED_BEHAVIOR);
      GLMESS (PORTABILITY); GLMESS (PERFORMANCE); GLMESS (MARKER);
      GLMESS (PUSH_GROUP); GLMESS (POP_GROUP); GLMESS (OTHER);
    } 
  return "UNKNOWN";
}
#undef GLMESS

#define GLMESS(x) case GL_DEBUG_SEVERITY_##x: return #x
static const char * debug_severity (unsigned int severity)
{
  switch (severity)
    {
      GLMESS (HIGH); GLMESS (MEDIUM);
      GLMESS (LOW); GLMESS (NOTIFICATION);
    } 
  return "UNKNOWN";
}
#undef GLMESS

void glgrib_window::debug (unsigned int source, unsigned int type, GLuint id, 
		           unsigned int severity, int length, const char * message)
{
  // ignore non-significant error/warning codes
  if (id == 131169 || id == 131185 || id == 131218 || id == 131204) 
    return; 
  printf ("%-20s | %-20s | %-30s | %10d | %s\n", debug_source (source), 
          debug_severity (severity), debug_type (type), id, message);
}

void glgrib_window::setOptions (const glgrib_options_window & o)
{
  if ((o.width != opts.width) || (o.height != opts.height))
    {
      glfwSetWindowSize(window, o.width, o.height);
      opts.width = o.width;
      opts.height = o.height;
    }
  if (o.title != opts.title)
    {
      opts.title = o.title;
      glfwSetWindowTitle (window, opts.title.c_str ());
    }
  if ((o.position.x != opts.position.x) || (o.position.y != opts.position.y))
    {
      opts.position = o.position;
      glfwSetWindowPos (window, opts.position.x, opts.position.y);
    }
}





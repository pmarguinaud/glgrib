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
#include <iostream>

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

void glgrib_window::onkey (int key, int scancode, int action, int mods)
{
  enum
  {
    NONE    = 0,
    SHIFT   = GLFW_MOD_SHIFT,
    CONTROL = GLFW_MOD_CONTROL,
    ALT     = GLFW_MOD_ALT
  };

#define if_key(mm, k, action) \
do { \
if ((key == GLFW_KEY_##k) && (mm == mods)) \
  {                                        \
    action;                                \
    return;                                \
  }                                        \
} while (0)


  if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
      if_key (NONE,    T     , toggle_cursorpos_display ());
      if_key (NONE,    TAB   , toggle_rotate            ());
      if_key (CONTROL, TAB   , toggle_rotate_light      ());
      if_key (NONE,    Y     , toggle_wireframe         ());
      if_key (NONE,    D     , framebuffer              ());
      if_key (NONE,    W     , widen_fov                ());
      if_key (NONE,    S     , snapshot                 ());
      if_key (NONE,    Q     , shrink_fov               ());
      if_key (NONE,    P     , toggle_flat              ());
      if_key (NONE,    6     , increase_radius          ());
      if_key (NONE,    EQUAL , decrease_radius          ());
      if_key (NONE,    SPACE , reset_view               ());
      if_key (NONE,    UP    , rotate_north             ());
      if_key (NONE,    DOWN  , rotate_south             ());
      if_key (NONE,    LEFT  , rotate_west              ());
      if_key (NONE,    RIGHT , rotate_east              ());

      if_key (NONE,    F1    , select_field           ( 0));
      if_key (NONE,    F2    , select_field           ( 1));
      if_key (NONE,    F3    , select_field           ( 2));
      if_key (NONE,    F4    , select_field           ( 3));
      if_key (NONE,    F5    , select_field           ( 4));
      if_key (NONE,    F6    , select_field           ( 5));
      if_key (NONE,    F7    , select_field           ( 6));
      if_key (NONE,    F8    , select_field           ( 7));
      if_key (NONE,    F9    , select_field           ( 8));
      if_key (NONE,    F10   , select_field           ( 9));
      if_key (NONE,    F11   , select_field           (10));
      if_key (NONE,    F12   , select_field           (11));

      if_key (CONTROL, F1    , { hide_all_fields (); select_field ( 0); toggle_hide_field (); });
      if_key (CONTROL, F2    , { hide_all_fields (); select_field ( 1); toggle_hide_field (); });
      if_key (CONTROL, F3    , { hide_all_fields (); select_field ( 2); toggle_hide_field (); });
      if_key (CONTROL, F4    , { hide_all_fields (); select_field ( 3); toggle_hide_field (); });
      if_key (CONTROL, F5    , { hide_all_fields (); select_field ( 4); toggle_hide_field (); });
      if_key (CONTROL, F6    , { hide_all_fields (); select_field ( 5); toggle_hide_field (); });
      if_key (CONTROL, F7    , { hide_all_fields (); select_field ( 6); toggle_hide_field (); });
      if_key (CONTROL, F8    , { hide_all_fields (); select_field ( 7); toggle_hide_field (); });
      if_key (CONTROL, F9    , { hide_all_fields (); select_field ( 8); toggle_hide_field (); });
      if_key (CONTROL, F10   , { hide_all_fields (); select_field ( 9); toggle_hide_field (); });
      if_key (CONTROL, F11   , { hide_all_fields (); select_field (10); toggle_hide_field (); });
      if_key (CONTROL, F12   , { hide_all_fields (); select_field (11); toggle_hide_field (); });
      if_key (CONTROL, H     , show_all_fields          ());

      if_key (NONE,    H     , toggle_hide_field        ());
      if_key (NONE,    G     , scale_field_up           ());
      if_key (CONTROL, G     , scale_field_down         ());
      if_key (NONE,    F     , scale_palette_up         ());
      if_key (CONTROL, F     , scale_palette_down       ());
      if_key (NONE,    J     , next_palette             ());
      if_key (NONE,    L     , toggle_light             ());
      if_key (CONTROL, UP    , rotate_light_north       ());
      if_key (CONTROL, DOWN  , rotate_light_south       ());
      if_key (CONTROL, LEFT  , rotate_light_west        ());
      if_key (CONTROL, RIGHT , rotate_light_east        ());
      if_key (NONE,    K,      movie                    ());
      if_key (CONTROL, C,      duplicate                ());
      if_key (CONTROL, P,      next_projection          ());
      if_key (SHIFT,   P,      toggle_transform_type    ());
      if_key (CONTROL, S,      save_current_palette     ());


    }

#undef if_key
}

void glgrib_window::save_current_palette ()
{
  glgrib_field * f = scene.getCurrentField ();
  if (f == NULL)
    return;
  f->saveSettings ();
}

void glgrib_window::set_field_palette_min (const float min)
{
  glgrib_field * f = scene.getCurrentField ();
  if (f == NULL)
    return;
  f->dopts.palette.min = min;
}

void glgrib_window::set_field_palette_max (const float max)
{
  glgrib_field * f = scene.getCurrentField ();
  if (f == NULL)
    return;
  f->dopts.palette.max = max;
}

void glgrib_window::set_field_palette (const std::string & p)
{
  glgrib_field * f = scene.getCurrentField ();
  if (f == NULL)
    return;
  f->setPalette (p);
}

void glgrib_window::remove_field (int rank)
{
  glgrib_field F;
  if ((rank < 0) || (rank > scene.fieldlist.size ()-1))
    return;
  scene.fieldlist[rank] = F;
}

void glgrib_window::load_field (const glgrib_options_field & opts, int rank)
{
  glgrib_field F;

  if ((rank < 0) || (rank > 11))
    return;

  makeCurrent ();

  F.init (opts);

  if (rank > scene.fieldlist.size () - 1)
    scene.fieldlist.push_back (F);
  else
    scene.fieldlist[rank] = F;
  
}

void glgrib_window::toggle_transform_type ()
{
  scene.view.toggleTransformType ();
}

void glgrib_window::next_projection ()
{
  scene.view.nextProjection ();
}

void glgrib_window::duplicate ()
{
  cloned = true;
}

void glgrib_window::movie ()
{
  scene.toggleMovie ();
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
  glgrib_field_display_options * fopt = scene.getCurrentFieldOpts ();
  if (fopt)
    fopt->palette = get_next_palette (fopt->palette);
}

void glgrib_window::scale_palette_up ()
{
  glgrib_field * fld  = scene.getCurrentField ();
  if (fld == NULL)
    return;
  glgrib_field_display_options * fopt = scene.getCurrentFieldOpts ();
  if (! fopt->palette.hasMin ()) fopt->palette.min = fld->getMinValue ();
  if (! fopt->palette.hasMax ()) fopt->palette.max = fld->getMaxValue ();
  float d = fopt->palette.max - fopt->palette.min;
  fopt->palette.min -= d * 0.025;
  fopt->palette.max += d * 0.025;
}

void glgrib_window::scale_palette_down ()
{
  glgrib_field * fld  = scene.getCurrentField ();
  if (fld == NULL)
    return;
  glgrib_field_display_options * fopt = scene.getCurrentFieldOpts ();
  if (! fopt->palette.hasMin ()) fopt->palette.min = fld->getMinValue ();
  if (! fopt->palette.hasMax ()) fopt->palette.max = fld->getMaxValue ();
  float d = fopt->palette.max - fopt->palette.min;
  fopt->palette.min += d * 0.025;
  fopt->palette.max -= d * 0.025;
}

void glgrib_window::select_field (int ifield)
{
  scene.setCurrentFieldRank (ifield);
}

void glgrib_window::scale_field_down ()
{
  glgrib_field_display_options * fopt = scene.getCurrentFieldOpts ();
  if (fopt)
    fopt->scale -= 0.01;
}

void glgrib_window::scale_field_up ()
{
  glgrib_field_display_options * fopt = scene.getCurrentFieldOpts ();
  if (fopt)
    fopt->scale += 0.01;
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
  for (int i = 0; i < scene.fieldlist.size (); i++)
    scene.fieldlist[i].hide ();
}

void glgrib_window::show_all_fields ()
{
  for (int i = 0; i < scene.fieldlist.size (); i++)
    scene.fieldlist[i].show ();
}

int glgrib_window::get_latlon_from_cursor (float * lat, float * lon)
{
  double xpos, ypos;

  glfwGetCursorPos (window, &xpos, &ypos);
  ypos = opts.height - ypos;
  
  return scene.view.get_latlon_from_screen_coords (xpos, ypos, lat, lon);
}

void glgrib_window::snapshot ()
{
  unsigned char * rgb = new unsigned char[opts.width * opts.height * 4];
  char filename[32];

  // glReadPixels does not seem to work well with all combinations of width/height
  // when GL_RGB is used; GL_RGBA on the other hand seems to work well
  // So we get it in RGBA mode and throw away the alpha channel
  glReadPixels (0, 0, opts.width, opts.height, GL_RGBA, GL_UNSIGNED_BYTE, rgb);

  for (int i = 0; i < opts.width * opts.height; i++)
    for (int j = 0; j < 3; j++)
      rgb[3*i+j] = rgb[4*i+j];

  while (1)
    {
      struct stat st;
      sprintf (filename, "snapshot_%4.4d.png", snapshot_cnt);
      if (stat (filename, &st) < 0)
        break;
      else
        snapshot_cnt++;
    }

  glgrib_png (filename, opts.width, opts.height, rgb);
  snapshot_cnt++;

  delete [] rgb;
}

void glgrib_window::framebuffer ()
{
  unsigned int framebuffer;
  glGenFramebuffers (1, &framebuffer);
  glBindFramebuffer (GL_FRAMEBUFFER, framebuffer);
 

  unsigned int textureColorbuffer;
  glGenTextures (1, &textureColorbuffer);
  glBindTexture (GL_TEXTURE_2D, textureColorbuffer);
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, opts.width, opts.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

  unsigned int rbo;
  glGenRenderbuffers (1, &rbo);
  glBindRenderbuffer (GL_RENDERBUFFER, rbo);

  glRenderbufferStorage (GL_RENDERBUFFER, GL_DEPTH32F_STENCIL8, opts.width, opts.height); 
  glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); 

  if (glCheckFramebufferStatus (GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    printf ("!= GL_FRAMEBUFFER_COMPLETE\n");

  scene.display ();

  snapshot ();

  glDeleteFramebuffers (1, &framebuffer);
  glBindFramebuffer (GL_FRAMEBUFFER, 0);
}

void glgrib_window::display_cursor_position (double xpos, double ypos)
{
  float lat, lon;
  if (get_latlon_from_cursor (&lat, &lon))
    {
      char title[128];
      const glgrib_field * field = scene.getCurrentField ();
      if (field)
        {
          int jglo = field->geometry ? field->geometry->latlon2index (lat, lon) : -1;
	  if (jglo >= 0)
            {
              float value = field->getValue (jglo);
              sprintf (title, "%6.2f %6.2f %6.2g", lat, lon, value);
	    }
	  else
            {
              sprintf (title, "%6.2f %6.2f", lat, lon);
	    }
	  scene.setMessage (std::string (title));
          glfwSetWindowTitle (window, title);
	  return;
        }
    }
  glfwSetWindowTitle (window, opts.title.c_str ());
}

void glgrib_window::toggle_cursorpos_display ()
{
  if (cursorpos)
    glfwSetCursorPosCallback (window, NULL);
  else
    glfwSetCursorPosCallback (window, cursor_position_callback);
  cursorpos = ! cursorpos;
  scene.setMessage (std::string (""));
  glfwSetWindowTitle (window, opts.title.c_str ());
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
  if (get_latlon_from_cursor (&scene.view.opts.lat, &scene.view.opts.lon))
    {
      scene.view.calcMVP ();
      float xpos, ypos;
      scene.view.get_screen_coords_from_latlon (&xpos, &ypos, scene.view.opts.lat, scene.view.opts.lon);
      glfwSetCursorPos (window, xpos, ypos);
    }
}

void glgrib_window::scroll (double xoffset, double yoffset)
{
  if (yoffset > 0)
    {
      if (scene.view.opts.fov < 1.0f)
        scene.view.opts.fov += 0.1f;
      else
        scene.view.opts.fov += 1.0f;
    }
  else 
    {
      if (scene.view.opts.fov < 1.0f)
        scene.view.opts.fov -= 0.1f;
      else
        scene.view.opts.fov -= 1.0f;
      if (scene.view.opts.fov <= 0.0f)
        scene.view.opts.fov = 0.1f;
    }
}

void glgrib_window::renderFrame ()
{
  nframes++;

  scene.update ();

  if (Shell.started ())
    Shell.lock ();

  makeCurrent ();
  scene.display (); 
  
  if (Shell.started ())
    Shell.unlock ();

  glfwSwapBuffers (window);
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
  glfwWindowHint (GLFW_SAMPLES, 4);
  glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 
  glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  if (opts.debug)
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
    opts.title = std::string ("Window #") + std::to_string (id_);

  scene.light.rotate = o.scene.light.rotate;
  scene.rotate_earth = o.scene.rotate_earth;
  if (o.scene.movie)
    scene.setMovie ();
  if (o.scene.light.on)
    scene.setLight ();


  createGFLWwindow (NULL);

  t0 = current_time ();
}

void glgrib_window::createGFLWwindow (GLFWwindow * context)
{
  setHints ();
  
  window = glfwCreateWindow (opts.width, opts.height, opts.title.c_str (), NULL, context);
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
  
  if (opts.debug)
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

  scene.setViewport (opts.width, opts.height);

}

  
glgrib_window::~glgrib_window ()
{
  if (window)
    glfwDestroyWindow (window);
  if (opts.statistics)
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
      for (glgrib_window_set::iterator it = begin (); 
           it != end (); it++)
        {
          glgrib_window * w = *it;

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
  for (glgrib_window_set::iterator it = begin (); it != end (); it++)
    if ((*it)->id () == id)
      return *it;
  return NULL;
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


#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glgrib_window.h"
#include "glgrib_png.h"
#include <iostream>


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


    }

#undef if_key
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
  glgrib_field_display_options * fopt = scene.currentFieldOpts;
  fopt->palette = get_next_palette (fopt->palette);
}

void glgrib_window::scale_palette_up ()
{
  glgrib_field * fld  = scene.currentField;
  if (fld == NULL)
    return;
  glgrib_field_display_options * fopt = scene.currentFieldOpts;
  if (! fopt->palette.hasMin ()) fopt->palette.min = fld->valmin;
  if (! fopt->palette.hasMax ()) fopt->palette.max = fld->valmax;
  float d = fopt->palette.max - fopt->palette.min;
  fopt->palette.min -= d * 0.025;
  fopt->palette.max += d * 0.025;
}

void glgrib_window::scale_palette_down ()
{
  glgrib_field * fld  = scene.currentField;
  if (fld == NULL)
    return;
  glgrib_field_display_options * fopt = scene.currentFieldOpts;
  if (! fopt->palette.hasMin ()) fopt->palette.min = fld->valmin;
  if (! fopt->palette.hasMax ()) fopt->palette.max = fld->valmax;
  float d = fopt->palette.max - fopt->palette.min;
  fopt->palette.min += d * 0.025;
  fopt->palette.max -= d * 0.025;
}

void glgrib_window::select_field (int ifield)
{
  scene.currentField      = scene.fieldlist[ifield];
  scene.currentFieldOpts = &scene.fieldoptslist[ifield];
}

void glgrib_window::scale_field_down ()
{
  scene.currentFieldOpts->scale -= 0.01;
}

void glgrib_window::scale_field_up ()
{
  scene.currentFieldOpts->scale += 0.01;
}

void glgrib_window::toggle_hide_field ()
{
  if (scene.hidden.find (scene.currentField) != scene.hidden.end ())
    scene.hidden.erase (scene.currentField);
  else
    scene.hidden.insert (scene.currentField);
}

void glgrib_window::hide_all_fields ()
{
  for (int i = 0; i < scene.fieldlist.size (); i++)
    scene.hidden.insert (scene.fieldlist[i]);
}

void glgrib_window::show_all_fields ()
{
  for (int i = 0; i < scene.fieldlist.size (); i++)
    scene.hidden.erase (scene.fieldlist[i]);
}

int glgrib_window::get_latlon_from_cursor (float * lat, float * lon)
{
  double xpos, ypos;

  glfwGetCursorPos (window, &xpos, &ypos);
  ypos = height - ypos;
  
  glm::vec3 centre (0.0f, 0.0f, 0.0f);
  glm::vec3 xc = scene.view.insersect_sphere (xpos, ypos, centre, 1.0f);

  if (centre != xc)
    {
      *lat = glm::degrees (asinf (xc.z));
      *lon = glm::degrees (atan2f (xc.y, xc.x));
      return 1;
    }

  return 0;
}

void glgrib_window::snapshot ()
{
  unsigned char * rgb = new unsigned char[width * height * 4];
  char filename[32];

  // glReadPixels does not seem to work well with all combinations of width/height
  // when GL_RGB is used; GL_RGBA on the other hand seems to work well
  // So we get it in RGBA mode and throw away the alpha channel
  glReadPixels (0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, rgb);

  for (int i = 0; i < width * height; i++)
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

  glgrib_png (filename, width, height, rgb);
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
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

  unsigned int rbo;
  glGenRenderbuffers (1, &rbo);
  glBindRenderbuffer (GL_RENDERBUFFER, rbo);

  glRenderbufferStorage (GL_RENDERBUFFER, GL_DEPTH32F_STENCIL8, width, height); 
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
      const glgrib_field * field = scene.currentField;
      int jglo = field->geometry->latlon2index (lat, lon);
      float value = field->getValue (jglo);

      sprintf (title, "(%7.2f, %7.2f, %f)", lat, lon, value);
      glfwSetWindowTitle (window, title);
    }
  else
    {
      glfwSetWindowTitle (window, title.c_str ());
    }
}

void glgrib_window::toggle_cursorpos_display ()
{
  if (cursorpos)
    glfwSetCursorPosCallback (window, NULL);
  else
    glfwSetCursorPosCallback (window, cursor_position_callback);
  cursorpos = ! cursorpos;
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
  if (get_latlon_from_cursor (&scene.view.params.latc, &scene.view.params.lonc))
    glfwSetCursorPos (window, width / 2., height / 2.);
}

void glgrib_window::scroll (double xoffset, double yoffset)
{
  if (yoffset > 0)
    scene.view.params.fov += 1;
  else
    scene.view.params.fov -= 1;
}

void glgrib_window::renderFrame ()
{
  scene.update ();

#pragma omp critical (RUN)
  {
    makeCurrent ();
    scene.display (); 
  }
  
  glfwSwapBuffers (window);
}

void glgrib_window::run (glgrib_shell * shell)
{
  renderFrame ();
  glfwPollEvents ();
  
  if ((glfwGetKey (window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
   || (glfwWindowShouldClose (window) != 0) || (shell && shell->closed ()))
    closed = true;;
}


void glgrib_window::resize (int width, int height)
{
  width = width;
  height = height;
  makeCurrent ();
  glViewport (0, 0, width, height);
  scene.view.setViewport (width, height);
}

void glgrib_window::setHints ()
{
  glfwWindowHint (GLFW_SAMPLES, 4);
  glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 
  glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

glgrib_window::glgrib_window (const glgrib_options & opts)
{
  create (opts);
}

void glgrib_window::create (const glgrib_options & opts)
{
  scene.rotate_light = opts.scene.rotate_light;
  scene.rotate_earth = opts.scene.rotate_earth;
  if (opts.scene.movie)
    scene.setMovie ();
  if (opts.scene.light)
    scene.setLight ();

  title = opts.landscape.geometry;
  width = opts.window.width;
  height = opts.window.height;

  createGFLWwindow (NULL);
}

void glgrib_window::createGFLWwindow (GLFWwindow * context)
{
  setHints ();
  
  window = glfwCreateWindow (width, height, title.c_str (), NULL, context);
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
  
  glewExperimental = true; // Needed for core profile
  if (glewInit () != GLEW_OK) 
    {
      fprintf (stderr, "Failed to initialize GLEW\n");
      glfwTerminate ();
      return;
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
}

glgrib_window * glgrib_window::clone ()
{
  glgrib_window * w = new glgrib_window ();

#define COPY(x) do { w->x = x; } while (0)
  COPY (width);
  COPY (height);
  COPY (title);

  w->createGFLWwindow (window); // use already existing context
  w->makeCurrent ();
  gl_init ();

  COPY (scene);                 // copy the scene; invoke operator=
#undef COPY
  w->scene.view.setViewport (width, height);

  return w;
}




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
      const glgrib_field * field = scene.field;
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

static 
void cursor_position_callback (GLFWwindow * window, double xpos, double ypos)
{
  glgrib_window * gwindow = (glgrib_window *)glfwGetWindowUserPointer (window);
  gwindow->display_cursor_position (xpos, ypos);
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

static 
void key_callback (GLFWwindow * window, int key, int scancode, int action, int mods)
{
#define if_key(mm, k, action) \
do { \
if ((key == GLFW_KEY_##k) && ((! mm) || (mm & mods))) \
  {                                                   \
    gwindow->action ();                               \
    return;                                           \
  }                                                   \
} while (0)

  glgrib_window * gwindow = (glgrib_window *)glfwGetWindowUserPointer (window);

  if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
      if_key (0, T     , toggle_cursorpos_display);
      if_key (0, TAB   , toggle_rotate           );
      if_key (0, Y     , toggle_wireframe        );
      if_key (0, F     , framebuffer             );
      if_key (0, W     , widen_fov               );
      if_key (0, S     , snapshot                );
      if_key (0, Q     , shrink_fov              );
      if_key (0, P     , toggle_flat             );
      if_key (0, 6     , increase_radius         );
      if_key (0, EQUAL , decrease_radius         );
      if_key (0, SPACE , reset_view              );
      if_key (0, UP    , rotate_north            );
      if_key (0, DOWN  , rotate_south            );
      if_key (0, LEFT  , rotate_west             );
      if_key (0, RIGHT , rotate_east             );
    }

#undef if_key
}

void glgrib_window::onclick (int button, int action, int mods)
{
  if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
      if (action == GLFW_PRESS) 
        {
	  if (get_latlon_from_cursor (&scene.view.latc, &scene.view.lonc))
	    glfwSetCursorPos (window, width / 2., height / 2.);
        }
    }
}

static
void mouse_button_callback (GLFWwindow * window, int button, int action, int mods)
{
  glgrib_window * gwindow = (glgrib_window *)glfwGetWindowUserPointer (window);
  gwindow->onclick (button, action, mods);
}

void glgrib_window::scroll (double xoffset, double yoffset)
{
  if (yoffset > 0)
    scene.view.fov += 1;
  else
    scene.view.fov -= 1;
}

static
void scroll_callback (GLFWwindow * window, double xoffset, double yoffset)
{
  glgrib_window * gwindow = (glgrib_window *)glfwGetWindowUserPointer (window);
  gwindow->scroll (xoffset, yoffset);
}

void glgrib_window::run (glgrib_shell * shell)
{
  while (1)
    {
      if (do_rotate)
        scene.view.lonc += 1.;
     
#pragma omp critical (RUN)
      {
        makeCurrent ();
        scene.display (); 
      }
  
      glfwSwapBuffers (window);
      glfwPollEvents ();
  
      if ((glfwGetKey (window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
       || (glfwWindowShouldClose (window) != 0) || (shell && shell->closed ()))
        break;
    } 
}


void glgrib_window::resize (int width, int height)
{
  width = width;
  height = height;
  makeCurrent ();
  glViewport (0, 0, width, height);
  scene.view.setViewport (width, height);
}

static void resize_callback (GLFWwindow * window, int width, int height)
{
  glgrib_window * gwindow = (glgrib_window *)glfwGetWindowUserPointer (window);
  gwindow->resize (width, height);
}

glgrib_window::glgrib_window (const glgrib_options & opts)
{
  title = opts.geometry;
  width = opts.width;
  height = opts.height;
  glfwWindowHint (GLFW_SAMPLES, 4);
  glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 
  glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
  window = glfwCreateWindow (width, height, title.c_str (), NULL, NULL);
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
  glfwDestroyWindow (window);
}



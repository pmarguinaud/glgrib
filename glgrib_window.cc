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

static
int get_latlon_from_cursor (GLFWwindow * window, float * lat, float * lon)
{
  double xpos, ypos;
  glgrib_window * gwindow = (glgrib_window *)glfwGetWindowUserPointer (window);
  glfwGetCursorPos (window, &xpos, &ypos);
  ypos = gwindow->width - ypos;
  
  glm::vec3 centre (0.0f, 0.0f, 0.0f);
  glm::vec3 xc = gwindow->scene.view.insersect_sphere (xpos, ypos, centre, 1.0f);

  if (centre != xc)
    {
      *lat = glm::degrees (asinf (xc.z));
      *lon = glm::degrees (atan2f (xc.y, xc.x));
      return 1;
    }

  return 0;
}

static
void snapshot (glgrib_window * gwindow)
{
  unsigned char * rgb = new unsigned char[gwindow->width * gwindow->height * 3];
  char filename[32];

  glReadPixels (0, 0, gwindow->width, gwindow->height, GL_RGB, GL_UNSIGNED_BYTE, rgb);

  while (1)
    {
      struct stat st;
      sprintf (filename, "snapshot_%4.4d.png", gwindow->snapshot_cnt);
      if (stat (filename, &st) < 0)
        break;
      else
        gwindow->snapshot_cnt++;
    }

  glgrib_png (filename, gwindow->width, gwindow->height, rgb);
  gwindow->snapshot_cnt++;

  delete [] rgb;
}

static
void framebuffer (glgrib_window * gwindow)
{
printf ("Framebuffer !\n");
  unsigned int framebuffer;
  glGenFramebuffers (1, &framebuffer);
  glBindFramebuffer (GL_FRAMEBUFFER, framebuffer);
 

  unsigned int textureColorbuffer;
  glGenTextures (1, &textureColorbuffer);
  glBindTexture (GL_TEXTURE_2D, textureColorbuffer);
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, gwindow->width, gwindow->height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

  unsigned int rbo;
  glGenRenderbuffers (1, &rbo);
  glBindRenderbuffer (GL_RENDERBUFFER, rbo);

  glRenderbufferStorage (GL_RENDERBUFFER, GL_DEPTH32F_STENCIL8, gwindow->width, gwindow->height); 
  glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); 

  if (glCheckFramebufferStatus (GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    printf ("!= GL_FRAMEBUFFER_COMPLETE\n");

  gwindow->scene.display (gwindow);

  snapshot (gwindow);

  glDeleteFramebuffers (1, &framebuffer);
  glBindFramebuffer (GL_FRAMEBUFFER, 0);
}

static 
void cursor_position_callback (GLFWwindow * window, double xpos, double ypos)
{
  float lat, lon;
  if (get_latlon_from_cursor (window, &lat, &lon))
    {
      char title[128];
      sprintf (title, "(%7.2f, %7.2f)", lat, lon);
      glfwSetWindowTitle (window, title);
    }
  else
    {
      glgrib_window * gwindow = (glgrib_window *)glfwGetWindowUserPointer (window);
      glfwSetWindowTitle (window, gwindow->title);
    }
}

static 
void key_callback (GLFWwindow * window, int key, int scancode, int action, int mods)
{
  glgrib_window * gwindow = (glgrib_window *)glfwGetWindowUserPointer (window);
  if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
      switch (key)
        {
          case GLFW_KEY_T:
            if (gwindow->cursorpos)
              glfwSetCursorPosCallback (window, NULL);
	    else
              glfwSetCursorPosCallback (window, cursor_position_callback);
	    gwindow->cursorpos = ! gwindow->cursorpos;
            glfwSetWindowTitle (window, gwindow->title);
            break;
          case GLFW_KEY_TAB:
            gwindow->do_rotate = ! gwindow->do_rotate;
            break;
          case GLFW_KEY_Y:
            gwindow->scene.landscape->toggle_wireframe ();
            break;
          case GLFW_KEY_F:
            framebuffer (gwindow);
            break;
          case GLFW_KEY_W:
            gwindow->scene.view.fov += 1.;
            break;
          case GLFW_KEY_S:
            snapshot (gwindow);
            break;
          case GLFW_KEY_Q:
            gwindow->scene.view.fov -= 1.;
            break;
          case GLFW_KEY_O:
	    if (gwindow->scene.field != NULL)
              gwindow->scene.field->toggle_hide ();
	    break;
          case GLFW_KEY_P:
            if (gwindow->scene.landscape != NULL)
              gwindow->scene.landscape->toggle_flat ();
            break;
          case GLFW_KEY_6:
            gwindow->scene.view.rc += 0.1;
            break;
          case GLFW_KEY_EQUAL:
            gwindow->scene.view.rc -= 0.1;
            break;
          case GLFW_KEY_SPACE:
            gwindow->scene.view.latc = 0.;
            gwindow->scene.view.lonc = 0.;
	    gwindow->scene.view.rc = 6.0;
	    gwindow->scene.view.fov = 20.;
	    break;
          case GLFW_KEY_UP:
            gwindow->scene.view.latc = gwindow->scene.view.latc + 5.;
            break;
          case GLFW_KEY_DOWN:
            gwindow->scene.view.latc = gwindow->scene.view.latc - 5.;
            break;
          case GLFW_KEY_LEFT:
            gwindow->scene.view.lonc = gwindow->scene.view.lonc - 5.;
            break;
          case GLFW_KEY_RIGHT:
            gwindow->scene.view.lonc = gwindow->scene.view.lonc + 5.;
            break;
	  default:
	    break;
	}
    }
}

static
void mouse_button_callback (GLFWwindow * window, int button, int action, int mods)
{
  if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
      if (action == GLFW_PRESS) 
        {
          glgrib_window * gwindow = (glgrib_window *)glfwGetWindowUserPointer (window);
	  if (get_latlon_from_cursor (window, &gwindow->scene.view.latc, &gwindow->scene.view.lonc))
	    glfwSetCursorPos (window, gwindow->width / 2., gwindow->height / 2.);
        }
    }
}

static
void scroll_callback (GLFWwindow * window, double xoffset, double yoffset)
{
  glgrib_window * gwindow = (glgrib_window *)glfwGetWindowUserPointer (window);
  if (yoffset > 0)
    gwindow->scene.view.fov += 1;
  else
    gwindow->scene.view.fov -= 1;
}

void glgrib_window::run (glgrib_shell * shell)
{
  while (1)
    {
      if (do_rotate)
        scene.view.lonc += 1.;
     
#pragma omp critical (RUN)
      {
        scene.display (this); 
      }
  
      glfwSwapBuffers (window);
      glfwPollEvents ();
  
      if ((glfwGetKey (window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
       || (glfwWindowShouldClose (window) != 0) || (shell && shell->closed ()))
        break;
    } 
}


glgrib_window::glgrib_window (const char * t, int w, int h)
{
  title = t;
  width = w;
  height = h;
  glfwWindowHint (GLFW_SAMPLES, 4);
  glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 
  glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
  window = glfwCreateWindow (width, height, title, NULL, NULL);
  glfwSetWindowUserPointer (window, this);

  if (window == NULL)
    {
      fprintf (stderr, "Failed to open GLFW window. "
                       "If you have an Intel GPU, they are not 3.3 compatible. "
                       "Try the 2.1 version of the tutorials.\n");
      glfwTerminate ();
      return;
    }

  glfwMakeContextCurrent (window);
  
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

}
  
glgrib_window::~glgrib_window ()
{

}



#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glgrib_png.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "glgrib_landscape_tex.h"
#include "glgrib_landscape_rgb.h"
#include "glgrib_field.h"
#include "glgrib_coords_world.h"
#include "glgrib_scene.h"
#include "glgrib_cube2.h"
#include "glgrib_grid.h"
#include "glgrib_view.h"
#include "glgrib_coastlines.h"


#include "glgrib_x11.h"

typedef struct glfw_ctx_t
{
  bool do_rotate = false;
  glgrib_view * view;
  int width, height;
  int snapshot_cnt = 0;
  glgrib_scene * scene;
  bool cursorpos = false;
  const char * title = "";
} glfw_ctx_t;

static
int get_latlon_from_cursor (GLFWwindow * window, float * lat, float * lon)
{
  double xpos, ypos;
  glfw_ctx_t * ctx = (glfw_ctx_t *)glfwGetWindowUserPointer (window);
  glfwGetCursorPos (window, &xpos, &ypos);
  ypos = ctx->width - ypos;
  
  glm::vec3 centre (0.0f, 0.0f, 0.0f);
  glm::vec3 xc = ctx->view->insersect_sphere (xpos, ypos, centre, 1.0f);

  if (centre != xc)
    {
      *lat = glm::degrees (asinf (xc.z));
      *lon = glm::degrees (atan2f (xc.y, xc.x));
      return 1;
    }

  return 0;
}

static
void snapshot (glfw_ctx_t * ctx)
{
  unsigned char * rgb = new unsigned char[ctx->width * ctx->height * 3];
  char filename[32];

  glReadPixels (0, 0, ctx->width, ctx->height, GL_RGB, GL_UNSIGNED_BYTE, rgb);

  while (1)
    {
      struct stat st;
      sprintf (filename, "snapshot_%4.4d.png", ctx->snapshot_cnt);
      if (stat (filename, &st) < 0)
        break;
      else
        ctx->snapshot_cnt++;
    }

  glgrib_png (filename, ctx->width, ctx->height, rgb);
  ctx->snapshot_cnt++;

  delete [] rgb;
}

static
void framebuffer (glfw_ctx_t * ctx)
{
printf ("Framebuffer !\n");
  unsigned int framebuffer;
  glGenFramebuffers (1, &framebuffer);
  glBindFramebuffer (GL_FRAMEBUFFER, framebuffer);
 

  unsigned int textureColorbuffer;
  glGenTextures (1, &textureColorbuffer);
  glBindTexture (GL_TEXTURE_2D, textureColorbuffer);
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, ctx->width, ctx->height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

  unsigned int rbo;
  glGenRenderbuffers (1, &rbo);
  glBindRenderbuffer (GL_RENDERBUFFER, rbo);

  glRenderbufferStorage (GL_RENDERBUFFER, GL_DEPTH32F_STENCIL8, ctx->width, ctx->height); 
  glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); 

  if (glCheckFramebufferStatus (GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    printf ("!= GL_FRAMEBUFFER_COMPLETE\n");

  ctx->scene->display ();

  snapshot (ctx);

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
      glfw_ctx_t * ctx = (glfw_ctx_t *)glfwGetWindowUserPointer (window);
      glfwSetWindowTitle (window, ctx->title);
    }
}

static 
void key_callback (GLFWwindow * window, int key, int scancode, int action, int mods)
{
  glfw_ctx_t * ctx = (glfw_ctx_t *)glfwGetWindowUserPointer (window);
  if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
      switch (key)
        {
          case GLFW_KEY_T:
            if (ctx->cursorpos)
              glfwSetCursorPosCallback (window, NULL);
	    else
              glfwSetCursorPosCallback (window, cursor_position_callback);
	    ctx->cursorpos = ! ctx->cursorpos;
            glfwSetWindowTitle (window, ctx->title);
            break;
          case GLFW_KEY_TAB:
            ctx->do_rotate = ! ctx->do_rotate;
            break;
          case GLFW_KEY_F:
            framebuffer (ctx);
            break;
          case GLFW_KEY_W:
            ctx->view->fov += 1.;
            break;
          case GLFW_KEY_S:
            snapshot (ctx);
            break;
          case GLFW_KEY_Q:
            ctx->view->fov -= 1.;
            break;
          case GLFW_KEY_O:
	    if (ctx->scene->field != NULL)
              ctx->scene->field->toggle_hide ();
	    break;
          case GLFW_KEY_P:
            if (ctx->scene->landscape != NULL)
              ctx->scene->landscape->toggle_flat ();
            break;
          case GLFW_KEY_6:
            ctx->view->rc += 0.1;
            break;
          case GLFW_KEY_EQUAL:
            ctx->view->rc -= 0.1;
            break;
          case GLFW_KEY_SPACE:
            ctx->view->latc = 0.;
            ctx->view->lonc = 0.;
	    ctx->view->rc = 6.0;
	    ctx->view->fov = 20.;
	    break;
          case GLFW_KEY_UP:
            ctx->view->latc = ctx->view->latc + 5.;
            break;
          case GLFW_KEY_DOWN:
            ctx->view->latc = ctx->view->latc - 5.;
            break;
          case GLFW_KEY_LEFT:
            ctx->view->lonc = ctx->view->lonc - 5.;
            break;
          case GLFW_KEY_RIGHT:
            ctx->view->lonc = ctx->view->lonc + 5.;
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
          glfw_ctx_t * ctx = (glfw_ctx_t *)glfwGetWindowUserPointer (window);
	  if (get_latlon_from_cursor (window, &ctx->view->latc, &ctx->view->lonc))
	    glfwSetCursorPos (window, ctx->width / 2., ctx->height / 2.);
        }
    }
}

static
void scroll_callback (GLFWwindow * window, double xoffset, double yoffset)
{
  glfw_ctx_t * ctx = (glfw_ctx_t *)glfwGetWindowUserPointer (window);
  if (yoffset > 0)
    ctx->view->fov += 1;
  else
    ctx->view->fov -= 1;
}

static
GLFWwindow * new_glfw_window (const char * geom, int width, int height)
{
  GLFWwindow * window;

  glfwWindowHint (GLFW_SAMPLES, 4);
  glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 
  glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
  window = glfwCreateWindow (width, height, geom, NULL, NULL);

  if (window == NULL)
    {
      fprintf (stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
      getchar ();
      glfwTerminate ();
      return NULL;
    }

  glfwMakeContextCurrent (window);
  
  glewExperimental = true; // Needed for core profile
  if (glewInit() != GLEW_OK) 
    {
      fprintf (stderr, "Failed to initialize GLEW\n");
      getchar ();
      glfwTerminate ();
      return NULL;
    }
  
  glfwSetInputMode (window, GLFW_STICKY_KEYS, GL_TRUE);
  glfwSetKeyCallback (window, key_callback);
  glfwSetScrollCallback (window, scroll_callback);
  glfwSetMouseButtonCallback (window, mouse_button_callback);

  return window;
}
  
static 
void free_glfw_window (GLFWwindow * window)
{
  glfwTerminate ();
}

void x11_display (const char * geom, int width, int height)
{
  glgrib_coastlines Coast;
  glgrib_grid Grid;
  glgrib_field Field;
  glgrib_scene Scene;
  glgrib_coords_world WorldCoords;
  glgrib_landscape_rgb Landscape_rgb;
  glgrib_landscape_tex Landscape_tex;
  glgrib_cube2 CubeA, CubeB;
  glgrib_view View;
  glfw_ctx_t ctx;

  View.setViewport (width, height);
  ctx.view = &View;
  ctx.width = width;
  ctx.height = height;
  ctx.title = geom;

  if (! glfwInit ())
    {
      fprintf (stderr, "Failed to initialize GLFW\n");
      getchar ();
      return;
    }

  GLFWwindow * Window = new_glfw_window (geom, width, height);
  glfwSetWindowUserPointer (Window, &ctx);
  
  gl_init ();

  glgrib_coords_cube Coords;
  Coords.init (&View);
  WorldCoords.init (geom);

  if(0){
  Landscape_rgb.init (geom, &WorldCoords);
  Scene.objlist.push_back (&Landscape_rgb);
  Scene.landscape = &Landscape_rgb;
  }
  if(1){
  Landscape_tex.init (geom, &WorldCoords);
  Scene.objlist.push_back (&Landscape_tex);
  Scene.landscape = &Landscape_tex;
  }
  if(0){
  CubeA.init (&Coords, 0., 0., 0.);
  Scene.objlist.push_back (&CubeA);
  }
  if(0){
  CubeB.init (&Coords, +0.5, +0.5, +0.5);
  Scene.objlist.push_back (&CubeB);
  }

  if(0){
  Grid.init ();
  Scene.objlist.push_back (&Grid);
  }

  if(0){
  Coast.init ("gshhs(3).rim");
  Scene.objlist.push_back (&Coast);
  }
  if(1){
  WorldCoords.init (geom);
  Field.init (NULL, &WorldCoords);
  Scene.objlist.push_back (&Field);
  Scene.field = &Field;
  }

  Scene.view = &View;

  ctx.scene = &Scene;


  if (1)
  while (1)
    {
      if (ctx.do_rotate)
        View.lonc += 1.;

      Scene.display ();

      glfwSwapBuffers (Window);
      glfwPollEvents ();
  
      if (glfwGetKey (Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        break;
      if (glfwWindowShouldClose (Window) != 0) 
        break;
    } 
  else
    framebuffer (&ctx);

  
  free_glfw_window (Window);
}



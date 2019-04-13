#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glgrib_render.h"
#include "glgrib_png.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>



using namespace glm;

#include "shader.h"


typedef struct glfw_ctx_t
{
  bool do_rotate = false;
  view_t * view;
  int width, height;
  int snapshot_cnt = 0;
} glfw_ctx_t;

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
void key_callback (GLFWwindow * window, int key, int scancode, int action, int mods)
{
  glfw_ctx_t * ctx = (glfw_ctx_t *)glfwGetWindowUserPointer (window);
  if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
      switch (key)
        {
          case GLFW_KEY_TAB:
            ctx->do_rotate = ! ctx->do_rotate;
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
  return;
  if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
      if (action == GLFW_PRESS) 
        {
          double xpos, ypos;
          glfw_ctx_t * ctx = (glfw_ctx_t *)glfwGetWindowUserPointer (window);
          glfwGetCursorPos (window, &xpos, &ypos);
	  double dox = (xpos - ctx->width  / 2) / (ctx->width  / 2),
		 doy = (ypos - ctx->height / 2) / (ctx->height / 2);
	  double ax = atan (dox * tan (ctx->view->fov / 2)),
	         ay = atan (doy * tan (ctx->view->fov / 2));
	  double gx = asin (ctx->view->rc * sin (ax)),
	         gy = asin (ctx->view->rc * sin (ax));

	  ctx->view->lonc = 180. * (M_PI / 2 + gx - ax) / M_PI;
	  ctx->view->latc = 180. * (M_PI / 2 + gy - ay) / M_PI;
printf (" dox = %f dox = %f\n", dox, doy);
printf (" ax  = %f ay  = %f\n", 180. / M_PI * ax, 180. / M_PI * ay);
printf (" %f %f\n", sin (ax), sin (ay));
printf (" %f %f\n", ctx->view->lonc, ctx->view->latc);

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
GLFWwindow * new_glfw_window (const char * file, int width, int height)
{
  GLFWwindow * window;

  if (! glfwInit ())
    {
      fprintf (stderr, "Failed to initialize GLFW\n");
      getchar ();
      return NULL;
    }
  
  glfwWindowHint (GLFW_SAMPLES, 4);
  glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 
  glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
  window = glfwCreateWindow (width, height, file, NULL, NULL);

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

void x11_display (const char * file, int width, int height)
{
  scene_t Scene;
  coastlines_t Coast;
  grid_t Grid;
  world_t World;
  cube_t Cube;
  prog_t Prog;
  view_t View;
  glfw_ctx_t ctx;

  ctx.view = &View;
  ctx.width = width;
  ctx.height = height;

  GLFWwindow * Window = new_glfw_window (file, width, height);
  glfwSetWindowUserPointer (Window, &ctx);
  
  gl_init ();
  Prog.init ();
  World.init (file);
  Cube.init ();
  View.init (&Prog);
  Grid.init ();
  Coast.init ("gshhs(3).rim");

  Scene.objlist.push_back (&World);
//Scene.objlist.push_back (&Cube);
  Scene.objlist.push_back (&Coast);
  Scene.objlist.push_back (&Grid);
  Scene.view = &View;
  Scene.prog = &Prog;
  
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
  
  free_glfw_window (Window);
}



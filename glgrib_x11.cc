#include "glgrib_x11.h"
#include <X11/Xlib.h>
#include <string.h>

/* The window which contains the text. */

static struct {
    int width;
    int height;
    const char * text;
    int text_len;

    /* X Windows related variables. */

    Display * display;
    int screen;
    Window window;
    GC gc;
    XFontStruct * font;
    unsigned long black_pixel;    
    unsigned long white_pixel;
}
text_box;

static bool linit = false;

static void init (Display * display, Window window)
{
  if (linit)
    return;
  linit = true;

  text_box.display = display;
  text_box.window = window;
  text_box.text = "Coucou";
  text_box.text_len = strlen (text_box.text);
  text_box.width = 300;
  text_box.height = 300;
 
  text_box.black_pixel = BlackPixel (text_box.display, text_box.screen);
  text_box.white_pixel = WhitePixel (text_box.display, text_box.screen);

  text_box.screen = DefaultScreen (text_box.display);
  text_box.gc = XCreateGC (text_box.display, text_box.window, 0, 0);
  XSetBackground (text_box.display, text_box.gc, text_box.white_pixel); 
  XSetForeground (text_box.display, text_box.gc, text_box.black_pixel); 

  const char * fontname = "-*-helvetica-*-r-*-*-14-*-*-*-*-*-*-*";
  text_box.font = XLoadQueryFont (text_box.display, fontname);
  /* If the font could not be loaded, revert to the "fixed" font. */
  if (! text_box.font) 
    {
        fprintf (stderr, "unable to load font %s: using fixed\n", fontname);
        text_box.font = XLoadQueryFont (text_box.display, "fixed");
    }
  XSetFont (text_box.display, text_box.gc, text_box.font->fid);

}



void glgribX11DrawText (glgrib_window * gwindow, const std::string & str)
{
  Window window = glfwGetX11Window (gwindow->window);
  Display * display = glfwGetX11Display ();
  init (display, window);

  int x;
  int y;
  int direction;
  int ascent;
  int descent;
  XCharStruct overall;

  XTextExtents (text_box.font, text_box.text, text_box.text_len,
                &direction, &ascent, &descent, &overall);
  x = (text_box.width - overall.width) / 2;
  y = text_box.height / 2 + (ascent - descent) / 2;
  XClearWindow (text_box.display, text_box.window);
  XDrawString (text_box.display, text_box.window, text_box.gc,
               x, y, text_box.text, text_box.text_len);

}

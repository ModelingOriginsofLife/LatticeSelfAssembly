extern bool extensions;

extern struct fb_var_screeninfo var;

extern unsigned char *screen;

extern int XScreen, YScreen;
extern int Bpp;
extern unsigned char *ScreenBuf;

#ifdef SDL
extern SDL_Surface *scr;
#endif

#ifdef SVGA
#include <vga.h>
#endif

#ifdef X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

extern Display *dis;
extern Window win;
extern Screen *screenptr;
extern int screennum;
extern Visual *vis;
extern GC gc;
extern XImage *ximage;
#endif

extern int XRes, YRes;

extern void InitSDL();
extern void InitX();
extern void InitFb();
extern void BlitScreen(unsigned char *Display);
extern void BlitBuf(unsigned char *Display,
		    int x, int y, int width,
		    int height);

#include "includes.h"

bool extensions = true;

#ifdef X11

Display *dis;
Window win;
Screen *screenptr;
int screennum;
Visual *vis;
GC gc;
XImage *ximage;

#endif

#ifdef SDL
SDL_Surface *scr;
#endif

unsigned char *screen;

int XScreen, YScreen;
int fb;

int Bpp;
int Console, Vt;

int XRes, YRes;
unsigned char *ScreenBuf;

void printLog(int id)
{
#ifdef LINUX
#endif
}

void InitSDL()
{
#ifdef SDL
   Bpp=4;
   if (SDL_Init(SDL_INIT_VIDEO)<0)
   {
      err("Error initializing SDL.");
   }
   atexit(SDL_Quit);

   SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);

   if (SDL_InitSubSystem(SDL_INIT_AUDIO) == -1)
   {
      err("Error initializing audio.");
   }
   scr=SDL_SetVideoMode(XRes, YRes, 32, SDL_DOUBLEBUF);
   if (scr==NULL)
   {
      err("Unable to set mode");
   }
#endif
}

void InitX()
{
#ifdef X11
   int Depth;

   Bpp=4;
   XScreen=XRes;
   YScreen=YRes;
   dis=XOpenDisplay(NULL);
   if (!dis)
     err("Error: Could not open X display\n");
   screenptr=DefaultScreenOfDisplay(dis);
   screennum=DefaultScreen(dis);
   Depth=DefaultDepth(dis, screennum);
   if (Depth<24)
     err("Error: Insufficient color depth. Need 24bpp or 32bpp\n");
   vis=DefaultVisualOfScreen(screenptr);
   win=XCreateWindow(dis, RootWindowOfScreen(screenptr), 0, 0, XRes, YRes, 0, Depth, InputOutput, vis, 0, NULL);
   if (!win)
     err("Error: Could not create window\n");
   XStoreName(dis, win, "Eldritch Dreams");
   XSelectInput(dis, win, KeyPressMask|KeyReleaseMask|ExposureMask);
   gc=XCreateGC(dis, win, 0, NULL);

   XMapRaised(dis, win);

   screen=(unsigned char*)malloc(XRes*YRes*Bpp);
   ximage=XCreateImage(dis, vis, Depth, ZPixmap, 0, screen,
		       XRes, YRes, 32, XRes*Bpp);
#endif
}

void InitFb()
{
#ifdef SVGA
   Bpp=3;
   screen=(unsigned char*)malloc(XRes*YRes*Bpp);
   XScreen=XRes;
   YScreen=YRes;
   vga_setmode(53);
   vga_getgraphmem();
#endif

#ifdef FB
   Console = open ("/dev/console", O_RDWR);
   ioctl (Console, VT_GETSTATE, &Vtstat);
   Vt=Vtstat.v_active;

   fb = open ("/dev/fb0", O_RDWR);

   if (fb==-1)
      err("Error opening framebuffer device.\n");

   ioctl (fb, FBIOGET_VSCREENINFO, &var);
   ioctl (fb, FBIOGET_FSCREENINFO, &fix);
   XScreen=var.xres;
   YScreen=var.yres;
   Bpp=var.bits_per_pixel/8;
   if (Bpp<3)
     err("Insufficient color depth. Requires 24bpp or 32bpp.\n");

/*   if (fix.visual==FB_VISUAL_DIRECTCOLOR)
   {
      palette=make_directcolor_cmap();
      ioctl(fb, FBIOPUTCMAP, palette);
   }
   */

   screen = mmap(0, XScreen*YScreen*Bpp, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);

   if (screen==NULL)
      err("Error mapping framebuffer device.\n");

   memset (screen, 0, XScreen*YScreen*Bpp);
#endif
}

void BlitScreen(unsigned char *display)
{
//   ioctl (Console, VT_GETSTATE, Vtstat);
  // if (Vtstat.v_active==Vt)
   memcpy (screen, display, XScreen*YScreen*Bpp);
}

void BlitBuf(unsigned char *display, int x, int y, int width, int height)
{
   int i, i2;

#ifdef SVGA
   memcpy(graph_mem, display, width*height*Bpp);
#endif

#ifdef FB
   ioctl (Console, VT_GETSTATE, &Vtstat);
   if (Vtstat.v_active==Vt)
   {
/*     if (fix.visual==FB_VISUAL_DIRECTCOLOR)
     {
        ioctl(fb, FBIOPUTCMAP, palette);
     }
 */

     for (i=y;i<y+height;i++)
     {
        memcpy (screen+x*Bpp+i*XScreen*Bpp, display+(i-y)*width*Bpp, width*Bpp);
     }
   }

#endif

#ifdef X11
   for (i=0;i<height;i++)
   {
        memcpy (screen+i*XScreen*Bpp, display+i*width*Bpp, width*Bpp);
   }
   XPutImage(dis, win, gc, ximage, 0, 0, 0, 0, width, height);
   XFlush(dis);
#endif

#ifdef SDL
   SDL_LockSurface(scr);

   memcpy((int*)(scr->pixels),
      	  display, Bpp*width*height);
      	  
   SDL_UnlockSurface(scr);

//   SDL_UpdateRect(scr, 0, 0, 0, 0);
   SDL_Flip(scr);
#endif
}


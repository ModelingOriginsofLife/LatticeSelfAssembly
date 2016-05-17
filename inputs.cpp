#include "includes.h"
FILE *Stdin;

int mx,my,mb;

//void DeInit();

void InitKeyboard()
{
#ifndef X11
#ifndef SDL
#ifdef CURS
   initscr();
   cbreak();
   noecho();
   nonl();
   intrflush(stdscr, FALSE);
   keypad(stdscr, TRUE);
   nodelay(stdscr, TRUE);
   curs_set(0);
#endif
#endif
#endif
   atexit(DeInit);
}

void DeInit()
{
#ifndef X11
#ifndef SDL
/*   nocbreak();
   echo();
   endwin();
   curs_set(1);*/
   //keyboard_close();
   //fclose (Stdin);
#endif
#endif
#ifdef X11
   //XDestroyImage(ximage);
   //free(screen);
   //XFreeGC(dis, gc);
   XDestroyWindow(dis, win);
   XCloseDisplay(dis);
#endif
   return;
}

int kmb=0;

int ReadKey()
{
   int Buf, Key, caps=0;
  
#ifdef CURS
   return getch();
#endif

#ifdef X11
   XEvent ev;
#endif
#ifdef SDL
   SDL_Event ev;
#endif
#ifdef FB
   Buf=getch();
   flushinp();
#endif
    
#ifdef X11
   XCheckMaskEvent(dis, KeyPressMask|KeyReleaseMask, &ev);

   switch (ev.type)
   {
    case KeyPress: Key=XLookupKeysym((XKeyEvent *)&ev, 0);
      switch (Key)
      {
       case XK_BackSpace: Buf=KEY_BACKSPACE; break;
       case XK_KP_Add:
       case XK_plus:
	 Buf='+';
	 break;
       case XK_KP_Subtract:
       case XK_minus:
	 Buf='-';
	 break;
       case XK_Up: Buf=KEY_UP; break;
       case XK_Left: Buf=KEY_LEFT; break;
       case XK_Right: Buf=KEY_RIGHT; break;
       case XK_Down: Buf=KEY_DOWN; break;
       case XK_Escape:
       case XK_Tab: Buf=9; break;
       default: Buf=Key&255; break;
      }
      break;
    case KeyRelease: Buf=0; break;
   }
#endif
   
#ifdef SDL
   Buf=0; 

   mb&=~kmb;
   kmb=0;
   while (SDL_PollEvent(&ev))
   {	
      if (ev.type==SDL_MOUSEMOTION)
      {
	 mx=ev.motion.x;
	 my=ev.motion.y;
      }
      else
      if (ev.type==SDL_MOUSEBUTTONDOWN)
      {
	 mx=ev.button.x;
	 my=ev.button.y;
	 
	 if (ev.button.button==SDL_BUTTON_LEFT)
	   mb|=1;
	 if (ev.button.button==SDL_BUTTON_MIDDLE)
	   mb|=2;
	 if (ev.button.button==SDL_BUTTON_RIGHT)
	   mb|=4;
	 if (ev.button.button==4) 
	   mb|=8;
	 if (ev.button.button==5)
	   mb|=16; 
	 
//	 printf("Event: %d\n",ev.button.button);
      }
      else
      if (ev.type==SDL_MOUSEBUTTONUP)
      {
	 mx=ev.button.x;
	 my=ev.button.y;
	 
	 if (ev.button.button==SDL_BUTTON_LEFT)
	   kmb|=1;
	 if (ev.button.button==SDL_BUTTON_MIDDLE)
	   kmb|=2;
	 if (ev.button.button==SDL_BUTTON_RIGHT)
	   kmb|=4;
	 if (ev.button.button==4)
	   kmb|=8;
	 if (ev.button.button==5)
	   kmb|=16;	 	 
      }
      else
      if (ev.type==SDL_KEYDOWN)
      {
	 Buf=ev.key.keysym.sym;

	 caps=0;
	 if ((ev.key.keysym.mod&KMOD_CAPS)||
	    (ev.key.keysym.mod&KMOD_LSHIFT)||
	    (ev.key.keysym.mod&KMOD_RSHIFT))
	  caps=1;
	 
	 if (caps)
	 {
	    if ((Buf>='a')&&(Buf<='z'))
	      Buf=Buf+'A'-'a';
	    switch (Buf)
	    {
	     case '\\': Buf='|'; break;
	     case '`': Buf='~'; break;
	     case ',': Buf='<'; break;
	     case '.': Buf='>'; break;
	     case '[': Buf='{'; break;
	     case ']': Buf='}'; break;
	     case '-': Buf='_'; break;
	     case '\'': Buf='\"'; break;
	     case ';': Buf=':'; break;
	     case '/': Buf='?'; break;
	     case '=': Buf='+'; break;
	     case '0': Buf=')'; break;
	     case '1': Buf='!'; break;
	     case '2': Buf='@'; break;
	     case '3': Buf='#'; break;
	     case '4': Buf='$'; break;
	     case '5': Buf='%'; break;
	     case '6': Buf='^'; break;
	     case '7': Buf='&'; break;
	     case '8': Buf='*'; break;
	     case '9': Buf='('; break;
	    }
	    caps=0;
	 }

	 switch(ev.key.keysym.sym)
	 {
	  case SDLK_RSHIFT:
	  case SDLK_LSHIFT:
	    Buf=0; caps=1; 
	    break;
	    
	  case SDLK_BACKSPACE: Buf=KEY_BACKSPACE; break;
	  case SDLK_TAB: Buf=9; break;
	    
	  case SDLK_KP2:
	  case SDLK_DOWN: Buf=KEY_DOWN; break;
	    
	  case SDLK_KP6:
	  case SDLK_RIGHT: Buf=KEY_RIGHT; break;
	    
	  case SDLK_KP4:
	  case SDLK_LEFT: Buf=KEY_LEFT; break;
	    
	  case SDLK_KP8:
	  case SDLK_UP: Buf=KEY_UP; break;
	  case SDLK_PLUS: Buf='+'; break;
	  case SDLK_MINUS: Buf='-'; break;
	    
	  case SDLK_KP7:
	  case SDLK_HOME: Buf=KEY_HOME; break;
	    
	  case SDLK_KP1:
	  case SDLK_END: Buf=KEY_END; break;
	    
	  case SDLK_KP9:
	  case SDLK_PAGEUP: Buf=KEY_PPAGE; break;
	    
	  case SDLK_KP3:
	  case SDLK_PAGEDOWN: Buf=KEY_NPAGE; break;
	    
	  case SDLK_F1:
	    Buf=KEY_F(1);
	    break;
	 }
      }
   }   
#endif
   
#ifdef SCRNSHOT
   if (Buf==KEY_F(1))
     PNMSave("shot.pnm", Capture); 
#endif
   
/*   if (Buf=='~')
   {
      err("User quit.");
   }*/
   
   return Buf;
}

#ifndef X11
#ifndef SDL
int NDelayReadKey()
{
   return ReadKey();
}
#endif
#endif

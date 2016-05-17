#include "includes.h"

void err(char *Str)
{
   printf("%s\n",Str);
   exit(0);
}


void PNMSave(char *Filename, Img Image)
{
  FILE *tmp;
  unsigned char Part1, Part2;
  int Colors,i; 
   
  tmp = fopen(Filename, "wb");
  if (tmp==NULL)
     err("Cannot save PNM.");
   
  fprintf (tmp, "P6\n%d %d\n255\n", Image.Width, Image.Height);
  
  for (i=0;i<Image.Width*Image.Height;i++)
  {
     fwrite (&Image.Image[2+Bpp*(i)], 1, 1, tmp);
     fwrite (&Image.Image[1+Bpp*(i)], 1, 1, tmp);
     fwrite (&Image.Image[0+Bpp*(i)], 1, 1, tmp);
  }
 
  fclose (tmp);
}


Img PNGLoad(char *Filename)
{
   Img Image, ITemp;    
   png_structp png_ptr;
   png_infop info_ptr;
   png_uint_32 width, height;
   png_bytep *storage;
   unsigned int sig_read = 0;
   int bit_depth, color_type, interlace_type;
   FILE *f, *logf;
   int i;
   
   f=fopen(Filename, "rb");
   if (f==NULL)
      err(Filename);//"Cannot load image.");
   
   png_ptr=png_create_read_struct(PNG_LIBPNG_VER_STRING,
				  NULL, NULL, NULL);
   info_ptr=png_create_info_struct(png_ptr);
   
   if (setjmp(png_ptr->jmpbuf))
   {
      png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
      fclose(f);
      err("Cannot load PNG.");
   }
   
   png_init_io(png_ptr, f);
   png_read_info(png_ptr, info_ptr);
   png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, 
		&color_type, &interlace_type, NULL, NULL);
 
   png_set_expand(png_ptr);
   
   if (color_type==PNG_COLOR_TYPE_GRAY)
     png_set_gray_to_rgb(png_ptr);
   
   png_set_bgr(png_ptr);
   
//   if (Bpp==4)
     png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
  
   png_read_update_info(png_ptr, info_ptr);
   png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,
		&color_type, &interlace_type, NULL, NULL);
   
   ITemp.Width=width; ITemp.Height=height;
   
   ITemp.Image=(unsigned char*)malloc(ITemp.Width*ITemp.Height*4);
//   ITemp.Mask=(unsigned char*)malloc(ITemp.Width*ITemp.Height);
   
   storage=(png_bytep*)malloc(height*sizeof(png_bytep));
   for (i=0;i<height;i++)
   {
      storage[i]=ITemp.Image+ITemp.Width*4*i;
   }
   
   png_read_image(png_ptr, storage);
   
   free(storage);
   
   png_read_end(png_ptr, info_ptr);
   
   fclose(f);
   
   Image.Width=ITemp.Width; Image.Height=ITemp.Height;
   Image.Image=(unsigned char*)malloc(Image.Width*Image.Height*Bpp);
   Image.Mask=(unsigned char*)malloc(Image.Width*Image.Height);
   
   for (i=0;i<Image.Width*Image.Height;i++)
   {
      Image.Image[2+Bpp*i]=ITemp.Image[2+4*i];
      Image.Image[1+Bpp*i]=ITemp.Image[1+4*i];
      Image.Image[0+Bpp*i]=ITemp.Image[0+4*i];
      if ((Image.Image[2+Bpp*(i)]==0)
         &&(Image.Image[1+Bpp*(i)]==0)
         &&(Image.Image[Bpp*i]==0))
	 Image.Mask[i]=0;
       else Image.Mask[i]=1;
   }
   free(ITemp.Image);

   png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
   png_destroy_info_struct(png_ptr, &info_ptr);
   
   return Image;
}

Img PNMLoad(char *Filename)
{
  FILE *tmp;
  unsigned char Part1, Part2;
  int Colors,i, Buf;
  Img Image;    
   char Ch;
   
  tmp = fopen(Filename, "rb");
  if (tmp==NULL)
     err(Filename);//"Cannot load image.");
  fscanf (tmp, "P%d\n%d %d\n%d", &Colors, &Image.Width, &Image.Height, &Buf);
  do
  {
     fread(&Ch,1,1,tmp);
  } while (Ch!='\n');
   
   //fseek (tmp, ftell(tmp)+1, SEEK_SET); 
  Image.Image = (unsigned char*)malloc((Image.Width)*(Image.Height)*Bpp);
  Image.Mask = (unsigned char*)malloc(Image.Width*Image.Height);
  if (Colors == 6)
  {
    //fread (Image.Image, Image.Width*Image.Height*3, 1, tmp);
    /*
     fread(&Part1, 1, 1, tmp);
    fread(&Part1, 1, 1, tmp);
    fread(&Part1, 1, 1, tmp);
    fread(&Part1, 1, 1, tmp);
    fread(&Part1, 1, 1, tmp);
    fread(&Part1, 1, 1, tmp);*/
    for (i=0;i<Image.Width*Image.Height;i++)
    {
       fread (&Part1, 1, 1, tmp);
       Image.Image[2+Bpp*(i)]=Part1;
       fread (&Part1, 1, 1, tmp);
       Image.Image[1+Bpp*(i)]=Part1;
       fread (&Part1, 1, 1, tmp);
       Image.Image[0+Bpp*(i)]=Part1;
       if ((Image.Image[2+Bpp*(i)]==0)
	 &&(Image.Image[1+Bpp*(i)]==0)
	 &&(Part1==0))
	 Image.Mask[i]=0;
       else Image.Mask[i]=1;
    }
  }
  else
  {
     fread(&Part1, 1, 1, tmp);
     fread(&Part1, 1, 1, tmp);
     fread(&Part1, 1, 1, tmp);
     for (i=3;i<Image.Width*Image.Height;i++)
     {
        fread (&Part1, 1, 1, tmp);	
      	Image.Image[Bpp*(i)]=Part1;
        Image.Image[1+Bpp*(i)]=Part1;
	Image.Image[2+Bpp*(i)]=Part1;
	if (Part1==0)
	  Image.Mask[i]=0;
	else Image.Mask[i]=1;
     }
  }
  fclose (tmp);
 
  return Image;
}

Img LoadImage(char *Filename)
{
   Img Image;
   char *ExecStr, *Exten;
   char *Test;
   FILE *Ident;   
   ExecStr=(char*)malloc(128);
   Exten = (char*)malloc(4);
   Test = (char*)malloc(4);
   
   Ident=fopen(Filename,"rb");
   if (Ident==NULL)
     err(Filename);//"Cannot load image.");
   
   fread(Test, 4, 1, Ident);
   	
   if (Test[0]=='\377') //!strcmp(Test, "\377\330\377\340"))
     strcpy(Exten, "JPG");
   if (Test[0]=='G')
     strcpy(Exten, "GIF");
   if (Test[0]=='\x89')
     strcpy(Exten, "PNG");
   if (Test[0]=='P')
     strcpy(Exten, "PNM");

   if ((Test[0]==0)&&(Test[1]==0)&&(Test[2]==0x02)&&(Test[3]==0))
     strcpy(Exten, "TGA");

   fclose(Ident);
   
   if (!strcasecmp(Exten, "PNG"))
   {
      //sprintf (ExecStr, "pngtopnm %s > tmp.pnm", Filename);
     Image = PNGLoad(Filename);
   }
   else
   {
      if (!strcasecmp(Exten, "JPG"))
        sprintf (ExecStr, "djpeg -pnm %s > tmp.pnm", Filename);
      if (!strcasecmp(Exten, "GIF"))
        sprintf (ExecStr, "giftopnm %s > tmp.pnm", Filename);
      if (!strcasecmp(Exten, "PNM"))
        sprintf (ExecStr, "cp %s tmp.pnm", Filename);
      if (!strcasecmp(Exten, "TGA"))
        sprintf (ExecStr, "tgatoppm %s > tmp.pnm", Filename);
      system (ExecStr);
   
      Image = PNMLoad("tmp.pnm");
      system ("rm tmp.pnm");
   }
   free (ExecStr);
   free (Exten);
   free (Test);
   return Image;
}

void BBlitImage (Img Image, int DrawType, int x, int y, int sw, int sh, unsigned char *display)
{
   int i, i2, mi, mi2, Offset, Clip;
   
   if (DrawType)
     {
   for (i=0;(i<sh)&&(i<Image.Height);i++)
   {
     for (i2=0;(i2<sw)&&(i2<Image.Width);i2++)
     {
       mi=(i*Image.Height/sh);
       mi2=(i2*Image.Width/sw);
          display[Bpp*(i2-2+i*sw)+2] = Image.Image[Bpp*((mi2)+(mi)*Image.Width)+2];
          display[Bpp*(i2-2+i*sw)+1] = Image.Image[Bpp*((mi2)+(mi)*Image.Width)+1];
          display[Bpp*(i2-2+i*sw)+0] = Image.Image[Bpp*((mi2)+(mi)*Image.Width)+0];          
    }
  }
  }
  else
  {     
     if (x<0)
       Offset=-1*x;
     else
       Offset=0;
     
     if (x+Image.Width>sw)
       Clip=x-(sw-Image.Width);
     else 
       Clip=0;
     
     for (i=0;(i+y<sh)&&(i<Image.Height);i++)
     {
        if (i+y>=0)
	  memcpy(display+(Offset+x+((i+y)*sw))*Bpp, Image.Image+(Offset+i*Image.Width)*Bpp,(Image.Width-Offset-Clip)*Bpp);
     }
  }
}

void BlitImage (Img Image, int DrawType, int x, int y, unsigned char *display)
{
   int i, i2, mi, mi2;
   for (i=0;i<YScreen;i++)
   {
     for (i2=0;i2<XScreen;i2++)
     {
          display[Bpp*(i2+i*XScreen)+2] = Image.Image[Bpp*((mi2)+(mi)*Image.Width)+2];
          display[Bpp*(i2+i*XScreen)+1] = Image.Image[Bpp*((mi2)+(mi)*Image.Width)+1];
          display[Bpp*(i2+i*XScreen)+0] = Image.Image[Bpp*((mi2)+(mi)*Image.Width)+0];          
    }
  }
}

void MBBlitImage (Img Image, int DrawType, 
		  int x, int y, int sw, int sh, 
		  unsigned char *display)
{
   int i, i2, mi, mi2;

   for (i=0;(i+y<sh)&&(i<Image.Height);i++)
   {
     for (i2=0;(i2+x<sw)&&(i2<Image.Width);i2++)
     {     
	if (Image.Mask[i2+i*Image.Width])          	     
        {
          if ((i2+x>=0)&&(i+y>=0))
	     memcpy (display+(Bpp*(x+i2+(i+y)*sw)), Image.Image+(Bpp*(i2+i*Image.Width)), Bpp);
	}    
     }
  }
}

void TBBlitImage(Img Image, int DrawType, int x, int y, int sw, int sh, unsigned char *display)
{
   int i, i2, mi, mi2, col;

   for (i=0;(i+y<sh)&&(i<Image.Height);i++)
   {
     for (i2=0;(i2+x<sw)&&(i2<Image.Width);i2++)
     {     
	if (Image.Mask[i2+i*Image.Width])          	     
        {
          if ((i2+x>=0) && (i+y>=0))
	  {
	     col=Image.Image[Bpp*(i2+i*Image.Width)];
	     display[Bpp*(x+i2+(i+y)*sw)]=(int)((display[Bpp*(x+i2+(i+y)*sw)]*(255.0-col)+col*col)/255.0);
	     
	     col=Image.Image[Bpp*(i2+i*Image.Width)+1];
	     display[Bpp*(x+i2+(i+y)*sw)+1]=(int)((display[Bpp*(x+i2+(i+y)*sw)+1]*(255.0-col)+col*col)/255.0);
	     
	     col=Image.Image[Bpp*(i2+i*Image.Width)+2];
	     display[Bpp*(x+i2+(i+y)*sw)+2]=(int)((display[Bpp*(x+i2+(i+y)*sw)+2]*(255.0-col)+col*col)/255.0);
	     

	     //memcpy (display+(Bpp*(x+i2+(i+y)*sw)), Image.Image+(Bpp*(i2+i*Image.Width)), Bpp);
	  }
       }    
     }
  }
}

void MBlitImage (Img Image, int DrawType, int x, int y, 
		 char maskr, char maskg, char maskb, 
		 unsigned char *display)
{
   int i, i2, mi, mi2;
   for (i=0;(i<YScreen)&&(i<Image.Height);i++)
   {
     for (i2=0;(i2<XScreen)&&(i2<Image.Width);i2++)
     {
       //mi=(i*Image.Height/YScreen);
       //mi2=(i2*Image.Width/XScreen);
       switch (DrawType)
       {
        case 0:
        {
	  if (
	      (Image.Image[Bpp*((i2)+(i)*Image.Width)+0]!=maskr) ||
	      (Image.Image[Bpp*((i2)+(i)*Image.Width)+1]!=maskg) ||
              (Image.Image[Bpp*((i2)+(i)*Image.Width)+2]!=maskb)          
	     )
          {
       	    display[Bpp*(x+i2+(i+y)*XScreen)+2] = Image.Image[Bpp*((i2)+(i)*Image.Width)+2];
	    display[Bpp*(x+i2+(i+y)*XScreen)+1] = Image.Image[Bpp*((i2)+(i)*Image.Width)+1];
            display[Bpp*(x+i2+(i+y)*XScreen)+0] = Image.Image[Bpp*((i2)+(i)*Image.Width)+0];          
          }
        } break;

        case 1:
        {
	  if (
       	      (Image.Image[Bpp*((mi2)+(mi)*Image.Width)+0]!=maskr) ||
	      (Image.Image[Bpp*((mi2)+(mi)*Image.Width)+1]!=maskg) ||
              (Image.Image[Bpp*((mi2)+(mi)*Image.Width)+2]!=maskb)          
	     )
	  {
	     display[Bpp*(i2+i*XScreen)+2] = Image.Image[Bpp*((mi2)+(mi)*Image.Width)+2];
             display[Bpp*(i2+i*XScreen)+1] = Image.Image[Bpp*((mi2)+(mi)*Image.Width)+1];
             display[Bpp*(i2+i*XScreen)+0] = Image.Image[Bpp*((mi2)+(mi)*Image.Width)+0];          
          }
	} break;
      }
    }
  }
}

typedef struct
{
  unsigned char *Image;
  unsigned char *Mask;
  int Width;
  int Height;
} Img;

extern void err(char *Str);

extern void PNMSave(char *Filename, Img Image);
extern Img PNMLoad(char *Filename);
extern Img LoadImage(char *Filename);  

extern void BBlitImage (Img Image, 
        		int DrawType, 
		        int x, int y, 
		        int sw, int sh, 
		        unsigned char *display);

extern void BlitImage (Img Image, 
		       int DrawType, 
		       int x, int y, 
		       unsigned char *display);

extern void MBlitImage (Img Image, int DrawType, int x, int y, 
		        char maskr, char maskg, char maskb, 
		        unsigned char *display);

extern void MBBlitImage (Img Image, int DrawType, 
  		         int x, int y, int sw, int sh, 
		         unsigned char *display);

extern void TBBlitImage(Img Image, int DrawType, 
			int x, int y, int sw, int sh, 
			unsigned char *display);

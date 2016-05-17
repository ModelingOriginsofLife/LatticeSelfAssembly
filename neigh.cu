#include "includes.h"

#ifdef CUDA
	#include <cuda_runtime_api.h>
	#include <curand.h>
	#include <curand_kernel.h>

	#define NTHREADS (XR*YR/(2*PATCHSIZE*PATCHSIZE))
	curandGenerator_t gen;
	float *randnum;
	int *dGrid;
#else
	#define __device__
	#define __host__
#endif

#define BASES 5
#define NEIGHBORS 9

#define XR 320
#define YR 80
#define PATCHSIZE 5
#define RNC 4

#define TEMP 0.5

int colors[3*BASES];
int *cGrid;
int *dN;

class Params
{
	public:
		float *W;
};

Params P;

void setParams()
{
	float *lW = (float*)malloc(BASES*BASES*NEIGHBORS*sizeof(float));
	
	memset(colors,0,sizeof(int)*3*BASES);
	colors[3+0] = 255; colors[3+1] = 0; colors[3+2] = 0;
	colors[6+0] = 0; colors[6+1] = 255; colors[6+2] = 0;
	colors[9+0] = 0; colors[9+1] = 0; colors[9+2] = 255;
	
	for (int i=4;i<BASES;i++)
		for (int j=0;j<3;j++)
			colors[3*i+j] = 64+rand()%180;
			
	for (int i=0;i<BASES*BASES*NEIGHBORS;i++)
		lW[i] = (rand()%2000001-1000000.0)/1000000.0;
	
	for (int i=0;i<BASES;i++)
		for (int j=0;j<NEIGHBORS;j++)
			lW[j + 0*NEIGHBORS + i*BASES*NEIGHBORS] = 0;
			
#ifdef CUDA
	cudaMemcpy(P.W,lW,sizeof(float)*BASES*BASES*NEIGHBORS,cudaMemcpyHostToDevice);
#else
	memcpy(P.W,lW,sizeof(float)*BASES*BASES*NEIGHBORS);
#endif

	free(lW);
}

__device__ __host__ int MapX(int x)
{
	return min(max(x,0),XR-1);
}

__device__ __host__ int MapY(int x)
{
	return min(max(x,0),YR-1);
}

__device__ __host__ void updateNeighbor(int x, int y, int *Grid, int *dN)
{
	int xm,ym,xm2,ym2;
	int ofs = (x+y*XR)*BASES;
	
	for (int i=0;i<BASES;i++)
		dN[i+ofs]=0;
		
	for (ym=y-1;ym<=y+1;ym++)
		for (xm=x-1;xm<=x+1;xm++)
		{
			xm2=MapX(xm);
			ym2=MapY(ym);
					
			dN[ofs + Grid[xm2+ym2*XR]] += 1;
		}
}

#ifdef CUDA
	__global__ void calcNeighbors(int *Grid, int *dN)
	{
		int idx = blockIdx.x * blockDim.x + threadIdx.x; // standard...
		int xm2,ym2;
		
		if (idx<XR*YR)
		{
			int x=idx%XR;
			int y=idx/XR;
			
			updateNeighbor(x,y,Grid,dN);
		}
	}				
#else
	void calcNeighbors(int *Grid, int *dN)
	{
		for (int y=0;y<YR;y++)
			for (int x=0;x<XR;x++)
				updateNeighbor(x,y,Grid,dN);
	}
#endif

void setNeighbors()
{
#ifdef CUDA
	int block_size = 32;
	int n_blocks2 = (XR*YR)/block_size + ((XR*YR)%block_size == 0 ? 0 : 1);

	calcNeighbors <<< n_blocks2, block_size >>> (dGrid, dN);
#else
	calcNeighbors(cGrid,dN);
#endif
}

__device__ __host__ float getLocalEnergy(int minx, int miny, int maxx, int maxy, int *Grid, Params P, int *dN)
{
	float E=0;
	int x,y,xm,ym,ofs,loc,i;
	
	for (y=miny;y<=maxy;y++)
		for (x=minx;x<=maxx;x++)
		{
			xm=MapX(x);
			ym=MapY(y);
			ofs = (xm+ym*XR)*BASES;
			loc = Grid[xm+ym*XR]; 
			
			for (i=0;i<BASES;i++)
				E += P.W[dN[ofs + i] - (loc==i) + loc*NEIGHBORS + i*BASES*NEIGHBORS];			
		}
	return E;
}

__device__ __host__ void Swap(int x1, int y1, int x2, int y2, int *Grid, int *dN, int src, int sink)
{
	int xm3,ym3;
	
	for (int y3=y1-1;y3<=y1+1;y3++)
		for (int x3=x1-1;x3<=x1+1;x3++)
		{
			xm3 = MapX(x3); 
			ym3 = MapY(y3);
			
			dN[(xm3+ym3*XR)*BASES+sink] += 1;
			dN[(xm3+ym3*XR)*BASES+src] -= 1;
		}
			
	for (int y3=y2-1;y3<=y2+1;y3++)
		for (int x3=x2-1;x3<=x2+1;x3++)
		{
			xm3 = MapX(x3); 
			ym3 = MapY(y3);
			
			dN[(xm3+ym3*XR)*BASES+sink] -= 1;
			dN[(xm3+ym3*XR)*BASES+src] += 1;
		}

	Grid[x1+y1*XR] = sink;
	Grid[x2+y2*XR] = src;	
}

__device__ __host__ void tryMove(int x1, int y1, int x2, int y2, int *Grid, int *dN, Params P, float r)
{
	int mnx = min(x1,x2), mny = min(y1,y2), mxx = max(x1,x2), mxy = max(y1,y2);
	x2 = MapX(x2);
	y2 = MapY(y2);
	
	int src = Grid[x1+y1*XR];
	int sink = Grid[x2+y2*XR];
			
	if (src != sink)
	{					
		float E1 = getLocalEnergy(mnx-1,mny-1,mxx+1,mxy+1,Grid,P,dN);
		
		Swap(x1,y1,x2,y2,Grid,dN,src,sink);
		
		float E2 = getLocalEnergy(mnx-1,mny-1,mxx+1,mxy+1,Grid,P,dN);
		
		if (E2>E1)
		{
			if (r>exp(-(E2-E1)/TEMP))
			{
				Swap(x1,y1,x2,y2,Grid,dN,sink,src);
			}
		}
	}
}

#ifdef CUDA
__global__ void PatchKernel(int *Grid, int ox, int oy, int flip, Params P, float *rnum, int iter, int *dN)
{
	int idx = blockIdx.x * blockDim.x + threadIdx.x; // standard...
	
	if (idx<NTHREADS)
	{
		int x1,y1,d,x2,y2,xp,yp;
		int ofs = idx*RNC+iter*RNC*NTHREADS;
		
		yp = idx/(XR/(2*PATCHSIZE));
		xp = 2*(idx%(XR/(2*PATCHSIZE)))+((yp+flip)%2);
		
		x1 = MapX(ox+(int)floor(rnum[ofs+0]*(PATCHSIZE-2)+1) + xp*PATCHSIZE);
		y1 = MapY(oy+(int)floor(rnum[ofs+1]*(PATCHSIZE-2)+1) + yp*PATCHSIZE);
		d = (int)floor(rnum[ofs+2]*9);
		
		x2 = x1 + (d%3)-1;
		y2 = y1 + (d/3)-1;
		
		tryMove(x1,y1,x2,y2,Grid,dN,P,rnum[ofs+3]);
	}
}
#endif

void doMCStep()
{
#ifdef CUDA
	int STEP = 2*PATCHSIZE*PATCHSIZE;
	int block_size = 32;
	int n_blocks = NTHREADS/block_size + (NTHREADS%block_size == 0 ? 0 : 1);
	
	curandGenerateUniform(gen, randnum, NTHREADS*RNC*STEP);
	for (int i=0;i<STEP;i++)
	{
		int ox=rand()%PATCHSIZE;
		int oy=rand()%PATCHSIZE;
		int flip=rand()%2;

		PatchKernel <<< n_blocks, block_size >>> (dGrid,ox,oy,flip,P,randnum,i,dN);
	}
	cudaMemcpy(cGrid,dGrid,sizeof(int)*XR*YR,cudaMemcpyDeviceToHost);
#else
	for (int i=0;i<XR*YR;i++)
	{
		int x1 = rand()%XR;
		int y1 = rand()%YR;
		int x2 = x1+rand()%3-1;
		int y2 = y1+rand()%3-1;
		
		tryMove(x1,y1,x2,y2,cGrid,dN,P,(rand()%1000001)/1000000.0);
	}
#endif
}

int drawFromEnvironment()
{
	int val = rand()%BASES;
	if (rand()%20 != 0) val = 0;
	
	return val;
}

void Init()
{
#ifdef CUDA
	curandCreateGenerator(&gen,CURAND_RNG_PSEUDO_DEFAULT);
	curandSetPseudoRandomGeneratorSeed(gen,1234ULL);
	
	cudaMalloc((void **)&randnum, PATCHSIZE*PATCHSIZE*2*NTHREADS*RNC*sizeof(float));
	cudaMalloc((void **)&dGrid, XR*YR*sizeof(int));
	cudaMalloc((void **)&dN, BASES*XR*YR*sizeof(int));
	cudaMalloc((void **)&P.W, BASES*BASES*NEIGHBORS*sizeof(float));
#else
	P.W = (float*)malloc(BASES*BASES*NEIGHBORS*sizeof(float));
	dN = (int*)malloc(BASES*XR*YR*sizeof(int));
#endif

	cGrid=(int*)malloc(sizeof(int)*XR*YR);
	
	for (int i=0;i<XR*YR;i++)
	{
		cGrid[i]=drawFromEnvironment();
	}
	
#ifdef CUDA
	cudaMemcpy(dGrid,cGrid,sizeof(int)*XR*YR,cudaMemcpyHostToDevice);
#endif
}

void Render()
{
	int r,g,b;
	int x,y,x0,y0;
	int sx = XRes/XR, sy = YRes/YR;
	int xm,ym;
	int loc;
	//~ memset(ScreenBuf,0,XRes*YRes*Bpp);
	
	for (y=0;y<YR;y++)
	{
		for (x=0;x<XR;x++)
		{
			x0 = x*sx; y0 = y*sy;
			loc = cGrid[x+y*XR];
			int sr = ScreenBuf[(x0+y0*XRes)*Bpp+0];
			int sg = ScreenBuf[(x0+y0*XRes)*Bpp+1];
			int sb = ScreenBuf[(x0+y0*XRes)*Bpp+2];
			
			r = (sr<colors[loc*3+0])*colors[loc*3+0]/20 + sr;
			g = (sg<colors[loc*3+1])*colors[loc*3+1]/20 + sg;
			b = (sb<colors[loc*3+2])*colors[loc*3+2]/20 + sb;
			
			r = min(r,255); g = min(g,255); b = min(b,255);
			
			for (ym=y0;ym<y0+sy;ym++)
				for (xm=x0;xm<x0+sx;xm++)
				{
					ScreenBuf[(xm+ym*XRes)*Bpp+0]=r;
					ScreenBuf[(xm+ym*XRes)*Bpp+1]=g;
					ScreenBuf[(xm+ym*XRes)*Bpp+2]=b;
				}
		}
	}
}

void doOcean()
{
	for (int y=0;y<30;y++)
	{		
		for (int x=0;x<XR;x++)
		{
			cGrid[x+y*XR]=drawFromEnvironment();
		}
	}
}

void doShift()
{
	for (int y=0;y<30;y++)
	{		
		for (int x=XR-1;x>0;x--)
		{
			cGrid[x+y*XR]=cGrid[x-1+y*XR];
		}
		cGrid[0+y*XR] = drawFromEnvironment();
	}
}

void doVShift()
{
	for (int x=0;x<XR;x++)
	{
		for (int y=YR-1;y>30;y--)
		{		
			cGrid[x+y*XR]=cGrid[x+(y-1)*XR];
		}
		cGrid[x+30*XR] = 0; //drawFromEnvironment();
	}
}

int main()
{
	char Str[512];
	Img I;
	int lastf=clock();
	
	srand(time(NULL));
	
	XRes=XR*4;
	YRes=YR*4;
	
	Bpp=4;
	ScreenBuf=(unsigned char*)malloc(XRes*YRes*Bpp);
	I.Width=XRes; I.Height=YRes;
	I.Image=ScreenBuf;
	InitSDL();
	
	Init();
	setParams();
	printf("Initialized\n");

	setNeighbors();
	int frame=0;
	
	while (1)
	{
		doMCStep();
		Render();
		
		if (frame%50 == 0)
		{
			FILE *f=fopen("hist.txt","a");
			int counts[BASES];
			memset(counts,0,sizeof(int)*BASES);
			for (int y=30;y<YR;y++)
				for (int x=0;x<XR;x++)				
					counts[cGrid[x+y*XR]]++;
			
			for (int i=0;i<BASES;i++)
				fprintf(f,"%d ",counts[i]);
			
			fprintf(f,"\n");
			fclose(f);
			
			PNMSave("tmp.pnm",I);
			sprintf(Str,"pnmtopng -force tmp.pnm > frames/%.6d.png", frame/10);
			system(Str);
			BlitBuf(ScreenBuf,0,0,XRes,YRes);
			for (int i=0;i<XRes*YRes*Bpp;i++)
				ScreenBuf[i]/=2.0;
		}
		frame++;
		
		doShift();
		//~ if (frame%150 == 0)
		//~ {
			//~ doVShift();
		//~ }
				
#ifdef CUDA
		cudaMemcpy(dGrid,cGrid,sizeof(int)*XR*YR,cudaMemcpyHostToDevice);
#endif
		setNeighbors();

		printf("%d\n",clock()-lastf);
		lastf=clock();
		
		int Ch=ReadKey();		
		if (Ch=='q') return 0;
		if (Ch=='p') setParams();
	}   
}

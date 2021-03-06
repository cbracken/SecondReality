#include <stdio.h>
#include <graph.h>

char far *vram=(char far *)0xa0000000L;

int	edge[512];

int ep[200];
int ec;
struct
{
	int	c,next;
	int	y1,y2;
	long	x1,x; // x used, x1=original
	long	dx;
} e[256];

int	list[256],listc;

unsigned draworders[32000],dop;

void	pass3(void)
{
	unsigned int u,ut,i,c;
	for(i=c=u=0;i<dop;i+=2)
	{
		ut=draworders[i+0];
		while(u<ut) vram[u++]=c;
		c^=draworders[i+1];
	}
}

void	pass2(void)
{
	int	j,k,i,y,c;
	long	x,lx;
	listc=0;
	dop=0;
	for(y=0;y<200;y++)
	{
		// add new ones this row
		i=ep[y];
		while(i)
		{
			list[listc++]=i;
			e[i].x=e[i].x1;
			i=e[i].next;
		}
		// sort list
		for(k=1;k<listc;k++)
		{
			x=e[i=list[k]].x;
			for(j=k-1;j>=0 && x<e[list[j]].x;j--) 
				list[j+1]=list[j];
			list[j+1]=i;
		}
		// do list & kill completed ones
		for(k=j=0;j<listc;j++)
		{	
			i=list[j];
			if(y>=e[i].y2) continue;
			if(k!=j) list[k]=i;
			k++;
			x=e[i].x;
			if(lx==x) draworders[dop-1]^=e[i].c;
			else
			{
				draworders[dop++]=y*320+(lx=(e[i].x>>16));
				draworders[dop++]=e[i].c;
			}
			vram[(e[i].x>>16)+y*320]=e[i].c;
			e[i].x+=e[i].dx;
		}
		listc=k;
	}
}

void	pass1(int *edge)
{
	int	dy,y1,y2,x1,x2,a;
	ec=1; memset(ep,0,200*2);
	while(*edge!=0x8000)
	{
		e[ec].c=edge[0];		
		y1=edge[3];
		y2=edge[5];
		if(y1>y2)
		{
			x1=y1; y1=y2; y2=x1; // y1<=>y2
			x1=edge[4];
			x2=edge[2];
		}
		else
		{
			x1=edge[2];
			x2=edge[4];
		}
		e[ec].x1=(long)x1<<16;
		e[ec].dx=((long)(x2-x1)<<16)/(y2-y1);
		// if y1<0 clip
		e[ec].y1=y1;
		e[ec].y2=y2;
		if(a=ep[y1])
		{ // add
			ep[y1]=ec;
			e[ec].next=a;
		}
		else
		{ // first
			ep[y1]=ec;
			e[ec].next=0;
		}
		ec++; 
		edge+=6;
	}
}

main()
{
	int	a,b,c,x,y;
	_setvideomode(_MRES256COLOR);
	for(a=0;a<60;a++) rand();
	b=0;
	for(c=1;c<=4;c<<=1)
	{
		x=100+c*10; y=100+c*10;
		for(a=0;a<8;a++)
		{
			edge[b++]=c; // color
			b++;
			edge[b++]=x; // X1
			edge[b++]=y; // Y1
			x=edge[b++]=rand()%320; // X2
			y=edge[b++]=rand()%200; // Y2
		}
		edge[b++]=c; // color
		b++;
		edge[b++]=x; // X1
		edge[b++]=y; // Y1
		edge[b++]=100+c*10; // X2
		edge[b++]=100+c*10; // Y2
	}
	edge[b++]=0x8000; // end of list
	
	#if 0
	_setcolor(8);
	for(b=0;edge[b]!=0x8000;b+=6)
	{
		_moveto(edge[b+2],edge[b+3]);
		_lineto(edge[b+4],edge[b+5]);
	}
	#endif
	pass1(edge);
	pass2();
	getch();
	pass3();
	getch();
	_setvideomode(_DEFAULTMODE);
}

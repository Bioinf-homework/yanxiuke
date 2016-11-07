// dtw.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"

#include "math.h"
#include "struct.h"

double dtwmatch(Vector *model, int mlen, Vector *check,int clen)
{
     int i,j,k,l;
	 DTWStru *g=new DTWStru[mlen*clen];
 	 double hd,vd,yd,temp;
     //初始化DTWStru
	 g[0].hdis=distance(check[0],model[0]);
	 g[0].sdis=g[0].hdis;
	 for(i=0;i<clen;i++)
		 for(j=0;j<mlen;j++)
		 {
			 k=i*mlen+j;
			 g[k].path=0;
			 g[k].hcount=0;
		 }
	//计算j=0的点	
	 for(i=1;i<clen;i++)
	 {
	     k=i*mlen;
		 l=(i-1)*mlen;
	 	 g[k].hdis=(distance(check[i],model[0])+g[l].hdis*(g[l].hcount+1))/(g[l].hcount+2);

		 g[k].sdis=g[l].sdis-g[l].hdis+g[k].hdis;
		 
	     g[k].path=1;
		 g[k].hcount=g[l].hcount+1;

	 }
	 //计算i=0的点
	 for(j=1;j<mlen;j++)
	 {
		 g[j].hdis=distance(check[0],model[j]);
         g[j].sdis=g[j-1].sdis+g[j].hdis;
		 g[j].path=2;
		 g[j].hcount=0;
	 }
	 
	
     //计算匹配网络
	 for(i=1;i<clen;i++)
       for(j=1;j<mlen;j++) 
	   {
		   k=i*mlen+j;
		   g[k].hdis=distance(check[i],model[j]);
		   l=(i-1)*mlen+(j-1);
		   yd=g[l].sdis+g[k].hdis;//对角累计
		   l=i*mlen+(j-1);
		   vd=g[l].sdis+g[k].hdis;//垂直累计
		   l=(i-1)*mlen+j;
		   temp=(g[k].hdis+g[l].hdis*(g[l].hcount+1))/(g[l].hcount+2);
		   hd=g[l].sdis-g[l].hdis+temp;//水平累计
		   g[k].path=getmin(hd,vd,yd);
		   
		   if(g[k].path==1)
		   {
                       g[k].hdis=temp;
		       g[k].sdis=hd;
		       g[k].hcount=g[l].hcount+1;  
		   }
		   else
		   {
		
		       if(g[k].path==2)
			       g[k].sdis=vd;
			   else
                   g[k].sdis=yd; 
		       g[k].hcount=0; 

		   }//else
	 }//for	
	 temp=g[clen*mlen-1].sdis;
	 delete g;
	 return temp;

}


double distance(Vector a,Vector b)
{
    double dist=0.0;
	for(int i=0;i<VECTOR_ORDER;i++)
         dist+=pow((a[i]-b[i]),2);
	
	return (sqrt(dist));
}

BYTE getmin(double a,double b,double c)
{
             if((a<b)&&(a<c))
				 return 1;
			 else if(b<c)
				 return 2;
			 else 
				 return 3;

}

#include "bcb.hpp"
void updateBCB(int &num,int *f,int *b,int i,int v){

	int p1,p2;

	if(i)
	{
		if(!num)
		{
			p2=f[0];
			f[0]=i;
			f[i]=p2;
			b[p2]=i;
			b[i]=0;
		}
		else if(!(num+v))
		{
			p1=b[i],p2=f[i];
			f[p1]=p2;
			b[p2]=p1;
		}
	}
	num += v;
}

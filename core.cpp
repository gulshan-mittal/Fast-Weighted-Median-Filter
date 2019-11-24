#include "core.hpp"

Mat filterCore(Mat &I,Mat &F, float **wMap,int r)
{
	F.convertTo(F, CV_32S);
	int nF=256,nI=256;
	int rows = I.rows, cols = I.cols;
	int alls = rows * cols;
	int winSize = (2*r+1)*(2*r+1);
	Mat outImg = I.clone();

	Mat mask = Mat(I.size(),CV_8U);
	mask = Scalar(1);

	int **H = int2D(nI,nF);
	// int *BCB = new int[nF];
	int BCB[nF];
	int **Hf = int2D(nI,nF), **Hb = int2D(nI,nF);
	int BCBf[nF],BCBb[nF];

	for(int x=0;x<cols;x++){
		memset(BCB, 0, sizeof(int)*nF);
		memset(H[0], 0, sizeof(int)*nF*nI);
		for(int i=0;i<nI;i++)
			Hf[i][0]=Hb[i][0]=0;
		BCBf[0]=BCBb[0]=0;

		int medianVal = -1;
		int downX = max(0,x-r);
		int upX = min(cols-1,x+r);

		// Initialize joint-histogram
		int upY = min(rows-1,r);
		for(int i=0;i<=upY;i++){

			int *IPtr = I.ptr<int>(i);
			int *FPtr = F.ptr<int>(i);
			uchar *maskPtr = mask.ptr<uchar>(i);

			for(int j=downX;j<=upX;j++){

				if(!maskPtr[j])continue;

				int fval = IPtr[j];
				int *curHist = H[fval];
				int gval = FPtr[j];

				// necklace table
				if(!curHist[gval] && gval){
					int *curHf = Hf[fval];
					int *curHb = Hb[fval];

					int p1=curHf[0];
					curHf[0]=gval;
					curHf[gval]=p1;
					curHb[0]=gval;
					curHb[gval]=0;
				}

				curHist[gval]++;
				updateBCB(BCB[gval],BCBf,BCBb,gval,-1);
			}
		}

		for(int y=0;y<rows;y++)
		{
			float balw = 0;
			int curIndex = F.ptr<int>(y,x)[0];
			float *fPtr = wMap[curIndex];
			int &cmedval = medianVal;
			int i=0;
			do{
				balw += BCB[i]*fPtr[i];
				i=BCBf[i];
			}while(i);

			if(balw >= 0){
				for(balw;balw >= 0 && cmedval;cmedval--){
					float curWeight = 0;
					int *nextHist = H[cmedval];
					int *nextHf = Hf[cmedval];

					int i=0;
					do{
						curWeight += (nextHist[i]*2)*fPtr[i];
						
						updateBCB(BCB[i],BCBf,BCBb,i,-(nextHist[i]*2));
						
						i=nextHf[i];
					}while(i);

					balw -= curWeight;
				}
			}
			else if(balw < 0){
				for(balw;balw < 0 && cmedval != nI-1; cmedval++){
					float curWeight = 0;
					int *nextHist = H[cmedval+1];
					int *nextHf = Hf[cmedval+1];

					int i=0;
					do{
						curWeight += (nextHist[i]*2)*fPtr[i];

						updateBCB(BCB[i],BCBf,BCBb,i,nextHist[i]*2);
						
						i=nextHf[i];
					}while(i);
					balw += curWeight;
				}
			}

			if(balw<0)outImg.ptr<int>(y,x)[0] = cmedval+1;
			else outImg.ptr<int>(y,x)[0] = cmedval;

			int fval,gval,*curHist;
			int rownum = y + r + 1;
			if(rownum < rows){

				int *inpptr = I.ptr<int>(rownum);
				int *gdptr = F.ptr<int>(rownum);
				uchar *maskPtr = mask.ptr<uchar>(rownum);

				for(int j=downX;j<=upX;j++){

					if(!maskPtr[j])
						continue;

					fval = inpptr[j];
					curHist = H[fval];
					gval = gdptr[j];
					if(!curHist[gval] && gval){
						int *curHf = Hf[fval];
						int *curHb = Hb[fval];

						int p1=0,p2=curHf[0];
						curHf[gval]=p2;
						curHb[gval]=p1;
						curHf[p1]=curHb[p2]=gval;
					}

					curHist[gval]++;
					updateBCB(BCB[gval],BCBf,BCBb,gval,((fval <= medianVal)<<1)-1);
				}
			}

			rownum = y - r;
			if(rownum >= 0){

				int *inpptr = I.ptr<int>(rownum);
				int *gdptr = F.ptr<int>(rownum);
				uchar *maskPtr = mask.ptr<uchar>(rownum);

				for(int j=downX;j<=upX;j++){

					if(!maskPtr[j])continue;

					fval = inpptr[j];
					curHist = H[fval];
					gval = gdptr[j];

					curHist[gval]--;
					if(!curHist[gval] && gval){
						int *curHf = Hf[fval];
						int *curHb = Hb[fval];

						int p1=curHb[gval],p2=curHf[gval];
						curHf[p1]=p2;
						curHb[p2]=p1;
					}
					updateBCB(BCB[gval],BCBf,BCBb,gval,-((fval <= medianVal)<<1)+1);
				}
			}
		}

	}
	return outImg;
}
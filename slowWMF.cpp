#include "global.hpp"
// #include ""
#define rep(i, a, b) for (int i = (a); i < (b); ++i)

Mat slowwmf(Mat &I, Mat &F, int r)
{
	int nI, nF;
	nI = nF = 256;
	Mat out_img = I.clone();

	I.convertTo(I, CV_32S);

	Mat padded;
	int padding = 3;
	int pad_rows = I.rows + 2*padding, pad_cols = I.cols + 2*padding;

	padded.create(pad_rows, pad_cols, I.type());
	padded.setTo(cv::Scalar::all(0));
	I.copyTo(padded(Rect(padding, padding, I.cols, I.rows)));

	rep(i, r, I.rows-r)
	{
		rep(j, r, I.cols-r)
		{
			vector<pair<int,float> > vc;

			rep(p, i-r, i+r+1)
			{
				rep(q, j-r, j+r+1)
				{
					float diff = abs((float)(I.at<int>(p,q) - I.at<int>(i,j)));
					float denom = (1.0f/(2*26*26));
					// vc.push_back(make_pair(I.at<int>(p,q),1.0f));
					vc.push_back(make_pair(I.at<int>(p,q),exp(-(diff*diff)*denom)));

				}
			}

			sort(vc.begin(),vc.end());
			float tot = 0, cur=0;
			int k=0;

			rep(ig, 0, vc.size()){
				tot+=vc[ig].second;
			}

			while(cur < 0.5*tot){
				cur+=vc[k].second;
				k++;
			}

			out_img.ptr<int>(i-r,j-r)[0] = vc[k].first;
			
			vc.clear();
		}
	}

	out_img.convertTo(out_img,CV_8U);
	
	return out_img;
}
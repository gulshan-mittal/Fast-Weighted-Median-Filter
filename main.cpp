#include "global.hpp"
#include "core.hpp"
#include "allocint.hpp"
#include "bcb.hpp"
#include "slowWMF.hpp"

double getMSE(const Mat& I1, const Mat& I2)
{
	Mat s1;
	absdiff(I1, I2, s1);       // |I1 - I2|
	s1.convertTo(s1, CV_32F);  // cannot make a square on 8 bits, so converting to 32 float
	s1 = s1.mul(s1);           // |I1 - I2|^2

	Scalar s = sum(s1);        // sum elements per channel

	double sse = s.val[0]; // sum channels


    double mse  = sse / (double)(I1.total());

    return mse;

 }

// I = Input Image, F = Feature Map, r= Radius of Kernel
// wt_type is 0 for unweighted, 1 for gaussian, 2 for Jacard
Mat req_filter(Mat &I, Mat &F, int r, int wt_type)
{
	int nI, nF;
	nI = nF = 256;

	// convert the values in image to 32-bit signed integers (32S)
	I.convertTo(I, CV_32S);

	
	float **ret = new float*[nF];
	ret[0] = new float[nF*nF];

	for(int i=1;i<nF;i++){
		ret[i] = *ret + nF*i;
	}

	float **wMap = ret;

	float denom = (1.0f/(2*pow(26, 2))), diff, wt;

	for(int i=0;i<nF;i++){
		for(int j=i;j<nF;j++){
			diff = fabs((float)(i-j));
			if(wt_type==1){
				// For gaussian weight types
				wt = exp(-(diff*diff)*denom);
			}
			else if(wt_type==2){
				// For Jacard weight types
				wt = (float)(min(i,j)*1.0/max(i,j));
			}
			else {
				// For Unweighted weight types
				wt = 1.0f;
			}
			wMap[i][j] = wt;
			wMap[j][i] = wt;
		}
	}

	//Filtering - Joint-Histogram Framework
	F.convertTo(F, CV_32S);
	I = filterCore(I, F, wMap, r);

	// return the filtered image by converting the values in image to 8-bit unsigned integers (8U)
	I.convertTo(I, CV_8U);
	return I;
}

int main(int argc, char** argv) {
	/*
	Arguments:
		1. <relative image filepath>
		2. <output image path>
		3. <kernel size>
		4. <type of weights> // 0: unweighted, 1: gaussian, 2: Jacard
		5. <comparison with naive implementaion> // 0: Yes, 1: No
	*/

	cv::Mat image, grayImage;
	
	// Argument parsing
	std::string image_path = argv[1]; // relative path of the image file
	std::string output_path = argv[2];

	std::string kernel_size = argv[3];
	cout << "Kernel Radius: " << kernel_size << endl;

	std::string wt_type = argv[4]; // Type of weights: 0: unweighted, 1: gaussian, 2: Jacard
	std::string filter_type;
	if (wt_type=="0") filter_type = "unweighted";
	if (wt_type=="1") filter_type = "Gaussian"; 
	if (wt_type=="2") filter_type = "Jacard";

	cout << "Weights Type: " << filter_type << endl; 
	

	int isCompare = 0;

	if (argc >= 6 && std::string(argv[5])=="1"){
		isCompare = 1;
	} 

	image = imread(image_path, CV_LOAD_IMAGE_COLOR);

	// convert the input image to grayscale
	cvtColor(image, grayImage, CV_BGR2GRAY);
	imwrite(image_path, grayImage);
	Mat inImg = grayImage.clone();

	Mat rest;

	clock_t t_start = clock();
	Mat filtered_img = req_filter(inImg, inImg, stoi(kernel_size), stoi(wt_type));

	double time_taken = ((double)(clock() - t_start))/CLOCKS_PER_SEC;

	// imwrite(output_path, filtered_img);
	cout << "Time Taken: " << time_taken << "s by the implementation proposed in paper" << endl;

	if (isCompare == 1){
		t_start = clock();
		rest = slowwmf(inImg, inImg, stoi(kernel_size));
		time_taken = ((double)(clock() - t_start))/CLOCKS_PER_SEC;
		cout  << "Time Taken: " << time_taken << "s by naive implementation"  << endl;

		// Code for Getting MSE between two images
		//cout << getMSE(filtered_img, rest) << "\n";
		imshow("Naive WMF", rest);
	}

	imshow("WFM Output", filtered_img);
	
	waitKey(0);

	// Plots & Analysis
	if(argc>=7 && std::string(argv[6]) == "i"){

		int iteration = 10;
		Mat wfm_img, naive_wfm;
		std::ofstream myfile1;
		myfile1.open ("analysis1.csv");
		myfile1 << "filter_sz,type,wt_type,time_taken\n";
		std::ofstream myfile2;
		myfile2.open ("analysis2.csv");
		myfile2 << "scale,type,wt_type,time_taken\n";

		for(int i=0;i<iteration;i++){
			
			// Varying Kernel Size
			t_start = clock();
			inImg = grayImage.clone();
			wfm_img = req_filter(inImg, inImg, i+1, stoi(wt_type));
			double time_taken = ((double)(clock() - t_start))/CLOCKS_PER_SEC;
			cout << "Time Taken: " << time_taken << "s by the implementation proposed in paper" << endl;
			myfile1 << to_string(i+1) + ",0," + wt_type + "," + to_string(time_taken) + "\n";

			t_start = clock();
			naive_wfm = slowwmf(inImg, inImg, i+1);
			time_taken = ((double)(clock() - t_start))/CLOCKS_PER_SEC;
			cout << "Time Taken: " << time_taken << "s by the naive implementation" << endl;
			myfile1 << to_string(i+1) + ",1," + wt_type + "," + to_string(time_taken) + "\n";

			//cout << getMSE(wfm_img, naive_wfm) << "\n";
		}

		float scale = 1.0;
		Mat wfm_img1, naive_wfm1;
		cv::Mat outImg;

		// Varying Image size
		for(int i=1;i<=5;i++, scale = scale/2.0){
			
			cout << scale << "\n";

			inImg = grayImage.clone();
			resize(inImg, outImg, cv::Size(inImg.cols *scale, inImg.rows * scale), 0, 0);
			t_start = clock();
			wfm_img1 = req_filter(outImg, outImg, stoi(kernel_size), stoi(wt_type));
			double time_taken = ((double)(clock() - t_start))/CLOCKS_PER_SEC;
			cout << "Time Taken: " << time_taken << "s by the implementation proposed in paper" << endl;
			myfile2 << to_string(scale) + ",0," + wt_type + "," + to_string(time_taken) + "\n";

			t_start = clock();
			naive_wfm1 = slowwmf(outImg, outImg, stoi(kernel_size));
			time_taken = ((double)(clock() - t_start))/CLOCKS_PER_SEC;
			cout << "Time Taken: " << time_taken << "s by the naive implementation" << endl;
			myfile2 << to_string(scale) + ",1," + wt_type + "," + to_string(time_taken) + "\n";
		}

	}
	else if(argc>=7 && std::string(argv[6]) == "d"){
		Mat demo_img = grayImage;
		for(int i=1;i<=100;i++){
			demo_img = req_filter(demo_img, demo_img, stoi(kernel_size) , stoi(wt_type));
			imwrite(output_path + to_string(i) + "_.png", demo_img);
		}
	}

	return 0;
}

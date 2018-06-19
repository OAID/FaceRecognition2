#include "recognition.h"

#include <stdlib.h>

//#include "head.h"

using namespace std;

Classifier::Classifier(){net_ = NULL;}

Classifier::~Classifier()
{
	if(net_)
		delete net_;
}


void Classifier::LoadModel(const string& model_file, const string& trained_file) {

#ifdef CPU_ONLY
	Caffe::set_mode(Caffe::CPU);
#else
	Caffe::set_mode(Caffe::GPU);
#endif

	net_ = new Net<float>((model_file + "/LightenedCNN_B.prototxt"), caffe::TEST);
	net_->CopyTrainedLayersFrom(model_file + "/LightenedCNN_B.caffemodel");

	CHECK_EQ(net_->num_inputs(), 1) << "Network should have exactly one input.";
	CHECK_EQ(net_->num_outputs(), 1) << "Network should have exactly one output.";

}
std::vector<float> Classifier::Classify(const cv::Mat& img1) {
	
	
	std::vector<float> output = Predict(img1);
	return output;
}

std::vector<float> Classifier::Predict(const cv::Mat& img1) {

	cv::Mat gray = img1;

	//cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);


	Blob<float>* input_blob = net_->input_blobs()[0];
	float * input_data=input_blob->mutable_cpu_data();


	if(gray.isContinuous())
	{
		unsigned char * p_pixel=gray.ptr(0);

		for(int i=0;i<128*128;i++)
		{
			input_data[i]=((float)p_pixel[i])/256;
		}
	}
	else
	{
		for(int i=0;i<128;i++)
		{
			unsigned char * p_row=gray.ptr(i);

			for(int j=0;j<128;j++)
			{
				unsigned char p_pixel=p_row[j];

				input_data[0]=((float)p_pixel)/255;
				input_data++;
			}
		}
	}
	
	net_->Forward();

	/* get output*/
	const Blob<float> * feature_blob=net_->blob_by_name("eltwise_fc1").get();


	const float * begin=feature_blob->cpu_data();
	const float* end = begin + feature_blob->channels();

	return std::vector<float>(begin, end);

}

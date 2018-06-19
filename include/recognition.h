#ifndef RECOGNITION_H
#define RECOGNITION_H

#include "my_head.h"
using namespace caffe;
using std::string;

class Classifier {
public:
	Classifier();
	~Classifier();
	void LoadModel(const string& model_file, const string& trained_file);
	std::vector<float> Classify(const cv::Mat& img1);

private:
	std::vector<float> Predict(const cv::Mat& img);


private:
	Net<float> * net_; 
};

#endif

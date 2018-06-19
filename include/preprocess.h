#ifndef Preprocess_H11
#define Preprocess_H11

#include <opencv2/opencv.hpp>

using namespace cv;
typedef struct FaceBaseInfo {
	Rect bbox;
	Point2f eyeleft;
	Point2f eyeright;
	Point2f nose;
	Point2f mouthleft;
	Point2f mouthright;
} FaceBaseInfo;
int preprocess(Mat& img, FaceBaseInfo& f, Size objsize, Mat& d1);

#endif
#include "preprocess.h"

int alignFace(Mat& img, Mat& dstimg, const FaceBaseInfo& f5pt, cv::Size crop_size, float ec_mc_y, float ec_y)
{
//	circle(img, f5pt.eyeleft, 2, Scalar(0, 0, 0), 2, 8, 0);
//	circle(img, f5pt.eyeright, 2, Scalar(60, 0, 0), 2, 8, 0);
//	circle(img, f5pt.nose, 2, Scalar(120, 0, 0), 2, 8, 0);
//	circle(img, f5pt.mouthleft, 2, Scalar(180, 0, 0), 2, 8, 0);
//	circle(img, f5pt.mouthright, 2, Scalar(240, 0, 0), 2, 8, 0);

    std::cout << "start align...\n";

	float dex = f5pt.eyeleft.x - f5pt.eyeright.x;
	float dey = f5pt.eyeleft.y - f5pt.eyeright.y;
	float ang_tan = dey / dex;
	float arc = atan(ang_tan);
	float ang = arc / CV_PI * 180;
	Mat img_rot(600,600,CV_8UC3);

	float ecx = (f5pt.eyeleft.x + f5pt.eyeright.x)*0.5;
	float ecy = (f5pt.eyeleft.y + f5pt.eyeright.y)*0.5;
	float mcx = (f5pt.mouthleft.x + f5pt.mouthright.x)*0.5;
	float mcy = (f5pt.mouthleft.y + f5pt.mouthright.y)*0.5;
	Point2f center(ecx,ecy);
	float mcy1 = (mcx - ecx)*sin(-arc) + (mcy - ecy)*cos(-arc) + ecy;
	float resize_scale = ec_mc_y / fabs(mcy1 - ecy);
//	printf(" %f,",resize_scale);
//	circle(img, center, 2, Scalar(255, 255, 255), 2, 8, 0);
	Mat rot = getRotationMatrix2D(center, ang, resize_scale);
	//Mat rot = getRotationMatrix2D(Point2f(200, 200), 30, 1);

	warpAffine(img, img_rot(Rect(50,50,500,500)), rot, Size(500, 500));

    circle(img_rot, center, 2, Scalar(255, 255, 255), 2, 8, 0);
    namedWindow("img1",0); imshow("img1", img);waitKey(0);
    namedWindow("rot1", 0); imshow("rot1", img_rot);waitKey(0);

	float crop_y = ecy - ec_y+50>0 ? ecy - ec_y + 50 :0;
	if (crop_y + crop_size.height > img_rot.rows){
        std::cout << "crop_y error!!!" << std::endl;
		return 0;}
	float crop_x = ecx - crop_size.width*0.5 + 50>0 ? ecx - crop_size.width*0.5 + 50 :0;
	if (crop_x + crop_size.width > img_rot.cols){
        std::cout << "crop_y error!!!" << std::endl;
		return 0;}
	dstimg = img_rot(Rect(crop_x+0.5, crop_y+0.5, crop_size.width, crop_size.height));

    namedWindow("crop1", 0);
    resizeWindow("crop1",400,400);
    imshow("crop1", dstimg);
    waitKey(0);

	return 1;
}

int preprocess(Mat& img, FaceBaseInfo& f,Size objsize, Mat& d1)
{
	Mat cropImg;
	int fg = alignFace(img, cropImg, f, Size(128, 128), 38, 50); //38 50
	if (fg)
	{
		resize(cropImg, d1, objsize);
		//cv::imwrite("a.bmp", d1);
		d1.convertTo(d1, CV_32FC3,1.0/128,-127.5/128);
		return 1;
	}
	return 0;
}

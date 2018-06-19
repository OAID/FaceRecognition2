#include <thread.h>
#include <cv.h>
#include <opencv2/opencv.hpp>

#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <algorithm>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv/cv.h>
#include <vector>

#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include "ai_type.h"
#include "util.h"
#include "config.h"
#include "face_demo.hpp"


using namespace std;
using namespace cv;
typedef struct mutisensor_Face {
    CvRect pos;
    String name;
	CvPoint node[5];
	int id;
	int time;
    int drawflag = 0;
} Mface;
typedef struct mutisensor_Face_Picture {
    Mat pic;
	CvRect pos;
    String name;
	int id = -1;
	int capTime = 0;
	int lasttime = 0;
} MfaceP;

class AlgThread : public Thread
{
private:
    int framescale = 1;
    int framewidth = 640;
    int frameheight = 480;
    Mat m_Mat;
    Mat m_srcMat;
    Mat m_facemat[9];

	
	Mat detect_faces[10];
	
    MfaceP faces[9];
	std::string m_facename[9];
    Mface  m_face[3];
    std::string registerFacesPath;
    Mutex mt;

    Rect res;
    FaceDemo mFace_demo;
    int frames = 0;
	int m_namecount = 0;
    int m_facecount = 0;
    int face_id = 10000;
	float fps = 5;
	float reccost = 200;
	ALG_TYPE alg_type = TENGINE; 
public:
	void  bindToCpu(int cpu1, int cpu2)
    {

        cpu_set_t mask;
        CPU_ZERO(&mask);
        //CPU_SET(cpu,&mask);
        CPU_SET(cpu1, &mask);
        CPU_SET(cpu2, &mask);

        if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) {
    		fprintf(stderr, "set thread affinity failed\n");
    	}
    }
    AlgThread();
    AlgThread(int width, int height, int scale, string path, ALG_TYPE type = TENGINE)
    {
        framescale = scale;
        framewidth = width;
        frameheight = height;
        registerFacesPath = path;
		alg_type = type; 
    };
    ~AlgThread();
    void sendFrame(Mat &mat, Mat &src);
    Mat getfacemat(int i);
    Mat getframe();
    Mat getSrcframe();
    Mface getFace(int i);
	float getFps();
	float getReccost();
	int getFaceID();
    void FaceDemoInit(double threshold_p, double threshold_r, double threshold_o, double threshold_score, double factor, int mim_size);
    char* FaceDemoRecognize(Mat &mRgb, bool track, char* posStr);
    void run();

	int flushRegistMat = 0;
};


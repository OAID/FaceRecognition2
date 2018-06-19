#include <AlgThread.h>
#include <string>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/syscall.h>

#include <algorithm>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv/cv.h>
#include "config.h"
#include "util.h"


#define CONFIG_FILE_PATH "demo.conf"

using namespace std;
using namespace cv;


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

void _strlwr_d(char* src)
{  
    while (*src != '\0')  
    {  
        if (*src >= 'A' && *src <= 'Z')
        {
            *src += 32;
        }  
        src++;
    }  
} 


int main(int argc, char * argv[])
{
    if(!Config::Instance()->LoadConfig(CONFIG_FILE_PATH)) {
        cerr << "load config failed" << endl;
        return -1;
    }

    int DeviceID = Util::String2Int(Config::Instance()->GetValue("VideoDeviceID"));
    int videoWidth = Util::String2Int(Config::Instance()->GetValue("VideoWidth"));
    int videoHeight = Util::String2Int(Config::Instance()->GetValue("VideoHeight"));
    int scale = Util::String2Int(Config::Instance()->GetValue("Scale"));
    int bDrawRect = Util::String2Int(Config::Instance()->GetValue("BoolDrawRect"));
    std::string facePicturePATH = Config::Instance()->GetValue("FacePicturePATH");
    std::string videoStreamAddress = Config::Instance()->GetValue("VideoURL");
    std::string engine = "Tengine";
    ALG_TYPE alg_type = TENGINE;

    if(argc == 2)
    {
        _strlwr_d(argv[1]);
        std::string str_src = argv[1];
        
        std::cout << "str_src = " << str_src << endl;
        
        if(str_src == "tengine")
        {
            engine = "Tengine";
            alg_type = TENGINE;
        }
        else if(str_src == "caffe-hrt")
        {
            engine = "Caffe-HRT";
            alg_type = CAFFE_HRT;
        }
    }

    VideoCapture capture;

    capture.open(DeviceID);
    if (!capture.isOpened()) { //判断能够打开摄像头
        cout << "can not open the camera" << endl;
        cin.get();
        exit(1);
    }
    capture.set(CV_CAP_PROP_FRAME_WIDTH, videoWidth);
    capture.set(CV_CAP_PROP_FRAME_HEIGHT, videoHeight);


    Mat frame;
    int width = videoWidth / scale;
    int height = videoHeight / scale;

    namedWindow("AID-SHOW", 0);
    moveWindow("AID-SHOW", 320, 0);


    AlgThread *algThd = new AlgThread(width, height, scale, facePicturePATH, alg_type);
    algThd->start();
    cv::Size ResImgSiz = cv::Size(videoWidth / scale, videoHeight / scale);
    cv::Size ShowImgSiz = cv::Size(videoWidth + 240, videoHeight);
    Mat ResImg = Mat(ResImgSiz, frame.type());
    Mat ShowImg = Mat(ShowImgSiz, CV_8UC3);
    Mat show_recognized = Mat(720, 120, CV_8UC3);
    Mat show_registed = Mat(720,120,CV_8UC3);

    int showreg = 0;
    bindToCpu(2, 3);
    while(1) {
        capture.read(frame);
        if(!frame.empty()) {

            cv::resize(frame, ResImg, ResImgSiz, CV_INTER_LINEAR);
            show_recognized -= show_recognized;
            algThd->sendFrame(ResImg, frame);
            if(1 == bDrawRect) {
                int step = 0;
                for(int i = 0; i < 3; i ++) {
                    Mface f = algThd->getFace(i);
                    if(f.drawflag == 1) {
                        if(f.id < 10000)
                            cv::putText(frame, Util::Int2String(f.time), cvPoint(f.pos.x * scale, (f.pos.y + f.pos.height) * scale), 1, 8, cvScalar(0, 0, 255, 0), 8);
                        else
                            cv::putText(frame, Util::Int2String(f.id), cvPoint(f.pos.x * scale, f.pos.y * scale + 20), 1, 1, cvScalar(0, 255, 0, 0), 2);
                        cv::rectangle(frame, cvPoint(f.pos.x * scale, f.pos.y * scale), cvPoint((f.pos.x + f.pos.width)*scale, (f.pos.y + f.pos.height)*scale), cvScalar(0, 255, 0, 0), 3);
                        std::string facepath = "./faces/" + f.name + ".png";
                        Mat showmat = imread(facepath, CV_LOAD_IMAGE_UNCHANGED);
                        if(!showmat.empty()) {
                            showreg = 1;
                            cv::rectangle(showmat, cvPoint(0, 0), cvPoint(120, 120), cvScalar(255, 255, 255, 0), 3);
                            Mat bgROIMat = show_recognized(Rect(0, step * 120, 120, 120));
                            step++;
                            showmat.copyTo(bgROIMat);
                        }
                    }
                }
            }
        }
        char fps[32];
        sprintf(fps, "fps  : %.1f", algThd->getFps());
        char cost[32];
        sprintf(cost, "cost : %.1f", algThd->getReccost());
        cv::putText(frame, String(fps), cvPoint(10, 80), 2, 3, cvScalar(192, 0, 0, 0), 6);
        cv::putText(frame, engine, cvPoint(570,680), 2, 3, cvScalar(0, 0, 192, 0), 6);
        //cv::putText(frame, String(cost), cvPoint(10, 80), 1, 3, cvScalar(192, 192, 192, 0), 2);
        if(algThd->flushRegistMat == 1){
            int step = 0;
            int face_id = algThd->getFaceID();
            for(int m = face_id; m > face_id - 6; m --) {
                char string_m[16];
                sprintf(string_m, "%d", m);
                std::string facepath = "./faces/" + String(string_m) + ".png";
                Mat rsmat = imread(facepath, CV_LOAD_IMAGE_UNCHANGED);
                cv::rectangle(rsmat, cvPoint(0, 0), cvPoint(120, 120), cvScalar(255, 255, 255, 0), 3);
                if(rsmat.empty()) {
                    continue;
                }
                Mat bgROIMat = show_registed(Rect(0, step * 120, 120, 120));
                step++;
                rsmat.copyTo(bgROIMat);
            }		
            show_registed.copyTo(ShowImg(cv::Rect(frame.cols+show_recognized.cols, 0,  show_registed.cols, show_registed.rows)));
            algThd->flushRegistMat = 0;
        }
        show_recognized.copyTo(ShowImg(cv::Rect(0, 0, show_recognized.cols, show_recognized.rows)));
        frame.copyTo(ShowImg(cv::Rect(120, 0, frame.cols, frame.rows)));
        imshow("AID-SHOW", ShowImg);
        waitKey(1);
    }
    algThd->join();
    return 0;
}

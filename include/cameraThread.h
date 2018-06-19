#include <thread.h>
#include <cv.h>
#include <opencv2/opencv.hpp>

using namespace std; 
using namespace cv;
class cameraThread : public Thread
{
	private:
		int framewidth = 640;
		int frameheight = 480;
		Mat m_Mat;
		CvRect m_rect[3];
		Mutex mt;
	public:


		void  bindToCpu(int cpu1, int cpu2) {

		    cpu_set_t mask;
		    CPU_ZERO(&mask);
		    //CPU_SET(cpu,&mask);
		    CPU_SET(cpu1,&mask);
		    CPU_SET(cpu2,&mask);
 
			if (sched_setaffinity(0, sizeof(mask), &mask) < 0) {
				perror("sched_setaffinity");
			}

		}
		cameraThread();
		cameraThread(int width, int height)
		{
			framewidth = width;
			frameheight = height;
		};
		~cameraThread();
		Mat getframe()
		{
			Mat m;
			mt.lock();
			m = m_Mat.clone();
			mt.unlock();
			return m;
		};
		void sendmessage(float* pos, int      num)
		{
			for(int i = 0; i < num; i ++)
			{
				m_rect[i].x = pos[num * 14 + 1];
				m_rect[i].y = pos[num * 14 + 2];
				m_rect[i].width = pos[num * 14 + 3] - pos[num * 14 + 1];
				m_rect[i].height= pos[num * 14 + 4] - pos[num * 14 + 2];
				printf("%f %f %f %f\n",m_rect[i].x,m_rect[i].y,m_rect[i].width,m_rect[i].height);
			}
		};
		void sendmessage(CvRect &rect)
		{
			m_rect[0] = rect;
		}
		void run()
		{
			//bindToCpu(4,5);
			VideoCapture capture(1);  
		    if (!capture.isOpened()) { //判断能够打开摄像头  
		        cout<<"can not open the camera"<<endl;  
		        cin.get();  
		        exit(1);  
		    }  
			
			capture.set(CV_CAP_PROP_FRAME_WIDTH, framewidth);  
    		capture.set(CV_CAP_PROP_FRAME_HEIGHT, frameheight);
			
			Mat frame;
			
			while(1)
			{   
		        capture.read(frame);
				mt.lock();
				m_Mat = frame.clone();
				mt.unlock();
				//cvRectangle(img,CvPoint(m_rect[0].x,m_rect[0].y), CvPoint(m_rect[0].x+m_rect[0].width,m_rect[0].y+m_rect[0].height), CV_RGB(0, 255, 0), 4);
		        imshow("face",frame);  
		        waitKey(1);  
    		} 
		};
};

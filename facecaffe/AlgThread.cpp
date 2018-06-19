#include "AlgThread.h"

using namespace std;
using namespace cv;

void AlgThread::sendFrame(Mat &mat, Mat &src)
{
    mt.lock();
    m_Mat = mat.clone();
    m_srcMat = src.clone();
    mt.unlock();
}
Mat AlgThread::getfacemat(int i)
{
    Mat m;
    if(faces[i].lasttime > 0) {
        m = faces[i].pic.clone();
    }
    return m;
}
Mat AlgThread::getframe()
{
    Mat m;
    m = m_Mat.clone();
    return m;
}
Mat AlgThread::getSrcframe()
{
    Mat m;
    m = m_srcMat.clone();
    return m;
}
Mface AlgThread::getFace(int i)
{
    Mface r;
    r = m_face[i];
    return r;
}
float AlgThread::getFps()
{
	return fps;
}
float AlgThread::getReccost()
{
	return reccost;
}
int AlgThread::getFaceID()
{
	return face_id;
}

void AlgThread::FaceDemoInit(double threshold_p, double threshold_r, double threshold_o, double threshold_score, double factor, int mim_size)
{
    int ret = mFace_demo.Init(threshold_p, threshold_r, threshold_o, threshold_score, factor, mim_size);
    mFace_demo.LocalLoad(MODEL_DIR);
}

char* AlgThread::FaceDemoRecognize(Mat &mRgb, bool track, char* posStr)
{
    std::string ss = mFace_demo.Recognize(mRgb, 3);
    memcpy(posStr, (char*)ss.c_str(), ss.length());

    return NULL;
}

void AlgThread::run()
{
    bindToCpu(4, 5);
    int minsize = Util::String2Int(Config::Instance()->GetValue("MinFaceSize"));
    float face_angle = Util::String2Double(Config::Instance()->GetValue("FaceAngle"));
    char position[1024];
    char handposi[1024];
    char regiposi[1024];
    const char *sep = " <>";
    Mat mat;
    Mat src;
    struct timeval tv_start, tv_end;
    struct timeval tv1, tv2;
    int framecount = 0;
    int cost1 = 0;
    float cost2 = 0;
    
    mFace_demo.SetAlgType(alg_type);
    
    FaceDemoInit(0.9, 0.9, 0.9, 0.6, 0.7, minsize);
    
    while(1) {
        gettimeofday(&tv_start, NULL);
        mt.lock();
        mat = getframe();
        src = getSrcframe();
        mt.unlock();
        if(0 < mat.total()) {
            	framecount ++;
            	gettimeofday(&tv1, NULL);
                FaceDemoRecognize(mat, false, position);
                gettimeofday(&tv2, NULL);
                cost1 += tv2.tv_sec * 1000 + tv2.tv_usec / 1000 - tv1.tv_sec * 1000 - tv1.tv_usec / 1000;
                char *p;
                int i = 0;
                char *sub[150];
                p = strtok(position, sep);
                while (p && i < 150) {
                    sub[i] = p;
                    p = strtok(NULL, sep);
                    i++;
                }
                int shift = 1;
                float pos2[50];
                for (int icount = 0; icount < atoi(sub[1]); icount++) {
                    for (int j = 1; j <= 4; j++) {
                        pos2[shift] = atof(sub[2 + icount * 47 + j * 3]);
                        shift++;
                    }
                    m_face[icount].pos.x = pos2[icount * 14 + 1];
                    m_face[icount].pos.y = pos2[icount * 14 + 2];
                    m_face[icount].pos.width = pos2[icount * 14 + 3] - pos2[icount * 14 + 1];
                    m_face[icount].pos.height = pos2[icount * 14 + 4] - pos2[icount * 14 + 2];
                    m_face[icount].name = String(sub[icount * 47 + 18]);
                    m_face[icount].id = atoi(sub[icount * 47 + 17]);
                    m_face[icount].drawflag = 1;

                    for (int j = 1; j <= 10; j++) {
                        pos2[shift] = atof(sub[18 + icount * 47 + j * 3]);
                        shift++;
                    }
                    m_face[icount].node[0].x = pos2[icount * 14 + 5];
                    m_face[icount].node[0].y = pos2[icount * 14 + 6];
                    m_face[icount].node[1].x = pos2[icount * 14 + 7];
                    m_face[icount].node[1].y = pos2[icount * 14 + 8];
                    m_face[icount].node[2].x = pos2[icount * 14 + 9];
                    m_face[icount].node[2].y = pos2[icount * 14 + 10];
                    m_face[icount].node[3].x = pos2[icount * 14 + 11];
                    m_face[icount].node[3].y = pos2[icount * 14 + 12];
                    m_face[icount].node[4].x = pos2[icount * 14 + 13];
                    m_face[icount].node[4].y = pos2[icount * 14 + 14];


                    Rect rcb;
                    rcb.x = m_face[icount].pos.x - (m_face[icount].pos.width >> 2);
                    rcb.width = (m_face[icount].pos.width * 3) >> 1;
                    rcb.y = m_face[icount].pos.y - (m_face[icount].pos.height >> 2);
                    rcb.height = (m_face[icount].pos.height * 3) >> 1;
                    if(rcb.width > mat.cols) {
                        rcb.width = mat.cols;
                    }
                    if(rcb.height > mat.rows) {
                        rcb.height = mat.rows;
                    }
                    if((rcb.x) < 0) {
                        rcb.x = 0;
                    }
                    if((rcb.y) < 0) {
                        rcb.y = 0;
                    }
                    if((rcb.x + rcb.width) > mat.cols) {
                        rcb.x = mat.cols - rcb.width;
                    }
                    if((rcb.y + rcb.height) > mat.rows) {
                        rcb.y = mat.rows - rcb.height;
                    }
                }

                for(int icount = 3; icount > atoi(sub[1]); icount--) {
                    m_face[icount - 1].drawflag = 0;
                }
                struct timeval tv1;
                gettimeofday(&tv1, NULL);
                int curtime = tv1.tv_sec * 1000 + tv1.tv_usec / 1000;
                for(int i = 0; i < 9; i ++) {
                    if(faces[i].lasttime >= 0) {
                        faces[i].lasttime = -10;
                        for (int j = 0; j < atoi(sub[1]); j++) {
                            if(faces[i].id == m_face[j].id) {
                                faces[i].pos = m_face[j].pos;
                                if((m_face[j].node[1].x-m_face[j].node[0].x)>(m_face[j].pos.width*face_angle))
                                	faces[i].lasttime = faces[i].capTime + 5000 - curtime;
                                else
                                    faces[i].lasttime = 5000;
                                if(faces[i].lasttime < 0) {
                                    face_id ++;
                                    char name[16];
                                    sprintf(name, "%d", face_id);
                                    int ret = mFace_demo.Register(faces[i].id, name);
                                    if(ret == 0)
                                        mFace_demo.LocalSave(MODEL_DIR);
                                    faces[i].lasttime = -10;

                                    Rect rcb;
                                    rcb.x = faces[i].pos.x - (faces[i].pos.width >> 2);
                                    rcb.width = (faces[i].pos.width * 3) >> 1;
                                    rcb.y = faces[i].pos.y - (faces[i].pos.height >> 2);
                                    rcb.height = (faces[i].pos.height * 3) >> 1;
                                    if(rcb.width > mat.cols) {
                                        rcb.width = mat.cols;
                                    }
                                    if(rcb.height > mat.rows) {
                                        rcb.height = mat.rows;
                                    }
                                    if((rcb.x) < 0) {
                                        rcb.x = 0;
                                    }
                                    if((rcb.y) < 0) {
                                        rcb.y = 0;
                                    }
                                    if((rcb.x + rcb.width) > mat.cols) {
                                        rcb.x = mat.cols - rcb.width;
                                    }
                                    if((rcb.y + rcb.height) > mat.rows) {
                                        rcb.y = mat.rows - rcb.height;
                                    }

                                    Mat mat_tmp = src(Rect(rcb.x * framescale, rcb.y * framescale, rcb.width * framescale, rcb.height * framescale));
                                    Mat rsmat = Mat(Size(120, 120), src.type());
                                    resize(mat_tmp, rsmat, Size(120, 120), CV_INTER_LINEAR);

                                    char path[64];
                                    sprintf(path, "./faces/%s.png", name);
                                    imwrite(String(path), rsmat);
                                    flushRegistMat = 1;

                                }
                            }
                        }
                    }
                }
                for (int icount = 0; icount < atoi(sub[1]); icount++) {
                    int showflag = 0;
                    for(int i = 0; i < 9; i ++) {
                        if(faces[i].lasttime > 0) {
                            if(faces[i].id == m_face[icount].id) {
                                m_face[icount].time = faces[i].lasttime / 1000;
                                showflag = 1;
                            }
                        }
                    }
                    if(showflag == 1)
                        continue;
                    for(int j = 0; j < 9; j ++) {
                        if(faces[j].lasttime == -10 && m_face[icount].id < 10000) {
                            faces[j].capTime = curtime;
                            faces[j].lasttime = 5000;
                            faces[j].id = m_face[icount].id;
                            faces[j].name = m_face[icount].name;
                            break;
                        }
                    }
                }
        }

        gettimeofday(&tv_end, NULL);
        cost2 += tv_end.tv_sec * 1000 + tv_end.tv_usec / 1000 - tv_start.tv_sec * 1000 - tv_start.tv_usec / 1000;

        if(framecount%10 == 0)
        {
            reccost = cost1/10;
            fps = (10*1000 / cost2);
            cost1 = 0;
            cost2 = 0;
        }
        mat.release();
        src.release();
    }
}



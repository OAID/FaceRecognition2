#ifndef __FACE_DEMO__HPP__
#define __FACE_DEMO__HPP__

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

#include <unistd.h>
#include <signal.h>

#include "ai_type.h"
#include "mtcnn.hpp"
#include "face_align.hpp"
#include "face_verify.hpp"

#include "face_mem_store.hpp"
#include "face_demo.hpp"
#include "recognition.h"

#include "utils.hpp"

#include "tengine_c_api.h"


#define DEBUG 0

using namespace cv;

struct face_window
{
    face_box box;
    unsigned int face_id;
    unsigned int frame_seq;
    float center_x;
    float center_y;
    std::string name;
    char title[128];
    std::list<float> score_stored;
    
    int add_score(float score)
    {
        if(score_stored.size() == 3)
        {
            score_stored.pop_front();
        }
        score_stored.push_back(score);
        return score_stored.size();
    }
    
    float get_avg_score()
    {
        float sum = 0;
        for(auto i:score_stored)
        	sum += i;
        return sum/score_stored.size();
    }
};


class FaceDemo {
    public:
        FaceDemo(ALG_TYPE _alg_type = TENGINE){
            current_frame_count = 0;
            win_keep_limit = 10;
            trace_pixels = 100;
            database_name = "/face_demo.dat";
            score_thresh = 0.55;
            alg_type = _alg_type;
        }
        int Init(double threshold_p, double threshold_r, double threshold_o, double threshold_score, double factor, int mim_size);
        std::string Recognize(cv::Mat &frame, int face_num);
        //Register a face in last frame by face_id
        int Register(int face_id, std::string name);
        //Register a face by a frame which must contain only 1 face
        int Register(cv::Mat &frame, std::string name);
        int LocalSave(std::string path);
        int LocalLoad(std::string path);
        void SetAlgType(ALG_TYPE _alg_type);

    protected:
        void get_data(float* input_data, Mat &gray, int img_h, int img_w);
        int get_new_unknown_face_id(void);
        unsigned int get_new_registry_id(void);
        void get_face_name_by_id(unsigned int face_id, std::string& name);
        void drop_aged_win(unsigned int frame_count);
        face_window * get_face_id_name_by_position(face_box& box,unsigned int frame_seq);
        void get_face_title(cv::Mat& frame,face_box& box,unsigned int frame_seq);

        std::vector<face_window*> face_win_list;
        mtcnn * p_mtcnn = NULL;
        //feature_extractor * p_extractor = NULL;
        Classifier classifier;
        face_verifier   * p_verifier = NULL;
        face_mem_store * p_mem_store = NULL;
        cv::Mat p_cur_frame;
        int current_frame_count;
        int win_keep_limit;
        int trace_pixels;
        std::string database_name;
        double score_thresh;

        ALG_TYPE alg_type;

        std::string model_dir = MODEL_DIR;
        const char *input_node_name = "input";
        const char *model_name = "lighten_cnn";
        std::string proto_name_ = model_dir+"/LightenedCNN_B.prototxt";
        std::string mdl_name_ = model_dir+"/LightenedCNN_B.caffemodel";
        graph_t graph;
        float *input_data;	
        const char *input_tensor_name = "data";
        std::string input_fname = model_dir+"test.jpg";
        tensor_t input_tensor;

};



#endif

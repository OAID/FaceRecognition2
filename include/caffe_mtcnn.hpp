#ifndef __CAFFE_MTCNN_HPP__
#define __CAFFE_MTCNN_HPP__

#include <string>
#include <vector>

#include <caffe/caffe.hpp>
#include <caffe/layers/memory_data_layer.hpp>

#include <opencv2/opencv.hpp>

#include "ai_type.h"
#include "mtcnn.hpp"
#include "comm_lib.hpp"

#include "tengine_c_api.h"


using namespace caffe;

class caffe_mtcnn: public mtcnn {

    public:
        caffe_mtcnn()=default;

        int load_3model(const std::string& model_dir);

        void detect(cv::Mat& img, std::vector<face_box>& face_list);

        ~caffe_mtcnn();

    protected:

        void copy_one_patch(const cv::Mat& img,face_box&input_box,float * data_to, int width, int height);

        int run_PNet(const cv::Mat& img, scale_window& win, std::vector<face_box>& box_list);
        void run_RNet(const cv::Mat& img,std::vector<face_box>& pnet_boxes, std::vector<face_box>& output_boxes);
        void run_ONet(const cv::Mat& img,std::vector<face_box>& rnet_boxes, std::vector<face_box>& output_boxes);


    private:
        int run_PNet_Tengine(const cv::Mat& img, scale_window& win, std::vector<face_box>& box_list);
        void run_RNet_Tengine(const cv::Mat& img,std::vector<face_box>& pnet_boxes, std::vector<face_box>& output_boxes);
        void run_ONet_Tengine(const cv::Mat& img,std::vector<face_box>& rnet_boxes, std::vector<face_box>& output_boxes);
        
        int run_PNet_Caffe_HRT(const cv::Mat& img, scale_window& win, std::vector<face_box>& box_list);
        void run_RNet_Caffe_HRT(const cv::Mat& img,std::vector<face_box>& pnet_boxes, std::vector<face_box>& output_boxes);
        void run_ONet_Caffe_HRT(const cv::Mat& img,std::vector<face_box>& rnet_boxes, std::vector<face_box>& output_boxes);

        Net<float> * PNet_ = NULL;
        Net<float> * RNet_ = NULL;
        Net<float> * ONet_ = NULL;

        const char * PNet_model_name="PNet";
        const char * RNet_model_name="RNet";
        const char * ONet_model_name="ONet";
        graph_t PNet_graph = NULL;
        graph_t RNet_graph = NULL;
        graph_t ONet_graph = NULL;

};


#endif

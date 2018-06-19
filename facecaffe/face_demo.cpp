
#include "face_demo.hpp"
#include "caffe_mtcnn.hpp"

#include "face_verify.hpp"
#include "preprocess.h"
#include "face_align.hpp"
#include "scale_angle.h"


#include <chrono>
#include <time.h>
#include <string.h>
#include <stdlib.h>


#include "conf/stdtostring.h"
#define UNKNOWN_FACE_ID_MAX 1000


int Split(const std::string& src, const std::string& separator, std::vector<std::string>& dest);
void draw_box_and_title(cv::Mat& frame, face_box& box, char * title);


static mtcnn * caffe_creator(void)
{
    return new caffe_mtcnn();
}

static face_verifier * cosine_distance_verifier_creator(const std::string& name)
{
    return  new cosine_distance_verifier();
}

static bool GreaterSort (face_box a, face_box b)
{
    return (abs(a.x1 - a.x0) * (a.y1 - a.y0) > abs((b.x1 - b.x0) * (b.y1 - b.y0)));
}


/*****************************************************************************************************/
void FaceDemo::SetAlgType(ALG_TYPE _alg_type)
{
    alg_type = _alg_type;
}

int FaceDemo::Init(double threshold_p, double threshold_r, double threshold_o, double threshold_score, double factor, int mim_size)
{
    const char * type = "caffe";

    REGISTER_MTCNN_CREATOR(caffe, caffe_creator);

    if(alg_type == TENGINE)
    {
        init_tengine_library();
        if (request_tengine_version("0.1") < 0)
            return -1;
    }

    p_mtcnn = mtcnn_factory::create_detector(type);

    if(p_mtcnn == nullptr) {
        std::cerr << type << " is not supported" << std::endl;
        std::cerr << "supported types: ";
        std::vector<std::string> type_list = mtcnn_factory::list();

        for(int i = 0; i < type_list.size(); i++)
            std::cerr << " " << type_list[i];

        std::cerr << std::endl;

        return -1;
    }

    if(alg_type == TENGINE)
    {
    	//classify init
        if (load_model(model_name, "caffe", proto_name_.c_str(), mdl_name_.c_str()) < 0)
            return 1;

        graph = create_runtime_graph("graph", model_name, NULL);
        set_graph_input_node(graph, &input_node_name, 1);

        if (!check_graph_valid(graph)) {
            std::cout << "create graph0 failed\n";
            return 1;
        }
        std::cout << "create graph done!\n";

        // input
        int img_h = 128;
        int img_w = 128;
        int img_size = img_h * img_w;
        input_data = (float *)malloc(sizeof(float) * img_size);

        input_tensor = get_graph_input_tensor(graph, 0, 0);
        int dims[] = {1, 1, img_h, img_w};
        set_tensor_shape(input_tensor, dims, 4);
        prerun_graph(graph);

    }
    
    p_mtcnn->set_AI_type(alg_type);
    p_mtcnn->load_3model(MODEL_DIR);

    p_mtcnn->set_threshold(threshold_p, threshold_r, threshold_o);
    p_mtcnn->set_factor_min_size(factor, mim_size);

    if(alg_type == CAFFE_HRT)
    {
        classifier.LoadModel(MODEL_DIR, MODEL_DIR);
    }

    REGISTER_SIMPLE_VERIFIER(cosine_distance, cosine_distance_verifier_creator);

    /* verifier*/
    p_verifier = get_face_verifier("cosine_distance");

    /* store */
    p_mem_store = new face_mem_store(256, 1000);

    score_thresh = threshold_score;

    return 0;
}

void FaceDemo::get_data(float* input_data, Mat &gray, int img_h, int img_w)
{
    int datasize = img_h * img_w ;
    if(gray.isContinuous())
    {
        unsigned char * p_pixel=gray.ptr(0);

        for(int i=0;i<datasize;i++)
        {
            input_data[i]=((float)p_pixel[i])/256;
        }
    }
    else
    {
        for(int i=0;i<img_h;i++)
        {
            unsigned char * p_row=gray.ptr(i);

            for(int j=0;j<img_w;j++)
            {
                unsigned char p_pixel=p_row[j];

                input_data[0]=((float)p_pixel)/255;
                input_data++;
            }
        }
    }
}


std::string FaceDemo::Recognize(cv::Mat &frame, int face_num)
{
    std::vector<face_box> face_info;
    std::string result = "";
    current_frame_count++;
    frame.copyTo(p_cur_frame);
    int face_count = 0;

#if DEBUG
    struct timeval tv_start, tv_end;
    gettimeofday(&tv_start, NULL);
#endif

    p_mtcnn->detect(frame, face_info);

#if DEBUG
    gettimeofday(&tv_end, NULL);
    printf("time of detect : %d\n", tv_end.tv_sec * 1000 + tv_end.tv_usec / 1000 - tv_start.tv_sec * 1000 - tv_start.tv_usec / 1000);
#endif

    if(face_info.size() > face_num) {
        std::sort(face_info.begin(), face_info.end(), GreaterSort);
        face_info.erase(face_info.begin() + face_num, face_info.end());
    }
    for(unsigned int i = 0; i < face_info.size(); i++) {
        face_box& box = face_info[i];
        get_face_title(frame, box, current_frame_count);
    }
    for(unsigned int i = 0; i < face_win_list.size(); i++) {
        if(face_win_list[i]->frame_seq != current_frame_count)
            continue;

       result = result + "[box, x0: <" + std::to_string(face_win_list[i]->box.x0) + ">, y0: <" \
                 + std::to_string(face_win_list[i]->box.y0) + ">, x1: <" + std::to_string(face_win_list[i]->box.x1) \
                 + ">, y1: <" + std::to_string(face_win_list[i]->box.y1) + ">, title: <" \
                 + face_win_list[i]->title \
                 + ">, markx0: <" + std::to_string(face_win_list[i]->box.landmark.x[0]) \
                 + ">, marky0: <" + std::to_string(face_win_list[i]->box.landmark.y[0]) \
                 + ">, markx1: <" + std::to_string(face_win_list[i]->box.landmark.x[1]) \
                 + ">, marky1: <" + std::to_string(face_win_list[i]->box.landmark.y[1]) \
                 + ">, markx2: <" + std::to_string(face_win_list[i]->box.landmark.x[2]) \
                 + ">, marky2: <" + std::to_string(face_win_list[i]->box.landmark.y[2]) \
                 + ">, markx3: <" + std::to_string(face_win_list[i]->box.landmark.x[3]) \
                 + ">, marky3: <" + std::to_string(face_win_list[i]->box.landmark.y[3]) \
                 + ">, markx4: <" + std::to_string(face_win_list[i]->box.landmark.x[4]) \
                 + ">, marky4: <" + std::to_string(face_win_list[i]->box.landmark.y[4]) \
                 + ">] ";

        face_count++;

        if(face_count >= face_num)
            break;

    }

    result = std::string("faces: <") + std::to_string(face_count) + ">; " + result;
    drop_aged_win(current_frame_count);

    return result;
}

int FaceDemo::Register(int face_id, std::string name)
{
    if(face_id < 0 || name == "") {
        //"bad arguments\n"
        return -1;
    }

    face_info * p_info = p_mem_store->find_record(face_id);

    if(p_info && p_info->name != name) {
        //"do not support change name
        return -2;
    }

    unsigned int i;
    face_window * p_win;

    for(i = 0; i < face_win_list.size(); i++) {
        if(face_win_list[i]->face_id == face_id &&
           face_win_list[i]->frame_seq == current_frame_count)
            break;
    }

    if(i == face_win_list.size()) {
        std::cout << "cannot find face with id: " << face_id << std::endl;
        return -3;
    }

    p_win = face_win_list[i];

    face_info info;

    info.p_feature = (float *)malloc(256* sizeof(float));
    face_box box = p_win->box;
    /*
    FaceBaseInfo fi;
    fi.bbox = cv::Rect(box.x0,box.y0,box.x1-box.x0,box.y1-box.y0);
    fi.eyeleft = cv::Point2f(box.landmark.x[0],box.landmark.y[0]);
    fi.eyeright = cv::Point2f(box.landmark.x[1],box.landmark.y[1]);
    fi.nose = cv::Point2f(box.landmark.x[2],box.landmark.y[2]);
    fi.mouthleft = cv::Point2f(box.landmark.x[3],box.landmark.y[3]);
    fi.mouthright = cv::Point2f(box.landmark.x[4],box.landmark.y[4]);
    */
    Mat aligned;

    int alignflag = get_aligned_face(p_cur_frame, (float *)&box.landmark, 5, 128, 128, aligned);

    

    if (alignflag) {
        	
        cvtColor(aligned, aligned, CV_BGR2GRAY);

        if(alg_type == CAFFE_HRT)
        {
            std::vector<float> cur_feature;
            cur_feature = classifier.Classify(aligned);
            for(int i = 0; i < 256; i++) {
                info.p_feature[i] = cur_feature[i];
            }
        }
        else if(alg_type == TENGINE)
        {
            float *data1;
            int img_h = 128;
            int img_w = 128;
            int img_size = img_h * img_w;
            get_data(input_data,aligned,img_h,img_w);
            if (set_tensor_buffer(input_tensor, input_data, img_size * 4) < 0)
            {
            	std::printf("set buffer for tensor: %s failed\n", input_tensor_name);
            	return -1;
            }		   
            run_graph(graph, 1);
            tensor_t mytensor1 = get_graph_tensor(graph, "eltwise_fc1");
            data1 = (float *)get_tensor_buffer(mytensor1);
            for(int i = 0; i < 256; i++) {
                info.p_feature[i] = data1[i];
            }
        }

    } else {
        return 1;
    }

    

    if(face_id < UNKNOWN_FACE_ID_MAX)
        info.face_id = get_new_registry_id();
    else
        info.face_id = face_id;

    info.name = name;
    info.feature_len = 256;


    p_mem_store->insert_new_record(info);

    p_verifier->insert_feature(info.p_feature, info.face_id);

    return 0;
}

int FaceDemo::LocalSave(std::string path)
{
    std::vector<face_info *> list;
    std::ofstream local_store(path + database_name, std::ios::trunc);
    if(!local_store.is_open())
        return -1;   //open failed
    int n_faces = p_mem_store->get_all_records(list);

    for(int i = 0; i < n_faces; i++) {
        local_store << list[i]->name << std::endl;
        local_store << list[i]->face_id << std::endl;
        local_store << list[i]->feature_len << std::endl;
        for(int j = 0; j < list[i]->feature_len; j++) {
            local_store << list[i]->p_feature[j] << " ";
        }
        local_store << std::endl;
    }
    local_store.close();
    return 0;
}

int FaceDemo::LocalLoad(std::string path)
{
    face_info * p_info =  new face_info();
    std::ifstream local_store(path + database_name);
    if(!local_store.is_open())
        return -1;   //open failed
    std::string db_name, db_id, db_len, db_feature;
    std::vector<std::string> features;
    while( std::getline(local_store, db_name) && std::getline(local_store, db_id) && std::getline(local_store, db_len) && std::getline(local_store, db_feature)) {
        features.clear();
        p_info->name = db_name;
        p_info->face_id = atoi(db_id.c_str());
        p_info->feature_len = atoi(db_len.c_str());
        p_info->p_feature = (float *)malloc(sizeof(float) * (p_info->feature_len));
        Split(db_feature, " ", features);
        for(int i = 0; i < p_info->feature_len; i++) {
            p_info->p_feature[i] = atof(features[i].c_str());
        }
        /* insert feature into mem db */

        p_mem_store->insert_new_record(*p_info);

        /* insert feature into verifier */

        p_verifier->insert_feature(p_info->p_feature, p_info->face_id);

        free(p_info->p_feature);
    }
    local_store.close();
    return 0;
}


/*****************************************************************************/


int FaceDemo::get_new_unknown_face_id(void)
{
    static unsigned int current_id = 0;

    return  (current_id++ % UNKNOWN_FACE_ID_MAX);
}

unsigned int FaceDemo::get_new_registry_id(void)
{
    static unsigned int register_id = 10000;

    register_id++;

    if(register_id == 20000)
        register_id = 10000;

    return register_id;
}



void FaceDemo::get_face_name_by_id(unsigned int face_id, std::string& name)
{
    face_info * p_info;

    p_info = p_mem_store->find_record(face_id);

    if(p_info == nullptr) {
        name = "nullname";
    } else {
        name = p_info->name;
    }
}


void FaceDemo::drop_aged_win(unsigned int frame_count)
{
    std::vector<face_window *>::iterator it = face_win_list.begin();

    while(it != face_win_list.end()) {
        if((*it)->frame_seq + win_keep_limit < frame_count) {
            delete (*it);
            face_win_list.erase(it);
        } else
            it++;
    }
}

face_window * FaceDemo::get_face_id_name_by_position(face_box& box, unsigned int frame_seq)
{
    int found = 0;
    float center_x = (box.x0 + box.x1) / 2;
    float center_y = (box.y0 + box.y1) / 2;
    face_window * p_win;

    std::vector<face_window *>::iterator it = face_win_list.begin();

    while (it != face_win_list.end()) {
        p_win = (*it);
        float offset_x = p_win->center_x - center_x;
        float offset_y = p_win->center_y - center_y;

        if((offset_x < trace_pixels) &&
           (offset_x > -trace_pixels) &&
           (offset_y < trace_pixels) &&
           (offset_y > -trace_pixels) &&
           (p_win->frame_seq + win_keep_limit) >= frame_seq) {
            found = 1;
            break;
        }
        it++;
    }


    if(!found) {
        p_win = new face_window();
        p_win->name = "unknown";
        p_win->face_id = get_new_unknown_face_id();
    }

    p_win->box = box;
    p_win->center_x = (box.x0 + box.x1) / 2;
    p_win->center_y = (box.y0 + box.y1) / 2;
    p_win->frame_seq = frame_seq;

    if(!found)
        face_win_list.push_back(p_win);

    return  p_win;

}




void FaceDemo::get_face_title(cv::Mat& frame, face_box& box, unsigned int frame_seq)
{
    float feature[256];
    int face_id;
    float score;
    face_window * p_win;

    p_win = get_face_id_name_by_position(box, frame_seq);

    if(box.x0 < 0 || box.y0 < 0 || box.x1 > frame.cols || box.y1 > frame.rows) {
        p_win->name = "unknown";
        p_win->face_id = get_new_unknown_face_id();
        sprintf(p_win->title, "%d %s", p_win->face_id, p_win->name.c_str());

        return;
    }

    Mat aligned;
    
    int alignflag = get_aligned_face(p_cur_frame, (float *)&box.landmark, 5, 128, 128, aligned);

    if (alignflag) {
        cvtColor(aligned, aligned, CV_BGR2GRAY);
        
        #if DEBUG
        struct timeval tv_start, tv_end;
        gettimeofday(&tv_start, NULL);
        #endif
        if(alg_type == CAFFE_HRT)
        {
            std::vector<float> cur_feature;
            cur_feature = classifier.Classify(aligned);
            for(int i = 0; i < 256; i++) {
                feature[i] = cur_feature[i];
            }
        }
        else if(alg_type == TENGINE)
        {
            int img_h = 128;
            int img_w = 128;
            int img_size = img_h * img_w;
            get_data(input_data,aligned,img_h,img_w);		
            if (set_tensor_buffer(input_tensor, input_data, img_size * 4) < 0)
            {
                std::printf("set buffer for tensor: %s failed\n", input_tensor_name);
                return;
            }
            run_graph(graph, 1);
            tensor_t mytensor1 = get_graph_tensor(graph, "eltwise_fc1");
            float *data1 = (float *)get_tensor_buffer(mytensor1);
            for(int i = 0; i < 256; i++) {
                feature[i] = data1[i];
            }
        }

#if DEBUG
        gettimeofday(&tv_end, NULL);
        printf("time of classify : %d\n", tv_end.tv_sec * 1000 + tv_end.tv_usec / 1000 - tv_start.tv_sec * 1000 - tv_start.tv_usec / 1000);
#endif

        /* search feature in db */
        int ret = p_verifier->search(feature, &face_id, &score);

        p_win->add_score(score);

        //float avg_score = p_win->get_avg_score();
        //std::cout << "get face title >> avg_score: " << avg_score << std::endl;
        /* found in db*/
        if(ret == 0 && score > score_thresh) {
            p_win->face_id = face_id;
            get_face_name_by_id(face_id, p_win->name);
        } else if(p_win->name != "unknown") {
            p_win->name = "unknown";
            p_win->face_id = get_new_unknown_face_id();
        }
    } else {
        /*std::cout << "align error!" << std::endl;*/
        p_win->name = "unknown";
        p_win->face_id = get_new_unknown_face_id();
        //return 1;
    }
    sprintf(p_win->title, "%d %s", p_win->face_id, p_win->name.c_str());
}

int Split(const std::string& src, const std::string& separator, std::vector<std::string>& dest)
{
    std::string str = src;
    std::string substring;
    std::string::size_type start = 0, index;
    start = str.find_first_not_of(separator, start);
    do {
        index = str.find_first_of(separator, start);
        if (index != std::string::npos) {
            substring = str.substr(start, index - start);
            dest.push_back(substring);
            start = str.find_first_not_of(separator, index);
            if (start == std::string::npos)
                return -1;
        }
    } while(index != std::string::npos);

    //the last token
    substring = str.substr(start);
    dest.push_back(substring);
    return 0;
}

void draw_box_and_title(cv::Mat& frame, face_box& box, char * title)
{

    float left, top;

    left = box.x0;
    top = box.y0 - 10;

    if(top < 0) {
        top = box.y1 + 20;
    }

    cv::putText(frame, title, cv::Point(left, top), CV_FONT_HERSHEY_PLAIN, 1.5, cv::Scalar(0, 255, 0), 2);
    cv::rectangle(frame, cv::Point(box.x0, box.y0), cv::Point(box.x1, box.y1), cv::Scalar(0, 255, 0), 1);

    for(int l = 0; l < 5; l++) {
        cv::circle(frame, cv::Point(box.landmark.x[l], box.landmark.y[l]), 1, cv::Scalar(0, 0, 255), 2);
    }

}

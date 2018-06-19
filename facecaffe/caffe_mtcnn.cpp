#include <string>
#include <vector>

#include "mtcnn.hpp"
#include "caffe_mtcnn.hpp"


caffe_mtcnn::~caffe_mtcnn(void)
{
    if(PNet_)
        delete PNet_;
    if(RNet_)
        delete RNet_;
    if(ONet_)
        delete ONet_;

    if(PNet_graph)
    {
        destroy_runtime_graph(PNet_graph);
        remove_model(PNet_model_name);
    }

    if(RNet_graph)
    {
        destroy_runtime_graph(RNet_graph);
        remove_model(RNet_model_name);
    }

    if(ONet_graph)
    {
        destroy_runtime_graph(ONet_graph);
        remove_model(ONet_model_name);
    }



}

int caffe_mtcnn::load_3model(const std::string &proto_model_dir)
{
    if(alg_type == CAFFE_HRT)
    {
        PNet_=new Net<float>((proto_model_dir + "/det1.prototxt"), caffe::TEST);
        PNet_->CopyTrainedLayersFrom(proto_model_dir + "/det1.caffemodel");


        RNet_=new Net<float>((proto_model_dir + "/det2.prototxt"), caffe::TEST);
        RNet_->CopyTrainedLayersFrom(proto_model_dir + "/det2.caffemodel");


        ONet_=new Net<float>((proto_model_dir + "/det3.prototxt"), caffe::TEST);
        ONet_->CopyTrainedLayersFrom(proto_model_dir + "/det3.caffemodel");
    }
    else if(alg_type == TENGINE)
    {
        std::string proto_name, mdl_name;
        const char * proto_name_, *mdl_name_;
        const char * input_node_name = "input";

        // Pnet
        proto_name = proto_model_dir + "/det1.prototxt";
        mdl_name = proto_model_dir + "/det1.caffemodel";
        proto_name_ = proto_name.c_str();
        mdl_name_ = mdl_name.c_str();
        if(load_model(PNet_model_name, "caffe", proto_name_, mdl_name_) < 0)
            return 1;
        std::cout << "load PNet model done!\n";
        PNet_graph = create_runtime_graph("PNet_graph", PNet_model_name, NULL);
        set_graph_input_node(PNet_graph, &input_node_name, 1);

        //Rnet
        proto_name = proto_model_dir + "/det2.prototxt";
        mdl_name = proto_model_dir + "/det2.caffemodel";
        proto_name_ = proto_name.c_str();
        mdl_name_ = mdl_name.c_str();
        if(load_model(RNet_model_name, "caffe", proto_name_, mdl_name_) < 0)
            return 1;
        std::cout << "load RNet model done!\n";
        RNet_graph = create_runtime_graph("RNet_graph", RNet_model_name, NULL);
        set_graph_input_node(RNet_graph, &input_node_name, 1);

        //Onet
        proto_name = proto_model_dir + "/det3.prototxt";
        mdl_name = proto_model_dir + "/det3.caffemodel";
        proto_name_ = proto_name.c_str();
        mdl_name_ = mdl_name.c_str();
        if(load_model(ONet_model_name, "caffe", proto_name_, mdl_name_) < 0)
            return 1;
        std::cout << "load ONet model done!\n";
        ONet_graph = create_runtime_graph("ONet_graph", ONet_model_name, NULL);
        set_graph_input_node(ONet_graph, &input_node_name, 1);
    }
    
    return 0;
}

void caffe_mtcnn::detect(cv::Mat& img, std::vector<face_box>& face_list)
{
    cv::Mat working_img;
    float alpha = 0.0078125;
    float mean = 127.5;
    img.convertTo(working_img, CV_32FC3);
    working_img = (working_img - mean) * alpha;
    working_img = working_img.t();
    cv::cvtColor(working_img, working_img, cv::COLOR_BGR2RGB);

    int img_h = working_img.rows;
    int img_w = working_img.cols;

    std::vector<scale_window> win_list;

    std::vector<face_box> total_pnet_boxes;
    std::vector<face_box> total_rnet_boxes;
    std::vector<face_box> total_onet_boxes;

    cal_pyramid_list(img_h, img_w, min_size_, factor_, win_list);
    for(int i = 0; i < win_list.size(); i++) {
        std::vector<face_box>boxes;
        run_PNet(working_img, win_list[i], boxes);
        total_pnet_boxes.insert(total_pnet_boxes.end(), boxes.begin(), boxes.end());
    }
    std::vector<face_box> pnet_boxes;
    process_boxes(total_pnet_boxes, img_h, img_w, pnet_boxes);

    if(!pnet_boxes.size())
        return;


    run_RNet(working_img, pnet_boxes, total_rnet_boxes);

    std::vector<face_box> rnet_boxes;
    process_boxes(total_rnet_boxes, img_h, img_w, rnet_boxes);

    if(!rnet_boxes.size())
        return;

    run_ONet(working_img, rnet_boxes, total_onet_boxes);


    //calculate the landmark
    for(unsigned int i = 0; i < total_onet_boxes.size(); i++) {
        face_box& box = total_onet_boxes[i];

        float h = box.x1 - box.x0 + 1;
        float w = box.y1 - box.y0 + 1;

        for(int j = 0; j < 5; j++) {
            box.landmark.x[j] = box.x0 + w * box.landmark.x[j] - 1;
            box.landmark.y[j] = box.y0 + h * box.landmark.y[j] - 1;
        }

    }

    //Get Final Result
    regress_boxes(total_onet_boxes);
    nms_boxes(total_onet_boxes, 0.7, NMS_MIN, face_list);

    //set_box_bound(face_list,img_h,img_w);

    //switch x and y, since working_img is transposed

    for(unsigned int i = 0; i < face_list.size(); i++) {
        face_box& box = face_list[i];

        std::swap(box.x0, box.y0);
        std::swap(box.x1, box.y1);

        for(int l = 0; l < 5; l++) {
            std::swap(box.landmark.x[l], box.landmark.y[l]);
        }
    }
}

void caffe_mtcnn::copy_one_patch(const cv::Mat& img, face_box&input_box, float * data_to, int width, int height)
{
    std::vector<cv::Mat> channels;

    set_input_buffer(channels, data_to, height, width);

    cv::Mat chop_img = img(cv::Range(input_box.py0, input_box.py1),
                           cv::Range(input_box.px0, input_box.px1));

    int pad_top = std::abs(input_box.py0 - input_box.y0);
    int pad_left = std::abs(input_box.px0 - input_box.x0);
    int pad_bottom = std::abs(input_box.py1 - input_box.y1);
    int pad_right = std::abs(input_box.px1 - input_box.x1);

    cv::copyMakeBorder(chop_img, chop_img, pad_top, pad_bottom, pad_left, pad_right,  cv::BORDER_CONSTANT, cv::Scalar(0));

    cv::resize(chop_img, chop_img, cv::Size(width, height), 0, 0);
    cv::split(chop_img, channels);

}

int caffe_mtcnn::run_PNet_Caffe_HRT(const cv::Mat& img, scale_window& win, std::vector<face_box>& box_list)
{
    cv::Mat  resized;
    int scale_h=win.h;
    int scale_w=win.w;
    float scale=win.scale;

    cv::resize(img, resized, cv::Size(scale_w, scale_h), 0, 0);

    Blob<float>* input_blob = PNet_->input_blobs()[0];
    input_blob->Reshape(1, 3, scale_h, scale_w);
    PNet_->Reshape();


    std::vector<cv::Mat> input_channels;
    float * input_data=PNet_->input_blobs()[0]->mutable_cpu_data();
    set_input_buffer(input_channels, input_data, scale_h, scale_w);

    cv::split(resized, input_channels);

    PNet_->Forward();

    Blob<float>* reg = PNet_->output_blobs()[0];
    Blob<float>* confidence = PNet_->output_blobs()[1];


    int feature_h=reg->shape(2);
    int feature_w=reg->shape(3);
    std::vector<face_box> candidate_boxes;

    generate_bounding_box(confidence->cpu_data(),
    		reg->cpu_data(), scale,pnet_threshold_,feature_h,feature_w,candidate_boxes,true);

    nms_boxes(candidate_boxes, 0.5, NMS_UNION,box_list);

    return 0;
}

int caffe_mtcnn::run_PNet_Tengine(const cv::Mat& img, scale_window& win, std::vector<face_box>& box_list)
{
    cv::Mat  resized;
	int scale_h=win.h;
	int scale_w=win.w;
	float scale=win.scale;
	cv::resize(img, resized, cv::Size(scale_w, scale_h), 0, 0);
    /* input */

    tensor_t input_tensor=get_graph_tensor(PNet_graph,"data"); 
    int dims[]={1,3,scale_h,scale_w};
    set_tensor_shape(input_tensor,dims,4);
    int in_mem=sizeof(float)*scale_h*scale_w*3;
    //std::cout<<"mem "<<in_mem<<"\n";
    float* input_data=(float*)malloc(in_mem);
   
    std::vector<cv::Mat> input_channels;
    set_input_buffer(input_channels, input_data, scale_h, scale_w);
    cv::split(resized, input_channels);

    set_tensor_buffer(input_tensor,input_data,in_mem);
    prerun_graph(PNet_graph);
    run_graph(PNet_graph,1);
    free(input_data);
    /* output */
    tensor_t tensor=get_graph_tensor(PNet_graph,"conv4-2");
    get_tensor_shape(tensor,dims,4);
    float *  reg_data=(float *)get_tensor_buffer(tensor);
    int feature_h=dims[2];
	int feature_w=dims[3];
    //std::cout<<"Pnet scale h,w= "<<feature_h<<","<<feature_w<<"\n";

    tensor=get_graph_tensor(PNet_graph,"prob1");
    float *  prob_data=(float *)get_tensor_buffer(tensor);
    std::vector<face_box> candidate_boxes;
    generate_bounding_box(prob_data,
			reg_data, scale,pnet_threshold_,feature_h,feature_w,candidate_boxes,true);


    postrun_graph(PNet_graph); 
	nms_boxes(candidate_boxes, 0.5, NMS_UNION,box_list);
    
    //std::cout<<"condidate boxes size :"<<candidate_boxes.size()<<"\n";
	return 0;
}

int caffe_mtcnn::run_PNet(const cv::Mat& img, scale_window& win, std::vector<face_box>& box_list)
{
    if(alg_type == TENGINE)
        return run_PNet_Tengine(img, win, box_list);
    else if(alg_type == CAFFE_HRT)
        return run_PNet_Caffe_HRT(img, win, box_list);
    else
        return 1;
    
    return 0;
}

void caffe_mtcnn::run_RNet_Caffe_HRT(const cv::Mat& img, std::vector<face_box>& pnet_boxes, std::vector<face_box>& output_boxes)
{
    int batch=pnet_boxes.size();
    int channel = 3;
    int height = 24;
    int width = 24;

    std::vector<int> input_shape={batch,channel,height,width};

    Blob<float> * input_blob=RNet_->input_blobs()[0];

    input_blob->Reshape(input_shape);

    RNet_->Reshape();

    float * input_data=input_blob->mutable_cpu_data();


    for(int i=0;i<batch;i++)
    {
        int img_size=channel*height*width;

        copy_one_patch(img,pnet_boxes[i],input_data,height,width);
        input_data+=img_size;
    }


    RNet_->Forward();

    const Blob<float>* reg = RNet_->output_blobs()[0];
    const Blob<float>* confidence = RNet_->output_blobs()[1];

    const float* confidence_data = confidence->cpu_data();
    const float* reg_data = reg->cpu_data();

    int conf_page_size=confidence->count(1);
    int reg_page_size=reg->count(1);

    for(int i=0;i<batch;i++)
    {

        if (*(confidence_data+1) > rnet_threshold_){

            face_box output_box;
            face_box& input_box=pnet_boxes[i];

            output_box.x0=input_box.x0;
            output_box.y0=input_box.y0;
            output_box.x1=input_box.x1;
            output_box.y1=input_box.y1;

            output_box.score = *(confidence_data+1);

            /*Note: regress's value is swaped here!!!*/

            output_box.regress[0]=reg_data[1];
            output_box.regress[1]=reg_data[0];
            output_box.regress[2]=reg_data[3];
            output_box.regress[3]=reg_data[2];

            output_boxes.push_back(output_box);
        }

        confidence_data+=conf_page_size;
        reg_data+=reg_page_size;
    }

}

void caffe_mtcnn::run_RNet_Tengine(const cv::Mat& img, std::vector<face_box>& pnet_boxes, std::vector<face_box>& output_boxes)
{
    int batch=pnet_boxes.size();
    int channel = 3;
    int height = 24;
    int width = 24;

    tensor_t input_tensor=get_graph_tensor(RNet_graph,"data"); 
    int dims[]={batch,channel,height,width};
    set_tensor_shape(input_tensor,dims,4);
    int img_size=channel*height*width;
    int in_mem=sizeof(float)*batch*img_size;
    float* input_data=(float*)malloc(in_mem);
    float* input_ptr=input_data;
    set_tensor_buffer(input_tensor,input_ptr,in_mem);

    for(int i=0;i<batch;i++)
    {
        copy_one_patch(img,pnet_boxes[i],input_ptr,height,width);
        input_ptr+=img_size;
    }

    prerun_graph(RNet_graph);
    run_graph(RNet_graph,1);
    free(input_data);
    //std::cout<<"run done ------\n";
    //
    /* output */
    tensor_t tensor=get_graph_tensor(RNet_graph,"conv5-2");
    float *  reg_data=(float *)get_tensor_buffer(tensor);

    tensor=get_graph_tensor(RNet_graph,"prob1");
    float *  confidence_data=(float *)get_tensor_buffer(tensor);

    int conf_page_size=2;
    int reg_page_size=4;

    for(int i=0;i<batch;i++)
    {

        if (*(confidence_data+1) > rnet_threshold_){

            face_box output_box;
            face_box& input_box=pnet_boxes[i];

            output_box.x0=input_box.x0;
            output_box.y0=input_box.y0;
            output_box.x1=input_box.x1;
            output_box.y1=input_box.y1;

            output_box.score = *(confidence_data+1);

            /*Note: regress's value is swaped here!!!*/

            output_box.regress[0]=reg_data[1];
            output_box.regress[1]=reg_data[0];
            output_box.regress[2]=reg_data[3];
            output_box.regress[3]=reg_data[2];

            output_boxes.push_back(output_box);
        }

        confidence_data+=conf_page_size;
        reg_data+=reg_page_size;
    }

    postrun_graph(RNet_graph);
}	


void caffe_mtcnn::run_RNet(const cv::Mat& img, std::vector<face_box>& pnet_boxes, std::vector<face_box>& output_boxes)
{
    if(alg_type == TENGINE)
        run_RNet_Tengine(img, pnet_boxes, output_boxes);
    else if(alg_type == CAFFE_HRT)
        run_RNet_Caffe_HRT(img, pnet_boxes, output_boxes);
}


void caffe_mtcnn::run_ONet_Caffe_HRT(const cv::Mat& img, std::vector<face_box>& rnet_boxes, std::vector<face_box>& output_boxes) 
{
    int batch=rnet_boxes.size();
    int channel = 3;
    int height = 48;
    int width = 48;

    std::vector<int> input_shape={batch,channel,height,width};

    Blob<float>* input_blob = ONet_->input_blobs()[0];

    input_blob->Reshape(input_shape);

    ONet_->Reshape();

    float * input_data=input_blob->mutable_cpu_data();

    for(int i=0;i<batch;i++)
    {
        copy_one_patch(img,rnet_boxes[i],input_data,height,width);
        input_data+=channel*height*width;
    }

    ONet_->Forward();

    const Blob<float>* reg = ONet_->output_blobs()[0];
    const Blob<float>* confidence = ONet_->output_blobs()[2];
    const Blob<float>* points_blob = ONet_->output_blobs()[1];

    const float* confidence_data = confidence->cpu_data(); 
    const float* reg_data = reg->cpu_data();
    const float* points_data=points_blob->cpu_data();

    int conf_page_size=confidence->count(1);
    int reg_page_size=reg->count(1);
    int points_page_size=points_blob->count(1);

    for(int i=0;i<batch;i++)
    {
        if (*(confidence_data+1) > onet_threshold_){
            face_box output_box;
            face_box& input_box=rnet_boxes[i];

            output_box.x0=input_box.x0;
            output_box.y0=input_box.y0;
            output_box.x1=input_box.x1;
            output_box.y1=input_box.y1;

            output_box.score=*(confidence_data+1);

            output_box.regress[0]=reg_data[1];
            output_box.regress[1]=reg_data[0];
            output_box.regress[2]=reg_data[3];
            output_box.regress[3]=reg_data[2];

            /*Note: switched x,y points value too..*/

            for (int j = 0; j<5; j++){
                output_box.landmark.x[j] = *(points_data + j+5);
                output_box.landmark.y[j] = *(points_data + j);
            }

            output_boxes.push_back(output_box);

        }

        confidence_data+=conf_page_size;
        reg_data+=reg_page_size;
        points_data+=points_page_size; 
    }
}

void caffe_mtcnn::run_ONet_Tengine(const cv::Mat& img, std::vector<face_box>& rnet_boxes, std::vector<face_box>& output_boxes) 
{
    int batch=rnet_boxes.size();

    int channel = 3;
    int height = 48;
    int width = 48;
    tensor_t input_tensor=get_graph_tensor(ONet_graph,"data"); 
    int dims[]={batch,channel,height,width};
    set_tensor_shape(input_tensor,dims,4);
    int img_size=channel*height*width;
    int in_mem=sizeof(float)*batch*img_size;
    float* input_data=(float*)malloc(in_mem);
    float*  input_ptr=input_data;
    set_tensor_buffer(input_tensor,input_ptr,in_mem);
    for(int i=0;i<batch;i++)
    {
        copy_one_patch(img,rnet_boxes[i],input_ptr,height,width);
        input_ptr+=img_size;
    }

    prerun_graph(ONet_graph);
    run_graph(ONet_graph,1);
    free(input_data);
    /* output */
    tensor_t tensor=get_graph_tensor(ONet_graph,"conv6-3");
    float *  points_data=(float *)get_tensor_buffer(tensor);

    tensor=get_graph_tensor(ONet_graph,"prob1");
    float *  confidence_data=(float *)get_tensor_buffer(tensor);

    tensor=get_graph_tensor(ONet_graph,"conv6-2");
    float *  reg_data=(float *)get_tensor_buffer(tensor);

    int conf_page_size=2;
    int reg_page_size=4;
    int points_page_size=10;
    for(int i=0;i<batch;i++)
    {
        if (*(confidence_data+1) > rnet_threshold_){
            face_box output_box;
            face_box& input_box=rnet_boxes[i];

            output_box.x0=input_box.x0;
            output_box.y0=input_box.y0;
            output_box.x1=input_box.x1;
            output_box.y1=input_box.y1;

            output_box.score=*(confidence_data+1);

            output_box.regress[0]=reg_data[1];
            output_box.regress[1]=reg_data[0];
            output_box.regress[2]=reg_data[3];
            output_box.regress[3]=reg_data[2];

            /*Note: switched x,y points value too..*/

            for (int j = 0; j<5; j++){
                output_box.landmark.x[j] = *(points_data + j+5);
                output_box.landmark.y[j] = *(points_data + j);
            }
            output_boxes.push_back(output_box);

        }

        confidence_data+=conf_page_size;
        reg_data+=reg_page_size;
        points_data+=points_page_size; 
    }
    postrun_graph(ONet_graph); 
}	


void caffe_mtcnn::run_ONet(const cv::Mat& img, std::vector<face_box>& rnet_boxes, std::vector<face_box>& output_boxes)
{
    if(alg_type == TENGINE)
        run_ONet_Tengine(img, rnet_boxes, output_boxes);
    else if(alg_type == CAFFE_HRT)
        run_ONet_Caffe_HRT(img, rnet_boxes, output_boxes);
}


static mtcnn * caffe_creator(void)
{
    return new caffe_mtcnn();
}

//REGISTER_MTCNN_CREATOR(caffe,caffe_creator);


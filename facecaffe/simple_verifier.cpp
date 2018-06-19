#include <malloc.h>
#include <string.h>

#include <cmath>
#include <vector>
#include <opencv2/opencv.hpp>
#include "face_verify.hpp"

cosine_distance_verifier::~cosine_distance_verifier(void)
{
    for(int i=0;i<feature_db_.size();i++)
    {
          free(feature_db_[i].p_feature);
    }
}

/* cosine distance */
float cosine_distance_verifier::compare(float * f0, float * f1, int len)
{

#if 0
std::cout << "len: " << len << std::endl;

std::cout << "f0:            f1:"<< std::endl;
for(int i =0;i <512; i++)
{
	std::cout << f0[i] << " " << f1[i] << std::endl;
}
#endif


	cv::Mat m1(len, 1, CV_32FC1, f0), m2(len, 1, CV_32FC1, f1);

	float similarity = m1.dot(m2) / cv::norm(m1, CV_L2) / cv::norm(m2, CV_L2);
//std::cout << "similarity in compare: " << similarity << std::endl;
	return similarity;

#if 0
	double  product=0;
	double  l2_sum0=0;
	double  l2_sum1=0;

	double score;


	for(int i=0;i<len;i++)
	{
		product+=f0[i]*f1[i];
		l2_sum0+=f0[i]*f0[i];
		l2_sum1+=f1[i]*f1[i];
	}

	score=product/sqrt(l2_sum0)/sqrt(l2_sum1);
std::cout << "score in compare: " << score << std::endl;
	return (float)score;
	#endif

}

int cosine_distance_verifier::search(float * f, int * p_face_id, float * p_score)
{

	p_score[0]=0;

	for(int i=0; i<feature_db_.size(); i++)
	{
        face_pair& e=feature_db_[i];

		float score=compare(f,e.p_feature,feature_len_);      
        //std::cout << "for feature: " << i << " ,score is: " << score << std::endl;
		if(score>p_score[0])
		{
			p_score[0]=score;
			p_face_id[0]=e.face_id;
		}

	}

	return 0;
}


int cosine_distance_verifier::insert_feature(float * feature, unsigned int face_id)
{
      face_pair fp;
    
      //enable override old record of the same face_id
      remove_feature(face_id);

      fp.p_feature=(float *)malloc(sizeof(float)*feature_len_);
      memcpy(fp.p_feature,feature,feature_len_*sizeof(float));
      fp.face_id=face_id;

      feature_db_.push_back(fp);

      return 0;
}


void cosine_distance_verifier::remove_feature(unsigned int face_id)
{
    std::vector<face_pair>::iterator it=feature_db_.begin();

   while(it!=feature_db_.end())
   {
       if(it->face_id==face_id)
       {
           free(it->p_feature);
           feature_db_.erase(it);
           break;
       }  
       it++;
   }
}


static face_verifier * cosine_distance_verifier_creator(const std::string& name)
{
	return  new cosine_distance_verifier();
}

REGISTER_SIMPLE_VERIFIER(cosine_distance,cosine_distance_verifier_creator);


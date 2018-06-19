#ifndef __FACE_VERIFY_HPP__
#define __FACE_VERIFY_HPP__


#include <string>

#include <malloc.h>
#include <string.h>

#include <cmath>
#include <vector>

class  face_verifier
{
public:
   virtual float compare(float * f0, float * f1, int len)=0;

   virtual int search(float * f, int * p_idx, float * p_score)=0;
   virtual int insert_feature(float * feature, unsigned int face_id)=0;
   virtual void set_feature_len(int feature_len)=0;
   virtual void remove_feature(unsigned int face_id)=0;

protected:

   int feature_len_;
   std::string name_;
};

struct face_pair
{
   float * p_feature;
   unsigned int face_id;
};

class cosine_distance_verifier: public face_verifier
{
	public:

		float compare(float * f0, float * f1, int len);

		int search(float * f, int * face_id, float * p_score);


                int insert_feature(float * feature, unsigned int face_id);
                
                void remove_feature(unsigned int face_id);

                void set_feature_len(int feature_len) {feature_len_=feature_len;};

		cosine_distance_verifier(void){feature_len_=256;};

               ~cosine_distance_verifier(void);


	private:
                std::vector<face_pair> feature_db_;
};

typedef face_verifier * (*face_verifier_creator)(const std::string& name);

face_verifier * get_face_verifier(const std::string& name);

int register_face_verifier(const std::string& name, face_verifier_creator verifier);

class  only_for_face_verfier_auto_register
{
	public:
		only_for_face_verfier_auto_register(const std::string& name, face_verifier_creator func)
		{
			register_face_verifier(name,func);
		}

};

#define REGISTER_SIMPLE_VERIFIER(name,func) \
	static only_for_face_verfier_auto_register dummy_simple_verifier_creator_## name(#name, func)

#endif

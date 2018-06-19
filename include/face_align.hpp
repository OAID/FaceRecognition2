#ifndef __FACE_ALIGN_HPP__
#define __FACE_ALIGN_HPP__

int get_aligned_face(const cv::Mat& img, float* landmark, int landmark_number, int desired_width, int desired_height,cv::Mat& out);

#endif

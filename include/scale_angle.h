#ifndef __SCALE_ANGLE_H__
#define __SCALE_ANGLE_H__

extern int cal_scale_and_angle(float * landmark, int landmark_number, int desired_width, int desired_height,float * scale, float * angle);
extern int dsvd(float a[][2], int m, int n, float *w, float v[][2]);

#endif

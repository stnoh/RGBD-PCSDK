/*******************************************************************************
Copyright (c) 2013-2015, Seung-Tak Noh (seungtak.noh [at] gmail.com)
All rights reserved.
*******************************************************************************/
#pragma once

#include <GL/freeglut.h>
#include <opencv2/opencv.hpp>

#include "IntelCamera.h"
#include "ConvertCVGL.h"

typedef void (*FuncPtr)(void);

namespace rgbd_viewer{

// set CV intrinsic & extrinsic parameters
void setIntrinsicParams(int cam_id, float* intrinsic_params);
void setStereoRT(float* stereo_rot, float* stereo_tr);

// get GL projection & modelview matrices
void getProjectionMatrix(int id, float* p_mat, float z_near, float z_far);
void getStereoViewMatrix(float* m_mat);

// draw
void drawBG(IplImage*, GLenum format);
void draw3D(int win_id, FuncPtr func);
}

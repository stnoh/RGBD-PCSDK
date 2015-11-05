/*******************************************************************************
Copyright (c) 2013-2015, Seung-Tak Noh (seungtak.noh [at] gmail.com)
All rights reserved.
*******************************************************************************/
#pragma once

#ifndef CVGL
#define CVGL

namespace cvgl{

void getGLProjectionMatrix(float* out_mat_GL,
	float fx, float fy, float cx, float cy, int xres, int yres, float z_near, float z_far);

void getGLModelviewMatrix(float* out_mat_GL, float* r_mat_CV, float* t_vec_CV);

}

#endif

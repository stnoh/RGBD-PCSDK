/*******************************************************************************
Copyright (c) 2013-2015, Seung-Tak Noh (seungtak.noh [at] gmail.com)
All rights reserved.
*******************************************************************************/
#include "ConvertCVGL.h"

namespace cvgl{

void getGLProjectionMatrix(float* out_mat_GL,
	float fx, float fy, float cx, float cy,
	int xres, int yres, float z_near, float z_far)
{
	float left   = - z_near / fx * cx;
	float right  =   z_near / fx *(xres - cx);
	float bottom = - z_near / fy *(yres - cy);
	float top    =   z_near / fy * cy;

	// as OpenGL expression
	out_mat_GL[0]  = 2*z_near/(right-left);
	out_mat_GL[4]  = 0.0f;
	out_mat_GL[8]  = (right+left)/(right-left);
	out_mat_GL[12] = 0.0f;

	out_mat_GL[1]  = 0.0f;
	out_mat_GL[5]  = 2*z_near/(top-bottom);
	out_mat_GL[9]  = (top+bottom)/(top-bottom);
	out_mat_GL[13] = 0.0f;
	
	out_mat_GL[2]  = 0.0f;
	out_mat_GL[6]  = 0.0f;
	out_mat_GL[10] = -(z_far+z_near)/(z_far-z_near);
	out_mat_GL[14] = -2.0f*(z_far*z_near)/(z_far-z_near);

	out_mat_GL[3]  = 0.0f;
	out_mat_GL[7]  = 0.0f;
	out_mat_GL[11] =-1.0f;
	out_mat_GL[15] = 0.0f;
}

void getGLModelviewMatrix(float* out_mat_GL,
	float* r_mat_CV, float* t_vec_CV)
{
	// convert rotation matrix by Rodriguess manner
	out_mat_GL[0]  =  r_mat_CV[0];
	out_mat_GL[4]  = -r_mat_CV[1]; // (CV <-> GL)
	out_mat_GL[8]  = -r_mat_CV[2]; // (CV <-> GL)

	out_mat_GL[1]  = -r_mat_CV[3]; // (CV <-> GL)
	out_mat_GL[5]  =  r_mat_CV[4];
	out_mat_GL[9]  =  r_mat_CV[5];

	out_mat_GL[2]  = -r_mat_CV[6]; // (CV <-> GL)
	out_mat_GL[6]  =  r_mat_CV[7];
	out_mat_GL[10] =  r_mat_CV[8];

	// translation vector
	out_mat_GL[12] =  t_vec_CV[0];
	out_mat_GL[13] = -t_vec_CV[1]; // (CV <-> GL)
	out_mat_GL[14] = -t_vec_CV[2]; // (CV <-> GL)

	// keep the affine matrix
	out_mat_GL[3] = out_mat_GL[7] = out_mat_GL[11] = 0.0f;
	out_mat_GL[15] = 1.0f;
}

}

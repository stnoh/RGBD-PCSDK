/*******************************************************************************
Copyright (c) 2013-2015, Seung-Tak Noh (seungtak.noh [at] gmail.com)
All rights reserved.
*******************************************************************************/
#include "RGBD_viewer.h"

namespace rgbd_viewer{;

////////////////////////////////////////////////////////////////////////////////
// for renderer
////////////////////////////////////////////////////////////////////////////////
const int dimx[2] = {DEPTH_WIDTH , COLOR_WIDTH };
const int dimy[2] = {DEPTH_HEIGHT, COLOR_HEIGHT};

float fx[2], fy[2], cx[2], cy[2];
float stereo_rot[9], stereo_tr[3];


////////////////////////////////////////////////////////////////////////////////
// set OpenCV intrinsic & extrinsic parameters
////////////////////////////////////////////////////////////////////////////////
void setIntrinsicParams(int cam_id, float* intrinsic_params)
{
	fx[cam_id] = intrinsic_params[0]; fy[cam_id] = intrinsic_params[1];
	cx[cam_id] = intrinsic_params[2]; cy[cam_id] = intrinsic_params[3];
}

void setStereoRT(float* _stereo_rot, float* _stereo_tr)
{
	for(int i=0;i<9;i++) stereo_rot[i] = _stereo_rot[i];
	for(int i=0;i<3;i++) stereo_tr [i] = _stereo_tr [i];
}

////////////////////////////////////////////////////////////////////////////////
// get OpenGL projection & modelview matrices
////////////////////////////////////////////////////////////////////////////////
void getProjectionMatrix(int id, float* p_mat, float z_near, float z_far)
{
	cvgl::getGLProjectionMatrix(p_mat, fx[id], fy[id], cx[id], cy[id], dimx[id], dimy[id], z_near, z_far);
}

void getStereoViewMatrix(float* m_mat)
{
	cvgl::getGLModelviewMatrix(m_mat, stereo_rot, stereo_tr);
}

void drawBG(IplImage* img, GLenum format)
{
	// set projection matrix (orthogonal camera)
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

	// set modelview matrix
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();

	// get zoom magnification
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	float zoom_w = viewport[2]/(float)img->width ;
	float zoom_h = viewport[3]/(float)img->height;

	// draw background (depth image)
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPixelZoom(zoom_w, -zoom_h); glRasterPos2f(-1, 1);
	glDrawPixels(img->width, img->height, format, GL_UNSIGNED_BYTE, img->imageData);
	glPopAttrib();
}

float light_position[4] = {0,0,1000,1}; // light position

void draw3D(int win_id, FuncPtr func)
{
	// get GL projection matrices
	float proj_mat[16];
	getProjectionMatrix(win_id, proj_mat, 0.03f, 10000.0f);

	// set projection matrix
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMultMatrixf(proj_mat);

	// enable 3D drawing attributes
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_BLEND);	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_NORMALIZE);	glEnable(GL_COLOR_MATERIAL); glEnable(GL_CULL_FACE);

	// enable lighting for shading
	glEnable(GL_LIGHT0); glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glEnable(GL_DEPTH_TEST); glEnable(GL_LIGHTING);

	// draw function from pointer
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	if(win_id!=0){
		float stereo_mat[16];
		getStereoViewMatrix(stereo_mat);
		glMultMatrixf(stereo_mat);
	}
	func();

	// disable lighting
	glDisable(GL_DEPTH_TEST); glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);

	// disable 3D drawing attributes
	glPopAttrib(); 
}

}

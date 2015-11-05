/*******************************************************************************
Copyright (c) 2013-2015, Seung-Tak Noh (seungtak.noh [at] gmail.com)
All rights reserved.
*******************************************************************************/
#pragma once

#ifndef _RGBD_CAMERA_
#define _RGBD_CAMERA_

#include "common.h"
#include <opencv2/opencv.hpp>

// constants
static const int COLOR_WIDTH  = 640;
static const int COLOR_HEIGHT = 480;
static const int DEPTH_WIDTH  = 320;
static const int DEPTH_HEIGHT = 240;

class RGBDCamera
{
public:
	RGBDCamera(){};
	virtual ~RGBDCamera(){};

	// 
	IplImage* getColorImage(){ return color_image; };
	IplImage* getDepthImage(){ return depth_image; };
	IplImage* getConfiImage(){ return confi_image; };

protected:
	IplImage* color_image;
	IplImage* depth_image;
	IplImage* confi_image;
};

#endif

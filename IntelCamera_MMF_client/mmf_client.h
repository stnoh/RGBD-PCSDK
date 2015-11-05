/*******************************************************************************
Copyright (c) 2013-2015, Seung-Tak Noh (seungtak.noh [at] gmail.com)
All rights reserved.
*******************************************************************************/
#include "stdAfx.h"
#include <opencv2/opencv.hpp>

////////////////////////////////////////////////////////////////////////////////
// export Intel Camera "client" part
////////////////////////////////////////////////////////////////////////////////
#include "IntelCamera.h"
IntelCameraClient *client;

extern "C"{

__declspec(dllexport) void initIntelCameraClient(int device_num);
__declspec(dllexport) void endIntelCameraClient();

__declspec(dllexport) void copyIntelCameraPointCloud(float* arrayPtr, int w, int h, int i_offset, int j_offset);

__declspec(dllexport) void getIntelCameraColorImage(int *w, int *h);
__declspec(dllexport) void copyIntelCameraColorImage(char* arrayPtr);

__declspec(dllexport) void getIntelCameraInfraImage(int *w, int *h);
__declspec(dllexport) void copyIntelCameraInfraImage(char* arrayPtr);

}

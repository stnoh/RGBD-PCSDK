/*******************************************************************************
Copyright (c) 2013-2015, Seung-Tak Noh (seungtak.noh [at] gmail.com)
All rights reserved.
*******************************************************************************/
#include "stdAfx.h"
#include <opencv2/opencv.hpp>

////////////////////////////////////////////////////////////////////////////////
// export Intel Camera "server" part
////////////////////////////////////////////////////////////////////////////////
#include "IntelCamera.h"
IntelCameraServer *server;
bool _update_thread = false;

extern "C"{

__declspec(dllexport) int  initIntelCameraServer(int device_num);
__declspec(dllexport) void startIntelCameraUpdateThread();
__declspec(dllexport) void stopIntelCameraUpdateThread();
__declspec(dllexport) void endIntelCameraServer();

}

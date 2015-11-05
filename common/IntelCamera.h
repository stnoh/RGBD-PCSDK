/*******************************************************************************
Copyright (c) 2013-2015, Seung-Tak Noh (seungtak.noh [at] gmail.com)
All rights reserved.
*******************************************************************************/
#pragma once

#ifndef _INTEL_CAMERA_
#define _INTEL_CAMERA_

#include "RGBDCamera.h"

// WINAPI for sharing memory by MMF
#include <Windows.h>

// Perceptual Computing SDK
#include <pxcsmartptr.h>
#include <pxcsession.h>
#include <pxccapture.h>
#include <pxcprojection.h>
#include <pxcmetadata.h>

// data on shared memory
static const int NUM_OF_MMF = 4;
static const char* DATA_NAME[NUM_OF_MMF] = {"color", "depth", "infra", "cloud"};


////////////////////////////////////////////////////////////////////////////////
// Read-Only Class for Intel Camera
////////////////////////////////////////////////////////////////////////////////
class IntelCameraClient : public RGBDCamera
{
public:
	IntelCameraClient(int device_num);
	virtual ~IntelCameraClient();

	PXCPoint3DF32* pos3d;

private:
	int InitReadMMF(int device_num);
	int EndReadMMF();

	HANDLE hMemoryMap[NUM_OF_MMF];
	LPBYTE pMemoryMap[NUM_OF_MMF];
};


////////////////////////////////////////////////////////////////////////////////
// Write-Only Class for Intel Camera
////////////////////////////////////////////////////////////////////////////////
class IntelCameraServer : public RGBDCamera
{
public:
	IntelCameraServer(int device_num);
	virtual ~IntelCameraServer();

	int InitDevice(int device_num);
	int NextFrame(void);

private:
	// related to MMF
	int InitWriteMMF(int device_num);
	int EndWriteMMF();

	// color, depth, point cloud
	int ProcessColor(void);
	int ProcessDepth(void);
	int MapColorToDepth(void);

	// 
	PXCPoint3DF32* pos3d;
	PXCPointF32*   posc;

	// projection mapping module
	PXCSmartPtr<PXCProjection> projection;

	// for using Camera by Perceptual Computing SDK
	PXCSmartPtr<PXCSession>              session;
	PXCSmartPtr<PXCCapture::Device>	     device;
	PXCSmartPtr<PXCCapture::VideoStream> pColorStream;
	PXCSmartPtr<PXCCapture::VideoStream> pDepthStream;

	// for synchronization & get image data
	PXCSmartSPArray sps;
	PXCSmartArray<PXCImage> images;

private:
	// for using Camera by Perceptual Computing SDK
	PXCSmartPtr<PXCCapture>              capture;

	// internal 
	PXCPoint3DF32* pos2d;
	pxcF32 no_confidence;
	pxcF32 depth_saturation;

	HANDLE hMemoryMap[NUM_OF_MMF];
	LPBYTE pMemoryMap[NUM_OF_MMF];
	DWORD  memorySize[NUM_OF_MMF];

	bool _camera_standby;
	bool _update_thread;
};


////////////////////////////////////////////////////////////////////////////////
// Single thread version for Intel Camera
////////////////////////////////////////////////////////////////////////////////
class IntelCamera : public RGBDCamera
{
public:
	IntelCamera(int device_num);
	virtual ~IntelCamera();

	int NextFrame(void);

	PXCPoint3DF32* pos3d;
	PXCPointF32*   posc;

private:
	// functions for internal process
	int InitCamera(int device_num);

	// color, depth, point cloud
	int ProcessColor(void);
	int ProcessDepth(void);
	int MapColorToDepth(void);

	// projection mapping module
	PXCSmartPtr<PXCProjection> projection;

	// for using Camera by Perceptual Computing SDK
	PXCSmartPtr<PXCSession>              session;
	PXCSmartPtr<PXCCapture::Device>	     device;
	PXCSmartPtr<PXCCapture::VideoStream> pColorStream;
	PXCSmartPtr<PXCCapture::VideoStream> pDepthStream;

	// for synchronization & get image data
	PXCSmartSPArray sps;
	PXCSmartArray<PXCImage> images;

private:
	// for using Camera by Perceptual Computing SDK
	PXCSmartPtr<PXCCapture>              capture;

	// internal 
	PXCPoint3DF32* pos2d;
	pxcF32 no_confidence;
	pxcF32 depth_saturation;
};


#endif

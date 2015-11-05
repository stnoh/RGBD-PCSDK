/*******************************************************************************
This is the C API wrapper implementation of Intel Hand Skeletal Library (HSKL).
Some of the code is originated from "HandTrackViewer.cpp" in HSKL.
https://software.intel.com/protected-download/393460/393178

Modified by Seung-Tak Noh (seungtak.noh [at] gmail.com), 2013-2015
*******************************************************************************/
#pragma once

#ifndef _HSKL_UNITY_INTERFACE
#define _HSKL_UNITY_INTERFACE

#include "stdAfx.h"
#include "IntelCamera.h"

#include "hsklu_sNoh.h"


////////////////////////////////////////////////////////////////////////////////
// C API for DLL module
////////////////////////////////////////////////////////////////////////////////
extern "C"{
// HSKL properties
__declspec(dllexport) void  initHSKLTracking(int cam_id, float fx, float fy);
__declspec(dllexport) void  endHSKLTracking();

__declspec(dllexport) float runHSKLTracking();
__declspec(dllexport) void  startHSKLTracking_thread();
__declspec(dllexport) void  stopHSKLTracking_thread();

__declspec(dllexport) void  setHSKLModelType(int type);
__declspec(dllexport) void  setHSKLMeasurements(float hand_width, float hand_height);

__declspec(dllexport) float getHSKLTrackingError();
__declspec(dllexport) void  getHSKLBoneMesh (int id, int* bone_tris, float* bone_verts);
__declspec(dllexport) void  getHSKLBonePoses(float* bone_poses);
}


////////////////////////////////////////////////////////////////////////////////
// for GLUT rendering
////////////////////////////////////////////////////////////////////////////////
void InitBGImage();
void UpdateBGImage();
void EndBGImage();

hskl::float4x4 PoseToMatrix(hskl::float4 q, hskl::float3 tr);
IplImage* GetColorImageRGB();
IplImage* GetDepthImage8U ();

#endif

/*******************************************************************************
This is the C API wrapper definition of Intel Hand Skeletal Library (HSKL).
Some of the code is originated from "HandTrackViewer.cpp" in HSKL.
https://software.intel.com/protected-download/393460/393178

Modified by Seung-Tak Noh (seungtak.noh [at] gmail.com), 2013-2015
*******************************************************************************/
#include "HSKL_MMF.h"
#include <process.h>


////////////////////////////////////////////////////////////////////////////////
// class hskl::Tracker
////////////////////////////////////////////////////////////////////////////////
IntelCameraClient *client;
hskl::Tracker *hsklTracker;

bool _update_thread_hskl = false;
float _error = 1.0f;


////////////////////////////////////////////////////////////////////////////////
// initializer, setter, finalizer
////////////////////////////////////////////////////////////////////////////////
void  initHSKLTracking(int cam_id, float fx, float fy)
{
	client = new IntelCameraClient(cam_id);
	hsklTracker = new hskl::Tracker();
	hsklTracker->Init(fx, fy, DEPTH_WIDTH, DEPTH_HEIGHT);
}

void  setHSKLModelType(int type)
{
	switch(type)
	{
	case 1:
		hsklTracker->SetModelType(HSKL_MODEL_LEFT_HAND);
		break;
	case 2:
		hsklTracker->SetModelType(HSKL_MODEL_TWO_HAND);
		break;
	default:
		hsklTracker->SetModelType(HSKL_MODEL_RIGHT_HAND);
		break;
	}
}

void  setHSKLMeasurements(float hand_width, float hand_height)
{
	hsklTracker->SetHandMeasurements(hand_width, hand_height);
}

void  endHSKLTracking()
{
	if(_update_thread_hskl){
		stopHSKLTracking_thread();
	}

	delete client;
	delete hsklTracker;
}


////////////////////////////////////////////////////////////////////////////////
// main routine
////////////////////////////////////////////////////////////////////////////////
float runHSKLTracking()
{
	// for drawing depth image : copy & change 16U->8U
	unsigned short *depth = (unsigned short*)client->getDepthImage()->imageData;
	unsigned short* confi = (unsigned short*)client->getConfiImage()->imageData;

	// update tracker
	hsklTracker->Update(depth, confi);

	return hsklTracker->GetTrackingError(0);
}

void startHSKLTracking_thread()
{
	// mimic the lambda in VS2010 [DEPRECATED]
	struct lambda{
		static unsigned int WINAPI UpdateByThread(void* args){
			IntelCameraClient* client = (IntelCameraClient*)args;

			while(_update_thread_hskl){
				_error = runHSKLTracking();
			}

			return 0;
		}
	};

	_update_thread_hskl = true;
	DWORD dwThreadID;
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, lambda::UpdateByThread, client, 0, (unsigned*)&dwThreadID);
}

void stopHSKLTracking_thread()
{
	_update_thread_hskl = false;
	Sleep(200);
}


////////////////////////////////////////////////////////////////////////////////
// getters
////////////////////////////////////////////////////////////////////////////////
const float scale = 1.0f; // scaling factor: [m]

float getHSKLTrackingError()
{
	return _error;
}

void getHSKLBoneMesh(int id, int* hand_tris, float* hand_verts)
{
	auto tris  = hsklTracker->GetTriangles(id);
	auto verts = hsklTracker->GetVertices(id);
			
	// 76 triangles per bone
	for(int j=0;j<76;j++){
		hskl_int3 idx = tris[j];
		hand_tris[3*j+0] = idx.x;
		hand_tris[3*j+1] = idx.y;
		hand_tris[3*j+2] = idx.z;
	}

	// 40 vertices per bone
	for(int j=0;j<40;j++){
		hskl::float3 v = verts[j];
		hand_verts[3*j+0] = scale*v.x;
		hand_verts[3*j+1] = scale*v.y;
		hand_verts[3*j+2] = scale*v.z;
	}
}

void getHSKLBonePoses(float* poses)
{
	// 17 bones per hand
	for(int id=0;id<34;id++)
	{
		// get RT as quarternion & vector3
		hskl::float4 rot = hsklTracker->GetOrientation(id);
		hskl::float3 tr  = hsklTracker->GetPosition(id); 

		// quaternion rotation
		poses[7*id+0] = rot.x;
		poses[7*id+1] = rot.y;
		poses[7*id+2] = rot.z;
		poses[7*id+3] = rot.w;

		// translation vector
		poses[7*id+4] = scale*tr.x;
		poses[7*id+5] = scale*tr.y;
		poses[7*id+6] = scale*tr.z;
	}
}


////////////////////////////////////////////////////////////////////////////////
// for each HSKL bone in GLUT
////////////////////////////////////////////////////////////////////////////////
hskl::float4x4 PoseToMatrix(hskl::float4 q, hskl::float3 tr)
{
	hskl::float4x4 m;
	(hskl::float3&)m.x = qxdir(q);
	(hskl::float3&)m.y = qydir(q);
	(hskl::float3&)m.z = qzdir(q);
	(hskl::float3&)m.w = tr; 
	m.w.w = 1;
	return m;
}


////////////////////////////////////////////////////////////////////////////////
// for background image in GLUT
////////////////////////////////////////////////////////////////////////////////
IplImage *color_image_rgb;
IplImage *depth_image_8u ;

IplImage* GetColorImageRGB(){ return color_image_rgb; }
IplImage* GetDepthImage8U (){ return depth_image_8u ; }

void InitBGImage()
{
	// create IplImage for visualization
	color_image_rgb = cvCreateImage( cvSize(COLOR_WIDTH, COLOR_HEIGHT), IPL_DEPTH_8U, 3);
	depth_image_8u  = cvCreateImage( cvSize(DEPTH_WIDTH, DEPTH_HEIGHT), IPL_DEPTH_8U, 1);
}

void UpdateBGImage()
{
	// for drawing color image : copy & change BGR->RGB
	memcpy(color_image_rgb->imageData, client->getColorImage()->imageData, sizeof(uchar )*COLOR_WIDTH*COLOR_HEIGHT*3);
	cvCvtColor(color_image_rgb, color_image_rgb, CV_BGR2RGB);

	// for drawing depth image : copy & change 16U->8U
	unsigned short *depth = (unsigned short*)(client->getDepthImage()->imageData);
	unsigned char *depth_8u = (unsigned char*)(depth_image_8u->imageData);
	for(int i=0;i<DEPTH_WIDTH*DEPTH_HEIGHT;i++){
		uchar gray = (uchar)0;
		if( depth[i]>=(unsigned short)(1500) ) gray = 0;
		else{
			gray = ~(uchar)(0.17f*depth[i]); // 1500*0.17 = 255
		}
		depth_8u[i] = gray;
	}
}

void EndBGImage()
{
	// release IplImage
	cvReleaseImage(&color_image_rgb);
	cvReleaseImage(&depth_image_8u );
}

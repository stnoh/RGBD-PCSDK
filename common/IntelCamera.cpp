/*******************************************************************************
Copyright (c) 2013-2015, Seung-Tak Noh (seungtak.noh [at] gmail.com)
All rights reserved.
*******************************************************************************/
#include "IntelCamera.h"

////////////////////////////////////////////////////////////////////////////////
// constructor & desctructor
////////////////////////////////////////////////////////////////////////////////
IntelCamera::IntelCamera(int device_num)
: sps(2), images(2)
{
	printf("start Soft Kinetic Camera... device #%d ",device_num);

	// create color image buffer
	color_image = cvCreateImage(cvSize(COLOR_WIDTH,COLOR_HEIGHT), IPL_DEPTH_8U , 3);
	depth_image = cvCreateImage(cvSize(DEPTH_WIDTH,DEPTH_HEIGHT), IPL_DEPTH_16U, 1);
	confi_image = cvCreateImage(cvSize(DEPTH_WIDTH,DEPTH_HEIGHT), IPL_DEPTH_16U, 1);

	// create buffer for projection & fill constant 2d coordinate at first
	pos2d = new PXCPoint3DF32[DEPTH_WIDTH*DEPTH_HEIGHT];
	for(int j=0;j<DEPTH_HEIGHT;j++)
	for(int i=0;i<DEPTH_WIDTH ;i++){
		pos2d[i+j*DEPTH_WIDTH].x = (pxcF32)i;
		pos2d[i+j*DEPTH_WIDTH].y = (pxcF32)j;
	}
	posc  = new PXCPointF32    [DEPTH_WIDTH*DEPTH_HEIGHT];
	pos3d = new PXCPoint3DF32  [DEPTH_WIDTH*DEPTH_HEIGHT];

	// initialize camera
	if(InitCamera(device_num)==-1){
		fprintf(stderr,"Failed to initialize Intel Camera device.\n");
		exit(1);
	}

	printf("working now!\n");
}
IntelCamera::~IntelCamera(void)
{
	// release image buffers
	cvReleaseImage(&color_image);
	cvReleaseImage(&depth_image);
	cvReleaseImage(&confi_image);

	// delete buffer
	delete posc;
	delete pos2d;
	delete pos3d;
}


////////////////////////////////////////////////////////////////////////////////
// public member functions
////////////////////////////////////////////////////////////////////////////////
int IntelCamera::NextFrame()
{
	// color & depth image : force to synchronize
	if ( ProcessColor()==-1 ) return -1;
	if ( ProcessDepth()==-1 ) return -1;

	// point cloud
	if ( MapColorToDepth()==-1 ) return -1;

	return 1;
}


////////////////////////////////////////////////////////////////////////////////
// protected member functions
////////////////////////////////////////////////////////////////////////////////
int IntelCamera::InitCamera(int device_num)
{
	// create a session
	pxcStatus sts = PXCSession_Create(&session);
	if (sts<PXC_STATUS_NO_ERROR) {
		fprintf(stderr,"Failed to create a session\n");
		return -1;
	}

	// create a video capture
	PXCSession::ImplDesc desc;
	memset(&desc, 0, sizeof(desc));
	desc.subgroup = PXCSession::IMPL_SUBGROUP_VIDEO_CAPTURE;

	PXCSmartPtr<PXCCapture> capture;
	sts = session->CreateImpl(&desc, PXCCapture::CUID, (void**)&capture);
	if (sts<PXC_STATUS_NO_ERROR) {
		fprintf(stderr,"Failed to create a capture\n");
		return -1;
	}

	// create a device
	PXCSmartPtr<PXCCapture::Device> device;
	sts = capture->CreateDevice(device_num, &device);
	if (sts<PXC_STATUS_NO_ERROR) {
		fprintf(stderr,"Failed to create a device\n");
		return -1;
	}

	// create color stream: 640x480
	sts = device->CreateStream(0, PXCCapture::VideoStream::CUID, (void**)&pColorStream);
	if(sts<PXC_STATUS_NO_ERROR){
		fprintf(stderr,"failed to create color stream\n");
		return -1;
	}
	PXCCapture::VideoStream::ProfileInfo pinfoColor;
	sts = pColorStream->QueryProfile(0, &pinfoColor);
	if(sts<PXC_STATUS_NO_ERROR){
		fprintf(stderr,"failed to get color stream profile\n");
		return -1;
	}

	// set for less error on depth image : because of sync color/depth
	pinfoColor.frameRateMax.denominator =  0;
	pinfoColor.frameRateMax.numerator   = 60;
	pinfoColor.frameRateMin.denominator =  0;
	pinfoColor.frameRateMin.numerator   = 60;

	sts = pColorStream->SetProfile(&pinfoColor);
	if(sts<PXC_STATUS_NO_ERROR){
		fprintf(stderr,"failed to get color stream profile\n");
		return -1;
	}

	// create depth streams: 320x240
	sts = device->CreateStream(1, PXCCapture::VideoStream::CUID, (void**)&pDepthStream);
	if(sts<PXC_STATUS_NO_ERROR){
		fprintf(stderr,"failed to create depth stream\n");
		return -1;
	}
	PXCCapture::VideoStream::ProfileInfo pinfoDepth;
	sts = pDepthStream->QueryProfile(0, &pinfoDepth);
	if(sts<PXC_STATUS_NO_ERROR){
		fprintf(stderr,"failed to get depth stream profile\n");
		return -1;
	}

	// set for less error on depth image
	pinfoDepth.frameRateMax.denominator =  1;
	pinfoDepth.frameRateMax.numerator   = 60;
	pinfoDepth.frameRateMin.denominator =  1;
	pinfoDepth.frameRateMin.numerator   = 60;

	sts = pDepthStream->SetProfile(&pinfoDepth);
	if(sts<PXC_STATUS_NO_ERROR){
		fprintf(stderr,"failed to get depth stream profile\n");
		return -1;
	}

	// read streams
	pColorStream->ReadStreamAsync(&images[0], &sps[0]);
	pDepthStream->ReadStreamAsync(&images[1], &sps[1]);

	// get constant values
	pxcUID prj_value;
	device->SetProperty(PXCCapture::Device::PROPERTY_DEPTH_SMOOTHING, 1); // depth smoothing
	device->QueryPropertyAsUID(PXCCapture::Device::PROPERTY_PROJECTION_SERIALIZABLE, &prj_value);
	device->QueryProperty(PXCCapture::Device::PROPERTY_DEPTH_LOW_CONFIDENCE_VALUE, &no_confidence);
	device->QueryProperty(PXCCapture::Device::PROPERTY_DEPTH_SATURATION_VALUE    , &depth_saturation);

	// get projection instance
	sts = session->DynamicCast<PXCMetadata>()->CreateSerializable<PXCProjection>(prj_value, &projection);
	if (sts<PXC_STATUS_NO_ERROR) {
		fprintf(stderr,"Failed to create a projection\n");
		return -1;
	}

	return 1;
}


////////////////////////////////////////////////////////////////////////////////
// 
////////////////////////////////////////////////////////////////////////////////
int IntelCamera::ProcessColor()
{
	// get the frame
	pxcStatus sts = sps[0]->Synchronize(0);
	if (sts<PXC_STATUS_NO_ERROR) return -1;
	sps.ReleaseRef(0);

	// copy color image
	PXCImage::ImageData colorData;
	images[0]->AcquireAccess(PXCImage::ACCESS_READ, &colorData);
	memcpy_s(color_image->imageData, sizeof(unsigned char)*COLOR_WIDTH*COLOR_HEIGHT*3, colorData.planes[0], sizeof(unsigned char )*COLOR_WIDTH*COLOR_HEIGHT*3);
	images[0]->ReleaseAccess(&colorData);
	
	// re-read
	sts = pColorStream->ReadStreamAsync(images.ReleaseRef(0), &sps[0]);
	if(sts<PXC_STATUS_NO_ERROR) return -1;

	return 1;
}

int IntelCamera::ProcessDepth()
{
	// get the frame
	pxcStatus sts = sps[1]->Synchronize(0);
	if (sts<PXC_STATUS_NO_ERROR) return -1;
	sps.ReleaseRef(1);

	// copy depth image
	PXCImage::ImageData depthData;
	images[1]->AcquireAccess(PXCImage::ACCESS_READ, &depthData);

	// capturing both "depth image" and "confidence map"
	memcpy_s(depth_image->imageData, sizeof(unsigned short)*DEPTH_WIDTH*DEPTH_HEIGHT*1, depthData.planes[0], sizeof(unsigned short)*DEPTH_WIDTH*DEPTH_HEIGHT*1);
	memcpy_s(confi_image->imageData, sizeof(unsigned short)*DEPTH_WIDTH*DEPTH_HEIGHT*1, depthData.planes[1], sizeof(unsigned short)*DEPTH_WIDTH*DEPTH_HEIGHT*1);
	for(int j=0;j<DEPTH_HEIGHT;j++)
	for(int i=0;i<DEPTH_WIDTH ;i++){
		pos2d[i+j*DEPTH_WIDTH].z = (unsigned short)(((unsigned short*)depth_image->imageData)[i+j*320]);
	}
	images[1]->ReleaseAccess(&depthData);

	// re-read
	sts = pDepthStream->ReadStreamAsync(images.ReleaseRef(1), &sps[1]);
	if(sts<PXC_STATUS_NO_ERROR) return -1;

	return 1;
}

int IntelCamera::MapColorToDepth()
{
	// project image to point cloud
	projection->ProjectImageToRealWorld(DEPTH_WIDTH*DEPTH_HEIGHT, pos2d, pos3d);
	projection->MapDepthToColorCoordinates(DEPTH_WIDTH*DEPTH_HEIGHT, pos2d, posc);

	return 1;
}

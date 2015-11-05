/*******************************************************************************
Copyright (c) 2013-2015, Seung-Tak Noh (seungtak.noh [at] gmail.com)
All rights reserved.
*******************************************************************************/
#include "IntelCamera.h"

IntelCameraServer::IntelCameraServer(int device_num)
: sps(2), images(2), _camera_standby(false)
{
	printf("Allocate MMF for camera server, id #%d ... ",device_num);

	// initialize MMF
	if(	InitWriteMMF(device_num)==0 ){
		fprintf(stderr,"Failed to initialize Memory Map File.\n");
		EndWriteMMF();
		exit(1);
	}

	// create image header for shared memory
	color_image = cvCreateImageHeader(cvSize(COLOR_WIDTH, COLOR_HEIGHT), IPL_DEPTH_8U, 3);
	depth_image = cvCreateImageHeader(cvSize(DEPTH_WIDTH, DEPTH_HEIGHT), IPL_DEPTH_16U, 1);
	confi_image = cvCreateImageHeader(cvSize(DEPTH_WIDTH, DEPTH_HEIGHT), IPL_DEPTH_16U, 1);

	// create buffer for projection & fill constant 2d coordinate at first
	posc = new PXCPointF32[DEPTH_WIDTH*DEPTH_HEIGHT];
	pos2d = new PXCPoint3DF32[DEPTH_WIDTH*DEPTH_HEIGHT];
	for (int j = 0; j<DEPTH_HEIGHT; j++)
	for (int i = 0; i<DEPTH_WIDTH; i++){
		pos2d[i + j*DEPTH_WIDTH].x = (pxcF32)i;
		pos2d[i + j*DEPTH_WIDTH].y = (pxcF32)j;
	}

	// set the memory address for data storage
	color_image->imageData = (char*)pMemoryMap[0];
	depth_image->imageData = (char*)pMemoryMap[1];
	confi_image->imageData = (char*)pMemoryMap[2];
	pos3d = (PXCPoint3DF32*)pMemoryMap[3];

	printf("allocated!\n");
}

IntelCameraServer::~IntelCameraServer()
{
	// finalize MMF
	EndWriteMMF();

	// release image header
	cvReleaseImageHeader(&color_image);
	cvReleaseImageHeader(&depth_image);
	cvReleaseImageHeader(&confi_image);

	// delete buffer
	delete posc;
	delete pos2d;
}


////////////////////////////////////////////////////////////////////////////////
// public member functions
////////////////////////////////////////////////////////////////////////////////
int IntelCameraServer::NextFrame()
{
	if (!_camera_standby) return 0;

	// color & depth image : force to synchronize
	if ( ProcessColor()==0 ) return 0;
	if ( ProcessDepth()==0 ) return 0;

	// point cloud
	if ( MapColorToDepth()==0 ) return 0;

	return 1;
}


////////////////////////////////////////////////////////////////////////////////
// private member functions on Memory-Mapped File
////////////////////////////////////////////////////////////////////////////////
int IntelCameraServer::InitWriteMMF(int device_num)
{
	// zero clear
	for(int id=0;id<NUM_OF_MMF;id++){
		hMemoryMap[id] = NULL;
		pMemoryMap[id] = NULL;
	}

	// set the shared memory name
	char memoryName[NUM_OF_MMF][1024];
	for(int id=0;id<NUM_OF_MMF;id++) sprintf(memoryName[id], "icam%d_%s", device_num, DATA_NAME[id] );

	// calculate memory size
	memorySize[0] = sizeof(uchar )*COLOR_WIDTH*COLOR_HEIGHT*3;
	memorySize[1] = sizeof(ushort)*DEPTH_WIDTH*DEPTH_HEIGHT*1;
	memorySize[2] = sizeof(ushort)*DEPTH_WIDTH*DEPTH_HEIGHT*1;
	memorySize[3] = sizeof(PXCPoint3DF32)*DEPTH_WIDTH*DEPTH_HEIGHT*1;

	// create memory-mapped file & file view for shared memory
	for(int id=0;id<NUM_OF_MMF;id++){
		hMemoryMap[id] = CreateFileMappingA(
			INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0,
			memorySize[id],
			memoryName[id] );
		if(!hMemoryMap[id]) {
			MessageBox(NULL, L"cannot create file mapping -- in IntelCameraServer.cpp", L"Error", MB_OK);
			return FALSE;
		}

		pMemoryMap[id] = (BYTE*)MapViewOfFile(hMemoryMap[id], FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if(!pMemoryMap[id])
		{
			MessageBox(NULL, L"cannot create map view of file -- in IntelCameraServer.cpp",  L"Error", MB_OK);
			return FALSE;
		}
	}

	return TRUE;
}

int IntelCameraServer::EndWriteMMF()
{
	// release memory-mapped file & file view for shared memory
	for(int id=0;id<NUM_OF_MMF;id++){
		if(pMemoryMap[id]) UnmapViewOfFile(pMemoryMap[id]);
		else return FALSE;
		if(hMemoryMap[id]) CloseHandle(hMemoryMap[id]);
		else return FALSE;
	}

	return TRUE;
}


////////////////////////////////////////////////////////////////////////////////
// private member functions on Perceptual Computing SDK
////////////////////////////////////////////////////////////////////////////////
int IntelCameraServer::InitDevice(int device_num)
{
	printf("start Soft Kinetic Camera... device #%d ", device_num);

	// create a session
	pxcStatus sts = PXCSession_Create(&session);
	if (sts<PXC_STATUS_NO_ERROR) {
		fprintf(stderr,"Failed to create a session\n");
		return 0;
	}

	// create a video capture
	PXCSession::ImplDesc desc;
	memset(&desc, 0, sizeof(desc));
	desc.subgroup = PXCSession::IMPL_SUBGROUP_VIDEO_CAPTURE;

	PXCSmartPtr<PXCCapture> capture;
	sts = session->CreateImpl(&desc, PXCCapture::CUID, (void**)&capture);
	if (sts<PXC_STATUS_NO_ERROR) {
		fprintf(stderr,"Failed to create a capture\n");
		return 0;
	}

	// create a device
	PXCSmartPtr<PXCCapture::Device> device;
	sts = capture->CreateDevice(device_num, &device);
	if (sts<PXC_STATUS_NO_ERROR) {
		fprintf(stderr,"Failed to create a device\n");
		return 0;
	}

	// create color stream: 640x480
	sts = device->CreateStream(0, PXCCapture::VideoStream::CUID, (void**)&pColorStream);
	if(sts<PXC_STATUS_NO_ERROR){
		fprintf(stderr,"failed to create color stream\n");
		return 0;
	}
	PXCCapture::VideoStream::ProfileInfo pinfoColor;
	sts = pColorStream->QueryProfile(0, &pinfoColor);
	if(sts<PXC_STATUS_NO_ERROR){
		fprintf(stderr,"failed to get color stream profile\n");
		return 0;
	}

	// set for less error on depth image : because of sync color/depth
	pinfoColor.frameRateMax.denominator =  0;
	pinfoColor.frameRateMax.numerator   = 60;
	pinfoColor.frameRateMin.denominator =  0;
	pinfoColor.frameRateMin.numerator   = 60;

	sts = pColorStream->SetProfile(&pinfoColor);
	if(sts<PXC_STATUS_NO_ERROR){
		fprintf(stderr,"failed to get color stream profile\n");
		return 0;
	}

	// create depth streams: 320x240
	sts = device->CreateStream(1, PXCCapture::VideoStream::CUID, (void**)&pDepthStream);
	if(sts<PXC_STATUS_NO_ERROR){
		fprintf(stderr,"failed to create depth stream\n");
		return 0;
	}
	PXCCapture::VideoStream::ProfileInfo pinfoDepth;
	sts = pDepthStream->QueryProfile(0, &pinfoDepth);
	if(sts<PXC_STATUS_NO_ERROR){
		fprintf(stderr,"failed to get depth stream profile\n");
		return 0;
	}

	// set for less error on depth image
	pinfoDepth.frameRateMax.denominator =  1;
	pinfoDepth.frameRateMax.numerator   = 60;
	pinfoDepth.frameRateMin.denominator =  1;
	pinfoDepth.frameRateMin.numerator   = 60;

	sts = pDepthStream->SetProfile(&pinfoDepth);
	if(sts<PXC_STATUS_NO_ERROR){
		fprintf(stderr,"failed to get depth stream profile\n");
		return 0;
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
		return 0;
	}

	printf("working now!\n");
	_camera_standby = true;
	return 1;
}

int IntelCameraServer::ProcessColor(void)
{
	// get the frame
	pxcStatus sts = sps[0]->Synchronize(0);
	if (sts<PXC_STATUS_NO_ERROR) return 0;
	sps.ReleaseRef(0);

	// copy color image
	PXCImage::ImageData colorData;
	images[0]->AcquireAccess(PXCImage::ACCESS_READ, &colorData);
	memcpy_s(color_image->imageData, sizeof(unsigned char)*COLOR_WIDTH*COLOR_HEIGHT*3, colorData.planes[0], sizeof(unsigned char )*COLOR_WIDTH*COLOR_HEIGHT*3);
	images[0]->ReleaseAccess(&colorData);
	
	// read next frame in advance
	sts = pColorStream->ReadStreamAsync(images.ReleaseRef(0), &sps[0]);
	if(sts<PXC_STATUS_NO_ERROR) return 0;

	return 1;
}

int IntelCameraServer::ProcessDepth(void)
{
	// get the frame
	pxcStatus sts = sps[1]->Synchronize(0);
	if (sts<PXC_STATUS_NO_ERROR) return 0;
	sps.ReleaseRef(1);

	// copy depth image
	PXCImage::ImageData depthData;
	images[1]->AcquireAccess(PXCImage::ACCESS_READ, &depthData);

	// capturing both "depth image" and "confidence map"
	memcpy_s(depth_image->imageData, sizeof(ushort)*DEPTH_WIDTH*DEPTH_HEIGHT, depthData.planes[0], sizeof(ushort)*DEPTH_WIDTH*DEPTH_HEIGHT);
	memcpy_s(confi_image->imageData, sizeof(ushort)*DEPTH_WIDTH*DEPTH_HEIGHT, depthData.planes[1], sizeof(ushort)*DEPTH_WIDTH*DEPTH_HEIGHT);
	for(int j=0;j<DEPTH_HEIGHT;j++)
	for(int i=0;i<DEPTH_WIDTH ;i++){
		pos2d[i+j*DEPTH_WIDTH].z = (unsigned short)(((unsigned short*)depth_image->imageData)[i+j*320]);
	}
	images[1]->ReleaseAccess(&depthData);

	// read next frame in advance
	sts = pDepthStream->ReadStreamAsync(images.ReleaseRef(1), &sps[1]);
	if(sts<PXC_STATUS_NO_ERROR) return 0;

	return 1;
}

int IntelCameraServer::MapColorToDepth(void)
{
	// convert depth image to point cloud
	pxcStatus sts = projection->ProjectImageToRealWorld(DEPTH_WIDTH*DEPTH_HEIGHT, pos2d, pos3d);
	if(sts<PXC_STATUS_NO_ERROR) return 0;

	// project color image to depth image
	sts = projection->MapDepthToColorCoordinates(DEPTH_WIDTH*DEPTH_HEIGHT, pos2d, posc);
	if(sts<PXC_STATUS_NO_ERROR) return 0;

	return 1;
}

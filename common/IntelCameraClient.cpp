/*******************************************************************************
Copyright (c) 2013-2015, Seung-Tak Noh (seungtak.noh [at] gmail.com)
All rights reserved.
*******************************************************************************/
#include "IntelCamera.h"

IntelCameraClient::IntelCameraClient(int device_num)
{
	printf("Instantiate Softkinetic camera client... device #%d ",device_num);

	// initialize MMF
	if(	InitReadMMF(device_num)==0 ){
		fprintf(stderr,"Failed to initialize Memory Map File.\n");
		EndReadMMF();
		exit(1);
	}

	// create image header for shared memory
	color_image = cvCreateImageHeader(cvSize(COLOR_WIDTH,COLOR_HEIGHT), IPL_DEPTH_8U , 3);
	depth_image = cvCreateImageHeader(cvSize(DEPTH_WIDTH,DEPTH_HEIGHT), IPL_DEPTH_16U, 1);
	confi_image = cvCreateImageHeader(cvSize(DEPTH_WIDTH,DEPTH_HEIGHT), IPL_DEPTH_16U, 1);

	////////////////////////////////////////////////////////////
	// set the memory address for data storage
	////////////////////////////////////////////////////////////
	color_image->imageData = (char*)pMemoryMap[0];
	depth_image->imageData = (char*)pMemoryMap[1];
	confi_image->imageData = (char*)pMemoryMap[2];
	pos3d = (PXCPoint3DF32*)pMemoryMap[3];
	posc = (PXCPointF32*)pMemoryMap[4];

	printf("working now!\n");
}

IntelCameraClient::~IntelCameraClient()
{
	EndReadMMF();

	// release image header
	cvReleaseImageHeader(&color_image);
	cvReleaseImageHeader(&depth_image);
	cvReleaseImageHeader(&confi_image);
}


////////////////////////////////////////////////////////////////////////////////
// private member functions on Memory-Mapped File
////////////////////////////////////////////////////////////////////////////////
int IntelCameraClient::InitReadMMF(int device_num)
{
	// zero clear
	for(int id=0;id<NUM_OF_MMF;id++){
		hMemoryMap[id] = NULL;
		pMemoryMap[id] = NULL;
	}

	// set the shared memory name
	char memoryName[NUM_OF_MMF][1024];
	for(int id=0;id<NUM_OF_MMF;id++) sprintf(memoryName[id], "icam%d_%s", device_num, DATA_NAME[id] );

	// create MMF by WINAPI for image reading
	for(int id=0;id<NUM_OF_MMF;id++){

		hMemoryMap[id] = OpenFileMappingA(FILE_MAP_READ, FALSE, memoryName[id] );
		if(!hMemoryMap[id]) {
			DWORD m_lastError = GetLastError();
			fprintf(stderr, "last error = %d\n",m_lastError);
			
			MessageBox(NULL, L"cannot open file mapping", L"Error", MB_OK);
			return FALSE;
		}

		pMemoryMap[id] = (BYTE*)MapViewOfFile(hMemoryMap[id],FILE_MAP_READ,0,0,0);
		if(!pMemoryMap[id]) 
		{
			MessageBox(NULL, L"cannot open view of file",  L"Error", MB_OK);
			return FALSE;
		}
	}

	return TRUE;
}

int IntelCameraClient::EndReadMMF()
{
	// release MMF
	for(int id=0;id<NUM_OF_MMF;id++){
		if(pMemoryMap[id]) UnmapViewOfFile(pMemoryMap[id]);
		else return FALSE;
		if(hMemoryMap[id]) CloseHandle(hMemoryMap[id]);
		else return FALSE;
	}

	return 1;
}

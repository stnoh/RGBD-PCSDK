/*******************************************************************************
Copyright (c) 2013-2015, Seung-Tak Noh (seungtak.noh [at] gmail.com)
All rights reserved.
*******************************************************************************/
#include "mmf_client.h"

extern "C"{

void initIntelCameraClient(int device_num)
{
	client = new IntelCameraClient(device_num);
}

void endIntelCameraClient()
{
	delete client;
}

void copyIntelCameraPointCloud(float* arrayPtr, int WIDTH, int HEIGHT, int i_offset, int j_offset)
{
	// direct copy
	int count = 0;
	for(int j=0;j<HEIGHT;j++)
	for(int i=0;i<WIDTH ;i++)
	{
		PXCPoint3DF32 pt = client->pos3d[(i+i_offset)+(j+j_offset)*DEPTH_WIDTH];
		arrayPtr[3*count+0] = pt.x;
		arrayPtr[3*count+1] = pt.y;
		arrayPtr[3*count+2] = pt.z;

		count++;
	}
}

void getIntelCameraColorImage(int *w, int *h)
{
	*w = client->getColorImage()->width;
	*h = client->getColorImage()->height;
}

void copyIntelCameraColorImage(char* arrayPtr)
{
	cv::Mat color(client->getColorImage(), true);
	cv::flip(color, color, 1); // mirroring

	char* colorImg = (char*)color.data;

	// direct copy to pointer: suppose RGBA image
	int count = 0;
	for(int j=0;j<COLOR_HEIGHT;j++)
	for(int i=0;i<COLOR_WIDTH ;i++)
	{
		arrayPtr[4*count+0] = colorImg[3*count+2]; // R
		arrayPtr[4*count+1] = colorImg[3*count+1]; // G
		arrayPtr[4*count+2] = colorImg[3*count+0]; // B

		count++;
	}
}

void getIntelCameraInfraImage(int *w, int *h)
{
	*w = client->getConfiImage()->width;
	*h = client->getConfiImage()->height;
}

void copyIntelCameraInfraImage(char* arrayPtr)
{
	cv::Mat confi(client->getConfiImage(), true);
	cv::flip(confi, confi, 1); // mirroring
	confi.convertTo(confi, CV_8UC1, 0.17); // 255/1500 = 0.17

	char* confiImg = (char*)confi.data;

	// direct copy to pointer: suppose RGBA image
	int count = 0;
	for(int j=0;j<DEPTH_HEIGHT;j++)
	for(int i=0;i<DEPTH_WIDTH ;i++)
	{
		char color = confiImg[count];

		arrayPtr[4*count+0] = color;
		arrayPtr[4*count+1] = color;
		arrayPtr[4*count+2] = color;

		count++;
	}
}

}

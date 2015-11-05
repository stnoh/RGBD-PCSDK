/*******************************************************************************
Copyright (c) 2013-2015, Seung-Tak Noh (seungtak.noh [at] gmail.com)
All rights reserved.
*******************************************************************************/
#include "mmf_server.h"
#include <process.h>

int initIntelCameraServer(int device_num)
{
	server = new IntelCameraServer(device_num);
	return server->InitDevice(device_num);
}

void startIntelCameraUpdateThread()
{
	// mimic the lambda in VC++
	//*
	struct lambda{
		static unsigned int WINAPI UpdateByThread(void* args){
			IntelCameraServer* server = (IntelCameraServer*)args;
			while(_update_thread){
				server->NextFrame();
			}
			return 0;
		}
	};
	//*/

	// VS2013 can use lambda directly [STASHED, because it generates warning]
	/*
	auto UpdateByThread = [](void* args)->unsigned WINAPI{
		IntelCameraServer* server = (IntelCameraServer*)args;
		while (_update_thread){
			server->NextFrame();
		}
		return 0;
	};
	//*/

	_update_thread = true;
	DWORD dwThreadID;
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, lambda::UpdateByThread, server, 0, (unsigned*)&dwThreadID);
//	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, UpdateByThread, server, 0, (unsigned*)&dwThreadID); // [STASHED, because it generates warning]
}

void stopIntelCameraUpdateThread()
{
	if( _update_thread ){
		_update_thread = false;
		Sleep(200);
	}
}

void endIntelCameraServer(){ 
	stopIntelCameraUpdateThread();
	delete server;
}



////////////////////////////////////////////////////////////
// for EXE
////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
	////////////////////////////////////////////////////////////
	// initialize
	////////////////////////////////////////////////////////////
	initIntelCameraServer(0);
	IntelCameraClient* client = new IntelCameraClient(0);

	////////////////////////////////////////////////////////////
	// main loop
	////////////////////////////////////////////////////////////
	bool init = false;
	while(1){
		IplImage* color = client->getColorImage();
		cvShowImage("color-client", color);

		int key = cv::waitKey(1);
		if(key==27) break;
		if(key==' '){
			if(!init){
				startIntelCameraUpdateThread();
				init = true;
			}
			else{
				stopIntelCameraUpdateThread();
				init = false;
			}
		}
	}

	////////////////////////////////////////////////////////////
	// finalize
	////////////////////////////////////////////////////////////
	delete client;
	stopIntelCameraUpdateThread();
	endIntelCameraServer();

	return 1;
}

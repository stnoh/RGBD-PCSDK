/*******************************************************************************
This is the runnable application (EXE) of Intel Hand Skeletal Library (HSKL). 
Some of the code is originated from "HandTrackViewer.cpp" in HSKL.
https://software.intel.com/protected-download/393460/393178

Modified by Seung-Tak Noh (seungtak.noh [at] gmail.com), 2013-2015
*******************************************************************************/
#include "stdAfx.h"
#include "HSKL_MMF.h"
#include <process.h>

#include "RGBD_viewer.h"
#include <GL/freeglut.h>
void DrawAllWindows();

// global variables for GLUT windows
int mainwin_id;
int subwin_id[2];

// hskl tracking error
float hskl_error = 1.0f;

// for mmf server
IntelCameraServer *server;
bool _update_server = false;


////////////////////////////////////////////////////////////////////////////////
// Draw HSKL hand mesh model from binary module
////////////////////////////////////////////////////////////////////////////////
void DrawHandModel(void)
{
	////////////////////////////////////////////////////////////
	// get mesh at first time
	////////////////////////////////////////////////////////////
	static bool init = false;

	static int**   hand_tris  = new int*  [17];
	static float** hand_verts = new float*[17];
	if(!init){
		for(int id=0;id<17;id++){
			hand_tris [id] = new int  [76*3*3];
			hand_verts[id] = new float[40*3*3];
			getHSKLBoneMesh(id, hand_tris[id], hand_verts[id]);
		}
		init = true;
	}

	////////////////////////////////////////////////////////////
	// draw hand mesh
	////////////////////////////////////////////////////////////
	static float* bone_poses = new float[34*7];
	getHSKLBonePoses(bone_poses);

	for(int id=0; id<17; id++)
	{
		// get the modelview matrix from hskl pose
		hskl::float4 q  = hskl::float4(bone_poses[7*id+0], bone_poses[7*id+1], bone_poses[7*id+2], bone_poses[7*id+3]);
		hskl::float3 tr = hskl::float3(bone_poses[7*id+4], bone_poses[7*id+5], bone_poses[7*id+6]);
		hskl::float4x4 mat = PoseToMatrix(q, tr);

		// convert to GL matrix
		glPushMatrix();
		glScalef(1,-1,-1);
		glMultMatrixf(mat);

		// draw triangles of each bone
		glBegin(GL_TRIANGLES);
		if(hskl_error < 0.5f) glColor4f(0.3f,0.3f,1,0.75f);
		else glColor4f(0.7f,0.3f,1,0.75f);
		
		for(int j=0; j<76 ; j++)
		{
			int idx0 = hand_tris[id][3*j+0];
			int idx1 = hand_tris[id][3*j+1];
			int idx2 = hand_tris[id][3*j+2];

			float v0_x = hand_verts[id][3*idx0+0];
			float v0_y = hand_verts[id][3*idx0+1];
			float v0_z = hand_verts[id][3*idx0+2];

			float v1_x = hand_verts[id][3*idx1+0];
			float v1_y = hand_verts[id][3*idx1+1];
			float v1_z = hand_verts[id][3*idx1+2];

			float v2_x = hand_verts[id][3*idx2+0];
			float v2_y = hand_verts[id][3*idx2+1];
			float v2_z = hand_verts[id][3*idx2+2];

			hskl::float3 v0 = hskl::float3(v0_x, v0_y, v0_z);
			hskl::float3 v1 = hskl::float3(v1_x, v1_y, v1_z);
			hskl::float3 v2 = hskl::float3(v2_x, v2_y, v2_z);

			glNormal3fv(hskl::cross(v1-v0,v2-v0));
			glVertex3fv(v0);
			glVertex3fv(v1);
			glVertex3fv(v2);
		}
		glEnd();

		glPopMatrix();
	}
}

void InitProgram()
{
	////////////////////////////////////////////////////////////
	// start mmf server
	////////////////////////////////////////////////////////////
	server = new IntelCameraServer(0);
	server->InitDevice(0);

	// mimic the lambda in VC++
	//*
	struct lambda{
		static unsigned int WINAPI UpdateByThread(void* args){
			IntelCameraServer* server = (IntelCameraServer*)args;
			while(_update_server){
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
		while (_update_server){
			server->NextFrame();
		}
		return 0;
	};
	//*/

	_update_server = true;
	DWORD dwThreadID;
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, lambda::UpdateByThread, server, 0, (unsigned*)&dwThreadID);
//	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, UpdateByThread, server, 0, (unsigned*)&dwThreadID); // [STASHED, because it generates warning]

	////////////////////////////////////////////////////////////
	// set intrinsic & extrinsic parameters of RGB-D camera
	////////////////////////////////////////////////////////////
	float color_intrinsic[4] = { 597.00f, 600.00f, 320.00f, 240.00f }; // get from PerC SDK
	float depth_intrinsic[4] = { 443.4065f*0.5f, 442.1876f*0.5f, 320.0f*0.5f, 240.0f*0.5f }; // get from PerC SDK

	// #id 0 (DEPTH) -> #id 1 (COLOR)
	float stereo_rot[9] = {
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f };
	float stereo_tr[3] = { 0.025f, 0.0f, 0.0f }; // in [mm]


	////////////////////////////////////////////////////////////
	// initialize hand tracking module
	////////////////////////////////////////////////////////////
	initHSKLTracking(0, depth_intrinsic[0], depth_intrinsic[1]);
	setHSKLModelType(0);
	setHSKLMeasurements(0.08f, 0.19f);
	startHSKLTracking_thread();

	InitBGImage();

	////////////////////////////////////////////////////////////
	// initialize GLUT
	////////////////////////////////////////////////////////////
	rgbd_viewer::setIntrinsicParams(0, depth_intrinsic);
	rgbd_viewer::setIntrinsicParams(1, color_intrinsic);
	rgbd_viewer::setStereoRT(stereo_rot, stereo_tr);
}

void idle()
{
	UpdateBGImage();
	hskl_error = getHSKLTrackingError();

	// draw all GLUT windows
	DrawAllWindows();
}

void EndProgram()
{
	////////////////////////////////////////////////////////////
	// finalize hand tracking modules
	////////////////////////////////////////////////////////////
	stopHSKLTracking_thread();
	endHSKLTracking();

	EndBGImage();

	////////////////////////////////////////////////////////////
	// end mmf server
	////////////////////////////////////////////////////////////
	_update_server = false;
	Sleep(200);
	delete server;
}


////////////////////////////////////////////////////////////////////////////////
// DO NOT TOUCH BELOW
////////////////////////////////////////////////////////////////////////////////
void display_main()
{
	// clear flags
	glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
}

void display_depth_realtime()
{
	// clear flags
	glClearColor(0.3f, 0.3f, 1.0f, 0.0f);
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

	// draw background
	IplImage* depthImage = GetDepthImage8U();
	rgbd_viewer::drawBG(depthImage, GL_LUMINANCE);

	// draw 3D information
	rgbd_viewer::draw3D(0, DrawHandModel );

	// draw & swap buffers
	glutSwapBuffers();
}

void display_color_realtime()
{
	// clear flags
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

	// draw background
	IplImage* bgImage = GetColorImageRGB();
	rgbd_viewer::drawBG(bgImage, GL_RGB);

	// draw 3D information
	rgbd_viewer::draw3D(1, DrawHandModel );

	// draw & swap buffers
	glutSwapBuffers();
}

void reshape(int width, int height)
{
	glutSetWindow(mainwin_id);
	glutPostRedisplay();

	int sub_w = width /2-1;

	// reset four windows at the same rate
	int x[2] = {0, sub_w+2};
	int y[2] = {0,       0};

	// subwindow #1-2
	for(int i=0;i<2;i++){
		int id = subwin_id[i];
		glutSetWindow(id);

		glutPositionWindow(x[i],y[i]);
		glutReshapeWindow(sub_w, height);
		glViewport(0,0,sub_w,height);

		glutPostRedisplay();
	}
}

void keyboard(unsigned char key, int x, int y)
{
	if(key==27) exit(1);
}

void DrawAllWindows()
{
	glutSetWindow(mainwin_id);
	glutPostRedisplay();

	// subwindow #1-2
	for(int i=0;i<2;i++){
		glutSetWindow(subwin_id[i]);
		glutPostRedisplay();
	}
}

int main(int argc, char** argv)
{
	// GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );

	// set global callbacks
	atexit(EndProgram);
	glutCloseFunc(EndProgram); // only available in FREEGLUT
	glutIdleFunc(idle);

	// create window
	int win_w = 640, win_h = 480;

	// main window
	glutInitWindowSize (win_w*2,win_h);
	glutInitWindowPosition(100, 100);
	mainwin_id = glutCreateWindow("HSKL test");
	glutDisplayFunc (display_main);
	glutReshapeFunc (reshape);
	glutKeyboardFunc(keyboard);

	// sub windows #1
	glutInitWindowSize (win_w,win_h);
	subwin_id[0] = glutCreateSubWindow(mainwin_id, 0,0,win_w,win_h);
	glutDisplayFunc(display_depth_realtime);
	glutKeyboardFunc(keyboard);

	// sub windows #2
	glutInitWindowSize (win_w,win_h);
	subwin_id[1] = glutCreateSubWindow(mainwin_id, win_w,0,win_w,win_h);
	glutDisplayFunc(display_color_realtime);
	glutKeyboardFunc(keyboard);

	// initialize program
	InitProgram();

	// enter to main loop
	glutMainLoop();
	return 1;
}

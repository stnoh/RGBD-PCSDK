#pragma once


////////////////////////////////////////////////////////////
// PCSDK for Intel Creative Gesture Camera
////////////////////////////////////////////////////////////
#ifdef _DEBUG
#pragma comment (lib, "libpxc_d.lib")
#pragma comment (lib, "libpxcutils_d.lib")

#else
#pragma comment (lib, "libpxc.lib")
#pragma comment (lib, "libpxcutils.lib")

#endif


////////////////////////////////////////////////////////////
// OpenCV
////////////////////////////////////////////////////////////
#ifdef _DEBUG
#pragma comment (lib, "opencv_core249d.lib")
#pragma comment (lib, "opencv_highgui249d.lib")
#pragma comment (lib, "opencv_imgproc249d.lib")

#else
#pragma comment (lib, "opencv_core249.lib")
#pragma comment (lib, "opencv_highgui249.lib")
#pragma comment (lib, "opencv_imgproc249.lib")

#endif


////////////////////////////////////////////////////////////
// Intel Hand Skeletal Library
////////////////////////////////////////////////////////////
#pragma comment(lib, "hskl.lib")


////////////////////////////////////////////////////////////
// FreeGLUT
////////////////////////////////////////////////////////////
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "freeglut.lib")

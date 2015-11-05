# RGBD-PCSDK
This repository supports to build DLL plugins of PCSDK for another program, such as Unity.  
Few projects in this repository also support to build EXE for simple testing.  


## Mandatory Hardware
It supports following cameras (actually, both are almost same).

- Intel Creative Gesture Camera
- Creative Senz3D


## 3rd Party Libraries
You need to install to compile this VS 2013 solution.  
The version is what I tested.

- Perceptual Computing SDK (PerC SDK) - R8-13842
- OpenCV - 2.4.9
- Intel Hand Skeletal Library (HSKL) - 20130531
- FreeGLUT - 3.0.0-2


## Installation
Follow [this installation guide](../../wiki/Home)...


## Known Issues
1. The latest version of PCSDK is R8-13842, but there is no downloadable file in public now...

2. The cameras have an weird issue with Windows 8.1.  
Some PC does not support Win32(x86) run-time mode, but you can probably run it in x64 mode.

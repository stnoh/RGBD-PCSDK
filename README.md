# RGBD-PCSDK
This repository supports to build DLL plugins of PCSDK for another program, such as Unity.  
Few projects in this repository also support to build EXE for simple testing.  


## Mandatory Hardware
It supports following cameras (actually, both are almost same).

- Intel Creative Gesture Camera
- Creative Senz3D


## 3rd Party Libraries
After this, you need to download the following 3rd party dependencies & copy the necessary files into the dependency folder. The version is what I tested.

- Perceptual Computing SDK (PerC SDK) - R8-13842
- OpenCV - 2.4.9
- Intel Hand Skeletal Library (HSKL) - 20130531
- FreeGLUT - 3.0.0-2


## Installation
All the dependency linking is already done in this Visual Studio 2013 solution. What you need is just copy the files into adequate folder.

Setting the folder like this:

[TODO: put the folder hierarchy image]


### PerC SDK
- You can download R7-12492 version from [Creative's support page](http://support.creative.com/Products/ProductDetails.aspx?catID=218&CatName=Web+Cameras&subCatID=1056&subCatName=Senz3D+Series&prodID=21514&prodName=Creative+Senz3D&bTopTwenty=1&VARSET=prodfaq:PRODFAQ_21514,VARSET=CategoryID:218)
- Install and find the folder in your PC. By its default, it should be "C:/Program Files (x86)/Intel/PCSDK"

Now what you need is to copy the **libpxc** and **libpxcutils** into dependency folder as **Multi-threaded (Debug) DLL** build.

- **libpxc** should be in the "{PCSDK}/src/libpxc." Open the "libpxc\_vs2012" , change the properties as **MD(d)** and compile it in every mode. After this, copy the "{PCSDK}/include" and "{PCSDK}/lib" into "dependency/PCSDK" .
- **libpxcutils** should be in the "{PCSDK}/sample/common" . Open the "libpxcutils\_vs2012" and re-compile like **libpxc**. After this, copy the "{libpxcutils}/include" and "{libpxcutils}/lib" into "dependency/PCSDK/common" .


### OpenCV
- This repository basically supports OpenCV-2.4.9, but you can modify the list in "common.h" of **common** VS project.
- You can download it from [official download page](http://opencv.org/downloads.html) and extract it in your PC.
- Copy the "{OpenCV}/build/include" folder into "dependency/OpenCV".
- Copy the files in "{OpenCV}/build/x86/vc12/lib" into "dependency/OpenCV/lib/x86", and "{OpenCV}/build/x86/vc12/bin" into "bin\_x86", respectively.
- Copy the files in "{OpenCV}/build/x64/vc12/lib" into "dependency/OpenCV/lib/x64", and "{OpenCV}/build/x64/vc12/bin" into "bin\_x64", respectively.


### HSKL
- You can download it from: https://software.intel.com/protected-download/393460/393178
- Extract it into "dependency" folder. You don't need to change anything.
- Copy the "hskl.dll" from "{hskl}/bin/Win32" to "bin\_x86" and "{hskl}/bin/x64" to "bin\_x64", respectively.

### FreeGLUT
- You can download the prebuilt version from [Transmission Zero's page](http://www.transmissionzero.co.uk/software/freeglut-devel).
- Extract it into "dependency" folder. You don't need to change anything.
- Copy the "{FreeGLUT}/freeglut.dll" to "bin\_x86" and "{FreeGLUT}/x64/freeglut.dll" to "bin\_x64", respectively.



## Setting run-time working folder
Two projects (**IntelCamera_MMF_client** and **HSKL_MMF**) support EXE build for testing.  
You can set its working directory like this:

[TODO: put the folder hierarchy image]

Then, you can build & run in EXE application.



## Known Issues
1. The latest version of PCSDK is R8-13842, but there is no downloadable file in public now...

2. The cameras have an weird issue with Windows 8.1.  
Some PC does not support Win32(x86) run-time mode, but you can probably run it in x64 mode.

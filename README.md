# Camera Timing for Android - PC

This is the PC software for the companion Android app available at
https://github.com/xc-racer99/android-camera-timing

## Getting Started
This program was developed in QT Creator as the IDE.

### Building on Linux for Linux:

Depedencies
- cmake
- QT5

On Debian Stretch, run
`$ sudo apt install cmake qtcreator libqt5serialport5-dev libqt5widgets5 libqt5xml5`

To build the executable, create a build directory and run
`cmake path/to/source` followed by `make`

#### Building a package
Run `cpack` or `make package` to build a tar.gz file.
A .deb can be built with `cpack -G DEB`
rpm and other package types are not supported right now

### Building on Windows for Windows
Install [cmake](https://cmake.org/) and your desired version of [QT](https://www.qt.io/download) and its compiler.
Make sure they are added to your PATH variable.  Create a build directory, and run
`cmake path/to/source`
If you are using Mingw, append `-G MinGW Makefiles` to the cmake command as cmake
default to generating build output for nmake or Visual Studio. 
Run `make` to generate the executable, where make is the command for your generator
(eg with mingw, run `mingw32-make`)

#### Building a package
Requires the installation of [NSIS](http://nsis.sourceforge.net/Download/)
Re-run `cmake` to pick up the now-included QT dlls, then run `make package`
again where "make" is the make command for your generator

### Building on Linux for Windows:
Use the [superbuild project](https://github.com/xc-racer99/android-camera-timing-superbuild)
Requires installing nsis (package nsis on Debian Stretch)

## Building with OCR:
This only available when building on Linux (either for Linux or via the superbuild)

### Additional dependencies
- OpenCV 2 (OpenCV 3 is not compatible)
- Boost
- Tesseract
- Leptonica

On Debian Stretch, run
`$ sudo apt install tesseract-ocr-dev libleptonica-dev libopencv-dev`
`$ sudo apt install libboost1.62-dev libboost-system1.62-dev libboost-filesystem1.62-dev`

You need to disable the `No_OCR` option (ie append `-DNo_OCR:BOOL=0` to the cmake command)
or directly edit CMakeLists.txt.  Note that the OCR has not been tested in the field
and may not be very reliable at all

Other build configurations may be possible (eg for Mac OS X), but have not been attempted.

## Training/Creating an SVM Model
Only available when built with OCR.
Use the bibnumber-cmd-line command line executable.  Then run:
`$ bibnumber-cmd-line -train dir_with_bib_number_images_only dir_with_full_images`
The resulting svm.xml file is your output.

## License
The main project is licensed under the GPLv2.  See the COPYING file for more info.
The 3rd-party libraries are licensed under their respective licenses.

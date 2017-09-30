
Camera Timing for Android - PC
===================================

This is the PC software for the companion Android app available at
https://github.com/xc-racer99/android-camera-timing

Pre-requisites
--------------

- cmake
- QT5
- OpenCV 2 (OpenCV 3 will work, but you won't be able to use the training feature)
- Boost
- Tesseract
- Leptonica

Make sure you have the development packages installed as well.

Getting Started
---------------
This program was developed in QT Creator as the IDE.

Building on Linux for Linux:
Create a build directory.  Change into it, and run "cmake
path/to/source", followed by "make", followed by "cpack"

Building on Linux for Windows:
Use the superbuild project at https://github.com/xc-racer99/android-camera-timing-superbuild

Other build configurations may be possible (eg for Mac OS X or on Windows
for Windows), but have not been tested.

Training/Creating an SVM Model
------------------------------
Use the bibnumber-cmd-line command line executable.  Then run:
bibnumber-cmd-line -train dir_with_bib_number_images_only dir_with_full_images
The resulting svm.xml file is your output.

License
-------

This is licensed under the GPLv2.  See the COPYING file for more info.
The 3rd-party libraries are licensed under their respective licenses.

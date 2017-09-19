# Add the crc library
add_library(bibnumber
	facedetection.cpp
	pipeline.cpp
	log.cpp
	textdetection.cpp
	textrecognition.cpp)

find_package(OpenCV REQUIRED core imgproc highgui objdetect ml)

################ Boost #################
set(Boost_USE_STATIC_LIBS        ON)
FIND_PACKAGE(Boost COMPONENTS system REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
########################################

target_link_libraries(bibnumber
    ${Boost}
    ${OpenCV_LIBS}
	lept
	opencv_core
	opencv_imgproc
	opencv_highgui
	opencv_objdetect
	opencv_ml
	tesseract
)

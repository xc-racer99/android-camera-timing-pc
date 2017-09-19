# Add the crc library
add_library(bibnumber
	facedetection.cpp
	pipeline.cpp
	textdetection.cpp
	textrecognition.cpp)

find_package(OpenCV REQUIRED core imgproc video highgui features2d)

################ Boost #################
set(Boost_USE_STATIC_LIBS        ON)
FIND_PACKAGE(Boost COMPONENTS system REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
########################################

target_link_libraries(bibnumber
    ${Boost}
    ${OpenCV_LIBS}
)

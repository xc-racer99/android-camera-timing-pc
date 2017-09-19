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
FIND_PACKAGE(Boost COMPONENTS system filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
########################################

target_link_libraries(bibnumber
    ${Boost_LIBRARIES}
    ${OpenCV_LIBS}
	lept
	opencv_core
	opencv_imgproc
	opencv_highgui
	opencv_objdetect
	opencv_ml
	tesseract
)

add_executable(bibnumber-cmd-line
	bibnumber.cpp
	batch.cpp
	train.cpp
)

target_link_libraries(bibnumber-cmd-line
	bibnumber
)

install(TARGETS bibnumber-cmd-line
	RUNTIME
	DESTINATION .)

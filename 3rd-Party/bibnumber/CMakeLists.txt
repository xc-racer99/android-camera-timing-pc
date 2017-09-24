find_package(OpenCV REQUIRED core imgproc highgui objdetect ml imgcodecs)

################ Boost #################
set(Boost_USE_STATIC_LIBS		ON)
FIND_PACKAGE(Boost COMPONENTS system filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
########################################

#find_package(PkgConfig REQUIRED)
#pkg_search_module(Tesseract REQUIRED tesseract)
find_library(Tesseract_LIBRARY NAMES libtesseract tesseract tesseract-3 libtesseract-3)

find_package(Leptonica REQUIRED)
include_directories(${Leptonica_INCLUDE_DIRS})

# Add the crc library
add_library(bibnumber
	SHARED
	facedetection.cpp
	pipeline.cpp
	log.cpp
	textdetection.cpp
	textrecognition.cpp
)

target_link_libraries(bibnumber
	${Boost_LIBRARIES}
	${OpenCV_LIBS}
	${Tesseract_LIBRARY}
	${Leptonica_LIBRARIES}
	opencv_core
	opencv_imgproc
	opencv_highgui
	opencv_objdetect
	opencv_ml
)

#add_executable(bibnumber-cmd-line
#	bibnumber.cpp
#	batch.cpp
#	train.cpp
#)

#target_link_libraries(bibnumber-cmd-line
#	bibnumber
#)

install(TARGETS bibnumber
	ARCHIVE
	DESTINATION .)

#install(TARGETS bibnumber-cmd-line
#	RUNTIME
#	DESTINATION .)

# Different libs required for OpenCV 2 and OpenCV 3
find_package(OpenCV REQUIRED core imgproc highgui objdetect ml)

if("${OpenCV_VERSION_MAJOR}" STREQUAL "3")
	find_package(OpenCV REQUIRED core imgproc highgui objdetect ml imgcodecs)
endif()

################ Boost #################
set(Boost_USE_STATIC_LIBS		ON)
FIND_PACKAGE(Boost COMPONENTS system filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
########################################

# Tesseract
find_library(Tesseract_LIBRARY NAMES libtesseract tesseract tesseract-3 libtesseract-3)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
	message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

# Add the library
add_library(bibnumber
	SHARED
	pipeline.cpp
	log.cpp
	textdetection.cpp
	textrecognition.cpp
)

target_link_libraries(bibnumber
	${Boost_LIBRARIES}
	${OpenCV_LIBS}
	${Tesseract_LIBRARY}
	opencv_core
	opencv_imgproc
	opencv_highgui
	opencv_objdetect
	opencv_ml
)

install(TARGETS bibnumber
	RUNTIME DESTINATION .
	ARCHIVE DESTINATION .
	LIBRARY DESTINATION .)

# This only builds with OpenCV 2
if("${OpenCV_VERSION_MAJOR}" STREQUAL "2")
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
endif()

# install licensing file
install(FILES Info DESTINATION licenses
	RENAME "bibnumber (OCR).txt")

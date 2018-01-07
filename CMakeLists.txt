cmake_minimum_required(VERSION 3.1.0)

project("Camera Timing for Android - PC")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Option to completely disable all OCR
OPTION(No_OCR "Disable compiling in OCR" OFF)

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

# Add the libcrc directory
include_directories ("${PROJECT_SOURCE_DIR}/3rd-Party/libcrc/include")
add_subdirectory (3rd-Party/libcrc)

# Find the QtWidgets library
find_package(Qt5Widgets)
find_package(Qt5Network)
find_package(Qt5SerialPort)
find_package(Qt5Xml)

# Add resources
qt5_add_resources(UI_RESOURCES images.qrc)

# Source files
set(CameraTiming_SRCs
	main.cpp
	mainwindow.cpp mytcpsocket.cpp
	summitemulator.cpp
	timingcamera.cpp timingpoint.cpp
	${UI_RESOURCES})

if(${No_OCR})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNO_OCR")
else()
	# Add bibnumber
	include_directories("${PROJECT_SOURCE_DIR}/3rd-Party/bibnumber")
	add_subdirectory(3rd-Party/bibnumber)

	set(CameraTiming_SRCs "${CameraTiming_SRCs}" ocrpipeline.cpp)
endif()

# Windows Icon
if(WIN32)
	enable_language(RC)
	configure_file(images/icon.ico ${CMAKE_CURRENT_BINARY_DIR}/icon.ico COPYONLY)
	list(APPEND CameraTiming_SRCs windows_icon.rc)
endif()

# Tell CMake to create the executable
add_executable(AndroidCameraTiming
	${CameraTiming_SRCs})

target_link_libraries (AndroidCameraTiming LibCRC)

if(NOT ${No_OCR})
	target_link_libraries (AndroidCameraTiming bibnumber)
endif()

# Include the needed QT5 modules
target_link_libraries(AndroidCameraTiming Qt5::Xml Qt5::SerialPort Qt5::Widgets Qt5::Network)

# build a CPack driven installer package
include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE
     "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set (CPACK_PACKAGE_VERSION_MAJOR 0)
set (CPACK_PACKAGE_VERSION_MINOR 2)

install(TARGETS AndroidCameraTiming
	RUNTIME
	DESTINATION .)

# install licensing file
install(FILES COPYING DESTINATION licenses
	RENAME "Main Program.txt")

if(MINGW)
	# package the needed libraries, superbuild installs them into ${PROJECT_BINARY_DIR}
	file(GLOB DLLs ${PROJECT_BINARY_DIR}/../install/ProgramFiles/bin/*.dll)
	install(FILES ${DLLs}
		DESTINATION .)

	# other qt files
	install(DIRECTORY ${PROJECT_BINARY_DIR}/../install/ProgramFiles/plugins
		DESTINATION .)

	# qt.conf so it finds the plugins
	install(FILES qt.conf
		DESTINATION .)

	# install tessdata directory
	install(DIRECTORY ${PROJECT_BINARY_DIR}/../install/ProgramFiles/share/tessdata
		DESTINATION .)

	# install license files - FIXME - Make this actually work
#	file(GLOB LICENSE_FILES "${PROJECT_BINARY_DIR}/../install/ProgramFiles/share/doc/copyright*.txt")
#	install(FILES "${LICENSE_FILES}"
#		DESTINATION licenses)

	# gcc libraries - FIXME - these should probably not be hard-coded...
	install(FILES /usr/lib/gcc/i686-w64-mingw32/6.3-win32/libstdc++-6.dll
		DESTINATION .)
	install(FILES /usr/lib/gcc/i686-w64-mingw32/6.3-win32/libgcc_s_sjlj-1.dll
		DESTINATION .)

	# NSIS start menu links will point to executables in this directory
	set(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")
	set(CPACK_PACKAGE_EXECUTABLES "AndroidCameraTiming;Camera Timing for Android - PC")
endif()

include (CPack)

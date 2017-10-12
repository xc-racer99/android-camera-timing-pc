cmake_minimum_required(VERSION 3.1.0)

project("Camera Timing for Android - PC")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the QtWidgets library
find_package(Qt5Widgets)
find_package(Qt5Network)
find_package(Qt5SerialPort)
find_package(Qt5Xml)

# Add resources
qt5_add_resources(UI_RESOURCES images.qrc)

# Add the libcrc directory
include_directories ("${PROJECT_SOURCE_DIR}/3rd-Party/libcrc/include")
add_subdirectory (3rd-Party/libcrc)

# Add bibnumber
include_directories("${PROJECT_SOURCE_DIR}/3rd-Party/bibnumber")
add_subdirectory(3rd-Party/bibnumber)

# Source files
set(CameraTiming_SRCs
	main.cpp
	mainwindow.cpp mytcpsocket.cpp
	ocrpipeline.cpp summitemulator.cpp
	timingcamera.cpp timingpoint.cpp
	${UI_RESOURCES})

if(WIN32)
	enable_language(RC)
	configure_file(images/icon.ico ${CMAKE_CURRENT_BINARY_DIR}/icon.ico COPYONLY)
	list(APPEND CameraTiming_SRCs windows_icon.rc)
endif()

# Tell CMake to create the executable
add_executable(AndroidCameraTiming
	${CameraTiming_SRCs})

target_link_libraries (AndroidCameraTiming bibnumber LibCRC)

# Use the Widgets module from Qt 5.
target_link_libraries(AndroidCameraTiming Qt5::Widgets)
target_link_libraries(AndroidCameraTiming Qt5::Network)
target_link_libraries(AndroidCameraTiming Qt5::SerialPort)
target_link_libraries(AndroidCameraTiming Qt5::Xml)

# build a CPack driven installer package
include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE
     "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set (CPACK_PACKAGE_VERSION_MAJOR 0)
set (CPACK_PACKAGE_VERSION_MINOR 1)

install(TARGETS AndroidCameraTiming
	RUNTIME
	DESTINATION .)

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

	# gcc libraries - Fix-ME - these should probably not be hard-coded...
	install(FILES /usr/lib/gcc/i686-w64-mingw32/5.3-win32/libstdc++-6.dll
		DESTINATION .)
	install(FILES /usr/lib/gcc/i686-w64-mingw32/5.3-win32/libgcc_s_sjlj-1.dll
		DESTINATION .)

	# NSIS start menu links will point to executables in this directory
	set(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")
	set(CPACK_PACKAGE_EXECUTABLES "AndroidCameraTiming;Camera Timing for Android - PC")
endif()

include (CPack)

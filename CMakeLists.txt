cmake_minimum_required(VERSION 3.1.0)

project(AndroidCameraTiming)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the QtWidgets library
find_package(Qt5Widgets)
find_package(Qt5Network)
find_package(Qt5SerialPort)

# Add resources
qt5_add_resources(UI_RESOURCES images.qrc)

# Tell CMake to create the executable
add_executable(AndroidCameraTiming main.cpp
	mainwindow.cpp mytcpsocket.cpp
	summitemulator.cpp
	timingcamera.cpp timingpoint.cpp
	${UI_RESOURCES})

# Add the libcrc directory
include_directories ("${PROJECT_SOURCE_DIR}/3rd-Party/libcrc/include")
add_subdirectory (3rd-Party/libcrc)
 
target_link_libraries (AndroidCameraTiming LibCRC)

# Use the Widgets module from Qt 5.
target_link_libraries(AndroidCameraTiming Qt5::Widgets)
target_link_libraries(AndroidCameraTiming Qt5::Network)
target_link_libraries(AndroidCameraTiming Qt5::SerialPort)

# build a CPack driven installer package
include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE
     "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set (CPACK_PACKAGE_VERSION_MAJOR 0)
set (CPACK_PACKAGE_VERSION_MINOR 1)
include (CPack)

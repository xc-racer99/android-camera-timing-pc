cmake_minimum_required(VERSION 3.1.0)

project("Camera Timing for Android - PC")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Option to completely disable all OCR
OPTION(No_OCR "Disable compiling in OCR" ON)

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

if(UNIX)
    install(TARGETS AndroidCameraTiming
        RUNTIME
        DESTINATION bin)
    # install licensing file
    # FIXME Don't abuse the bin directory for this file
    install(FILES COPYING DESTINATION bin/copyright
        RENAME "Main Program.txt")
else()
    install(TARGETS AndroidCameraTiming
        RUNTIME
        DESTINATION .)
    # install licensing file
    install(FILES COPYING DESTINATION copyright
        RENAME "Main Program.txt")
endif()

if(MINGW AND CMAKE_HOST_UNIX)
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

    # install license files
    install(DIRECTORY "${PROJECT_BINARY_DIR}/../install/ProgramFiles/share/doc/copyright"
        DESTINATION .)

    # Determine location libgcc and libstdc++ and install them
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -print-libgcc-file-name OUTPUT_VARIABLE STATIC_LIBGCC_PATH)
    get_filename_component(MINGW_LIBS_PATH ${STATIC_LIBGCC_PATH} DIRECTORY)

    file(GLOB LIBSTDCPP ${MINGW_LIBS_PATH}/libstdc++*.dll)
    file(GLOB LIBGCC ${MINGW_LIBS_PATH}/libgcc*.dll)
    install(FILES ${LIBSTDCPP} ${LIBGCC}
        DESTINATION .)
elseif(WIN32 AND CMAKE_HOST_WIN32)
    # Building on Windows for Windows

    # Run windeployqt
    get_target_property(QT5_QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
    get_filename_component(QT5_WINDEPLOYQT_EXECUTABLE ${QT5_QMAKE_EXECUTABLE} PATH)
    set(QT5_WINDEPLOYQT_EXECUTABLE "${QT5_WINDEPLOYQT_EXECUTABLE}/windeployqt.exe")
    add_custom_command(TARGET AndroidCameraTiming POST_BUILD
        COMMAND ${QT5_WINDEPLOYQT_EXECUTABLE} --release $<TARGET_FILE:AndroidCameraTiming>)

    # Install dlls
    file(GLOB DLLs ${PROJECT_BINARY_DIR}/*.dll)
    install(FILES ${DLLs}
        DESTINATION .)

    # other qt directories
    install(DIRECTORY ${PROJECT_BINARY_DIR}/bearer
        ${PROJECT_BINARY_DIR}/iconengines
        ${PROJECT_BINARY_DIR}/imageformats
        ${PROJECT_BINARY_DIR}/platforms
        ${PROJECT_BINARY_DIR}/styles
        ${PROJECT_BINARY_DIR}/translations
        DESTINATION .)
elseif(UNIX AND EXISTS /usr/bin/dpkg AND EXISTS /usr/bin/lsb_release)
    # Building on Debian or derivative
    set(CPACK_GENERATOR_DEFAULT "DEB")
    set(CPACK_PACKAGE_NAME "camera-timing-android")
    set(CPACK_DEBIAN_PACKAGE_NAME "camera-timing-android")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt5core5a, libqt5gui5, libqt5widgets5, libqt5xml5, libqt5serialport5")
    if(NOT ${No_OCR})
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libtesseract3, tesseract-ocr-eng, liblept5,
            libopencv-core2.4v5, libopencv-highgui2.4-deb0, libopencv-imgproc2.4v5, libopencv-ml2.4v5, libopencv-objdetect2.4v5,
            libboost-system1.62.0, libboost-filesystem1.62.0")
    endif()

    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Jonathan Bakker <xc-racer2@live.ca>")
endif()

if(WIN32)
    # NSIS start menu links will point to executables in this directory
    set(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")
    set(CPACK_PACKAGE_EXECUTABLES "AndroidCameraTiming;Camera Timing for Android - PC")
endif()

include (CPack)

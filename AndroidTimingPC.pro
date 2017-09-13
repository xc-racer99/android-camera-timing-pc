QT += core gui
QT += widgets
QT += network
QT += serialport

SOURCES += \
    main.cpp \
    timingpoint.cpp \
    mytcpsocket.cpp \
    mainwindow.cpp \
    timingcamera.cpp \
    summitemulator.cpp

SOURCES += \
    3rd-Party/libcrc/src/crc16.c

INCLUDEPATH += \
    3rd-Party/libcrc/include

HEADERS += \
    timingpoint.h \
    mytcpsocket.h \
    mainwindow.h \
    timingcamera.h \
    summitemulator.h \
    3rd-Party/libcrc/include/checksum.h

RESOURCES += \
    images.qrc

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

HEADERS += \
    timingpoint.h \
    mytcpsocket.h \
    mainwindow.h \
    timingcamera.h \
    summitemulator.h

RESOURCES += \
    images.qrc

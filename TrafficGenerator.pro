QT       += core gui widgets

CONFIG += c++11

VERSION = 1.0

QMAKE_TARGET_COMPANY     = FeZar97
QMAKE_TARGET_PRODUCT     = TrafficGenerator
QMAKE_TARGET_DESCRIPTION = TrafficGenerator
QMAKE_TARGET_COPYRIGHT   = FeZar97

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    main.cpp \
    widget.cpp

HEADERS += \
    widget.h

FORMS += \
    widget.ui

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    img.qrc

win32: RC_ICONS = $$PWD/img/TRAFFIC.ico

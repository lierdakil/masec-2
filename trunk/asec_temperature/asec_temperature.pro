TEMPLATE = app
DESTDIR = ../build/
TARGET = asec_temperature
QT += core \
    gui \
    dbus
CONFIG += exceptions
HEADERS += adaptors.h \
    ctrl/temp.h \
    QTempTimeline.h \
    settings.h \
    asec_temperature.h
SOURCES += ctrl/temp.cpp \
    adaptors.cpp \
    QTempTimeline.cpp \
    settings.cpp \
    main.cpp \
    asec_temperature.cpp
FORMS += settings.ui \
    asec_temperature.ui
RESOURCES +=
INCLUDEPATH += ../asec_gpib \
        ../visa
LIBS += -L../build \
    -lasec_gpib

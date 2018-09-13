#-------------------------------------------------
#
# Project created by QtCreator 2017-06-23T09:07:37
#
#-------------------------------------------------

QT += core network

TARGET = fetchtv
TEMPLATE = app
CONFIG += c++14 console

SOURCES += main.cpp \
		   task.cpp

HEADERS += task.hpp


win32:CONFIG(release, debug|release): LIBS += $$OUT_PWD/../qtupnp/release/libqtupnp.a
else:win32:CONFIG(debug, debug|release): LIBS += $$OUT_PWD/../qtupnp/debug/libqtupnp.a

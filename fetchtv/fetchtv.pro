#-------------------------------------------------
#
# Project created by QtCreator 2017-06-23T09:07:37
#
#-------------------------------------------------

QT += core network

TARGET = fetchtv
TEMPLATE = app
CONFIG += c++14

SOURCES += main.cpp \
		   task.cpp

HEADERS += task.hpp

win32 {
	CONFIG(release, debug|release) {
		#message(Window release)
		#LIBS += $$OUT_PWD/../qtupnp/release/libqtupnp.a
		LIBS += -L$$OUT_PWD/../qtupnp/release/ -lqtupnp
		CONFIG += console
	}
	CONFIG(debug, debug|release) {
		#message(Window debug)
		LIBS += -L$$OUT_PWD/../qtupnp/debug/ -lqtupnp

	}
}

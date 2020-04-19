#include(gtest_dependency.pri)

INCLUDEPATH += /usr/include/gtest # replace it with your path at need
LIBS += -lgtest

QMAKE_LFLAGS += -no-pie

TEMPLATE = app
CONFIG += console c++14
CONFIG -= app_bundle
CONFIG += thread
CONFIG -= qt

HEADERS += \
        tst_psqlcomet.h \
        ../../web_socket_crypto.h \
        ../../ws_server.h \
    accept_test.h \
    tst_psqlcomet_common.h \
    get_msg_size_test.h

SOURCES += \
        ../../web_socket_crypto.cpp \
        ../../ws_server_utils.cpp \
        main.cpp

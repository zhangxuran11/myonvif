CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++11 -Iinc/ -I/opt/project/boost_1_74_0/ -Igsoap/include/ -Igsoap/share/gsoap/plugin/ -Igsoap/ -Wall -g -DWITH_OPENSSL -DBOOST_BIND_GLOBAL_PLACEHOLDERS
INCLUDEPATH += /opt/project/boost_1_74_0 \
    gsoap \
    gsoap/include \
    gsoap/share/gsoap/plugin/ \
    inc
SOURCES += gsoap/share/gsoap/plugin/wsseapi.cpp \
           gsoap/stdsoap2.cpp \
           gsoap/soapClient.cpp \
           gsoap/soapC.cpp \
           gsoap/dom.cpp \
           gsoap/share/gsoap/plugin/wsaapi.c \
           gsoap/share/gsoap/plugin/mecevp.c \
           gsoap/share/gsoap/plugin/smdevp.c \
           gsoap/share/gsoap/custom/struct_timeval.c \
           src/main.cpp \
           src/client_main.cpp

HEADERS += \
    inc/OnvifSoap.h \
    inc/EchoServer.h \
    inc/EchoClient.h

DISTFILES += \
    makefile

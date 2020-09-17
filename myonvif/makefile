TARGET=myonvif
C_FLAGS=-Iinc/ -I/opt/project/boost_1_74_0/ -Igsoap/include/ -Igsoap/share/gsoap/plugin/ -Igsoap/ -Wall -g -DWITH_OPENSSL
CXX_FLAGS= -std=c++11 ${C_FLAGS} -DBOOST_BIND_GLOBAL_PLACEHOLDERS

LD_CXX_FLAGS=-lssl -lcrypto -lpthread
#CXX = arm-hisiv400-linux-g++
CXX = g++

CXX_SRC= gsoap/share/gsoap/plugin/wsseapi.cpp \
		 gsoap/stdsoap2.cpp \
		 gsoap/soapClient.cpp \
		 gsoap/soapC.cpp \
		 gsoap/dom.cpp \
		 src/main.cpp
C_SRC = gsoap/share/gsoap/plugin/wsaapi.c \
		gsoap/share/gsoap/plugin/mecevp.c \
		gsoap/share/gsoap/plugin/smdevp.c \
		gsoap/share/gsoap/custom/struct_timeval.c
CXXOBJS := $(CXX_SRC:.cpp=.ox)
COBJS := $(C_SRC:.c=.o)

all: ${TARGET}

%.ox:%.cpp
	$(CXX) $< $(CXX_FLAGS)  -c -o $@                             
%.o:%.c
	$(CXX) $<  $(CXX_FLAGS)  -c -o $@

${TARGET}: ${CXXOBJS} ${COBJS}
	${CXX} $^ ${LD_CXX_FLAGS} -o $@
	${CXX} ${CXX_FLAGS} src/client_main.cpp ${LD_CXX_FLAGS} -o testClient
client: src/client_main.cpp
	${CXX} ${CXX_FLAGS} $^ ${LD_CXX_FLAGS} -o testClient

clean:
	rm ${CXXOBJS} ${COBJS} ${TARGET} -f

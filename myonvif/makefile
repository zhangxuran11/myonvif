TARGET=myonvif
C_FLAGS= -I./ -Iinc/ -I${BOOST_DIR} -Igsoap/include/ -Igsoap/share/gsoap/plugin/ -Igsoap/ -Wall -g -DWITH_OPENSSL
APPEND_FLAGS= -DBOOST_BIND_GLOBAL_PLACEHOLDERS  -DBOOST_ASIO_DISABLE_STD_EXCEPTION_PTR
CXX_FLAGS= -std=c++11 ${C_FLAGS}  ${APPEND_FLAGS} #-DBOOST_ASIO_DISABLE_STD_NESTED_EXCEPTION -DBOOST_ASIO_NO_EXCEPTIONS  #-D__GCC_ATOMIC_INT_LOCK_FREE=2 #-DBOOST_NO_CXX11_HDR_ATOMIC   # -DBOOST_ASIO_DISABLE_VARIADIC_TEMPLATES  -D__GCC_ATOMIC_INT_LOCK_FREE=2 -DBOOST_ASIO_DISABLE_STD_NESTED_EXCEPTION

ARM_OPENSSL_DIR=/opt/Emveded/IPC_InputCard/ipc_hi3536/project/video/mpp_single/sample/
CXX = arm-hisiv400-linux-g++ -march=armv7-a -I${ARM_OPENSSL_DIR}
LD_CXX_FLAGS=-Llib_hi3536 -lssl -lcrypto -lpthread

#CXX = g++ 
#LD_CXX_FLAGS=-lssl -lcrypto -lpthread

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
client: src/client_main.cpp
	${CXX} ${CXX_FLAGS} $^ ${LD_CXX_FLAGS} -o testClient

clean:
	rm ${CXXOBJS} ${COBJS} ${TARGET} -f

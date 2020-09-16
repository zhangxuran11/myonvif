TARGET=myonvif
C_FLAGS=-Igsoap/include/ -Igsoap/share/gsoap/plugin/ -Igsoap/
CXX_FLAGS=${C_FLAGS}
#CXX = arm-hisiv400-linux-g++
CXX = g++

CXX_SRC= gsoap/stdsoap2.cpp \
		 gsoap/soapClient.cpp \
		 gsoap/soapC.cpp \
		 main.cpp
C_SRC = gsoap/share/gsoap/plugin/wsaapi.c
CXXOBJS := $(CXX_SRC:.cpp=.ox)
COBJS := $(C_SRC:.c=.o)

all: ${TARGET}

%.ox:%.cpp
	$(CXX) $< $(CXX_FLAGS)  -c -o $@                             
%.o:%.c
	$(CXX) $<  $(CXX_FLAGS)  -c -o $@

${TARGET}: ${CXXOBJS} ${COBJS}
	${CXX} $^ -o $@

clean:
	rm ${CXXOBJS} ${COBJS} ${TARGET} -f

CXX = nvcc $(CXXFLAGS)
CXXFLAGS = --std=c++11
IDIR =  -I ~/CS7210/global -I /usr/include/nvidia/gdk
LIB_PATH = -L /usr/src/gdk/nvml/lib $(LIB)
PROTOBUF = ../global/message.pb.cc
LIB = -lprotobuf -lnvidia-ml

all :
	$(CXX)  $(IDIR) $(PROTOBUF) Jammer.cpp -o Jammer $(LIB_PATH)
clean :
	rm -f Jammer.h.gch. *.o Jammer ~/CS7210/log/Jammer*



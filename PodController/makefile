CXX = g++ $(CXXFLAGS)
CXXFLAGS = --std=c++11 
IDIR =  -I ~/CS7210/global
PROTOBUF = ../global/message.pb.cc
LIB = -lprotobuf
LIB_PATH = $(LIB)

all :
	$(CXX)  $(IDIR) $(PROTOBUF) PodController.cpp -o PodController $(LIB_PATH)
	cp PodController ../main
clean :
	rm -f PodController.h.gch. *.o PodController



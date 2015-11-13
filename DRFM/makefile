CXX = g++ $(CXXFLAGS)
CXXFLAGS = --std=c++11 
IDIR =  -I ~/CS7210/global
PROTOBUF = ../global/message.pb.cc
LIB = -lprotobuf
LIB_PATH = $(LIB)
all :
	$(CXX) $(IDIR) $(PROTOBUF) DRFM.cpp -o DRFM $(LIB_PATH) 
clean :
	rm -f DRFM.h.gch. *.o DRFM ~/CS7210/log/DRFM*



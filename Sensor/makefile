CXX = g++ $(CXXFLAGS)
CXXFLAGS = --std=c++11 
IDIR =  -I ~/CS7210/global
PROTOBUF = ../global/message.pb.cc
LIB = -lprotobuf
LIB_PATH = $(LIB)
all :
	$(CXX) $(IDIR)  $(PROTOBUF) Sensor.cpp -o Sensor $(LIB_PATH)
clean :
	rm -f Sensor.h.gch. *.o Sensor ~/CS7210/log/Sensor*



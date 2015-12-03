// #ifndef GLOBAL_H
// #define GLOBAL_H
//Local 
#include <sys/times.h>
#include <time.h>
#include <ctype.h>
#include <memory.h>
#include <assert.h>
#include <sys/stat.h>

#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/sysinfo.h>
#include <sys/vtimes.h>
#include "message.pb.h"
// C++

#include <iostream>
#include <chrono>
#include <map>
#include <fstream> 
// C
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>



// Google protocol buffer
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

const unsigned MESSAGE_SIZE = 255;
using namespace std;
using namespace google::protobuf::io;
using namespace google::protobuf;
// struct timeval start_hoststate, end_hs, end_appstate, elapsed;
typedef struct _mon_rec {
    double cpu_util;
    double mem;
    double utilGPU_cpu;
    double utilGPU_mem;
    bool app;
} mon_rec;
map<string, string> name2hostname = {
    {"pod","node1"},
    {"DRFM","node1"},
    {"Sensor","node1"},
    {"Jammer1","node1"},
    {"Jammer2","node1"},
    {"Jammer3","node1"}
};

std::map<string, string> Name2port {
    {"DRFM","22341"},
    {"Sensor","32341"},
    {"Jammer1","42341"},
    {"Jammer2","52341"},
    {"Jammer3","62341"}
};
std::map<string,string>device2Name {
    {"Jammer1", "Jammer"},
    {"Jammer2", "Jammer"},
    {"Jammer3", "Jammer"},
    {"Sensor", "Sensor"}
};
std::map<string,string>device2NameNumber {
    {"Jammer1", " Jammer1"},
    {"Jammer2", " Jammer2"},
    {"Jammer3", " Jammer3"},
    {"Sensor", ""}
};
string exec(const char* cmd)
{
	FILE* pipe = popen(cmd, "r");
    if (!pipe) return "ERROR";
    char buffer[128];
    std::string result = "";
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != NULL)
            result += buffer;
    }
    pclose(pipe);
    return result;
}
void sendStringMessage(int socket,string message){
    const char * g_szMessage = message.c_str();
    write(socket,g_szMessage,MESSAGE_SIZE);
}

string receiveStringMessage(int socket){
    char buffer[MESSAGE_SIZE+1];
    bzero(buffer,MESSAGE_SIZE+1);
    if(recv(socket, (void *)buffer, MESSAGE_SIZE,MSG_WAITALL) < 0)
      return "";
    return string(buffer);
}
google::protobuf::uint32 readHdr(char *buf)
{
  uint32 size;
  ArrayInputStream ais(buf,4);
  CodedInputStream coded_input(&ais);
  coded_input.ReadVarint32(&size);//Decode the HDR and get the size
  return size;
}

ProtoMessage readProtobufBody(int csock,uint32 siz)
{
  char buffer [siz+4];//size of the payload and hdr
  //Read the entire buffer including the hdr
  if((recv(csock, (void *)buffer, 4+siz, MSG_WAITALL))== -1){
                fprintf(stderr, "Error receiving data %d\n", errno);
        }
  //Assign ArrayInputStream with enough memory 
  ArrayInputStream ais(buffer,siz+4); //google::protobuf::io::
  CodedInputStream coded_input(&ais); //google::protobuf::io::
  //Read an unsigned integer with Varint encoding, truncating to 32 bits.
  coded_input.ReadVarint32(&siz);
  //After the message's length is read, PushLimit() is used to prevent the CodedInputStream 
  //from reading beyond that length.Limits are used when parsing length-delimited 
  //embedded messages
  google::protobuf::io::CodedInputStream::Limit msgLimit = coded_input.PushLimit(siz); //
  //De-Serialize
  ProtoMessage payload;
  payload.ParseFromCodedStream(&coded_input);
  //Once the embedded message has been parsed, PopLimit() is called to undo the limit
  coded_input.PopLimit(msgLimit);
  return payload;

}
ProtoMessage receiveProtobufMessage(int socket){

    char buffer[4];
    memset(buffer, '\0', 4);
    while (1) {
        //Peek into the socket and get the packet size
        if(recv(socket,buffer,4, MSG_PEEK) <= 0) break;
        return readProtobufBody(socket,readHdr(buffer));
    }
    return ProtoMessage();
}
void sendProtobufMessage(int socket,ProtoMessage message){
    int siz = message.ByteSize()+4;
    char *pkt = new char [siz];
    ArrayOutputStream aos(pkt,siz); //google::protobuf::io
    CodedOutputStream *coded_output = new CodedOutputStream(&aos); 
    coded_output->WriteVarint32(message.ByteSize());
    message.SerializeToCodedStream(coded_output);
    write (socket,pkt,siz);

    delete[] pkt;
    delete coded_output;
}
double getTimeSinceLastNano(chrono::high_resolution_clock::time_point start){
  chrono::high_resolution_clock::time_point finish = chrono::high_resolution_clock::now();
  return chrono::duration_cast<chrono::microseconds>(finish-start).count();
}
long getCurrrentimeInMicroseconds(){
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return tv.tv_sec*1000000 + tv.tv_usec;
}
// #endif // GLOBAL_H
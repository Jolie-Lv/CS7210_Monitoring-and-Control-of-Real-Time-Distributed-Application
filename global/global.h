// #ifndef GLOBAL_H
// #define GLOBAL_H
//Local 
#include "message.pb.h"
// C++
#include <iostream>
#include <map>
// C
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
// struct timeval start_hoststate, end_hs, end_appstate, elapsed;
typedef struct _mon_rec {
    double cpu_util;
    double mem;
    double utilGPU_cpu;
    double utilGPU_mem;
    bool app;
} mon_rec;
map<string, string> name2hostname = {
    {"pod","node3"},
    {"drfm","node2"},
    {"sensor","node3"},
    {"jammer1","node1"},
    {"jammer2","node2"},
    {"jammer3","node1"}
};

std::map<string, string> podName2port {
    {"drfm","2234"},
    {"sensor","3234"},
    {"jammer1","4234"},
    {"jammer2","5234"},
    {"jammer3","6234"}
};
std::map<string, string> DrfmName2port {
    {"sensor","3235"},
    {"jammer1","4235"},
    {"jammer2","5235"},
    {"jammer3","6235"}
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
bool sendStringMessage(int socket,string message){
    const char * g_szMessage = message.c_str();
    write(socket,g_szMessage,MESSAGE_SIZE);
}
bool sendProtobufMessage(int socket,JammerMessage message){
    cout<<"size after serilizing is "<<message.ByteSize()<<endl;
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
string receiveMessage(int socket){
    char buffer[MESSAGE_SIZE+1];
    bzero(buffer,MESSAGE_SIZE+1);
    recv(socket, buffer, MESSAGE_SIZE,0);
    return string(buffer);
}
// #endif // GLOBAL_H
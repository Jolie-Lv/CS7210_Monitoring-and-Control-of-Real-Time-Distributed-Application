
#include "global.h"
void startAll();
void terminateAll(int);
// void startDevice(string device);
void terminateDevice(string device);
void resetDevice(string device);
void checkJammer(ProtoMessage msg);
int drfmSocket;
map<string,bool> deviceStatus;
map<string,ProtoMessage::JammerMessage> jammerStatus;
map<string,long> deviceResetTimer;
bool runing;
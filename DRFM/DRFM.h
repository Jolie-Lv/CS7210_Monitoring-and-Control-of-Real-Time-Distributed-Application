
#include "global.h"
#include <thread> 
#include <mutex>
bool startAll();
void terminateAll(int);
void startDevice(string device);
void terminateDevice(string device);
void startDeviceThread(string device);
void resetDevice(string device);
map<string, int> name2socket;
map<string, int> name2acceptor;
map<string,thread> device2Thread;
bool runing;
mutex mtx;
const string self = "DRFM";
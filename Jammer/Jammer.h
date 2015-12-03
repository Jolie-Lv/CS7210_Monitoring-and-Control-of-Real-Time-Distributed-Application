
#include "global.h"
#include "common.h"

#include <nvml.h>
#include <cuda_runtime.h>

__global__ void sumArraysOnGPU(float *A, float *B, float *C, const int N);

bool checkResult(float *hostRef, float *gpuRef, const int N);
bool loadGPU();
void start(const char * idc);
void initialData(float *ip, int size);
void sumArraysOnHost(float *A, float *B, float *C, const int N);
void terminator(ProtoMessage msg);
void resetDevice(ProtoMessage msg);
void get_mon_data_MEM(mon_rec *mon_data);
void get_mon_data_GPU(mon_rec *mon_data);
void get_mon_data_CPU(mon_rec *mon_data);

bool fexists(string filename);
long getTimeFromFile(string filename);
int load;
bool runing;

int numProcessors,
	drfmSocket,
	podSocket;

mon_rec mon_data;
string id;
string resetFilename;
clock_t lastCPU, lastSysCPU, lastUserCPU;
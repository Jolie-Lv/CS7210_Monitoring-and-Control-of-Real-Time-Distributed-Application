#include "Jammer.h"

#include <time.h>
#include <ctype.h>
#include <memory.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/sysinfo.h>
#include <sys/vtimes.h>
// /usr/include/nvidia/gdk/
#include <nvml.h>
#include <cuda_runtime.h>


int main(int argc, char const *argv[])
{
	Jammer jammer;
	if(argc > 2) {
		jammer = Jammer(argv[1]);
	} 
	jammer.get_mon_data_MEM(&jammer.mon_data);
	jammer.get_mon_data_CPU(&jammer.mon_data);
	jammer.get_mon_data_GPU(&jammer.mon_data);

	printf ("mon_data %lf %lf %lf %lf %u \n", jammer.mon_data.mem, jammer.mon_data.cpu_util, jammer.mon_data.utilGPU_cpu, jammer.mon_data.utilGPU_mem, jammer.mon_data.app); 
	
	JammerMessage jamMessage;
	jamMessage.set_gpuutil(jammer.mon_data.utilGPU_cpu);
	jamMessage.set_gpumemutil(jammer.mon_data.utilGPU_mem);
	jamMessage.set_cpuutil(jammer.mon_data.cpu_util);
	jamMessage.set_cpumemutil(jammer.mon_data.mem);
	
	
	jammer.initialConnection();
	sendProtobufMessage(jammer.drfmSocket,jamMessage);
	// More logic to be added here
	return 0;
}

Jammer::Jammer(const char * idc) {
	id = atoi(idc);
}
Jammer::Jammer() {
    id = 1;
}

bool Jammer::initialConnection(){
	tcpConnectionSetup();
}

bool Jammer::tcpConnectionSetup(){
	// 1. Connect to Pod
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; 
	struct addrinfo *result,*rp;
	int s;
	if (id == 1)
		s = getaddrinfo(name2hostname["pod"].c_str(),
						podName2port["jammer1"].c_str(),
						&hints,
						&result);
	else if (id == 2)
		s = getaddrinfo(name2hostname["pod"].c_str(),
						podName2port["jammer2"].c_str(),
						&hints,
						&result);
	else 
		s = getaddrinfo(name2hostname["pod"].c_str(),
						podName2port["jammer3"].c_str(),
						&hints,
						&result);
    if (s != 0) exit(EXIT_FAILURE);
 	for (rp = result; rp != NULL; rp = rp->ai_next) {
        podSocket = socket(rp->ai_family, rp->ai_socktype,
                     rp->ai_protocol);
        if (podSocket == -1)
            continue;

       if (connect(podSocket, rp->ai_addr, rp->ai_addrlen) != -1) {
            break;                  /* Success */
       }
       close(podSocket);
    }
   	if (rp == NULL) exit(EXIT_FAILURE);

    // 2. Connect drfm
    freeaddrinfo(result);
    if (id == 1)
		s = getaddrinfo(name2hostname["drfm"].c_str(),
						DrfmName2port["jammer1"].c_str(),
						&hints,
						&result);
	else if (id == 2)
		s = getaddrinfo(name2hostname["drfm"].c_str(),
						DrfmName2port["jammer2"].c_str(),
						&hints,
						&result);
	else 
		s = getaddrinfo(name2hostname["drfm"].c_str(),
						DrfmName2port["jammer3"].c_str(),
						&hints,
						&result);
    if (s != 0) exit(EXIT_FAILURE);

 	for (rp = result; rp != NULL; rp = rp->ai_next) {
        drfmSocket = socket(rp->ai_family, rp->ai_socktype,
                     rp->ai_protocol);
        if (drfmSocket == -1)
            continue;

       if (connect(drfmSocket, rp->ai_addr, rp->ai_addrlen) != -1) {
            break;                  /* Success */
       }
       close(drfmSocket);
    }
   	if (rp == NULL) exit(EXIT_FAILURE);
    freeaddrinfo(result);
}
void Jammer::get_mon_data_CPU(mon_rec *mon_data)
{
  
    static clock_t lastCPU, lastSysCPU, lastUserCPU;
    static int numProcessors;
    FILE* file;
    struct tms timeSample;
    char line[128];
    lastCPU = times(&timeSample);
    lastSysCPU = timeSample.tms_stime;
    lastUserCPU = timeSample.tms_utime;
    file = fopen("/proc/cpuinfo", "r");
    numProcessors = 0;
    while(fgets(line, 128, file) != NULL){
        if (strncmp(line, "processor", 9) == 0) numProcessors++;
    }
    fclose(file);
    clock_t now;
    double percent;
    now = times(&timeSample);
    if (now <= lastCPU || timeSample.tms_stime < lastSysCPU ||
        timeSample.tms_utime < lastUserCPU){
        double M, N;
        M = 10;
        N = 100;
        percent = (rand() / (RAND_MAX / (N-M)));
    } else {
        percent = (timeSample.tms_stime - lastSysCPU) +
            (timeSample.tms_utime - lastUserCPU);
        percent /= (now - lastCPU);
        percent /= numProcessors;
        percent *= 100;
    }
    lastCPU = now;
    lastSysCPU = timeSample.tms_stime;
    lastUserCPU = timeSample.tms_utime;
	mon_data->cpu_util = percent;
}
void Jammer::get_mon_data_GPU(mon_rec *mon_data)
{
    nvmlReturn_t r;
    unsigned int deviceCount;
    int i;
    unsigned int k;
    if ((r = nvmlInit()) != NVML_SUCCESS)
    {
            printf("Could not init NVML: %s\n", nvmlErrorString(r));
            return;
    }

    if ((r = nvmlDeviceGetCount(&deviceCount)) != NVML_SUCCESS)
    {
            printf("Could not get device count: %s\n", nvmlErrorString(r));
            nvmlShutdown();
            return;
    }

    printf("Found %d device(s):\n", deviceCount);

    for (i = 0; i < deviceCount; i++)
    {
        nvmlDevice_t device;
        char name[NVML_DEVICE_NAME_BUFFER_SIZE];

        if ((r = nvmlDeviceGetHandleByIndex(i, &device)) != NVML_SUCCESS)
        {
            printf("Skipping device %d, could not get handle: %s\n",
                i, nvmlErrorString(r));
            continue;
        }

        if (nvmlDeviceGetName(device, name, NVML_DEVICE_NAME_BUFFER_SIZE)!= NVML_SUCCESS)
            strcpy(name, "UNKNOWN");
        printf("\tDevice %d, \"%s\":\n", i, name);

        nvmlUtilization_t utilization;
        printf("\t\tUtilization: ");
        if ((r = nvmlDeviceGetUtilizationRates(device, &utilization)) != NVML_SUCCESS)
            printf("%s\n", nvmlErrorString(r));
        else if (i == 0)
        {
            mon_data->utilGPU_cpu = utilization.gpu;
            mon_data->utilGPU_mem = utilization.memory;
        }       
        printf("%d%% GPU, %d%% MEM\n",
                               utilization.gpu,
                               utilization.memory);
        printf("\t\tPower usage: ");
        if ((r = nvmlDeviceGetPowerUsage(device, &k))
            != NVML_SUCCESS)
            printf("%s\n", nvmlErrorString(r));
        else
            printf("%dW\n", k/1000);
    }
    nvmlShutdown();
}
void Jammer::get_mon_data_MEM(mon_rec *mon_data)
{
    struct sysinfo memInfo;
    sysinfo (&memInfo);
    long long totalPhysMem = memInfo.totalram;
    //Multiply in next statement to avoid int overflow on right hand side...
    totalPhysMem *= memInfo.mem_unit;

    long long physMemUsed = memInfo.totalram - memInfo.freeram;
    //Multiply in next statement to avoid int overflow on right hand side...
    physMemUsed *= memInfo.mem_unit;	
    double  percent = ((double)physMemUsed * 100.00/(double)totalPhysMem) ;
	mon_data->mem = percent;
}
#include "Jammer.h"
// #include "common.h"
#include <cuda_runtime.h>
#include <stdio.h>
#include <sys/times.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <string.h>
#include <sys/vtimes.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <memory.h>
#include <assert.h>

#include </usr/include/nvidia/gdk/nvml.h>

bool Jammer::initialConnection(){
    cout << "Jammer is Started" <<endl;
    return true;
}
Jammer::Jammer() {
    id = 1;
}




void get_mon_data_MEM(mon_rec *mon_data)
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
void get_mon_data_GPU(mon_rec *mon_data)
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
void get_mon_data_CPU(mon_rec *mon_data)
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
int main(int argc, char **argv)
{
    printf("%s Starting...\n", argv[0]);

    Jammer jammer;
    jammer.initialConnection();

    get_mon_data_MEM(&mon_data);
    printf ("Main mem %f\n", mon_data.mem);
    get_mon_data_CPU(&mon_data);
    printf ("Main CPU %f\n", mon_data.cpu_util);
    get_mon_data_GPU(&mon_data);
    printf ("mon_data %lf %lf %lf %lf %u \n", mon_data.mem, mon_data.cpu_util, mon_data.utilGPU_cpu, mon_data.utilGPU_mem, mon_data.app); 
    return(0);
}


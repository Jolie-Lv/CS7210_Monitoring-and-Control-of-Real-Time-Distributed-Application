#include "Jammer.h"


// /usr/include/nvidia/gdk/



int main(int argc, char const *argv[])
{
    start((argc > 1 ) ? argv[1] : "Jammer1");
    
    
    while(runing) {
        cout<<"Waiting on DRFM Message every second..."<<endl;
        ProtoMessage msg = receiveProtobufMessage(drfmSocket);
        if(msg.dest_device() == id){
            if(msg.signal() == ProtoMessage::TERMINATE){
                terminator(msg);
            } else if (msg.signal() == ProtoMessage::RESET){
                resetDevice(msg);
            } else if (msg.has_sensor()) {
                ProtoMessage::SensorMessage smsg = msg.sensor();
                load = smsg.workload();
            }
        } 
        if(runing) {
            
            for (int i = 0; i < load; ++i)
            {
                loadGPU();
            }
            std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
            get_mon_data_GPU(&mon_data);
            get_mon_data_CPU(&mon_data);
            get_mon_data_MEM(&mon_data);
            ProtoMessage sendMsg;
            sendMsg.set_dest_device(id);
            sendMsg.set_signal(ProtoMessage::DEFAULT);
            ProtoMessage::JammerMessage *jmsg = sendMsg.mutable_jammer();
            jmsg->set_performacnce(getTimeSinceLastNano(start));
            jmsg->set_gpuutil(mon_data.cpu_util);
            jmsg->set_cpumemutil(mon_data.mem);
            jmsg->set_gpumemutil(mon_data.utilGPU_mem);
            jmsg->set_gpuutil(mon_data.utilGPU_cpu);
            jmsg->set_workload(load);
            jmsg->set_app(mon_data.app);
            sendMsg.set_time(getCurrrentimeInMicroseconds());
            sendProtobufMessage(drfmSocket,sendMsg);
            cout <<"\t\tCPU : "<<mon_data.cpu_util<<endl;
            cout <<"\t\tMemory :"<<mon_data.mem<<endl;
        }
    }
	google::protobuf::ShutdownProtobufLibrary();
	return 0;
}

void start(const char * idc){
    
    id = string(idc);
    resetFilename = id+"reset";
    runing = true;
    load = 1;

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

    // Connect DRFM();

    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; 
    struct addrinfo *result,*rp;   
    getaddrinfo(name2hostname["DRFM"].c_str(),
                Name2port[id].c_str(),
                &hints,
                &result);

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        drfmSocket = socket(rp->ai_family, rp->ai_socktype,
                     rp->ai_protocol);
        if (drfmSocket == -1) continue;
            struct timeval t;    
            t.tv_sec = 1;
            t.tv_usec = 0;
            setsockopt(
                drfmSocket,     // Socket descriptor
                SOL_SOCKET, // To manipulate options at the sockets API level
                SO_RCVTIMEO,// Specify the receiving or sending timeouts 
                (const void *)(&t), // option values
                sizeof(t) 
            );
        if (connect(drfmSocket, rp->ai_addr, rp->ai_addrlen) != -1) {
            cout << "Connected to DRFM" << endl;
            break;                  /* Success */
       }
       close(drfmSocket);
    }
    if (rp == NULL) {
        cerr <<"Failed to connect to DRFM\n";
        exit(1);
    }
    freeaddrinfo(result);
    
    ProtoMessage msg;
    msg.set_dest_device(id);
    if(fexists(resetFilename)){
        long time = getTimeFromFile(resetFilename);
        long now = getCurrrentimeInMicroseconds();
        cout <<"File time " <<time;
        cout << ", current time " <<now<<endl;
        cout <<"Time different :" << now - time<<endl;
        long diff = now - time;
        if (diff < 0) diff = 0;
        msg.set_signal(ProtoMessage::RESET);
        msg.set_time(diff);
    } else {
        msg.set_signal(ProtoMessage::START);
    }
    sendProtobufMessage(drfmSocket,msg);
    
}

long getTimeFromFile(string  filename){
    FILE * pFile;
    char buffer [100];
    pFile = fopen (filename.c_str() , "r");
    if (pFile == NULL) perror ("Error opening file");
    else
    {
        fgets (buffer , 100 , pFile);
        fclose (pFile);
    }
    return atof(buffer);
}
void terminator(ProtoMessage msg){
    cout << "Caught terminating signal, exiting...\n";
    runing = false;
    sendProtobufMessage(drfmSocket,msg);
    remove(resetFilename.c_str());

}
void resetDevice(ProtoMessage msg){
    runing = false;
    ofstream ofs;
    ofs.open (resetFilename, std::ofstream::out);
    ofs << getCurrrentimeInMicroseconds();
    ofs.close();
    sendProtobufMessage(drfmSocket,msg);
    google::protobuf::ShutdownProtobufLibrary();
    
    exit(1);
}
bool fexists(string filename)
{
  ifstream ifile(filename);
  return ifile;
}
void get_mon_data_CPU(mon_rec *mon_data)
{
    struct tms timeSample;   
    clock_t now;
    double percent;
    now = times(&timeSample);
    if (now <= lastCPU || timeSample.tms_stime < lastSysCPU ||
        timeSample.tms_utime < lastUserCPU){
        unsigned long
        num_bins = 100 + 1,
        num_rand = (unsigned long) RAND_MAX + 1,
        bin_size = num_rand / num_bins,
        defect   = num_rand % num_bins;
        long x;
        do
            x = random();
        while (num_rand - defect <= (unsigned long)x);
        percent =  x/bin_size;
    }
    else{
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

    // printf("Found %d device(s):\n", deviceCount);

    for (i = 0; i < /*deviceCount*/ 1; i++)
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


bool checkResult(float *hostRef, float *gpuRef, const int N)
{
    //double epsilon = 1.0E-8; //Jammer 1 and 3
    double epsilon = -20.0E-8; // Jammer 2
    bool match = 1;
    for (int i = 0; i < N; i++)
    {
        if (abs(hostRef[i] - gpuRef[i]) > epsilon)
        {
            match = 0;
            // printf("Arrays do not match!\n");
            // printf("host %5.2f gpu %5.2f at current %d\n", hostRef[i],
            //        gpuRef[i], i);
            break;
        }
    }

    // if (match) printf("Arrays match.\n\n");
    return match;
}

void initialData(float *ip, int size)
{
    // generate different seed for random number
    time_t t;
    srand((unsigned) time(&t));

    for (int i = 0; i < size; i++)
    {
        ip[i] = (float)( rand() & 0xFF ) / 10.0f;
    }

    return;
}

void sumArraysOnHost(float *A, float *B, float *C, const int N)
{
    for (int idx = 0; idx < N; idx++)
    {
        C[idx] = A[idx] + B[idx];
    }
}
__global__ void sumArraysOnGPU(float *A, float *B, float *C, const int N)
{
    int i = blockIdx.x * blockDim.x + threadIdx.x;

    if (i < N) C[i] = A[i] + B[i];
}
bool loadGPU()
{
    int dev = 0;
    cudaDeviceProp deviceProp;
    CHECK(cudaGetDeviceProperties(&deviceProp, dev));
    printf("Using Device %d: %s\n", dev, deviceProp.name);
    CHECK(cudaSetDevice(dev));

    // set up data size of vectors
    int nElem = 1 << 24;
    // printf("Vector size %d\n", nElem);

    // malloc host memory
    size_t nBytes = nElem * sizeof(float);

    float *h_A, *h_B, *hostRef, *gpuRef;
    h_A     = (float *)malloc(nBytes);
    h_B     = (float *)malloc(nBytes);
    hostRef = (float *)malloc(nBytes);
    gpuRef  = (float *)malloc(nBytes);

    double iStart, iElaps;

    // initialize data at host side
    iStart = seconds();
    initialData(h_A, nElem);
    initialData(h_B, nElem);
    iElaps = seconds() - iStart;
    // printf("initialData Time elapsed %f sec\n", iElaps);
    memset(hostRef, 0, nBytes);
    memset(gpuRef,  0, nBytes);

    // add vector at host side for result checks
    iStart = seconds();
    sumArraysOnHost(h_A, h_B, hostRef, nElem);
    iElaps = seconds() - iStart;
    // printf("sumArraysOnHost Time elapsed %f sec\n", iElaps);

    // malloc device global memory
    float *d_A, *d_B, *d_C;
    CHECK(cudaMalloc((float**)&d_A, nBytes));
    CHECK(cudaMalloc((float**)&d_B, nBytes));
    CHECK(cudaMalloc((float**)&d_C, nBytes));

    // transfer data from host to device
    CHECK(cudaMemcpy(d_A, h_A, nBytes, cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_B, h_B, nBytes, cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_C, gpuRef, nBytes, cudaMemcpyHostToDevice));
   // nvmlReturn_t r;

    // invoke kernel at host side
    int iLen = 512;
    dim3 block (iLen);
    dim3 grid  ((nElem + block.x - 1) / block.x);
    for (int i=1; i>0; i--) {
    iStart = seconds();
    sumArraysOnGPU<<<grid, block>>>(d_A, d_B, d_C, nElem);
    CHECK(cudaDeviceSynchronize());
    iElaps = seconds() - iStart;
    // printf("sumArraysOnGPU <<<  %d, %d  >>>  Time elapsed %f sec\n", grid.x,
    //        block.x, iElaps);
    }
    // check kernel error
    //nvmlReturn_t r;
    CHECK(cudaGetLastError()) ; 

    // copy kernel result back to host side
    CHECK(cudaMemcpy(gpuRef, d_C, nBytes, cudaMemcpyDeviceToHost));

    // check device results
    bool match = checkResult(hostRef, gpuRef, nElem);

    // free device global memory
    CHECK(cudaFree(d_A));
    CHECK(cudaFree(d_B));
    CHECK(cudaFree(d_C));
    // free host memory
    free(h_A);
    free(h_B);
    free(hostRef);
    free(gpuRef);

    return match;

}


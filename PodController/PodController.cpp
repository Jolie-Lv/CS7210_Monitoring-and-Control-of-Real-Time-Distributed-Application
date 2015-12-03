#include "PodController.h"




int main(int argc, char const *argv[])
{
	startAll();
	signal(SIGINT, terminateAll);
	runing = true;
	while(runing) {
		
		ProtoMessage msg = receiveProtobufMessage(drfmSocket);
		string device = msg.dest_device();
	    if(Name2port.find(device)!=Name2port.end())
	    {
			if(msg.signal() == ProtoMessage::START){
				cout <<device <<" Started!"<<endl;
				deviceStatus[device] = true;
			} else if (msg.signal() == ProtoMessage::RESET){
				cout <<device <<" Reset Successful!"<<endl;
				deviceStatus[device] = true;
				if(deviceResetTimer[device] > 0){
					cout <<"\t"<< device<<" Reset round trip Time : "
					<<getCurrrentimeInMicroseconds() -deviceResetTimer[device]
					<<" microseconds"<<endl
					<<"\t restart time for "<<device<<" is: " <<msg.time()<<endl;
				}
			}
			if(msg.has_jammer()){

				long t1 = msg.time();
				long t2 =  getCurrrentimeInMicroseconds();
				cout <<"\t"<< device<<" Data collection time :"
					 <<msg.jammer().performacnce() <<" microseconds"<<endl
					 <<"\tTime to send data : "<<t2-t1<<" microseconds"<<endl;

				checkJammer(msg);
				jammerStatus[device] = msg.jammer();
				
			} 
	    }
	 //    cout << "Current device deviceStatus :"<<endl;
		// for(auto& pair : deviceStatus) {
		// 	cout <<"\t"<<pair.first<<" : "<<((pair.second)?"ON":"OFF")<<endl;
		// 	if(pair.first!="DRFM"){
		// 		cout<<"\tCpu: "<<jammerStatus[pair.first].cpuutil()<<endl;
		// 		cout<<"\tMemory: "<<jammerStatus[pair.first].cpumemutil()<<endl;
		// 		cout<<"\tGPU: "<<jammerStatus[pair.first].gpuutil()<<endl;
		// 		cout<<"\tGPU Memory: "<<jammerStatus[pair.first].gpumemutil()<<endl;
		// 		cout<<"\tWork Load: "<<jammerStatus[pair.first].workload()<<endl;

		// 	}
		// 	cout<<"\t==================="<<endl;
		// }
	    // sleep(1);
		
	}
	google::protobuf::ShutdownProtobufLibrary();
	return 0;
}
void checkJammer(ProtoMessage msg){
	double t1 = getCurrrentimeInMicroseconds();
	if (msg.jammer().gpuutil() > 30)
	{
		resetDevice(msg.dest_device());
		deviceResetTimer[msg.dest_device()] = getCurrrentimeInMicroseconds();
		cout << "\tRestarting " <<msg.dest_device()
			 <<" due to GPU :" <<msg.jammer().gpuutil()<<endl;
	}
	double t2 = getCurrrentimeInMicroseconds();
	cout <<"\tTime for pod to check data : "<<t2-t1<<" microseconds"<<endl;
	
}
void resetDevice(string device){
	ProtoMessage msg;
	msg.set_signal(ProtoMessage::RESET);
	msg.set_dest_device(device);
	msg.set_time(-1);
	sendProtobufMessage(drfmSocket,msg);
	deviceStatus[device] = false;
}
// void startDevice(string device){
// 	if (device == "DRFM") return startAll();
// 	ProtoMessage msg;
// 	msg.set_signal(ProtoMessage::START);
// 	msg.set_dest_device(device);
// 	sendProtobufMessage(drfmSocket,msg);
// 	deviceStatus[device] = false;
// }

void startAll()
{
	// Create/Bind Socket for DRFM
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; 	 
    hints.ai_flags = AI_PASSIVE; 

    struct addrinfo *result,*rp;
 	int s = getaddrinfo(NULL,Name2port["DRFM"].c_str(),&hints,&result);
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        drfmSocket = socket(rp->ai_family, rp->ai_socktype,
                rp->ai_protocol);
         

        if (drfmSocket == -1)continue;
        int enable = 1;
		setsockopt(
			drfmSocket,
			SOL_SOCKET,
			SO_REUSEADDR,
			(const void *)(&enable),
			sizeof(int)
		);

		
       	if (bind(drfmSocket, rp->ai_addr, rp->ai_addrlen) == 0){
            break;                  /* Success */
       	}
        else 
        	fprintf(stderr,
        		"Error binding to socket, make sure nothing else is listening on this port %d, DRFM\n",
        		errno);
       close(drfmSocket);
    }
	if(listen(drfmSocket, 10) == -1){
		fprintf(stderr, "Error listening %d\n",errno);
		exit(1);
	}
	if (rp == NULL) {
        cerr <<"Failed to connect to DRFM"<<endl;
        exit(1);
    }
	freeaddrinfo(result);
	// Starting DRFM
	string drfmConnCmd = 
		"ssh " + 
		name2hostname["DRFM"] + 
		" nohup ~/CS7210/DRFM/DRFM "+
		"> ~/CS7210/log/DRFM.out "+
		"2> ~/CS7210/log/DRFM.err "+
		"< /dev/null &";
	cout<<exec(drfmConnCmd.c_str());

	drfmSocket = accept(drfmSocket, NULL, NULL);
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
	while(receiveProtobufMessage(drfmSocket).signal() != ProtoMessage::START);
	deviceStatus["DRFM"] = true;
}
void terminateDevice(string device){
	ProtoMessage msg;
	cout <<"Terminating" <<device <<endl;
	msg.set_signal(ProtoMessage::TERMINATE);
	msg.set_dest_device(device);
	sendProtobufMessage(drfmSocket,msg);
}
void terminateAll(int)
{
	printf("Caught signal TERMINATE, exiting...\n");
	terminateDevice("DRFM");
	while(runing) {
	    ProtoMessage msg = receiveProtobufMessage(drfmSocket);
	    string device = msg.dest_device();
	    if(Name2port.find(device)!=Name2port.end()){
	    	cout << "Message from " << device <<endl;
		    	if(msg.signal() == ProtoMessage::TERMINATE) {
		    	deviceStatus.erase(device); 
		    }
		    cout << device << " TERMINATED!!"<<endl;
		    if(deviceStatus.size() <= 0) {
		    	runing = false;
		    }
	    }
	}
}
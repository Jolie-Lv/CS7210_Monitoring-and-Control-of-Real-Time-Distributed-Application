#include "DRFM.h"

int main(int argc, char const *argv[])
{
	startAll();
	signal(SIGINT, terminateAll);
	while(runing) {

        ProtoMessage msg = receiveProtobufMessage(name2socket["pod"]);
        string device = msg.dest_device();
        cout << "Waiting on message " << device<< endl;
        if(Name2port.find(device) != Name2port.end()){
        	if( msg.signal() == ProtoMessage::TERMINATE) {
        		cout <<device <<" is terminating"<<endl;
        		terminateDevice(device);
	        	
	        }  else if (msg.signal() == ProtoMessage::RESET){
				resetDevice(device);
			}
        }
    }
    google::protobuf::ShutdownProtobufLibrary();
    cout <<"termination Successful!!"<<endl;
	return 0;
}
void terminateAll(int signum)
{
	printf("Caught signal terminate from %s, exiting...\n",signum ? "User" : "PodController");
	runing = false;
	for (auto& pair : Name2port){
		if(pair.first != self){
			cout <<pair.first <<" is terminating"<<endl;
			terminateDevice(pair.first);
		}
	}

}
void terminateDevice(string device){
	ProtoMessage msg;
	msg.set_signal(ProtoMessage::TERMINATE);
	msg.set_dest_device(device);
	if (device == self) {
		terminateAll(0);
		sendProtobufMessage(name2socket["pod"],msg);
	} else{
		sendProtobufMessage(name2acceptor[device],msg);
		cout <<"Stopping " <<device <<endl;
		device2Thread[device].join();
		cout<<device <<" Stopped"<<endl;
	}
}

void resetDevice(string device){
	ProtoMessage msg;
	msg.set_signal(ProtoMessage::RESET);
	msg.set_dest_device(device);
	msg.set_time(-1);
	sendProtobufMessage(name2acceptor[device],msg);
	device2Thread[device].join();
	device2Thread.erase(device);
	close(name2acceptor[device]);
	cout <<"Reseting " <<device <<endl;
	startDevice(device);
}
void startDevice(string device){
	// Starting DRFM
	if (device == self || Name2port.find(device) == Name2port.end()) return;
	string deviceConnCmd =
		"ssh " + 
		name2hostname[device] + 
		" nohup ~/CS7210/" + device2Name[device] + "/" +device2Name[device]+device2NameNumber[device]+
		" > ~/CS7210/log/"+device+".out "+
		"2> ~/CS7210/log/"+device+".err "+
		"< /dev/null &";
	cout<<exec(deviceConnCmd.c_str());
	// Accept socket into acceptor
	name2acceptor[device] = accept(name2socket[device] ,NULL, NULL);
	if(name2acceptor[device] < 0)
		cerr <<"Failed to accept to " <<device<<endl;
	
	device2Thread[device] = thread(startDeviceThread,device);
}
void startDeviceThread(string device){
	bool theadRuning = true;
	while(theadRuning) {
		ProtoMessage msg = receiveProtobufMessage(name2acceptor[device]);
        if(msg.dest_device() == device){
        	if (msg.has_sensor()){
			 	for(auto& pair : name2acceptor){
			 		if(pair.first != device)
			 			sendProtobufMessage(name2acceptor[pair.first],msg);
			 	}
			} else if(msg.signal() == ProtoMessage::TERMINATE ){
				theadRuning = false;
				sendProtobufMessage(name2socket["pod"],msg);
			} else if(msg.signal() == ProtoMessage::RESET && msg.time() < 0){
				theadRuning = false;
			} else {
				sendProtobufMessage(name2socket["pod"],msg);
			}
        }
	}
}
bool startAll(){
	runing = true;
	for (auto& pair: Name2port)
	{
		if(pair.first == self) {
			// continue;
			// 1. Connect to Pod
			struct addrinfo hints;
			memset(&hints, 0, sizeof(struct addrinfo));
		    hints.ai_family = AF_INET;    /* Allow IPv4 or IPv6 */
		    hints.ai_socktype = SOCK_STREAM; 

		    
			struct addrinfo *result,*rp;
			getaddrinfo(name2hostname["pod"].c_str(),
						pair.second.c_str(),
						&hints,
						&result);
		 	for (rp = result; rp != NULL; rp = rp->ai_next) {
		        name2socket["pod"] = socket(rp->ai_family, rp->ai_socktype,
		                     rp->ai_protocol);
		        if (name2socket["pod"] == -1)continue;
		        struct timeval t;    
				t.tv_sec = 1;
				t.tv_usec = 0;
		        setsockopt(
					name2socket["pod"],     // Socket descriptor
					SOL_SOCKET, // To manipulate options at the sockets API level
					SO_RCVTIMEO,// Specify the receiving or sending timeouts 
					(const void *)(&t), // option values
					sizeof(t) 
				);
		       	if (connect(name2socket["pod"], rp->ai_addr, rp->ai_addrlen) != -1) {
		       		cout << "Connected to pod"<<endl;
		            break;                  /* Success */
		       	}
		       	close(name2socket["pod"]);
		    }
		    if (rp == NULL) {
		        cerr <<"Failed to connect to Pod\n";
		        exit(1);
		    }
		    freeaddrinfo(result);
		    ProtoMessage msg;
			msg.set_signal(ProtoMessage::START);
			msg.set_dest_device(self);
			sendProtobufMessage(name2socket["pod"],msg);
		} else {
			struct addrinfo hints;
			memset(&hints, 0, sizeof(struct addrinfo));
		    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
		    hints.ai_socktype = SOCK_STREAM; 	 
		    hints.ai_flags = AI_PASSIVE; 

		    struct addrinfo *result,*rp;
		 	getaddrinfo(NULL,pair.second.c_str(),&hints,&result);
		    for (rp = result; rp != NULL; rp = rp->ai_next) {
		        name2socket[pair.first] = socket(rp->ai_family, rp->ai_socktype,
		                rp->ai_protocol);
		        if (name2socket[pair.first] == -1) continue;

		        int enable = 1;
		        setsockopt(
					name2socket[pair.first],
					SOL_SOCKET,
					SO_REUSEADDR,
					(const void *)(&enable),
					sizeof(int)
				);
		        struct timeval t;    
				t.tv_sec = 1;
				t.tv_usec = 0;
				setsockopt(
					name2socket[pair.first],     // Socket descriptor
					SOL_SOCKET, // To manipulate options at the sockets API level
					SO_RCVTIMEO,// Specify the receiving or sending timeouts 
					(const void *)(&t), // option values
					sizeof(t) 
				);
		       	if (bind(name2socket[pair.first], rp->ai_addr, rp->ai_addrlen) == 0)
		            break;                  /* Success */
		       	close(name2socket[pair.first]);
		    }
			listen(name2socket[pair.first], 10);
			if (rp == NULL) {
		        cerr <<"Failed to connect to "<<pair.first<<endl;
		        exit(1);
		    }
			freeaddrinfo(result);
			startDevice(pair.first);
			
		}
	}
	
}



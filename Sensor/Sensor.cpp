#include "Sensor.h"


int main(int argc, char const *argv[])
{
	start();
	while(runing) {
    cout<<"Waiting on DRFM Message every second..."<<endl;
    ProtoMessage msg = receiveProtobufMessage(drfmSocket);
    if(msg.dest_device() =="Sensor"){
      if(msg.signal() == ProtoMessage::TERMINATE)
        terminator(msg);
      else {
        ProtoMessage sendMsg;
        sendMsg.set_signal(ProtoMessage::DEFAULT);
        sendMsg.set_dest_device("Sensor");
        ProtoMessage::SensorMessage * smsg = sendMsg.mutable_sensor();
        smsg->set_workload(load++%3);
        sendProtobufMessage(drfmSocket,sendMsg);
      }
    }
    sleep(1);
  }
  google::protobuf::ShutdownProtobufLibrary();
	return 0;
}
void terminator(ProtoMessage msg){
	printf("Caught signal terminate, coming out...\n");
  runing = false;
  sendProtobufMessage(drfmSocket,msg);
}
void start(){
  runing = true;

  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;    /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_STREAM; 
  struct addrinfo *result,*rp;   
  getaddrinfo(name2hostname["DRFM"].c_str(),
    					Name2port["Sensor"].c_str(),
    					&hints,
    					&result);
 	for (rp = result; rp != NULL; rp = rp->ai_next) {
        drfmSocket = socket(rp->ai_family, rp->ai_socktype,
                     rp->ai_protocol);
    if (drfmSocket == -1) continue;
   	if (connect(drfmSocket, rp->ai_addr, rp->ai_addrlen) != -1){
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
      cout << "Connected to DRFM" << endl;
   		break;/* Success */
   	}
    close(drfmSocket);
  }
  if (rp == NULL) {
  	cerr <<"Failed to connect to DRFM\n";
  	exit(1);
  }
  freeaddrinfo(result);
  ProtoMessage msg;
  msg.set_signal(ProtoMessage::START);
  msg.set_dest_device("Sensor");
  sendProtobufMessage(drfmSocket,msg);
}

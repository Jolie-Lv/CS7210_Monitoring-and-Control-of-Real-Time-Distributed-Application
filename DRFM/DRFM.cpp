#include "DRFM.h"

int main(int argc, char const *argv[])
{
	DRFM drfm;
	drfm.initialConnection();
	// More logic to be added here
	return 0;
}

bool DRFM::initialConnection(){
	tcpConnectionSetup();
	tcpAcceptorSetup();
	string sensorConnCmd = 
		"ssh " + 
		name2hostname["sensor"] + 
		" nohup ~/CS7210/Sensor/Sensor "+
		"> ~/CS7210/log/Sensor.out "+
		"2> ~/CS7210/log/Sensor.err "+
		"< /dev/null &";
	string jammer1ConnCmd = 
		"ssh " + 
		name2hostname["jammer1"] + 
		" nohup ~/CS7210/Jammer/Jammer 1 "+
		"> ~/CS7210/log/Jammer1.out "+
		"2> ~/CS7210/log/Jammer1.err "+
		"< /dev/null &";
	string jammer2ConnCmd = 
		"ssh " + 
		name2hostname["jammer2"] + 
		" nohup ~/CS7210/Jammer/Jammer 2 "+
		"> ~/CS7210/log/Jammer2.out "+
		"2> ~/CS7210/log/Jammer2.err "+
		"< /dev/null &";
	cout <<exec(sensorConnCmd.c_str())
		 <<exec(jammer1ConnCmd.c_str())
		 <<exec(jammer2ConnCmd.c_str());

	for (auto& pair: name2socket) {
		name2acceptor[pair.first] = accept(pair.second, NULL, NULL);
	}
	cout << "DR: Trying to connect to pod"<<endl;
}

bool DRFM::tcpConnectionSetup(){
	// 1. Connect to Pod
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; 

    
	struct addrinfo *result,*rp;
	getaddrinfo(name2hostname["pod"].c_str(),
				podName2port["drfm"].c_str(),
				&hints,
				&result);
 	for (rp = result; rp != NULL; rp = rp->ai_next) {
        podSocket = socket(rp->ai_family, rp->ai_socktype,
                     rp->ai_protocol);
        if (podSocket == -1)continue;
       	if (connect(podSocket, rp->ai_addr, rp->ai_addrlen) != -1) {
       		cout << "DR: Trying to connect to pod"<<endl;
            break;                  /* Success */
       	}
       	close(podSocket);
    }
    freeaddrinfo(result);
    
}
bool DRFM::tcpAcceptorSetup(){
	for (auto& pair: DrfmName2port)
	{
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
	       	if (bind(name2socket[pair.first], rp->ai_addr, rp->ai_addrlen) == 0)
	            break;                  /* Success */
	       	close(name2socket[pair.first]);
	    }
		listen(name2socket[pair.first], 10);
		freeaddrinfo(result);
	}
}
#include "PodController.h"


int main(int argc, char const *argv[])
{

	PodController pod;
	pod.initialConnection();
	// More logic to be added here
	
	return 0;
}

bool PodController::initialConnection()
{
	tcpAcceptorSetup();
	string drfmConnCmd = 
		"ssh " + 
		name2hostname["drfm"] + 
		" nohup ~/CS7210/DRFM/DRFM "+
		"> ~/CS7210/log/DRFM.out "+
		"2> ~/CS7210/log/DRFM.err "+
		"< /dev/null &";
	cout<<exec(drfmConnCmd.c_str());
	for (auto& pair: name2socket) 
		name2acceptor[pair.first] = accept(pair.second, NULL, NULL);
}

bool PodController::tcpAcceptorSetup(){

	for (auto& pair: podName2port)
	{
		struct addrinfo hints;
		memset(&hints, 0, sizeof(struct addrinfo));
	    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
	    hints.ai_socktype = SOCK_STREAM; 	 
	    hints.ai_flags = AI_PASSIVE; 

	    struct addrinfo *result,*rp;
	 	int s = getaddrinfo(NULL,pair.second.c_str(),&hints,&result);
	    for (rp = result; rp != NULL; rp = rp->ai_next) {
	        name2socket[pair.first] = socket(rp->ai_family, rp->ai_socktype,
	                rp->ai_protocol);
	        if (name2socket[pair.first] == -1)continue;
	       	if (bind(name2socket[pair.first], rp->ai_addr, rp->ai_addrlen) == 0)
	            break;                  /* Success */
	       close(name2socket[pair.first]);
	    }
		listen(name2socket[pair.first], 10);
		freeaddrinfo(result);
	}
}
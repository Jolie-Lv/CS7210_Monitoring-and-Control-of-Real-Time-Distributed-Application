#include "Sensor.h"

int main(int argc, char const *argv[])
{
	Sensor sensor;
	sensor.initialConnection();
	return 0;
}

bool Sensor::initialConnection(){
	tcpConnectionSetup();
}
bool Sensor::tcpConnectionSetup(){
	// 1. Connect to Pod
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; 
	struct addrinfo *result,*rp;
	int s = getaddrinfo(name2hostname["pod"].c_str(),
						podName2port["sensor"].c_str(),
						&hints,
						&result);
 	for (rp = result; rp != NULL; rp = rp->ai_next) {
        podSocket = socket(rp->ai_family, rp->ai_socktype,
                     rp->ai_protocol);
        if (podSocket == -1) continue;
       	if (connect(podSocket, rp->ai_addr, rp->ai_addrlen) != -1)
       		break;/* Success */
       close(podSocket);
    }
   	freeaddrinfo(result);
    // 2. Connect to DRFM
	s = getaddrinfo(name2hostname["drfm"].c_str(),
						DrfmName2port["sensor"].c_str(),
						&hints,
						&result);
 	for (rp = result; rp != NULL; rp = rp->ai_next) {
        drfmSocket = socket(rp->ai_family, rp->ai_socktype,
                     rp->ai_protocol);
        if (drfmSocket == -1) continue;
       	if (connect(drfmSocket, rp->ai_addr, rp->ai_addrlen) != -1)
       		break;/* Success */
       close(drfmSocket);
    }
    freeaddrinfo(result);
}
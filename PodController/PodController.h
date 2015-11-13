
#include "global.h"
class PodController {
public:
	bool initialConnection();
	std::map<string, int> name2acceptor;
private:
	std::map<string, int> name2socket;
	/*
	* This function should be run before start connection
	*/
	bool tcpAcceptorSetup();
};
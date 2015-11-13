
#include "global.h"

class DRFM
{
public:
	bool initialConnection();
	std::map<string, int> name2acceptor;
	int podSocket;

private:
	std::map<string, int> name2socket;
	bool tcpAcceptorSetup(); // accecptor before connection
	bool tcpConnectionSetup();
};
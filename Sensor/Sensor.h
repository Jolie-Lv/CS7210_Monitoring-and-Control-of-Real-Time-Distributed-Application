
#include "global.h"

class Sensor {
public:
	bool initialConnection();
	int podSocket;
	int drfmSocket;
private:
	bool tcpConnectionSetup();
};

#include "global.h"

class Jammer {
public:
	Jammer();
	Jammer(const char * id);
	bool initialConnection();

	void get_mon_data_MEM(mon_rec *mon_data);
	void get_mon_data_GPU(mon_rec *mon_data);
	void get_mon_data_CPU(mon_rec *mon_data);
	int podSocket;
	int drfmSocket;
	int id;
	mon_rec mon_data;
	
private:

	bool tcpConnectionSetup();
};
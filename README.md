# CS7210_Final_Project

Alan Nussbuam


Jian Chen

==============Introduction===============
This project created a distributed Cyber Electronic Warfare (EW) application that requires coordination and consistency across all the distributed functions that are executing in various nodes.This project is the distributed architecture that we created to support the Cyber EW mission.
All software was written in C++ and CUDA for there are 5 software components of our distributed system:
1.	PodController – Software lifecycle, collect all global monitor data and provide system control.
2.	Sensor Data (Radar Simulator) – Receive radar data and format simulated data then transmit results to DFRM.
3.	DFRM – Process radar type data that was filtered and clustered to data for the Jammer(s).
4.	3 Jammer(s) – Each Jammer application algorithm is in CUDA and capability of collecting and Host and Device  metrics.

============Buidling Project=============
Before building this project, please make sure that the root folder name for this project is /CS7210/

1. Steps to build :
  	$ ssh to node1
  	$ make

3. To clear build
	
	$ make clean

4. To run:
  	$ ./main 

5. Steps to change Nodes and ports for each processes to run on:
	$ cd global
	$ vim global.h
	// Change "std::map<string, string> name2hostname"  for hostnames
	// change "std::map<string, string> Name2port" for each ports

6. Logs for each processes are in ~/CS7210/log/

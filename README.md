# CS7210_Final_Project
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

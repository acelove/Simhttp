#include "master.h"

#include <iostream>

int main(int argc, char** argv){
	Master master("127.0.0.1",8000);
	std::cout << "Simhttp"  << std::endl;
	if(!master.StartMaster())
		std::cout << "Start Master fail" << std::endl;
	std::cout << "Simhttp Closed!" << std::endl;
	return 0;
}
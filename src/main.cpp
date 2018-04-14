#include "master.h"

#include <iostream>

int main(int argc, char** argv){
	Master master;
	std::cout << "Simhttp"  << std::endl;
	if(!master.StartMaster())
		std::cout << "Start Master fail" << std::endl;
	std::cout << "Simhttp Closed!" << std::endl;
	return 0;
}
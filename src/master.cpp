#include "master.h"
#include "worker.h"

#include <iostream>

Master :: Master(){
	num_of_child = 4;
	m_base = nullptr;
	m_exit_event = nullptr;
	m_chld_event = nullptr;
}

Master :: ~Master(){
	if(m_base){
		event_free(m_exit_event);
		event_free(m_chld_event);
		event_base_free(m_base);
	}
	std::cout << "Master Closed" << std::endl;
}

bool Master :: StartMaster(){
	std::cout << "Master Start" << std::endl;
	worker.master = this;

	while(num_of_child){
		switch(fork()){
			case -1:
			    return false;
			case 0 :{
				worker.Run();
			    return true;
			}
			default:
			    num_of_child--;
			    break;
		}
	}

	m_base = event_base_new();
	m_exit_event = evsignal_new(m_base,SIGINT,Master::MasterExitSignal,m_base);
	m_chld_event = evsignal_new(m_base,SIGCHLD,Master::MasterChldSignal,this);
	evsignal_add(m_exit_event,NULL);
	evsignal_add(m_chld_event,NULL);

	event_base_dispatch(m_base);
	return true;
}

void Master::MasterExitSignal(evutil_socket_t signo,short event,void* arg){
	//kill(0,SIGINT);
	event_base_loopexit((struct event_base*)arg,NULL);
}

void Master::MasterChldSignal(evutil_socket_t signo,short event,void* arg){
	Master* master = (Master*)arg;
	pid_t  pid;
	int    stat;
	while((pid = waitpid(-1,&stat,WNOHANG))>0){
		master->num_of_child--;
		std::cout << "Child" << pid << "terminated" <<std::endl;
	}
} 
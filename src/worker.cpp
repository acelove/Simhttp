#include "master.h"
#include "worker.h"

#include <stdlib.h>
#include <iostream>

Worker::Worker(){
	master = nullptr;
	w_base = nullptr;
	w_exit_event = nullptr;
}

Worker::~Worker(){
	if(w_exit_event)
		event_free(w_exit_event);
	if(w_base)
		event_base_free(w_base);
	std::cout << "Worker Closed!" << std::endl;
}

void Worker::WorkerExitSignal(evutil_socket_t signo,short event,void* arg){
	event_base_loopexit((struct event_base*)arg,NULL);
}

void Worker::Run(){
	w_base = event_base_new();
	w_exit_event = evsignal_new(w_base,SIGINT,Worker::WorkerExitSignal,w_base);
	evsignal_add(w_exit_event,NULL);

	event_base_dispatch(w_base);
	return ;
}
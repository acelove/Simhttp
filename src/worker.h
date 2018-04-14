#ifndef _WORKER_H
#define _WORKER_H

#include <string>
#include <map>

#include "event2/event.h"
#include "event2/util.h"

class Master;

class Worker{
public:
	Worker();
	~Worker();

	void Run();

	static void WorkerExitSignal(evutil_socket_t signo,short event,void* arg);

	Master            *master;

	struct event_base *w_base;
	struct event      *w_exit_event;


};
#endif
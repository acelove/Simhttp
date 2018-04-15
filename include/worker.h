#ifndef _WORKER_H
#define _WORKER_H

#include <string>
#include <map>

#include "event2/event.h"
#include "event2/util.h"

#include "listener.h"
#include "connection.h"

#include "util.h"

class Master;

class Worker{
public:
	typedef std::map<evutil_socket_t,Connection*> ConnectionMap;
	Worker(const std::string &ip,unsigned short port);
	~Worker();

	void Run();

	static void WorkerExitSignal(evutil_socket_t signo,short event,void* arg);

	Master            *master;

	struct event_base *w_base;
	struct event      *w_exit_event;

    Listener           listener;
    ConnectionMap      con_map;
};
#endif
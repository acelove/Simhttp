#ifndef _LISTENER_H
#define _LISTENER_H

#include <string>

#include "event2/event.h"
#include "event2/util.h"

#include "util.h"

class Worker;

class Listener{
public:
	Listener(const std::string& ip,unsigned short port);
	~Listener();

	bool InitListener(Worker *worker);
	void AddListenEvent();

	static void ListenEventCallback(evutil_socket_t fd,short event,void* arg);

	Worker              *listen_worker;
	evutil_socket_t      listen_sockfd;
	struct sockaddr_in   listen_addr;
	struct event        *listen_event;
	uint64_t             cnt_connection;   
};
#endif
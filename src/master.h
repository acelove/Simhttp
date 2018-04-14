#ifndef _MASTER_H
#define _MASTER_H

#include "worker.h"

#include <string>

#include "event2/event.h"
#include "event2/util.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

class Master{
public:

	Master();
	~Master();

	bool StartMaster();

	static void MasterExitSignal(evutil_socket_t signo,short event,void* arg);//SIGINT信号回调函数
	static void MasterChldSignal(evutil_socket_t signo,short event,void* arg);

	Worker worker;

	struct event_base *m_base;
	struct event      *m_exit_event;
	struct event      *m_chld_event;

	int               num_of_child;

};

#endif
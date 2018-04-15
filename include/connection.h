#ifndef _CONNECTION_H
#define _CONNECTION_H

#include <string>
#include <queue>

#include "event2/event.h"
#include "event2/util.h"

#include "util.h"

#include "http.h"

class Worker;

class Connection{
public:
	Connection();
	~Connection();

	bool InitConnection(Worker *worker);

public:

	typedef std::queue<HttpRequest*> req_queue_t;

	static void ConEventCallback(evutil_socket_t fd,short event,void *arg);

	Worker           *con_worker;

	evutil_socket_t   con_sockfd;
	struct event     *read_event;
	struct event     *write_event;
    req_queue_t       req_queue;
	std::string       con_inbuf;
	std::string       con_intmp;
	std::string       con_outbuf;

	HttpRequest      *http_request_parser;    //用于解析
	HttpRequest      *http_request_process;   //用于处理请求
	HttpResponse      http_response;
	HttpParser        http_parser;

	static void FreeConnection(Connection *con);
 
private:
	void WantRead();
	void NotWantRead();
	void WantWrite();
	void NotWantWrite();

};
#endif
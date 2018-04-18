#ifndef _CONNECTION_H
#define _CONNECTION_H

#include <string>
#include <queue>

#include "event2/event.h"
#include "event2/util.h"

#include "util.h"

#include "http.h"

typedef enum{

	CON_STATE_CONNECT,
	CON_STATE_REQUEST_START,
	CON_STATE_READ,
	CON_STATE_REQUEST_END,
	CON_STATE_HANDLE_REQUEST,
	CON_STATE_RESPONSE_START,
	CON_STATE_WRITE,
	CON_STATE_RESPONSE_END,
	CON_STATE_ERROR
}connection_state_t;

typedef enum{
	REQ_ERROR,
	REQ_IS_COMPLETE,
	REQ_NOT_COMPLETE
}request_state_t;

class Worker;

class Connection{
public:
	Connection();
	~Connection();

	bool InitConnection(Worker *worker);
	void ResetCon();

public:

	typedef std::queue<HttpRequest*> req_queue_t;

	static void ConEventCallback(evutil_socket_t fd,short event,void *arg);

	Worker           *con_worker;

	evutil_socket_t   con_sockfd;
	struct event     *con_read_event;
	struct event     *con_write_event;
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
	void SetWriteEvent();
	void UnsetWriteEvent();
	void ResetConnection();
	void PrepareResponse();
	void SetErrorResponse();
	bool StateMachine();
	void SetState(connection_state_t state);
	request_state_t GetParsedRequest();

private:
	bool                   con_want_write;
	bool                   con_want_read;
	bool                   con_keep_alive;
    int                    con_req_cnt;

	connection_state_t     con_state;
	request_state_t        req_state;

};
#endif
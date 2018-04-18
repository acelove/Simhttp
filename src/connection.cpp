#include "connection.h"
#include "worker.h"

#include <iostream>

Connection::Connection(){
	con_worker = nullptr;

	con_read_event = nullptr;
	con_write_event = nullptr;
}

Connection::~Connection()
{
	if(con_read_event && con_write_event){
		event_free(con_read_event);
		event_free(con_write_event);
		std::cout << con_sockfd << "closed" << std::endl;
		close(con_sockfd);
	}
}

void Connection::FreeConnection(Connection *con){

	Worker *worker = con->con_worker;

	if(con->con_read_event && con->con_write_event){
		Worker::ConnectionMap::iterator con_iter = worker->con_map.find(con->con_sockfd);
		worker->con_map.erase(con_iter);
	}

	delete con;

}

bool Connection::InitConnection(Worker *worker){
	con_worker = worker;

	try{
		//这里需要内存优化
		con_intmp.reserve(10*1024);
		con_inbuf.reserve(10*1024);
		con_outbuf.reserve(10*1024);

		evutil_make_socket_nonblocking(con_sockfd);
		//test:监听读事件，从客户端度，然后回传给客户端
		con_read_event = event_new(con_worker->w_base,con_sockfd,EV_PERSIST | EV_READ, Connection::ConEventCallback, this);
		con_write_event = event_new(con_worker->w_base,con_sockfd,EV_PERSIST | EV_WRITE,Connection::ConEventCallback,this);		
	}
	catch(std::bad_alloc){
		std::cout << "InitConnection():bad_alloc" << std::endl;
	}

	http_parser.InitParser(this);

	SetState(CON_STATE_REQUEST_START);

	if(!StateMachine()){
		std::cerr<< "Connection::InitConnection(): StateMachine()" << std::endl;
		return false;
	}

	return true;
}

void Connection::ConEventCallback(evutil_socket_t sockfd,short event,void* arg){

	Connection *con = (Connection*)arg;

	if(event & EV_READ){
		int cap = con->con_intmp.capacity();
		int ret = read(sockfd,&con->con_intmp[0],cap);

		if(ret==-1){
			if(errno!=EAGAIN && errno!=EINTR){
				FreeConnection(con);
				return;
			}
		}
		else if(ret==0){
			FreeConnection(con);
			return;
		}
		else{
			con->con_inbuf.clear();
			con->con_inbuf.append(con->con_intmp.c_str(),ret);
		}

	}

	if(event & EV_WRITE){

		std::cout << con->con_outbuf << std::endl;
		int ret = write(sockfd,con->con_outbuf.c_str(),con->con_outbuf.size());
        std::cout << ret << std::endl;
		if(ret==-1){
			if(errno!=EAGAIN && errno!=EINTR){
				FreeConnection(con);
				return;
			}
		}
		else{
			con->con_outbuf.erase(con->con_outbuf.begin(),con->con_outbuf.begin() + ret);
			if(con->con_outbuf.size()==0 && !con->con_want_write)
				con->UnsetWriteEvent();
		}

	}

	if(!con->StateMachine())
		FreeConnection(con);
}

bool Connection::StateMachine(){

	request_state_t req_state;

	while(true){

		switch(con_state){

			case CON_STATE_CONNECT:

			        ResetConnection();
			        break;

			case CON_STATE_REQUEST_START:

			        http_response.ResetResponse();
			        con_req_cnt++;
			        WantRead();
			        SetState(CON_STATE_READ);
			        break;

			case CON_STATE_READ:

			        req_state = GetParsedRequest();
			        if(req_state==REQ_ERROR){
			        	std::cerr << "Connection::StateMachine(): GetParsedRequest()" << std::endl;
			        	return false;
			        }
			        else if (req_state == REQ_IS_COMPLETE){
			        	SetState(CON_STATE_REQUEST_END);
			        	break;
			        }
			        else{
			        	return true;
			        }
			        break;

			case CON_STATE_REQUEST_END:

			        NotWantRead();
			        SetState(CON_STATE_HANDLE_REQUEST);
			        break;

			case CON_STATE_HANDLE_REQUEST:
			        PrepareResponse();
			        SetState(CON_STATE_RESPONSE_START);
			        break;

			case CON_STATE_RESPONSE_START:

			        WantWrite();
			        SetState(CON_STATE_WRITE);
			        break;

			case CON_STATE_WRITE:

			        con_outbuf += http_response.GetResponse();
			        SetState(CON_STATE_RESPONSE_END);
			        break;

			case CON_STATE_RESPONSE_END:

			        NotWantWrite();
			        delete http_request_process;
			        http_request_process = nullptr;
			        http_response.ResetResponse();
			        SetState(CON_STATE_REQUEST_START);
			        break;

			case CON_STATE_ERROR:

			        http_response.ResetResponse();
			        SetErrorResponse();
			        con_outbuf += http_response.GetResponse();
			        if(con_outbuf.empty()){
			        	return false;
			        }
			        return true;

			default:

			        return false;
		}
	}

	return true;
}

void Connection::SetState(connection_state_t state){
	con_state = state;
}

void Connection::WantRead(){
	con_want_read = true;
	event_add(con_read_event,NULL);
}

void Connection::NotWantRead(){
	con_want_read = false;
	event_del(con_read_event);
}

void Connection::WantWrite(){
	con_want_write = true;
	SetWriteEvent();
}

void Connection::NotWantWrite(){
	con_want_write = false;
	//当还有未发送的数据时不去除写事件的监控
	if(!con_outbuf.size())
	    UnsetWriteEvent();
}

void Connection::SetWriteEvent(){
	event_add(con_write_event,NULL);
}

void Connection::UnsetWriteEvent(){
	event_del(con_write_event);
}

void Connection::ResetConnection(){
	http_response.ResetResponse();
	while(!req_queue.empty())
		req_queue.pop();
	con_req_cnt = 0;
}

//for test
void Connection::PrepareResponse(){
	http_response.http_code   = 200;
	http_response.http_phrase = "ok";
	http_response.http_body   = "<html><body>hello,world.</body></html>";
}

void Connection::SetErrorResponse(){
	http_response.http_code   = 500;
	http_response.http_phrase = "Server Error";
}

request_state_t Connection::GetParsedRequest(){

	if(!req_queue.empty()){
		http_request_process = req_queue.front();
		req_queue.pop();
		return REQ_IS_COMPLETE;
	}

	int ret = http_parser.HttpParseRequest(con_inbuf);

	if(ret == -1){
		return REQ_ERROR;
	}

	if(ret == 0){
		return REQ_NOT_COMPLETE;
	}//当读取空串或者不完整的field-value对时，httpparser解析会返回0

	con_inbuf.erase(0,ret);

	if(!req_queue.empty()){
		http_request_process = req_queue.front();
		req_queue.pop();
		return REQ_IS_COMPLETE;
	}

	return REQ_NOT_COMPLETE;
}
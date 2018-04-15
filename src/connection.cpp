#include "connection.h"
#include "worker.h"

#include <iostream>

void Connection::WantRead(){
	event_add(read_event,NULL);
}

void Connection::NotWantRead(){
	event_del(read_event);
}

void Connection::WantWrite(){
	event_add(write_event,NULL);
}

void Connection::NotWantWrite(){
	event_del(write_event);
}

Connection::Connection(){
	con_worker = nullptr;

	read_event = nullptr;
	write_event = nullptr;
}

Connection::~Connection()
{
	if(read_event && write_event){
		event_free(read_event);
		event_free(write_event);
		std::cout << con_sockfd << "closed" << std::endl;
		close(con_sockfd);
	}
}

void Connection::FreeConnection(Connection *con){

	Worker *worker = con->con_worker;

	if(con->read_event && con->write_event){
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
		read_event = event_new(con_worker->w_base,con_sockfd,EV_PERSIST | EV_READ, Connection::ConEventCallback, this);
		write_event = event_new(con_worker->w_base,con_sockfd,EV_PERSIST | EV_WRITE,Connection::ConEventCallback,this);		
	}
	catch(std::bad_alloc){
		std::cout << "InitConnection():bad_alloc" << std::endl;
	}

	http_parser.InitParser(this);

	WantRead();

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

		//在这里测试http-parser的作用
        con->http_parser.HttpParseRequest(con->con_inbuf);
        std::cout << con->req_queue.front()->http_method << std::endl;
        //////////////////////////////////////////////////////////////////////

		con->con_outbuf = con->con_inbuf;
		con->NotWantRead();
		con->WantWrite();
	}

	if(event & EV_WRITE){
		int ret = write(sockfd,con->con_outbuf.c_str(),con->con_outbuf.size());

		if(ret==-1){
			if(errno!=EAGAIN && errno!=EINTR){
				FreeConnection(con);
				return;
			}
		}
		con->NotWantWrite();
		con->WantRead();
	}
}
#include "listener.h"
#include "worker.h"
#include "connection.h"

#include <iostream>

Listener::Listener(const std::string &ip,unsigned short port){
	//ipv4
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = inet_addr(ip.c_str());
	listen_addr.sin_port = htons(port);
	listen_event = nullptr;
	cnt_connection = 0;
	std::cout << "Init Listener" << std::endl;
}

Listener::~Listener(){
	if(listen_event){
		event_free(listen_event);
		close(listen_sockfd);
	}
	std::cout << "Listener closed" << std::endl;
}

bool Listener::InitListener(Worker *worker){
	//创建套接字
	if (-1==(listen_sockfd = socket(AF_INET,SOCK_STREAM,0))){
		return false;
	}

	//非阻塞
	evutil_make_socket_nonblocking(listen_sockfd);

	int reuse = 1;

	//重用
	setsockopt(listen_sockfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));

	if(0!=bind(listen_sockfd,(struct sockaddr*)&listen_addr,sizeof(listen_addr))){
		return false;
	}
	if(0!=listen(listen_sockfd,5)){
		return false;
	}

	listen_worker = worker;
	return true;

}

void Listener::AddListenEvent(){
	listen_event = event_new(listen_worker->w_base,listen_sockfd,EV_READ|EV_PERSIST,ListenEventCallback,this);
	event_add(listen_event,NULL);
}

void Listener::ListenEventCallback(evutil_socket_t sockfd,short event,void* arg){
	evutil_socket_t con_fd;
	struct sockaddr_in con_addr;
	socklen_t addr_len = sizeof(con_addr);
	if(-1==(con_fd=accept(sockfd,(struct sockaddr*)&con_addr,&addr_len))){
		return;
	}


	Listener *listener = (Listener*)arg;
	Connection *con = new Connection();

	con->con_sockfd = con_fd;

	pid_t pid = getpid();
	std::cout << "listen accept" << con->con_sockfd << "by process" << pid << std::endl;

	if(!con->InitConnection(listener->listen_worker)){
		Connection::FreeConnection(con);
		return ;
	}

	con->con_worker->con_map[con->con_sockfd] = con;
	listener->cnt_connection++;

}
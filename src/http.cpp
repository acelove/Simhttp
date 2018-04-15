#include "http.h"

#include <iostream>
#include <sstream>

#include <string>
#include "connection.h"

std::string HttpResponse::GetResponse(){
	std::ostringstream ostream;
	ostream << "HTTP/1.1" << " " << http_code << " " << http_phrase << "\r\n" << "Connection:keep-alive" << "\r\n";

	header_iter_t iter = http_headers.begin();

	while(iter != http_headers.end()){
		ostream << iter->first << ":" << iter->second << "\r\n";
		iter++;
	}
	ostream << "Content-Length:" << http_body.size() << "\r\n\r\n";
	ostream << http_body;

	return ostream.str();
}

void HttpResponse::ResetResponse(){
	//http_version = "HTTP/1.1";
	http_code = 200;
	http_phrase = "OK";

	http_body.clear();
	http_headers.clear();
}

//初始化parser,注册各个阶段的回调函数
void HttpParser::InitParser(Connection *con){
	bzero(&settings,sizeof(settings));
	settings.on_message_begin     = OnMessageBeginCallback;
	settings.on_url               = OnUrlCallback;
	settings.on_header_field      = OnHeaderFieldCallback;
	settings.on_header_value      = OnHeaderValueCallback;
	settings.on_headers_complete   = OnHeadersCompleteCallback;
	settings.on_body              = OnBodyCallback;
	settings.on_message_complete  = OnMessageCompleteCallback;

	http_parser_init(&parser,HTTP_REQUEST);

	parser.data = con;
}

//执行http_parser_execute时将调用一系列回调函数
int HttpParser::HttpParseRequest(const std::string &inbuf){

	int nparsed = http_parser_execute(&parser,&settings,inbuf.c_str(),inbuf.size());

	if(parser.http_errno != HPE_OK){
		return -1;
	}

	return nparsed;
}

//初始化http_request_parser
int HttpParser::OnMessageBeginCallback(http_parser *parser){

	Connection *con = (Connection*)parser->data;

	con->http_request_parser = new HttpRequest();

	return 0;
}

//将解析好的url赋值到请求对象的url成员中
int HttpParser::OnUrlCallback(http_parser *parser,const char *at,size_t length){

	Connection *con = (Connection*)parser->data;

	con->http_request_parser->http_url.assign(at,length);

	return 0;
}

//将解析到的请求头键暂存在http_header_field
int HttpParser::OnHeaderFieldCallback(http_parser *parser,const char *at,size_t length){

	Connection *con = (Connection*)parser->data;

	con->http_request_parser->http_header_field.assign(at,length);

	return 0;
}

//将解析到的请求头部的值与键一一对应
int HttpParser::OnHeaderValueCallback(http_parser *parser,const char *at,size_t length){

	Connection *con = (Connection*)parser->data;

	HttpRequest *request = con->http_request_parser;

	request->http_headers[request->http_header_field] = std::string(at,length);

	return 0;
}

int HttpParser::OnHeadersCompleteCallback(http_parser *parser){

	Connection *con = (Connection*)parser->data;
	HttpRequest *request = con->http_request_parser;
	request->http_method = http_method_str((http_method)parser->method);
	return 0;
}

//请求body的回调，可能调用不止一次，所以用append而不是赋值assign
int HttpParser::OnBodyCallback(http_parser *parser,const char *at,size_t length){

	Connection *con = (Connection*)parser->data;

	con->http_request_parser->http_body.append(at,length);

	return 0;
}

//将解析完成的请求对象加入连接的请求队列中
int HttpParser::OnMessageCompleteCallback(http_parser* parser){

	Connection *con = (Connection*)parser->data;
	HttpRequest *request = con->http_request_parser;

	con->req_queue.push(request);
	con->http_request_parser = NULL;

	return 0;
}
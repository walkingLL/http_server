#include"imgSvr.h"
#include<iostream>

using namespace std;

const char *server::s_http_port = "8000";
struct mg_serve_http_opts server::s_http_server_opts;

void ev_handler(struct mg_connection *nc, int ev, void *ev_data)
{
	cout << "zhoulanlan" << endl;
}

void server::server_init(void)
{
	//初始化
	mg_mgr_init(&this->mgr, NULL);

	s_http_server_opts.document_root = "./";

	//设置
	memset(&this->bind_opts, 0, sizeof(bind_opts));
	bind_opts.error_string = &this->err_str;

	//绑定
	this->nc = mg_bind_opt(&this->mgr, s_http_port, ev_handler, this->bind_opts);
	if (nc == NULL) {
		fprintf(stderr, "Error starting server on port %s: %s\n", s_http_port,
			*bind_opts.error_string);
		exit(1);
	}

	//设置协议
	mg_set_protocol_http_websocket(nc);
}

void server::server_run(void)
{
	printf("Starting RESTful server on port %s, serving %s\n", s_http_port,
		s_http_server_opts.document_root);
	for (;;) {
		mg_mgr_poll(&mgr, 1000);
	}
	mg_mgr_free(&mgr);
}

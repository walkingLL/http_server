#ifndef IMGSVR_H_
#define IMGSVR_H_

#include "mongoose.h"


class server {
public:
	//与socket相关的变量初始化
	void server_init(void);
	void server_run(void);
private:
	struct mg_mgr mgr;
	struct mg_connection *nc;
	struct mg_bind_opts bind_opts;
	int i;
	char *cp;
	const char *err_str;
	static const char *s_http_port;
	static struct mg_serve_http_opts s_http_server_opts;
};
#endif // !IMGSVR_H_

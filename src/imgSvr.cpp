#include"imgSvr.h"
#include "cJSON.h"
#include<iostream>

using namespace std;

//通信端口
const char *server::s_http_port = "8000";
struct  mg_serve_http_opts server::s_http_server_opts;
static char buff[35 * 1024];
static char dst[35 * 1024];

//服务器每一次收到来自客户端的请求，都会调用该函数。事件处理回调函数
void ev_handler(struct mg_connection *nc, int ev, void *ev_data)
{
	//存储了所有来自客户端的请求信息，包含HTTP头，请求的url，post的数据
	struct http_message *hm = (struct http_message *) ev_data;

	switch (ev) {
	//每一次收到客户端请求，都会进入该分支
	case MG_EV_HTTP_REQUEST:
		if (mg_vcmp(&hm->uri, "/upload") == 0) {
			//客户端post的数据保存于hm->body中
			memcpy(buff, hm->body.p,
				sizeof(buff) - 1 < hm->body.len ? sizeof(buff) - 1 : hm->body.len);

			std::cout << "buff:" << buff << std::endl;

			//初始化json字符串
			cJSON* json = cJSON_Parse(buff);

			//从json字符串中获取键‘name’的值
			const cJSON* name = cJSON_GetObjectItem(json, "name");

			std::cout << "name:" << name->valuestring << std::endl;

			//从json字符串中获取键‘data’的值
			const cJSON* data = cJSON_GetObjectItem(json, "data");

			std::cout << "data:" << data->valuestring << std::endl;

			//将base64字符串解码
			cs_base64_decode((const unsigned char*)data->valuestring ,strlen(data->valuestring), dst, NULL);

			//将解码的数据保存至文件
			FILE *fp = fopen(name->valuestring, "wb");
			fwrite(dst,1, strlen(data->valuestring),fp);
			fclose(fp);

			std::cout << "save file sucees" << std::endl;

			//发送回应至客户端
			mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
			mg_printf_http_chunk(nc, "{ \"result\":123 }");
			mg_send_http_chunk(nc, "", 0); /* Send empty chunk, the end of response */
		}
		break;
	default:
		break;
	}
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

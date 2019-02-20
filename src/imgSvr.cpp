#include"imgSvr.h"
#include "cJSON.h"
#include<iostream>

using namespace std;

const char *server::s_http_port = "8000";
struct  mg_serve_http_opts server::s_http_server_opts;
static char buff[35 * 1024];
static char dst[35 * 1024];

void ev_handler(struct mg_connection *nc, int ev, void *ev_data)
{
	struct http_message *hm = (struct http_message *) ev_data;
	switch (ev) {
	case MG_EV_HTTP_REQUEST:
		if (mg_vcmp(&hm->uri, "/upload") == 0) {
			memcpy(buff, hm->body.p,
				sizeof(buff) - 1 < hm->body.len ? sizeof(buff) - 1 : hm->body.len);

			std::cout << "buff:" << buff << std::endl;

			cJSON* json = cJSON_Parse(buff);

			const cJSON* name = cJSON_GetObjectItem(json, "name");

			std::cout << "name:" << name->valuestring << std::endl;

			const cJSON* data = cJSON_GetObjectItem(json, "data");

			std::cout << "data:" << data->valuestring << std::endl;


			cs_base64_decode((const unsigned char*)data->valuestring ,strlen(data->valuestring), dst, NULL);

			FILE *fp = fopen(name->valuestring, "wb");
			fwrite(dst,1, strlen(data->valuestring),fp);
			fclose(fp);

			std::cout << "save file sucees" << std::endl;
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

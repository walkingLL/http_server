#include"imgSvr.h"
#include "cJSON.h"
#include<iostream>

using namespace std;

//ͨ�Ŷ˿�
const char *server::s_http_port = "8000";
struct  mg_serve_http_opts server::s_http_server_opts;
static char buff[35 * 1024];
static char dst[35 * 1024];

//������ÿһ���յ����Կͻ��˵����󣬶�����øú������¼�����ص�����
void ev_handler(struct mg_connection *nc, int ev, void *ev_data)
{
	//�洢���������Կͻ��˵�������Ϣ������HTTPͷ�������url��post������
	struct http_message *hm = (struct http_message *) ev_data;

	switch (ev) {
	//ÿһ���յ��ͻ������󣬶������÷�֧
	case MG_EV_HTTP_REQUEST:
		if (mg_vcmp(&hm->uri, "/upload") == 0) {
			//�ͻ���post�����ݱ�����hm->body��
			memcpy(buff, hm->body.p,
				sizeof(buff) - 1 < hm->body.len ? sizeof(buff) - 1 : hm->body.len);

			std::cout << "buff:" << buff << std::endl;

			//��ʼ��json�ַ���
			cJSON* json = cJSON_Parse(buff);

			//��json�ַ����л�ȡ����name����ֵ
			const cJSON* name = cJSON_GetObjectItem(json, "name");

			std::cout << "name:" << name->valuestring << std::endl;

			//��json�ַ����л�ȡ����data����ֵ
			const cJSON* data = cJSON_GetObjectItem(json, "data");

			std::cout << "data:" << data->valuestring << std::endl;

			//��base64�ַ�������
			cs_base64_decode((const unsigned char*)data->valuestring ,strlen(data->valuestring), dst, NULL);

			//����������ݱ������ļ�
			FILE *fp = fopen(name->valuestring, "wb");
			fwrite(dst,1, strlen(data->valuestring),fp);
			fclose(fp);

			std::cout << "save file sucees" << std::endl;

			//���ͻ�Ӧ���ͻ���
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
	//��ʼ��
	mg_mgr_init(&this->mgr, NULL);

	s_http_server_opts.document_root = "./";

	//����
	memset(&this->bind_opts, 0, sizeof(bind_opts));
	bind_opts.error_string = &this->err_str;

	//��
	this->nc = mg_bind_opt(&this->mgr, s_http_port, ev_handler, this->bind_opts);
	if (nc == NULL) {
		fprintf(stderr, "Error starting server on port %s: %s\n", s_http_port,
			*bind_opts.error_string);
		exit(1);
	}

	//����Э��
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

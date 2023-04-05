#include <stdio.h>
#include <string.h>



#include <sys/types.h>
#include <sys/stat.h>
//����ͨѶ��Ҫ������ͷ�ļ�����Ҫ���صĿ��ļ�
#include <WinSock2.h>
#pragma comment(lib,"WS2_32.lib")
#define PRINTF(str) printf("[%s - %d]"#str"=%s\n", __func__, __LINE__, str)

void error_die(const char* str) {
	perror(str);//ֱ�Ӵ�ӡ����ԭ��
	exit(1);
}
//ʵ�ֳ�ʼ��
//����ֵ��Ƕ����(�����ͷ)���������˿ڵ�Ƕ���֣�
//�˿�
//������port ��ʾ�˿�
//		���*port��ֵ��0����ô���Զ�����һ�����õĶ˿�
//		Ϊ����tinyhttpd�������¾�
int starup(unsigned short* port) {//ָ�붯̬���ض˿�
	//to do
	//1.����ͨ�ŵĳ�ʼ��
	WSADATA data;
	int ret = WSAStartup(
		MAKEWORD(1, 1), //1.1�汾��Э��
		&data);
	if (ret) {//ret !=0
		return -1;
	}

	int server_socket = socket(PF_INET,//�׽�������
		SOCK_STREAM,//������
		IPPROTO_TCP);
	if (server_socket == -1) {
		//��ӡ������ʾ������������
		error_die("�׽���");
	}

	//���ö˿ڸ���
	int opt = 1;
	ret = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
	if (ret == -1) {
		error_die("setsockopt");
	}

	//���÷������˵������ַ
	struct sockaddr_in server_addr;//���������ַ����
	memset(&server_addr, 0, sizeof(server_addr));//�����ڴ�
	server_addr.sin_family = AF_INET;//��������Э��
	server_addr.sin_port = htons(*port);//host to neiwort short
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);//����IP��ַ(�κε�ַ)

	//���׽���
	if (bind(server_socket, (struct  sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		error_die("bind");
	}
	//��̬����һ���˿�
	int namelen = sizeof(server_addr);
	if (*port == 0) {
		if(getsockname(server_socket,
			(struct sockaddr*)&server_addr, &namelen) < 0){
			error_die("getsockname");
			}

		*port = server_addr.sin_port;
}

	//������������
	if(listen(server_socket, 5) < 0){
		error_die("error_die");
	}
	return server_socket;


}

//��ָ���Ŀͻ����׽��֣���ȡһ�����ݣ����浽buff
//����ʵ�ʶ�ȡ�����ֽ���
int get_line(int sock, char* buff, int size) {
	char c = 0;//'\0'
	int i = 0;

// \r\n
while (i < size - 1 && c != '\n') {//����Ƿ�Խ��
	int n = recv(sock, &c, 1, 0);//n�Ƿ���ɹ� recv �������ȡ
	if (n > 0) {
		if (c == '\r') {
			n == recv(sock, &c, 1, MSG_PEEK);//�����һ���ַ�
			if (n > 0 && c == '\n') {
				recv(sock, &c, 1, 0);
			}
			else {
				c = '\n';
			}
		}
		buff[i++] = c;
	}
	else {
		//to do
		c = '\n';
	}
}


buff[i] = 0;
return 0;
}


void unimplement(int client) {
	//��ָ�����׽��֣�����һ����ʾ��û��ʵ�ֵĴ���ҳ��


}

void not_found(int client) {
	//����404��Ӧ
	char buff[1024];

	strcpy(buff, "HTTP/1.0 404 NOT FOUND\r\n");
	send(client, buff, strlen(buff), 0);

	strcpy(buff, "Server: MoonHttpd/0.1\r\n");
	send(client, buff, strlen(buff), 0);


	strcpy(buff, "Content-type:text/html\n");
	send(client, buff, strlen(buff), 0);

	strcpy(buff, "\r\n");
	send(client, buff, strlen(buff), 0);

	//����404��ҳ����
	sprintf(buff, 
		"<HTML>\
			<TITLE>Not Found</TITLE>\
			<BODY>						\
			<H2>The resorce is unavailable.</H2>\
			<img src=\"1611.png\" />		\
			</BODY>\
		</HTML>");
	
	send(client, buff, strlen(buff), 0);
}

void headers(int client, const char* type) {
	//������Ӧ��ͷ��Ϣ
	char buff[1024];

	strcpy(buff, "HTTP/1.0 200 OK\r\n");
	send(client, buff, strlen(buff), 0);

	strcpy(buff, "Server: MoonHttpd/0.1\r\n");
	send(client, buff, strlen(buff), 0);


	strcpy(buff, "Content-type:text/html\n");
	send(client, buff, strlen(buff), 0);

	strcpy(buff, "\r\n");
	send(client, buff, strlen(buff), 0);

}


void cat(int client, FILE* resource) {
	//һ�ζ�4096�ֽڣ�Ȼ���ٰ�����ֽڷ��͸������
	char buff[4096];
	int count = 0;

	while (1) {
		int ret = fread(buff,sizeof(char),sizeof(buff),resource);
		//���ص�Ԫ��
		if (ret < 0) {
			break;
		}
		send(client, buff, ret, 0);
		count += ret;

	}
	printf("һ������[%d]�ֽڸ������\n",count);
}



const char* getHeadType(const char* fileName) {
	const char* ret = "text/html";
	const char* p = strchr(fileName, '.');
	if (!p)return ret;

	p++;
	if (!strcmp(p, "css")) ret = "text/css";
	else if (!strcmp(p, "jpg")) ret = "image/jpeg";
	else if (!strcmp(p, "png")) ret = "image/png";
	else if (!strcmp(p, "js")) ret = "application/x-javascript";

	return ret;

}
void server_file(int client, const char* fileName) {
	int numchars = 1;
	char buff[1024];

	//�������ݰ���ʣ�������У�����
	while (numchars > 0 && strcmp(buff, "\n"))
	{
		numchars = get_line(client, buff, sizeof(buff));
		PRINTF(buff);
	}

	FILE* resource = NULL;//�ı���ʽ��
	if (strcmp(fileName, "moon/index.html") == 0) {
		resource = fopen(fileName, "r");
	}
	else
	{
		resource = fopen(fileName, "rb");
	}
	if (resource == NULL) {
		not_found(client);
	}
	else {
		//��ʽ������Դ�������
		
		headers(client,getHeadType(fileName));

		//�����������Դ��Ϣ
		cat(client, resource);

		printf("�ļ���Դ�������\n");

		}
	

	fclose(resource);


}

//�����û�������̺߳���
DWORD WINAPI accept_request(LPVOID arg) {//����ͻ����׽���
	char buff[1024];//1k

	int client = (SOCKET)arg;//�ͻ����׽���

	//��һ������
	//0x015ffad8 "GET / HTTP /1.1\n"
	int numchars = get_line(client, buff, sizeof(buff));
	PRINTF(buff);

	char method[255];
	int j = 0, i = 0;
	//ɨ�� �����հ��ַ������±��Ƿ�Խ��
	while (!isspace(buff[j]) &&  i < sizeof(method) - 1) {
		method[i++] = buff[j++];
	}
	method[i] = 0; //'\0'
	PRINTF(method);

	//�������ķ��������������Ƿ�֧��
	if (stricmp(method, "GET") && stricmp(method, "POST")) {
		//�����������һ��������ʾҳ��
		//to do
		unimplement(client);
		return 0;

	}
	//������Դ�ļ���·��
	char url[255];//����������Դ������·��
	i = 0;
	while (isspace(buff[j]) && j < sizeof(buff)) {
		j++;
	}

	while (!isspace(buff[j]) && i < sizeof(url) - 1 && j < sizeof(buff)) {
		url[i++] = buff[j++];
	}
	url[i] = 0;
	PRINTF(url);

	//www.rock.com
	//127.0.0.1/
	//url /
	//moon/

	char path[512] = "";
	sprintf(path, "moon%s", url);
	if (path[strlen(path) - 1] == '/') {
		strcat(path, "index.html");
	}
	PRINTF(path);

	struct stat status;
	if (stat(path, &status) == -1) {//moon/abc.html
		//�������ʣ�����ݶ�ȡ���
		while (numchars > 0 && strcmp(buff, "\n"))
		{
			numchars = get_line(client, buff, sizeof(buff));
		}
		not_found(client);
	}
	else {
		if ((status.st_mode & S_IFMT) == S_IFDIR){//λ���� �ж��Ƿ�ΪĿ¼
		strcat(path, "/index.html");
		}
		server_file(client, path);
	
	}

	closesocket(client);

	return 0;
}

int main(void) {
	unsigned short port = 80;//�˿ڹ淶
	int server_sock = starup(&port);
	printf("httpd�����Ѿ����������ڼ���%d�˿�...",port);

	struct sockaddr_in client_addr;
	int client_addr_len = sizeof(client_addr);
	//to do
	while (1) {
		//û�з��ʾ�һֱ�ȴ� ����ʽ�ȴ��û�ͨ�����������
		int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_len);//���׽���_�ͻ����׽��֣������²�ڣ��Ӵ���
		if (client_sock == -1) {
			error_die("accept");
		}
		//����һ���µ��߳�
		//����-��������߳� 
		DWORD threadId = 0;//�߳�ID
		CreateThread(0, 0, accept_request, (void*)client_sock, 0, &threadId);//�����߳�

		//ʹ��client_sock���û����з���
		//to do ... 60s

	
	}
	// "/"��վ��������ԴĿ¼�µ� index.html



	closesocket(server_sock);
	return 0;

}
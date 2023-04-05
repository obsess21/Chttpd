#include <stdio.h>
#include <string.h>



#include <sys/types.h>
#include <sys/stat.h>
//网络通讯需要包含的头文件，需要加载的库文件
#include <WinSock2.h>
#pragma comment(lib,"WS2_32.lib")
#define PRINTF(str) printf("[%s - %d]"#str"=%s\n", __func__, __LINE__, str)

void error_die(const char* str) {
	perror(str);//直接打印错误原因
	exit(1);
}
//实现初始化
//返回值：嵌套字(网络插头)（服务器端口的嵌套字）
//端口
//参数：port 表示端口
//		如果*port的值是0，那么就自动分配一个可用的端口
//		为了向tinyhttpd服务器致敬
int starup(unsigned short* port) {//指针动态返回端口
	//to do
	//1.网络通信的初始化
	WSADATA data;
	int ret = WSAStartup(
		MAKEWORD(1, 1), //1.1版本的协议
		&data);
	if (ret) {//ret !=0
		return -1;
	}

	int server_socket = socket(PF_INET,//套接字类型
		SOCK_STREAM,//数据流
		IPPROTO_TCP);
	if (server_socket == -1) {
		//打印错误提示，并结束程序
		error_die("套接字");
	}

	//设置端口复用
	int opt = 1;
	ret = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
	if (ret == -1) {
		error_die("setsockopt");
	}

	//配置服务器端的网络地址
	struct sockaddr_in server_addr;//定义网络地址变量
	memset(&server_addr, 0, sizeof(server_addr));//设置内存
	server_addr.sin_family = AF_INET;//配置网络协议
	server_addr.sin_port = htons(*port);//host to neiwort short
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);//设置IP地址(任何地址)

	//绑定套接字
	if (bind(server_socket, (struct  sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		error_die("bind");
	}
	//动态分配一个端口
	int namelen = sizeof(server_addr);
	if (*port == 0) {
		if(getsockname(server_socket,
			(struct sockaddr*)&server_addr, &namelen) < 0){
			error_die("getsockname");
			}

		*port = server_addr.sin_port;
}

	//创建监听队列
	if(listen(server_socket, 5) < 0){
		error_die("error_die");
	}
	return server_socket;


}

//从指定的客户端套接字，读取一行数据，保存到buff
//返回实际读取到的字节数
int get_line(int sock, char* buff, int size) {
	char c = 0;//'\0'
	int i = 0;

// \r\n
while (i < size - 1 && c != '\n') {//检查是否越界
	int n = recv(sock, &c, 1, 0);//n是否读成功 recv 从网络获取
	if (n > 0) {
		if (c == '\r') {
			n == recv(sock, &c, 1, MSG_PEEK);//检查下一个字符
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
	//向指定的套接字，发送一个提示还没有实现的错误页面


}

void not_found(int client) {
	//发送404响应
	char buff[1024];

	strcpy(buff, "HTTP/1.0 404 NOT FOUND\r\n");
	send(client, buff, strlen(buff), 0);

	strcpy(buff, "Server: MoonHttpd/0.1\r\n");
	send(client, buff, strlen(buff), 0);


	strcpy(buff, "Content-type:text/html\n");
	send(client, buff, strlen(buff), 0);

	strcpy(buff, "\r\n");
	send(client, buff, strlen(buff), 0);

	//发送404网页内容
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
	//发送相应包头信息
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
	//一次读4096字节，然后再把这个字节发送给浏览器
	char buff[4096];
	int count = 0;

	while (1) {
		int ret = fread(buff,sizeof(char),sizeof(buff),resource);
		//返回单元数
		if (ret < 0) {
			break;
		}
		send(client, buff, ret, 0);
		count += ret;

	}
	printf("一共发送[%d]字节给浏览器\n",count);
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

	//请求数据包的剩余数据行，读完
	while (numchars > 0 && strcmp(buff, "\n"))
	{
		numchars = get_line(client, buff, sizeof(buff));
		PRINTF(buff);
	}

	FILE* resource = NULL;//文本方式打开
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
		//正式发送资源给浏览器
		
		headers(client,getHeadType(fileName));

		//发送请求的资源信息
		cat(client, resource);

		printf("文件资源发送完毕\n");

		}
	

	fclose(resource);


}

//处理用户请求的线程函数
DWORD WINAPI accept_request(LPVOID arg) {//传入客户端套接字
	char buff[1024];//1k

	int client = (SOCKET)arg;//客户端套接字

	//读一行数据
	//0x015ffad8 "GET / HTTP /1.1\n"
	int numchars = get_line(client, buff, sizeof(buff));
	PRINTF(buff);

	char method[255];
	int j = 0, i = 0;
	//扫描 跳过空白字符，看下标是否越界
	while (!isspace(buff[j]) &&  i < sizeof(method) - 1) {
		method[i++] = buff[j++];
	}
	method[i] = 0; //'\0'
	PRINTF(method);

	//检查请求的方法，本服务器是否支持
	if (stricmp(method, "GET") && stricmp(method, "POST")) {
		//向浏览器返回一个错误提示页面
		//to do
		unimplement(client);
		return 0;

	}
	//解析资源文件的路径
	char url[255];//存放请求的资源的完整路径
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
		//请求包的剩余数据读取完毕
		while (numchars > 0 && strcmp(buff, "\n"))
		{
			numchars = get_line(client, buff, sizeof(buff));
		}
		not_found(client);
	}
	else {
		if ((status.st_mode & S_IFMT) == S_IFDIR){//位操作 判断是否为目录
		strcat(path, "/index.html");
		}
		server_file(client, path);
	
	}

	closesocket(client);

	return 0;
}

int main(void) {
	unsigned short port = 80;//端口规范
	int server_sock = starup(&port);
	printf("httpd服务已经启动，正在监听%d端口...",port);

	struct sockaddr_in client_addr;
	int client_addr_len = sizeof(client_addr);
	//to do
	while (1) {
		//没有访问就一直等待 阻塞式等待用户通过浏览器访问
		int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_len);//新套接字_客户端套接字，分配新插口（接待）
		if (client_sock == -1) {
			error_die("accept");
		}
		//创建一个新的线程
		//进程-包含多个线程 
		DWORD threadId = 0;//线程ID
		CreateThread(0, 0, accept_request, (void*)client_sock, 0, &threadId);//创建线程

		//使用client_sock对用户进行访问
		//to do ... 60s

	
	}
	// "/"网站服务器资源目录下的 index.html



	closesocket(server_sock);
	return 0;

}
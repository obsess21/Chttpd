//管道
//使用（匿名）管道实现通信
#include <stdio.h>
#include <Windows.h>

int main(void) {
	//创建管道
	HANDLE output[2];
	
	SECURITY_ATTRIBUTES la;
	la.nLength = sizeof(la);
	la.bInheritHandle = true;//继承
	la.lpSecurityDescriptor = 0;//安全描述 0默认值



	BOOL bCreate = CreatePipe(&output[0], &output[1], &la, 0);
	if (bCreate == false) {
		MessageBox(0, "create cgi pipe error!", 0, 0);
		return 1;
	}
	//创建子进程
	char cmd[] = "ping www.baidu.com";

	//子进程启动信息
	STARTUPINFO si = { 0 };
	si.cb = sizeof(si);
	si.hStdInput = output[1];//管道写端
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

	PROCESS_INFORMATION pi = { 0 };

	bCreate = CreateProcess(NULL,cmd,0,0,TRUE,0,0,0,&si,&pi);
	if (bCreate == false) {
		printf("进程创建失败!\n");
		return 1;
	}

	char buff[1024];
	DWORD size;
	while (1) {
		/*printf("请输入：");
		gets_s(buff, sizeof(buff));

		WriteFile(output[1], buff, sizeof(buff) + 1, &size,NULL);
		printf("已经写入了%d字节\n", size);*/


		ReadFile(output[0], buff, sizeof(buff), &size, NULL);
		buff[size] = '\0';
		printf("已经读到了%d个字节：[%s]", size, buff);
	


	}

}
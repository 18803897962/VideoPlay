#include <iostream>
#include"RTSPServer.h"
int main()
{
	RTSPServer server;
	server.Init();
	server.Invoke();
	printf("press any key to stop\r\n");
	getchar();
	server.Stop();
	return 0;
}

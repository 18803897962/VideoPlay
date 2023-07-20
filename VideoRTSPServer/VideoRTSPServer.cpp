#include <iostream>
#include"RTSPServer.h"
int main()
{
	RTSPServer server;
	server.Init();
	server.Invoke();
	getchar();
	server.Stop();
	return 0;
}

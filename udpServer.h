#pragma once
#include <WS2tcpip.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

class UDPServer
{
public:
	SOCKET listening;

	bool initializeWinSocket();
	bool createAndBinSocket(int port);
	void CloseUDPSocket();
};


#include "udpServer.h"

bool UDPServer::initializeWinSocket()
{
	WSAData data;
	WORD version = MAKEWORD(2, 2);

	int wsOK = WSAStartup(version, &data);

	if (wsOK != 0)
	{
		std::cout << "cannot start Winsock ..." << std::endl; 
		return false;
	}

	return true;
}

bool UDPServer::createAndBinSocket(int port)
{
	listening = socket(AF_INET, SOCK_DGRAM, 0);

	sockaddr_in serverHint;

	serverHint.sin_addr.S_un.S_addr = ADDR_ANY;
	serverHint.sin_family = AF_INET;
	serverHint.sin_port = htons(port);

	if (bind(listening, (sockaddr*)&serverHint, sizeof(serverHint)) == SOCKET_ERROR)
	{
		std::cout << "cannot binf socket" << WSAGetLastError() << std::endl;
		return false;
	}

	return true;
}

UDPServer::UDPServer() 
{
	IsReady = initializeWinSocket();
}

UDPServer::~UDPServer()
{
	std::cout << "closing server socket ...." << std::endl;
	closesocket(listening);

	WSACleanup();
}

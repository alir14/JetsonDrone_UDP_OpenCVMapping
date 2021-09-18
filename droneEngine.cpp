#include "droneEngine.h"

DroneEngine::DroneEngine() {
	initializeWinSocket();
}

DroneEngine::~DroneEngine() {
	std::cout << "closing socket ....";
	closesocket(command_socket);

	WSACleanup();
}

void DroneEngine::initializeWinSocket() {
	WSADATA data;
	WORD version = MAKEWORD(2, 2);

	int wsOK = WSAStartup(version, &data);

	if (wsOK != 0) {
		throw "cannot start socket ...";
	}
}

bool DroneEngine::BindAndConnect() {
	// bind socket
	auto result = BindSocket();

	if (!result.first) {
		std::cout << result.second << std::endl;
		return false;
	}

	// get drone addr_in info
	result = GetSocketAddr();

	if (!result.first) {
		std::cout << result.second << std::endl;
		return false;
	}

	// set to command mode
	std::cout << "Connecting Tello ..." << std::endl;
	SetTelloToCommandMode();
	std::cout << "Entered SDK mode ..." << std::endl;

	return true;
}

std::pair<bool, std::string> DroneEngine::BindSocket() {
	command_socket = socket(AF_INET, SOCK_DGRAM, 0);

	sockaddr_in listen_addr;
	listen_addr.sin_port = htons(CLIENT_COMMAND_PORT);
	listen_addr.sin_addr.S_un.S_addr = ADDR_ANY;
	listen_addr.sin_family = AF_INET;

	int result = bind(command_socket, (sockaddr*)&listen_addr, sizeof(listen_addr));

	if (result == SOCKET_ERROR) {
		std::cout << "cannot bind socket " << WSAGetLastError() << std::endl;
		return { false, "cannot bind socket" };
	}

	return { true, "socket bind - OK" };
}

std::pair<bool, std::string> DroneEngine::GetSocketAddr() {
	addrinfo* result_list = nullptr;
	addrinfo hints;

	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	int result = getaddrinfo(TELLO_SERVER_IP, TELLO_SERVER_COMMAND_PORT, &hints, &result_list);

	if (result != 0) {
		std::cout << "getaddrinfo " << result << std::endl;
		return { false, "cannot find the socket address" };
	}

	memcpy(&server_command_addr, result_list->ai_addr, result_list->ai_addrlen);

	freeaddrinfo(result_list);

	return { true, "" };
}

void DroneEngine::SetTelloToCommandMode() {
	SendCommand("command");

	auto response = ReceiveResponse();

	if (!response.first) {
		std::cout << response.second << std::endl;
	}
}

bool DroneEngine::SendCommand(const std::string& command) {
	const socklen_t addr_len = sizeof(server_command_addr);

	int result = sendto(command_socket, command.c_str(), command.size(), 0, (sockaddr*)&server_command_addr, addr_len);

	if (result == SOCKET_ERROR) {
		std::cout << "failed to send the message ... " << errno << std::endl;
		return false;
	}

	std::cout << "127.0.0.1:" << " >>>> "
		<< result << " bytes >>>> " << TELLO_SERVER_IP << ":" << TELLO_SERVER_COMMAND_PORT << ":" << command << std::endl;

	Sleep(1);

	return true;

}

std::pair<bool, std::string> DroneEngine::ReceiveResponse() {
	sockaddr_in client;
	int clientSize = sizeof(client);
	ZeroMemory(&client, clientSize);

	char buff[1024];
	ZeroMemory(buff, 1024);

	while (true)
	{
		int byteIn = recvfrom(command_socket, buff, 1024, 0, (sockaddr*)&client, &clientSize);

		if (byteIn == SOCKET_ERROR) {
			std::cout << "recfrom: " << errno << std::endl;
			return { -1 , "failed to receive message" };
		}

		std::cout << "msg received: " << buff << std::endl;

		std::string result(buff);

		result.erase(result.find_last_not_of(" \n\r\t") + 1);

		if (byteIn > 1)
			return { true, result };
		else
			return { false, "failed to receive data ..." };
	}
}
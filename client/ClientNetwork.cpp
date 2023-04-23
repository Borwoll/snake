#include <stdio.h>
#include <ws2tcpip.h>
#include "ClientNetwork.h"
#include "Networking.h"

/* Данная функция принимает сообщение и код ошибки, конвертирует код ошибки в строку, добавляет его к сообщению
 * и выводит это сообщение в окне сообщения. Она используется для вывода сообщений об ошибках пользователю. */
void ClientNetwork::outputError(char* msg, int error) {
	if (error != 0) {
		std::string s = std::to_string(error);
		char const *pchar = s.c_str();
		char *result = new char[100];
		strcpy_s(result, 100, msg);
		strcat_s(result, 100, pchar);
		msg = result;
	}
	MessageBox(NULL, msg, "Error", MB_OK);
}

/* Данная конструкторная функция инициализирует клиентскую сеть, настраивая необходимый API Windows Sockets,
 * создавая сокет для подключения к серверу с использованием заданного IP-адреса и номера порта, и устанавливая сокет
 * в неблокирующий режим. Она также устанавливает опцию TCP_NODELAY для улучшения производительности сети. */
ClientNetwork::ClientNetwork(char *ip, char *port) {
	WSADATA wsaData;
	ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = nullptr, *ptr = nullptr, hints;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (iResult != 0)
		outputError("WSAStartup failed with error: \n", iResult);

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo(ip, port, &hints, &result); 
	if (iResult != 0) {
		outputError("getaddrinfo failed with error: \n", iResult);
		WSACleanup();
	}

	for (ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			outputError("socket failed with error: \n", WSAGetLastError());
			WSACleanup();
		}

		iResult = connect(ConnectSocket, ptr->ai_addr, int(ptr->ai_addrlen));
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			outputError("The server is down", 0);
		}
	}

	freeaddrinfo(result);
	if (ConnectSocket == INVALID_SOCKET) {
		outputError("Unable to connect to server!\n", 0);
		WSACleanup();
	}

	u_long iMode = 1;

	iResult = ioctlsocket(ConnectSocket, FIONBIO, &iMode);
	if (iResult == SOCKET_ERROR) {
		outputError("ioctlsocket failed with error: \n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
	}
	char value = 1;
	setsockopt(ConnectSocket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));
}


ClientNetwork::~ClientNetwork()
{
}

/* Данная функция принимает пакеты данных от сервера через сокетное соединение и сохраняет данные в
 * предоставленном буфере. Она возвращает количество принятых байтов. Если соединение закрыто или происходит
 * ошибка, то функция выводит сообщение об ошибке с помощью функции outputError и закрывает сокет и API Windows Sockets. */
int ClientNetwork::receivePackets(char *buf) {
	iResult = NetworkServices::receiveMessage(ConnectSocket, buf, MAX_PACKET_SIZE);
	if (iResult == 0) {
		outputError("Connection closed\n", 0);
		closesocket(ConnectSocket);
		WSACleanup();
	}

	return iResult;
}

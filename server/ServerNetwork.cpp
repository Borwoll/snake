#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <ws2tcpip.h>
#include "ServerNetwork.h"
#include "Networking.h"

// Выводит на экран IP-адрес и порт сервера. Для этого она использует функции из библиотеки Winsock: WSAStartup, gethostname, gethostbyname, а также структуру in_addr.
void printServerIP(char *port) {
	char szBuffer[1024];

	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 0);
	if (WSAStartup(wVersionRequested, &wsaData) != 0)
		return;

	if (gethostname(szBuffer, sizeof(szBuffer)) == SOCKET_ERROR) {
		WSACleanup();
		return;
	}

	hostent *host = gethostbyname(szBuffer);
	if (host == nullptr) {
		WSACleanup();
		return;
	}

	auto b1 = reinterpret_cast<struct in_addr *>((host->h_addr))->S_un.S_un_b.s_b1;
	auto b2 = reinterpret_cast<struct in_addr *>((host->h_addr))->S_un.S_un_b.s_b2;
	auto b3 = reinterpret_cast<struct in_addr *>((host->h_addr))->S_un.S_un_b.s_b3;
	auto b4 = reinterpret_cast<struct in_addr *>((host->h_addr))->S_un.S_un_b.s_b4;
	Log("IP: %d.%d.%d.%d:%s\n", b1, b2, b3, b4, port);
	WSACleanup();
}

/* Создает новый сетевой сервер, используя порт, переданный в качестве параметра.
 * Для этого он также использует функции из библиотеки Winsock: WSAStartup, getaddrinfo, socket, bind, listen, а также структуру addrinfo. */
ServerNetwork::ServerNetwork(char *port) {
	WSADATA wsaData;
	ListenSocket = INVALID_SOCKET;
	ClientSocket = INVALID_SOCKET;
	struct addrinfo *result = nullptr, hints;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		Log("WSAStartup failed with error: %d\n", iResult);
		exit(1);
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(nullptr, port, &hints, &result);
	if (iResult != 0) {
		Log("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		exit(1);
	}

	printServerIP(port);
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (ListenSocket == INVALID_SOCKET) {
		Log("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		exit(1);
	}

	u_long iMode = 1;

	if (ioctlsocket(ListenSocket, FIONBIO, &iMode) == SOCKET_ERROR) {
		Log("ioctlsocket failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		exit(1);
	}

	if (bind(ListenSocket, result->ai_addr, int(result->ai_addrlen)) == SOCKET_ERROR) {
		Log("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		exit(1);
	}

	freeaddrinfo(result);

	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		Log("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		exit(1);
	}
}


ServerNetwork::~ServerNetwork(void)
{
}

/* Позволяет серверу принимать новых клиентов. Она использует функцию accept и устанавливает опцию
 * TCP_NODELAY для сокета клиента. Если клиент был успешно принят, то его сокет добавляется в sessions
 * (контейнер std::map) с уникальным идентификатором id + 1. Функция возвращает true, если клиент был
 * успешно принят, и false в противном случае. */
bool ServerNetwork::acceptNewClient(unsigned int &id) {
	ClientSocket = accept(ListenSocket, nullptr, nullptr);
	if (ClientSocket != INVALID_SOCKET) {
		char value = 1;
		setsockopt(ClientSocket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));
		sessions.insert(std::pair<unsigned int, SOCKET>(id + 1, ClientSocket));
		return true;
	}
	return false;
}

/* Отвечает за получение данных от клиента. Она ищет соответствующий клиентский сокет в хранилище сессий,
 * и при наличии такого сокета вызывает метод "receiveMessage" из класса "NetworkServices" для получения
 * данных. Если полученный результат равен 0, то это означает, что соединение закрыто, и функция закрывает
 * сокет и удаляет его из хранилища сессий. Результат работы функции - количество полученных данных. */
int ServerNetwork::receiveData(unsigned int client_id, char *recvbuf) {
	if (sessions.find(client_id) != sessions.end()) {
		SOCKET currentSocket = sessions[client_id];
		iResult = NetworkServices::receiveMessage(currentSocket, recvbuf, MAX_PACKET_SIZE);
		if (iResult == 0) {
			Log("Connection closed\n");
			closesocket(currentSocket);
			sessions.erase(currentSocket);
		}
		return iResult;
	}
	return 0;
}

/* Отправляет данные определенному клиенту по его идентификатору. Она ищет соответствующий клиентский сокет в хранилище
 * сессий, и при наличии такого сокета вызывает метод "sendMessage" из класса "NetworkServices" для отправки данных.
 * Если отправка не удалась, то функция закрывает сокет и удаляет его из хранилища сессий. */
void ServerNetwork::sendToOne(unsigned int id, char* packets, int totalSize) {
	SOCKET socket = sessions.at(id);
	int iSendResult = NetworkServices::sendMessage(socket, packets, totalSize);
	if (iSendResult == SOCKET_ERROR) {
		Log("send failed with error: %d\n", WSAGetLastError());
		closesocket(socket);
		sessions.erase(socket);
	}
}

/* Отправляет данные всем клиентам, кроме одного, по его идентификатору. Она проходит по всем клиентским сокетам в
 * хранилище сессий, и при наличии сокета исключает из отправки клиента с заданным идентификатором. Затем вызывается
 * метод "sendMessage" из класса "NetworkServices" для отправки данных. Если отправка не удалась, то функция закрывает
 * сокет и удаляет его из хранилища сессий. */
void ServerNetwork::sendToAllButOne(unsigned int id, char* packets, int totalSize) {
	SOCKET currentSocket;
	std::map<unsigned int, SOCKET>::iterator iter = sessions.begin();
	int iSendResult;

	while (iter != sessions.end()) {
		if (iter->first == id) {
			++iter;
			continue;
		}

		currentSocket = iter->second;
		iSendResult = NetworkServices::sendMessage(currentSocket, packets, totalSize);

		if (iSendResult == SOCKET_ERROR) {
			Log("send failed with error: %d\n", WSAGetLastError());
			closesocket(currentSocket);
			iter = sessions.erase(iter);
		}  else
			++iter;
	}
}

/* Отправляет данные всем клиентам, находящимся в хранилище сессий. Она проходит по всем клиентским сокетам в хранилище
 * сессий и вызывает метод "sendMessage" из класса "NetworkServices" для отправки данных. Если отправка не удалась, то
 * функция закрывает сокет и удаляет его из хранилища сессий. */
void ServerNetwork::sendToAll(char *packets, int totalSize) {
	SOCKET currentSocket;
	std::map<unsigned int, SOCKET>::iterator iter = sessions.begin();
	int iSendResult;

	while (iter != sessions.end()) {
		currentSocket = iter->second;
		iSendResult = NetworkServices::sendMessage(currentSocket, packets, totalSize);

		if (iSendResult == SOCKET_ERROR) {
			Log("send failed with error: %d\n", WSAGetLastError());
			closesocket(currentSocket);
			iter = sessions.erase(iter);
		} else
			++iter;
	}
}

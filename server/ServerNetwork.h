#pragma once
#include <winsock2.h>
#include <map>
#include "Logger.h"
#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512

class ServerNetwork {
	int iResult;
	SOCKET ListenSocket;
	SOCKET ClientSocket;

public:
	std::map<unsigned int, SOCKET> sessions;
	explicit ServerNetwork(char *port);
	~ServerNetwork(void);
	bool acceptNewClient(unsigned int &id);
	int receiveData(unsigned int client_id, char *recvbuf);
	void sendToOne(unsigned int id, char *packets, int totalSize);
	void sendToAllButOne(unsigned int id, char *packets, int totalSize);
	void sendToAll(char *packets, int totalSize);
};

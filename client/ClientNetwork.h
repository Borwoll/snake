#pragma once
#include <winsock2.h>
#include <string>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512


class ClientNetwork {
	int iResult;

public:
	SOCKET ConnectSocket;
	ClientNetwork(char *ip, char *port);
	~ClientNetwork();

	int receivePackets(char *buf);
	void outputError(char* msg, int error);
};

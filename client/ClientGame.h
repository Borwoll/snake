#pragma once
#include "ClientNetwork.h"
#include "Networking.h"
#include "DirectionEnum.h"
#include "Drawing.h"

class ClientGame {
	HWND hWnd;
	unsigned int tiles;
	unsigned int clientID;
	ClientNetwork* network;
	char network_buffer[MAX_PACKET_SIZE];

public:
	ClientGame(char *ip, char *port, HWND hWnd);
	~ClientGame();

	void sendReady() const;
	void sendAction(DirectionEnum direction) const;
	void update(Tile board[MAX_X * MAX_Y]);

private:
	void handleInit(char data[], Tile board[MAX_X * MAX_Y]);
	void handleTick(char data[], Tile board[MAX_X * MAX_Y]);
	void handleRestart(char data[], Tile board[MAX_X * MAX_Y]);
	void handleNewPlayer(char data[], Tile board[MAX_X * MAX_Y]);
};
